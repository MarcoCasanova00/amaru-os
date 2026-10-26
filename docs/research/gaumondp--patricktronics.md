# Patricktronics

**A generative sound-bed firmware for the M-VAVE FM-1.** Patricktronics turns a
small, fixed set of notes and one FM timbre into an endless, self-evolving
ambient loop — the same sounds, never arranged the same way twice. It is a thin
layer built on top of [Felucca](https://github.com/hugelton/Felucca), reusing its
FM engine, sequencer and effects, and adding only what a living Frippertronics
loop needs.

## Features

- **Locked note palette.** Enter up to 5 notes and one FM patch. These never
  change: the sound stays exactly what you chose.
- **Generative evolution.** The arrangement — which note plays, when, at what
  octave, with what accent, which steps fall silent — mutates on its own over
  time, driven by a pseudo-random generator. You set *how often* and *how much*;
  the loop does the rest.
- **Non-deterministic by design.** The generator is seeded from hardware entropy
  at every launch, so starting the same configuration twice gives two different
  renders. A re-seed re-rolls the performance without touching your palette or
  rules.
- **Ambient effects.** Felucca's delay and reverb (HALL / ROOM / SPRING) are at
  the core of the drone and ambient textures — high delay feedback into HALL is
  the signature Patricktronics wash.
- **16 configurations.** Store 16 compact rule-sets {palette, rules, effects} and
  recall them. The seed is never stored, so a recalled configuration is always a
  fresh performance.
- **Non-destructive install.** Patricktronics never writes over your existing
  Felucca projects, samples or presets. Switch to it today, switch back to stock
  Felucca tomorrow — your data is untouched.

## Why

I am a guitar player. I built Patricktronics to have a generative sound bed to
improvise over: a texture that keeps moving on its own, out of a small set of
notes I choose, so I can pick up the guitar and play against something alive
rather than a static backing track. The FM-1 is cheap, pocket-sized and
battery-powered, which makes it the right box for that — a self-playing ambient
machine you can take anywhere.

### What Frippertronics was

Frippertronics is a tape-loop technique developed by guitarist **Robert Fripp**
with **Brian Eno** in the 1970s. Two reel-to-reel tape machines were set up side
by side: a phrase played into the first was carried on the tape to the second and
fed back into the first, so each new phrase layered onto the slowly decaying echo
of everything played before. The result was a dense, evolving ambient texture
that the guitarist improvised over, live — the record *(No Pussyfooting)* and
Fripp's *Let the Power Fall* (its piece *1987*) are the touchstones.

Patricktronics is that idea rebuilt without tape: a short musical material that
loops, layers and decays, mutating as it goes, for a guitarist to play over.

## Install

In Chrome or Edge, connect the FM-1 by USB and install Patricktronics from the
[FM-1 Firmware Hub](https://fm1.designburgapps.com/), or run
`python3 tools/fm1_install.py build/patricktronics.fwsc` from a terminal.

**Back to stock any time.** Install the official M-VAVE **FM-1 V15** from the hub,
or hold **OCT−** while powering on for USB rescue. If the device will not start,
[FM-1 Transporter](https://github.com/kurogedelic/FM-1-transporter) recovers it.
Installing firmware is at your own risk; your Felucca data is preserved either
way.

## Build from source

### Development environment

| Need | How |
| --- | --- |
| JieLi toolchain | `tools/get_toolchain.sh` (clang for pi32v2 → `~/.jieli/toolchain`) |
| JieLi AC79 SDK | `git clone --depth 1 --branch AC79NN_SDK_V1.2.1_2023-12-13 https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK.git ~/fw-AC79_AIoT_SDK` |
| macOS build | Docker Desktop — the Linux x86-64 toolchain runs in a `linux/amd64` container (Rosetta). Linux x86-64 runs it natively. |
| Python | Python 3 with `Pillow` and `fonttools` (UI assets are rasterised at build) |
| Host tests | a C compiler on the dev machine; Node.js and Emscripten optional for the browser emulator |

### Commands

```sh
./build.sh                 # -> build/patricktronics.fwsc
tests/run_tests.sh         # host tests (no FM-1 needed)
```

The Patricktronics host tests (`tests/ptrx_*`) cover the PRNG stream, the
palette-bounded mutation, the mutation timer, the non-destructive config store
and the mutation cost guard — all runnable without a device.

## License

GPL-3.0-only, as Felucca and the FM-1 firmware family. Patricktronics is a fork of
[Felucca](https://github.com/hugelton/Felucca) by Leo Kuroshita (Hügelton
Instruments); its platform descends from Felucca and SLOOP. M-VAVE and FM-1 are
trademarks of their respective owners; this project is not affiliated with or
endorsed by them.
