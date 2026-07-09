#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <vector>

// Bus compressor DSP — S mode (SSL-style) and A mode (Lindell 50 / API-style)
class BusCompressor
{
public:
    enum class Mode { S, A };

    void prepare(double sampleRate, int blockSize)
    {
        fs = sampleRate;

        juce::dsp::ProcessSpec spec{ sampleRate, (juce::uint32)blockSize, 2 };
        hpf.prepare(spec);
        auto hpfCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 120.0f, 0.7071f);
        *hpf.get<0>().coefficients = *hpfCoeffs;
        *hpf.get<1>().coefficients = *hpfCoeffs;
        hpf.reset();

        envL = envR = 0.0f;
        grSmoothed = 0.0f;
        threshSmoothed = -12.0f;
    }

    // Returns current gain reduction in dB (≤ 0)
    float getGainReductionDb() const { return grSmoothed; }

    void process(float* left, float* right, int numSamples,
                 float threshDb, float ratioVal, Mode mode)
    {
        // Fixed per-mode character (matches the reference hardware settings):
        //   S: SSL Channel, filter split to SC (120 Hz HPF), F.ATK, fully linked stereo
        //   A: Lindell 50, VCA, hard knee (SOFT off), 70% link, +9 dB preamp drive (UNITY)
        const bool  sidechainHpf = (mode == Mode::S);
        const float stereoLink   = (mode == Mode::A) ? 0.70f : 1.0f;
        const float knee         = 0.0f; // hard knee in both modes

        // Per-block threshold smoothing (~10 ms) to eliminate zipper noise on automation
        const float thSmoothCoeff = std::exp(-float(numSamples) / (float(fs) * 0.010f));
        threshSmoothed = threshDb + thSmoothCoeff * (threshSmoothed - threshDb);

        float attTc, relTc;
        if (mode == Mode::S) {
            attTc = calcTc(0.001f);   // 1 ms (F.ATK)
            relTc = calcTc(0.400f);   // 400 ms (Release knob at "4" = 0.40 s)
        } else {
            attTc = calcTc(0.0001f);  // 0.1 ms (Attack .1)
            relTc = calcTc(0.750f);   // 750 ms (Release .75)
        }

        // Sidechain copy
        std::vector<float> scL(numSamples), scR(numSamples);
        std::copy(left,  left  + numSamples, scL.data());
        std::copy(right, right + numSamples, scR.data());

        if (sidechainHpf) {
            for (int i = 0; i < numSamples; ++i) {
                scL[i] = hpf.get<0>().processSample(scL[i]);
                scR[i] = hpf.get<1>().processSample(scR[i]);
            }
        }

        for (int i = 0; i < numSamples; ++i)
        {
            // Peak detection (both reference units are peak-sensing designs)
            const float detLdb = juce::Decibels::gainToDecibels(std::abs(scL[i]), -120.0f);
            const float detRdb = juce::Decibels::gainToDecibels(std::abs(scR[i]), -120.0f);

            float grLdb = computeStaticGR(detLdb, threshSmoothed, ratioVal, knee);
            float grRdb = computeStaticGR(detRdb, threshSmoothed, ratioVal, knee);

            // Stereo link: blend toward the channel with more reduction
            if (stereoLink > 0.0f) {
                const float linked = std::min(grLdb, grRdb);
                grLdb = grLdb + stereoLink * (linked - grLdb);
                grRdb = grRdb + stereoLink * (linked - grRdb);
            }

            // Program-dependent release for S mode: 2× faster when GR > 6 dB
            float relTcL = relTc, relTcR = relTc;
            if (mode == Mode::S) {
                if (-envL > 6.0f) relTcL = calcTc(0.200f);
                if (-envR > 6.0f) relTcR = calcTc(0.200f);
            }

            auto applyBallistics = [](float& env, float target, float att, float rel) {
                const float tc = (target < env) ? att : rel;
                env = target + tc * (env - target);
            };
            applyBallistics(envL, grLdb, attTc, relTcL);
            applyBallistics(envR, grRdb, attTc, relTcR);

            left[i]  *= juce::Decibels::decibelsToGain(envL);
            right[i] *= juce::Decibels::decibelsToGain(envR);

            // A mode: Lindell 50 preamp character (UNITY + GAIN 9).
            // Drive +9 dB into a soft saturator, compensate back — adds
            // API-style harmonics without changing level.
            if (mode == Mode::A) {
                left[i]  = saturate(left[i]);
                right[i] = saturate(right[i]);
            }
        }

        grSmoothed = std::min(envL, envR);
    }

private:
    double fs = 44100.0;
    float  envL = 0.0f, envR = 0.0f;
    float  grSmoothed = 0.0f;
    float  threshSmoothed = -12.0f;

    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>> hpf;

    float calcTc(float timeSeconds) const
    {
        return std::exp(-1.0f / (float(fs) * timeSeconds));
    }

    // +9 dB drive → tanh → -9 dB. Gentle enough to avoid needing oversampling.
    static float saturate(float x)
    {
        constexpr float drive = 2.8184f;          // +9 dB
        constexpr float comp  = 1.0f / drive;
        return std::tanh(x * drive) * comp;
    }

    // Gain reduction in dB (≤ 0). Hard knee (kneeDb = 0) or quadratic soft knee.
    static float computeStaticGR(float inputDb, float threshDb, float ratio, float kneeDb)
    {
        const float overDb = inputDb - threshDb;
        if (kneeDb > 0.0f) {
            const float halfKnee = kneeDb * 0.5f;
            if (overDb < -halfKnee) return 0.0f;
            if (overDb >  halfKnee) return -(overDb * (1.0f - 1.0f / ratio));
            const float x = overDb + halfKnee;
            return -(x * x / (2.0f * kneeDb)) * (1.0f - 1.0f / ratio);
        } else {
            if (overDb <= 0.0f) return 0.0f;
            return -(overDb * (1.0f - 1.0f / ratio));
        }
    }
};
