#pragma once
#include <JuceHeader.h>
#include <cmath>

// Bus compressor DSP — S mode (SSL-style) and A mode (1176/VCA-style)
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
    }

    // Returns current gain reduction in dB (≤ 0)
    float getGainReductionDb() const { return grSmoothed; }

    void process(float* left, float* right, int numSamples,
                 float threshDb, float ratioVal, Mode mode)
    {
        const bool  sidechainHpf = (mode == Mode::S);
        const float stereoLink   = (mode == Mode::A) ? 0.70f : 1.0f;
        const float knee         = (mode == Mode::A) ? 6.0f  : 0.0f;

        float attTc, relTc;
        if (mode == Mode::S) {
            attTc = calcTc(0.001f);   // 1 ms
            relTc = calcTc(0.400f);   // 400 ms
        } else {
            attTc = calcTc(0.0001f);  // 0.1 ms
            relTc = calcTc(0.750f);   // 750 ms
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

        for (int i = 0; i < numSamples; ++i) {
            const float detLdb = juce::Decibels::gainToDecibels(std::abs(scL[i]), -120.0f);
            const float detRdb = juce::Decibels::gainToDecibels(std::abs(scR[i]), -120.0f);

            // GR in dB (≤ 0)
            float grLdb = computeStaticGR(detLdb, threshDb, ratioVal, knee);
            float grRdb = computeStaticGR(detRdb, threshDb, ratioVal, knee);

            // Stereo link (A mode)
            if (mode == Mode::A) {
                const float linked = std::min(grLdb, grRdb); // most reduction
                grLdb = grLdb + stereoLink * (linked - grLdb);
                grRdb = grRdb + stereoLink * (linked - grRdb);
            }

            // Program-dependent release for S mode: 2× faster when GR > 6 dB
            float relTcL = relTc, relTcR = relTc;
            if (mode == Mode::S) {
                if (-envL > 6.0f) relTcL = calcTc(0.200f);
                if (-envR > 6.0f) relTcR = calcTc(0.200f);
            }

            // Ballistics: target < env means more reduction (attack), otherwise release
            auto applyBallistics = [](float& env, float target, float att, float rel) {
                const float tc = (target < env) ? att : rel;
                env = target + tc * (env - target);
            };
            applyBallistics(envL, grLdb, attTc, relTcL);
            applyBallistics(envR, grRdb, attTc, relTcR);

            left[i]  *= juce::Decibels::decibelsToGain(envL);
            right[i] *= juce::Decibels::decibelsToGain(envR);
        }

        // Pass raw peak reduction to meter — GRMeter handles its own ballistics
        grSmoothed = std::min(envL, envR);
    }

private:
    double fs = 44100.0;
    float  envL = 0.0f, envR = 0.0f;
    float  grSmoothed = 0.0f;

    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>> hpf;

    float calcTc(float timeSeconds) const
    {
        return std::exp(-1.0f / (float(fs) * timeSeconds));
    }

    // Returns gain reduction in dB (≤ 0). Hard or soft knee.
    static float computeStaticGR(float inputDb, float threshDb, float ratio, float kneeDb)
    {
        const float overDb = inputDb - threshDb;
        if (kneeDb > 0.0f) {
            const float halfKnee = kneeDb * 0.5f;
            if (overDb < -halfKnee) return 0.0f;
            if (overDb >  halfKnee) return -(overDb * (1.0f - 1.0f / ratio));
            const float x = (overDb + halfKnee) / kneeDb;
            return -(x * x * overDb * (1.0f - 1.0f / ratio) * 0.5f);
        } else {
            if (overDb <= 0.0f) return 0.0f;
            return -(overDb * (1.0f - 1.0f / ratio));
        }
    }
};
