# Karplus-Strong Plucked String Synthesiser

A real-time plucked string synthesiser built in **JUCE/C++**, implementing the classic **Karplus-Strong** physical modelling algorithm.

## Overview

The Karplus-Strong algorithm simulates the vibration of a plucked string using a short noise burst fed into a delay line with feedback. With just a few lines of DSP, the system produces a surprisingly realistic plucked string tone — no samples, no oscillators, just physics.

This project was developed as part of **ECS7012P – Music and Audio Programming** at Queen Mary University of London.

## How It Works

1. **Excitation** — A short burst of white noise is generated when the pluck button is pressed. Its duration is controlled by the *Width* parameter. White noise is used because it contains a broad range of frequency components.
2. **Delay line** — A circular delay buffer forms the core of the synthesis. The delay length determines the fundamental frequency of the output tone (shorter delay → higher pitch).
3. **Feedback & decay** — The delayed signal is fed back and multiplied by a decay factor slightly less than 1. This keeps the system stable and simulates energy loss in a vibrating string.
4. **Low-pass filtering** — A first-order low-pass filter inside the feedback loop models natural string damping, so higher harmonics decay faster than lower ones — exactly as a real string behaves.

## Parameters

| Control | Function |
|---------|----------|
| **Delay** | Sets the pitch of the plucked note |
| **Decay** | Controls how long the note sustains |
| **Width** | Strength and brightness of the initial pluck |
| **Pluck** | Triggers the excitation |

## Results

The plugin produces a clean plucked string tone with a clear harmonic structure. Spectrogram analysis confirms the expected behaviour — strong vertical excitation at the onset, horizontal harmonic bands at the fundamental and overtones, and faster decay at higher frequencies.

Full results, waveform plots, and spectrograms are available in [`report.pdf`](./report.pdf).

## Audio Samples

Three rendered audio examples demonstrating different parameter settings are included in the project folder (`.wav` files).

## Project Structure

```
karplus strong/
├── Source/
│   ├── PluginProcessor.cpp    # Core DSP and audio processing
│   ├── PluginProcessor.h
│   ├── PluginEditor.cpp       # UI and parameter controls
│   └── PluginEditor.h
├── karplus_strong.jucer       # JUCE project file
├── report.pdf                 # Full lab report with spectrograms
└── *.wav                      # Audio samples
```

## Build Instructions

1. Install [JUCE](https://juce.com/) (tested with the standard JUCE framework)
2. Open `karplus_strong.jucer` in **Projucer**
3. Export to your IDE (Xcode on macOS, Visual Studio on Windows)
4. Build as a **Standalone** application

## Tech Stack

- **C++** with the JUCE framework
- Real-time DSP: circular delay buffer, IIR low-pass filter, feedback loop
- Built as a standalone audio plugin

## Author

**Sanjay Yamasandhi Sundresh** — MSc Electronics, Queen Mary University of London

---

