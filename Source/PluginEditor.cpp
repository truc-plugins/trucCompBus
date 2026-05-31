#include "PluginEditor.h"

static constexpr int W = 520;
static constexpr int H = 340;

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static juce::Font monoFont(float size)
{
    return juce::Font(juce::FontOptions{}
        .withName(juce::Font::getDefaultMonospacedFontName())
        .withHeight(size));
}

void TrucCompBusEditor::setupKnob(juce::Slider& knob, juce::Label& valueLabel,
                                   juce::Label& textLabel, const juce::String& labelText)
{
    knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(knob);

    valueLabel.setFont(monoFont(10.0f));
    valueLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFE8E8E8));
    valueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(valueLabel);

    textLabel.setFont(monoFont(9.0f));
    textLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF666666));
    textLabel.setJustificationType(juce::Justification::centred);
    textLabel.setText(labelText, juce::dontSendNotification);
    addAndMakeVisible(textLabel);
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

TrucCompBusEditor::TrucCompBusEditor(TrucCompBusProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setLookAndFeel(&laf);
    setSize(W, H);
    setResizable(false, false);

    // ── Threshold knob ──────────────────────────────────────────────────
    threshKnob.setRange(-40.0, 0.0, 0.1);
    setupKnob(threshKnob, threshValueLabel, threshTextLabel, "THRESHOLD");
    threshAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        proc.apvts, "threshold", threshKnob);

    // ── Output knob ─────────────────────────────────────────────────────
    outputKnob.setRange(-20.0, 20.0, 0.1);
    setupKnob(outputKnob, outputValueLabel, outputTextLabel, "OUTPUT");
    outputAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        proc.apvts, "output", outputKnob);

    // ── Ratio buttons ───────────────────────────────────────────────────
    for (auto* btn : { &ratio35Btn, &ratio8Btn }) {
        btn->setClickingTogglesState(true);
        btn->setRadioGroupId(1);
        addAndMakeVisible(btn);
    }
    ratio35Btn.setToggleState(true, juce::dontSendNotification);

    ratio35Btn.onClick = [this] {
        if (ratio35Btn.getToggleState()) {
            ratio8Btn.setToggleState(false, juce::dontSendNotification);
            proc.apvts.getParameter("ratio")->setValueNotifyingHost(0.0f);
        }
    };
    ratio8Btn.onClick = [this] {
        if (ratio8Btn.getToggleState()) {
            ratio35Btn.setToggleState(false, juce::dontSendNotification);
            proc.apvts.getParameter("ratio")->setValueNotifyingHost(1.0f);
        }
    };

    ratioTextLabel.setFont(monoFont(9.0f));
    ratioTextLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF666666));
    ratioTextLabel.setJustificationType(juce::Justification::centred);
    ratioTextLabel.setText("RATIO", juce::dontSendNotification);
    addAndMakeVisible(ratioTextLabel);

    // ── Mode buttons ────────────────────────────────────────────────────
    for (auto* btn : { &modeSBtn, &modeABtn }) {
        btn->setClickingTogglesState(true);
        btn->setRadioGroupId(2);
        addAndMakeVisible(btn);
    }
    modeSBtn.setToggleState(true, juce::dontSendNotification);

    modeSBtn.onClick = [this] {
        if (modeSBtn.getToggleState()) {
            modeABtn.setToggleState(false, juce::dontSendNotification);
            proc.apvts.getParameter("mode")->setValueNotifyingHost(0.0f);
        }
    };
    modeABtn.onClick = [this] {
        if (modeABtn.getToggleState()) {
            modeSBtn.setToggleState(false, juce::dontSendNotification);
            proc.apvts.getParameter("mode")->setValueNotifyingHost(1.0f);
        }
    };

    modeTextLabel.setFont(monoFont(9.0f));
    modeTextLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF666666));
    modeTextLabel.setJustificationType(juce::Justification::centred);
    modeTextLabel.setText("MODE", juce::dontSendNotification);
    addAndMakeVisible(modeTextLabel);

    // ── GR Meter ────────────────────────────────────────────────────────
    addAndMakeVisible(grMeter);

    grTextLabel.setFont(monoFont(9.0f));
    grTextLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF666666));
    grTextLabel.setJustificationType(juce::Justification::centred);
    grTextLabel.setText("GR", juce::dontSendNotification);
    addAndMakeVisible(grTextLabel);

    startTimerHz(30);
}

TrucCompBusEditor::~TrucCompBusEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

// ─────────────────────────────────────────────────────────────────────────────
// Timer
// ─────────────────────────────────────────────────────────────────────────────

void TrucCompBusEditor::timerCallback()
{
    // Update knob value displays
    const float tv = proc.apvts.getRawParameterValue("threshold")->load();
    threshValueLabel.setText(juce::String(tv, 1) + " dB", juce::dontSendNotification);

    const float ov = proc.apvts.getRawParameterValue("output")->load();
    const juce::String ovStr = (ov >= 0.0f ? "+" : "") + juce::String(ov, 1) + " dB";
    outputValueLabel.setText(ovStr, juce::dontSendNotification);

    // GR meter
    grMeter.setGainReductionDb(proc.getCurrentGRDb());
    grMeter.repaint();
}

// ─────────────────────────────────────────────────────────────────────────────
// Paint
// ─────────────────────────────────────────────────────────────────────────────

void TrucCompBusEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF1A1A1A));

    // Panel background
    g.setColour(juce::Colour(0xFF222222));
    g.fillRect(15, 32, W - 30, H - 48);

    // Plugin name
    g.setColour(juce::Colour(0xFF888888));
    g.setFont(monoFont(11.0f));
    g.drawText("trucComp@Bus", 0, 10, W, 16, juce::Justification::centred, false);
}

// ─────────────────────────────────────────────────────────────────────────────
// Layout
// ─────────────────────────────────────────────────────────────────────────────

void TrucCompBusEditor::resized()
{
    // ── THRESHOLD  (left half — large knob) ──────────────────────────────
    // Left half: x=15..255 (240px wide), knob centred inside
    constexpr int thKnobSize = 150;
    constexpr int thCentreX  = 15 + 240 / 2;          // 135
    const     int thKnobX    = thCentreX - thKnobSize / 2;  // 60
    constexpr int thKnobY    = 75;

    threshValueLabel.setBounds(thKnobX - 5, thKnobY - 18, thKnobSize + 10, 14);
    threshKnob      .setBounds(thKnobX,     thKnobY,       thKnobSize,     thKnobSize);
    threshTextLabel .setBounds(thKnobX - 5, thKnobY + thKnobSize + 5, thKnobSize + 10, 13);

    // ── RATIO / MODE buttons  (right-middle area, top) ───────────────────
    // Middle section: x=255..385 (130px)
    constexpr int btnW   = 52;
    constexpr int btnH   = 32;
    constexpr int btnGap = 6;
    constexpr int ratioX = 258;
    constexpr int modeX  = 320;   // ratioX + btnW + 10
    constexpr int btnTopY = 58;

    ratioTextLabel.setBounds(ratioX, btnTopY - 14, btnW, 12);
    ratio35Btn    .setBounds(ratioX, btnTopY,       btnW, btnH);
    ratio8Btn     .setBounds(ratioX, btnTopY + btnH + btnGap, btnW, btnH);

    modeTextLabel.setBounds(modeX, btnTopY - 14, btnW, 12);
    modeSBtn     .setBounds(modeX, btnTopY,       btnW, btnH);
    modeABtn     .setBounds(modeX, btnTopY + btnH + btnGap, btnW, btnH);

    // ── OUTPUT knob  (below RATIO / MODE buttons) ────────────────────────
    // Buttons bottom: btnTopY + btnH + btnGap + btnH = 58+32+6+32 = 128
    constexpr int outKnobSize = 68;
    // Centre horizontally in the middle section (ratioX..modeX+btnW = 258..372)
    constexpr int outCentreX  = (ratioX + modeX + btnW) / 2;   // 315
    const     int outKnobX    = outCentreX - outKnobSize / 2;   // 281
    constexpr int outKnobY    = 165;

    outputValueLabel.setBounds(outKnobX - 5, outKnobY - 16, outKnobSize + 10, 13);
    outputKnob      .setBounds(outKnobX,     outKnobY,       outKnobSize,     outKnobSize);
    outputTextLabel .setBounds(outKnobX - 5, outKnobY + outKnobSize + 4, outKnobSize + 10, 13);

    // ── GR Meter  (far right, full height) ───────────────────────────────
    constexpr int meterX = 390;
    constexpr int meterY = 38;
    constexpr int meterW = 115;
    const     int meterH = H - meterY - 32;

    grMeter    .setBounds(meterX, meterY, meterW, meterH);
    grTextLabel.setBounds(meterX, meterY + meterH + 4, meterW, 13);
}
