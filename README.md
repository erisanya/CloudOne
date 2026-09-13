# Cloud One

A one-knob "brighter" EQ, inspired by the Waves OneKnob Brighter. Turn it up,
things get brighter. That's the whole plugin.

- **One control**: 0 to +24 dB
- **One filter**: high shelf, fixed at 8 kHz, Q 0.707 (Butterworth — no
  resonant peak at the knee)
- **Boost only** — the knob cannot cut
- **VST3 + Standalone**, built with JUCE

## Honest technical notes

A few things worth knowing rather than just claiming:

- **Latency**: genuinely 0 samples. This is a minimum-phase IIR filter
  (not linear-phase FIR), so there's no added delay and no need for
  PDC in a host.
- **Phase**: a shelf filter — any shelf filter, on any plugin, including
  the units this is inspired by — has to shift phase near its corner
  frequency. That's mathematically inherent to minimum-phase filters and
  can't be engineered away without switching to a linear-phase design,
  which *would* add latency. This plugin doesn't touch phase anywhere it
  doesn't have to (single filter, no oversampling artifacts, no extra
  stages), but "0 phase shift" isn't an honest claim for this kind of EQ.
- **Artifacts**: the gain is smoothed (20 ms) before it reaches the
  filter coefficients, so moving the knob during playback — including
  via automation — won't click or zipper.
- **Bugs**: I can't promise zero, only that the code is small, does one
  thing, and has no state a host wouldn't expect. Test it in your DAW
  before relying on it in a session.

## Building

You'll need [JUCE](https://github.com/juce-framework/JUCE) and CMake 3.22+.

```bash
# Option A — you already have a local JUCE checkout:
cmake -B build -DJUCE_DIR=/path/to/JUCE
cmake --build build --config Release

# Option B — let CMake fetch JUCE for you (needs network access):
cmake -B build
cmake --build build --config Release
```

The built VST3 will be copied to your system's VST3 folder automatically
(`COPY_PLUGIN_AFTER_BUILD TRUE`), and a Standalone app will be built too
for quick testing without opening a DAW.

## License

Intended to be free and open source, not sold. JUCE's free tier requires
either the paid JUCE license or releasing under **GPLv3** — GPLv3 is the
right fit here: see https://www.gnu.org/licenses/gpl-3.0.txt for the full
license text, and add a `LICENSE` file with that text plus your name as
copyright holder before you publish the repo.
