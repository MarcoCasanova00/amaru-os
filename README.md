# AMARU OS / BUNKER ADVANCED

An experimental, **alpha-stage** custom firmware for the M-VAVE FM-1, built for
hard genres: tekno, acid, hardcore, jungle, drum & bass, and psytrance.

It is built on **Bunker Evolved 1.0** — reusing its four-track sequencer, synth
engines, storage and recovery — and adds:

- A deterministic `GEN` page: `GENRE` / `SEED` / `ENERGY` / `FORM` (LOOP / PARTS / SONG).
  Turn OCT+ to advance the seed and play a new song; OCT- resets the seed and plays it.
  The recipe names identify the sound: `HYPOGEUM CHANT`, `SILBURY WAIL`, `MYC PUNCHY`… (Carnac/Mycelium source shapes).
- Nine first-class genres: tribe tekno, mental tekno, hardtekk, acidcore,
  jungle, drum & bass, breakbeat tekno, techno, psytrance — with genre-specific
  drum grammar (jungle/DnB are **not** four-on-the-floor).
- Three strict monochrome terminal themes: green/black, black/white, amber/black.
- Natively rebuilt Carnac/Mycelium-inspired timbres (wavetable ghosts, metal
  blooms, psy swarms, tomb drones) through Bunker's own engines — no binaries imported.

> ⚠️ **This is an alpha. It has never been flashed on real FM-1 hardware.**
> Do not flash it yet. See [`docs/AMARU-ANALYSIS.md`](docs/AMARU-ANALYSIS.md)
> for the engineering record and the 9-question release gate.

## Try it in the browser

The WASM emulator is published to GitHub Pages:

➡️ https://marcocasanova00.github.io/amaru-os/

It runs the firmware's real DSP/UI compiled with Emscripten and matches the
native host build bit-for-bit.

## Building locally

Firmware image (Jieli AC79 toolchain):

```sh
python3 firmware/tools/gen_bunker_library.py
cd firmware
JIELI_TOOLCHAIN=/your/.jieli/toolchain \
AC79_SDK=/your/fw-AC79_AIoT_SDK \
FELUCCA_CPU2=0 sh ./build.sh
```

WebAssembly emulator (Emscripten):

```sh
cd firmware
EMU_PREVIEW=0 sh web/emu/build.sh
node web/emu/emu_test.mjs build/emu/felucca.wasm build/emu/native.f32
```

## Tests

```sh
python3 firmware/tests/run_bunker_checks.py     # 19 release suites
cc -O1 -w -ffp-contract=off -Ifirmware/src -o /tmp/amaru_gen_test \
   firmware/tests/amaru_gen_test.c firmware/src/amaru_gen.c -lm && /tmp/amaru_gen_test
```

## Credits

Built on **Bunker Evolved 1.0** (https://github.com/kidbee85-beep/bunker-evolved),
GPL-3.0-only, with its original notices and attributions retained in `firmware/`
and `docs/ATTRIBUTION.md`. Genre concepts come from **JIANT** (GPL-3.0-only);
moving-spectrum/timbral ideas are referenced from **Carnac OS** (GPL-3.0) and
**Mycelium FM-1** (GPL-3.0-only), used as inspiration only — no binaries or
brand assets are imported.

## License

AMARU OS is **GPL-3.0-only**. Bunker's vendored components retain their own
licenses (Felucca-Engines MIT, msfa Apache-2.0, CC0 samples, OFL fonts).
See [`LICENSE`](LICENSE) and [`docs/ATTRIBUTION.md`](docs/ATTRIBUTION.md).
