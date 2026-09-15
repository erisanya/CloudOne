# Cloud One

<img width="250" height="340" alt="image" src="https://github.com/user-attachments/assets/981afc03-a856-45e0-8405-ab91745fe793" />

A one-knob "brighter" EQ, inspired by the Waves OneKnob Brighter. Turn it up,
things get brighter. That's the whole plugin.

- One control: 0 to +24 dB
- One filter: high shelf, fixed at 8 kHz, Q 0.707 (Butterworth — no
  resonant peak at the knee)
- Boost only — the knob cannot cut
- VST3 + Standalone, built with JUCE

## Honest technical notes

A few things worth knowing rather than just claiming:

- Latency: genuinely 0 samples. This is a minimum-phase IIR filter
  (not linear-phase FIR), so there's no added delay and no need for
  PDC in a host.
- Phase: a shelf filter — any shelf filter, on any plugin, including
  the units this is inspired by — has to shift phase near its corner
  frequency. That's mathematically inherent to minimum-phase filters and
  can't be engineered away without switching to a linear-phase design,
  which *would* add latency. This plugin doesn't touch phase anywhere it
  doesn't have to (single filter, no oversampling artifacts, no extra
  stages), but "0 phase shift" isn't an honest claim for this kind of EQ.
- Artifacts: the gain is smoothed (20 ms) before it reaches the
  filter coefficients, so moving the knob during playback — including
  via automation — won't click or zipper.
- Bugs: I can't promise zero, only that the code is small, does one
  thing, and has no state a host wouldn't expect. Test it in your DAW
  before relying on it in a session.

## Requirements

- CMake (3.22+)
- Git (needed so CMake can fetch JUCE automatically)
- Visual Studio Community (with the "Desktop development with C++" workload)

## Building on Windows

1. Unzip this project.
2. Open the **Developer PowerShell for VS** (Start menu → your Visual Studio version)
3. Run:

```powershell
cd C:\*YOUR-PATH*
cmake -B build
cmake --build build --config Release
```

The first build will take a while — CMake's `FetchContent` downloads JUCE itself
the first time. After that, rebuilds are much faster.

## Output location

After a successful build:

- VST3: `build\CloudOne_artefacts\Release\VST3\CloudOne.vst3`
- Standalone app: `build\CloudOne_artefacts\Release\Standalone\CloudOne.exe`

Copy the `.vst3` into your DAW's VST3 folder (usually
`C:\Program Files\Common Files\VST3`) if it isn't picked up automatically —
`COPY_PLUGIN_AFTER_BUILD` is already set in `CMakeLists.txt` so this normally
happens for you.
