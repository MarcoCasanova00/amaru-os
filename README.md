# AMARU OS / BUNKER ADVANCED

An experimental firmware image for the M‑VAVE FM‑1, fusing:

- **Bunker Evolved** — four-track sequencer, engines, GROW/EVOLVE, storage, recovery.
- **JIANT tekno fork** — genre concepts, drum/bass grammar, tekno articulation (adapted to four layers).
- **Carnac OS** — moving-spectrum / haunted-timbre character, rebuilt natively.
- **Mycelium FM‑1** — organic/psy texture vocabulary, rebuilt natively.

It adds a deterministic `GEN` page: `GENRE` / `SEED` / `ENERGY` / `FORM`, with
tribe tekno, mental tekno, hardtekk, acidcore, jungle, drum & bass, breakbeat
tekno, techno and psytrance, plus three strict monochrome terminal themes
(green/black, black/white, amber/black).

## ⚠️ This is an unreleased alpha

- We have **not** flashed or validated this firmware on actual FM‑1 hardware.
- **Do not flash it to a unit yet.** It is not released.
- Rollback to the stock image remains possible through the external UFW installer
  / UBOOT recovery path; AMARU never erases the bootloader, but no release
  guarantees are made until the hardware validation phase.

See [`docs/AMARU-ANALYSIS.md`](docs/AMARU-ANALYSIS.md) for the full engineering
record and the 9-question release gate.

## Try it in the browser (alpha)

The WASM emulator is built and published to GitHub Pages by a GitHub Actions
workflow on every push to `alpha`:

➡️ https://MarcoCasanova00.github.io/amaru-os/

It loads the FM‑1's real DSP and UI from `firmware/src` (no audio samples are
embedded; the emulator plays silence until a sound is loaded).

## Building locally

Firmware image (requires the Jieli AC79 toolchain):

```sh
python3 firmware/tools/gen_bunker_library.py
cd firmware
JIELI_TOOLCHAIN=/your/.jieli/toolchain \
AC79_SDK=/your/fw-AC79_AIoT_SDK \
FELUCCA_CPU2=0 sh ./build.sh
```

WebAssembly emulator (requires Emscripten):

```sh
cd firmware
EMU_PREVIEW=0 sh web/emu/build.sh
```

## Credits

Built on **Bunker Evolved 1.0** (https://github.com/kidbee85-beep/bunker-evolved),
GPL‑3.0‑only, with its original notices and asset attributions retained in
`firmware/` and `docs/ATTRIBUTION.md`. Genre concepts and tekno
articulation come from **JIANT** (GPL‑3.0‑only); moving-spectrum/timbral ideas
are referenced from **Carnac OS** (GPL‑3.0) and **Mycelium FM‑1**
(GPL‑3.0‑only), both used as inspiration only — no binaries or brand assets
are imported.

## License

AMARU OS is GPL‑3.0‑only. See [`LICENSE`](LICENSE) and
[`docs/ATTRIBUTION.md`](docs/ATTRIBUTION.md) for the full component list
(Felucca-Engines MIT, msfa Apache-2.0, CC0 samples, OFL fonts, etc.).
