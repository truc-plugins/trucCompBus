# trucComp@Bus

A stereo bus compressor plugin with two character modes, built with JUCE.  
**VST3 / AU — macOS Apple Silicon (arm64)**

---

## Overview

trucComp@Bus is a transparent-to-coloured bus compressor designed for mix bus and group bus duties.  
Two distinct compression characters are selectable via the **S / A Mode** switch.

### S Mode — SSL-style

- Sidechain high-pass filter at 120 Hz (removes low-end from the detector, keeping kick/bass from pumping the compressor)
- Attack 1 ms · Release 400 ms (program-dependent: release doubles in speed when gain reduction exceeds 6 dB)
- Hard knee
- Transparent glue character

### A Mode — VCA / 1176-style

- No sidechain filtering
- Attack 0.1 ms · Release 750 ms
- Soft knee (6 dB transition zone)
- 70% stereo link
- Punchy, forward VCA character

### Parameters

| Control | Range | Default |
|---|---|---|
| THRESHOLD | −40 to 0 dB | −12 dB |
| RATIO | 3.5:1 / 8:1 | 3.5:1 |
| MODE | S / A | S |
| OUTPUT | −20 to +20 dB | 0 dB |

### GR Meter

Segmented vertical gain-reduction meter with dB scale.  
Green (0 → −3) · Yellow (−3 → −9) · Orange (−9 → −15) · Red (−15 → −20)

---

## macOS Security Warning / macOS セキュリティ警告

When opening the plugin for the first time, macOS may show a warning:  
**"trucComp@Bus.vst3 cannot be opened because Apple cannot check it for malicious software."**

This is because the plugin is not signed with an Apple Developer certificate.  
Run the following commands in Terminal to allow it:

```bash
xattr -cr ~/Library/Audio/Plug-Ins/VST3/trucComp@Bus.vst3
xattr -cr ~/Library/Audio/Plug-Ins/Components/trucComp@Bus.component
```

Then re-scan plug-ins in your DAW.

---

初回起動時に以下のような警告が表示される場合があります：  
**「"trucComp@Bus.vst3" は開いていません」**

これはAppleの開発者証明書による署名がないためです。  
ターミナルで以下のコマンドを実行することで使用できるようになります：

```bash
xattr -cr ~/Library/Audio/Plug-Ins/VST3/trucComp@Bus.vst3
xattr -cr ~/Library/Audio/Plug-Ins/Components/trucComp@Bus.component
```

実行後、DAW でプラグインを再スキャンしてください。

---

## Requirements

- macOS 11.0+ (Apple Silicon, arm64)
- JUCE 7 or later
- CMake 3.22+
- Xcode Command Line Tools

---

## Build

```bash
# Clone the repository
git clone https://github.com/truc-plugins/trucCompBus.git
cd trucCompBus

# Configure (set JUCE_PATH to your local JUCE installation)
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DJUCE_PATH=/path/to/JUCE

# Build
cmake --build build --config Release -j$(sysctl -n hw.logicalcpu)
```

Output artefacts will be in:
```
build/trucCompBus_artefacts/Release/VST3/trucComp@Bus.vst3
build/trucCompBus_artefacts/Release/AU/trucComp@Bus.component
build/trucCompBus_artefacts/Release/Standalone/trucComp@Bus.app
```

### Install

Copy the built bundles to your plug-in folders:

```bash
# VST3
cp -r build/trucCompBus_artefacts/Release/VST3/trucComp@Bus.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/

# AU
cp -r build/trucCompBus_artefacts/Release/AU/trucComp@Bus.component \
      ~/Library/Audio/Plug-Ins/Components/
```

Then re-scan plug-ins in your DAW.

---

## File Structure

```
trucCompBus/
  CMakeLists.txt
  Source/
    PluginProcessor.h / .cpp   — Audio processing, APVTS parameters
    PluginEditor.h  / .cpp     — UI (380×260 px fixed, 30 Hz repaint timer)
    Compressor.h               — DSP: S mode and A mode compressor logic
    GRMeter.h                  — Gain reduction meter component
    CustomLookAndFeel.h        — Industrial flat UI style
```

---

## License

MIT License — see [LICENSE](LICENSE)

---

*by truc*
