#pragma once
#include <JuceHeader.h>

// Vertical GR meter with dB scale on the right side.
// Call setGainReductionDb() from the audio thread (atomic), repaint from a timer.
class GRMeter : public juce::Component
{
public:
    GRMeter() { currentGR.store(0.0f); }

    void setGainReductionDb(float grDb) // grDb <= 0
    {
        const float incoming = juce::jlimit(-20.0f, 0.0f, grDb);
        const float prev = currentGR.load();
        // At 30 fps (33.3 ms/frame):
        //   Attack  5 ms  → coeff = exp(-33.3/5)   ≈ 0.0013 (near-instant)
        //   Release 200 ms → coeff = exp(-33.3/200) ≈ 0.846
        const float next = (incoming < prev)
            ? incoming + 0.001f * (prev - incoming)   // attack: jump almost immediately
            : incoming + 0.846f * (prev - incoming);  // release: 200 ms
        currentGR.store(next);
    }

    void paint(juce::Graphics& g) override
    {
        const auto  bounds = getLocalBounds().toFloat();
        const float totalW = bounds.getWidth();
        const float totalH = bounds.getHeight();

        // Split: left bar area, right scale area
        constexpr float scaleW  = 32.0f;
        const float     barW    = totalW - scaleW - 2.0f;
        const float     barX    = bounds.getX();
        const float     barTop  = bounds.getY() + 1.0f;
        const float     barH    = totalH - 2.0f;

        // Bar background
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillRoundedRectangle(barX, barTop, barW, barH, 2.0f);
        g.setColour(juce::Colour(0xFF333333));
        g.drawRoundedRectangle(barX + 0.5f, barTop + 0.5f, barW - 1.0f, barH - 1.0f, 2.0f, 1.0f);

        // Fill
        const float gr = juce::jlimit(-20.0f, 0.0f, currentGR.load());
        const float fillFraction = -gr / 20.0f;
        const float fillH = barH * fillFraction;

        if (fillH >= 1.0f) {
            struct Seg { float startDb; float endDb; juce::Colour colour; };
            const Seg segs[] = {
                {  0.0f,  -3.0f, juce::Colour(0xFF4CAF50) },
                { -3.0f,  -9.0f, juce::Colour(0xFFFFEB3B) },
                { -9.0f, -15.0f, juce::Colour(0xFFFF9800) },
                {-15.0f, -20.0f, juce::Colour(0xFFF44336) },
            };

            for (const auto& seg : segs) {
                const float segTop    = barTop + (-seg.startDb / 20.0f) * barH;
                const float segBottom = barTop + (-seg.endDb   / 20.0f) * barH;
                const float fillEnd   = barTop + fillH;
                if (fillEnd <= segTop) continue;
                const float drawnBottom = std::min(fillEnd, segBottom);
                const float drawnH = drawnBottom - segTop;
                if (drawnH < 0.5f) continue;
                g.setColour(seg.colour);
                g.fillRect(barX + 2.0f, segTop, barW - 4.0f, drawnH);
            }
        }

        // Scale ticks and labels
        const float scaleX   = barX + barW + 2.0f;
        const int   ticks[]  = { 0, -3, -6, -9, -12, -15, -20 };
        g.setFont(juce::Font(juce::FontOptions{}.withName(juce::Font::getDefaultMonospacedFontName()).withHeight(7.0f)));

        for (int db : ticks) {
            const float y = barTop + (-float(db) / 20.0f) * barH;
            g.setColour(juce::Colour(0xFF555555));
            g.fillRect(scaleX, y - 0.5f, 4.0f, 1.0f);
            g.setColour(juce::Colour(0xFF555555));
            const juce::String label = (db == 0) ? "0" : juce::String(db);
            g.drawText(label, scaleX + 5.0f, y - 5.0f, scaleW - 6.0f, 10.0f,
                       juce::Justification::centredLeft, false);
        }
    }

private:
    std::atomic<float> currentGR { 0.0f };
};
