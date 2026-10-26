# Bunker Evolved 1.0

Generative ambient firmware for the M-VAVE FM-1. Product `FM-1_7100002`, display `BE 1.0-RC2`.

By **Kid Bunka**. Free software under **GPL-3.0-only**, with the original component licenses and credits retained.

[Website](https://bunker-evolved.sugarypixie10.chatgpt.site/) · [Firmware release](https://github.com/kidbee85-beep/bunker-evolved/releases/tag/v1.0.0) · [User guide](https://bunker-evolved.sugarypixie10.chatgpt.site/manual.html) · [Play in browser](https://bunker-evolved.sugarypixie10.chatgpt.site/simulator/) · [Chrome editor](https://bunker-evolved.sugarypixie10.chatgpt.site/editor/)

**The complete Release 1.0 source is now in this repository.** Start with [`firmware/`](firmware/), the build instructions below, [`LICENSE`](LICENSE), and [`firmware/LICENSING.md`](firmware/LICENSING.md). Engine sources are included as ordinary files; no submodule download is required. [`dist/`](dist/) contains website and editor sources, and [`docs/`](docs/) contains the recorded tests and hardware report. See [`SOURCE-PUBLICATION.txt`](SOURCE-PUBLICATION.txt) for the source archive checksum and publication scope.

A source-built Felucca fork for generative ambient music. It adds four-role sound banks (64 recipes), 24 worlds, 24 motifs, 32 scales, seven bounded mutations plus lock, GROW scale variations with exact one-step undo, and six HOME views that edit the actual synthesizer and sequencer state. Full Felucca engine editors remain accessible.

**Release 1.0 was approved after the owner completed Bunker → original → Bunker → original on their FM-1 on 17 October 2026, with audio and controls working.** The binary is unchanged: its internal display label remains `BE 1.0-RC2`. No reflash is needed if already running `FM-1_7100002`. The owner did not separately record the stock version string. This is normal-update acceptance on one device, not power-loss recovery validation. See `docs/release/hardware-acceptance.md` and `docs/release/test-status.txt`.

## Source provenance

- Felucca 1.6.1: https://github.com/hugelton/Felucca commit `5738617968a22b5d4c0d3470cfcc95559052f446`.
- Felucca-Engines: https://github.com/hugelton/Felucca-Engines commit `c46e0086f69bdc26df1cd144dca9e683cc4531dd`. Its actual source files are vendored in `firmware/firmware/engines/`; no submodule initialization is necessary in this archive.
- Browser emulator design derives through Felucca from Charles Vestal's X0X. Headers and original notices retained.
- Bunker generator/UI: `firmware/firmware/src/bunker_gen.c`, `bunker_ui.c`, generated `bunker_library.inc`; recipes in `firmware/tools/gen_bunker_library.py`.
- Research snapshots: `docs/research/`. The choice is documented in `dist/research.html`; it is not an audit of every firmware in the catalog.
- Main license: GPL-3.0-only. Preserve `firmware/LICENSE`, `firmware/LICENSING.md`, all `LICENSES/` files and asset attributions. The custom engines remain MIT, MSFA portions Apache-2.0, instrument samples CC0, and fonts retain their respective licenses.

## Build the device firmware

Use Python 3, Pillow, fontTools, the JieLi pi32v2 Linux x86-64 toolchain, and AC79 SDK tag `AC79NN_SDK_V1.2.1_2023-12-13`. The upstream instructions with download locations are in `firmware/BUILDING.md`. On macOS this toolchain runs via Docker as documented there. No Mac is required for the source work.

From this archive's root:

```sh
mkdir -p dist
python3 firmware/tools/gen_bunker_library.py
cd firmware
JIELI_TOOLCHAIN=/absolute/path/to/toolchain AC79_SDK=/absolute/path/to/sdk FELUCCA_CPU2=0 sh ./build.sh
```

The output is `firmware/build/felucca.fwsc`. Its internal package identity is Bunker's. **Do not pass `--release`**: that upstream switch overrides the product identity with Felucca's release numbering. One CPU core is the Bunker default. The two-core experiment is not part of this RC2. Keep the original `build.sh` and packaging checks; do not flash `felucca.bin` directly.

The browser-based source upload stores files without executable flags. Use `sh` for shell scripts (for example `sh tools/get_toolchain.sh`) and `python3` for Python scripts, or restore executable permissions with `chmod +x` before following upstream examples that run them directly. The original release source ZIP retains its original file permissions.

## Build and test the simulator

After a target build has generated the tables and assets, install/activate Emscripten (tested: 4.0.23):

```sh
cd firmware
EMU_PREVIEW=0 sh web/emu/build.sh
cc -O1 -w -ffp-contract=off -Ibuild/gen -Ifirmware/src -o build/emu/native_check web/emu/native_check.c -lm
build/emu/native_check build/emu/native.f32
node web/emu/emu_test.mjs build/emu/felucca.wasm build/emu/native.f32
```

Serve `firmware/build/emu` on localhost or HTTPS. The simulator is the C firmware in WebAssembly with an AudioWorklet, not a separate JavaScript synthesizer. MIDI output, USB audio, physical NOR timing and on-chip CPU limits are not emulated. It stores virtual projects in browser-local storage. Browser and device projects are not automatically synchronized.

## Native verification

```sh
cd firmware
python3 tests/run_bunker_checks.py
mkdir -p build/host
cc -O1 -w -ffp-contract=off -Ibuild/gen -Ifirmware/src -o build/host/bunker_runtime tests/bunker_runtime.c -lm
build/host/bunker_runtime smoke
cc -O1 -w -ffp-contract=off -Ibuild/gen -Ifirmware/src -o build/host/bunker_grow_test tests/bunker_grow_test.c -lm
build/host/bunker_grow_test
build/host/bunker_runtime stress 3600
python3 tests/bunker_audio_bank.py
```

The audio bank script renders all 64 sounds and 24 worlds, checks audible output and distinct PCM hashes, voice count and sample range. It is an audibility/regression check, not an objective measure of artistic quality. `tests/run_bunker_checks.py` is the precise 19-suite release subset; do not conflate it with every optional upstream test. The original full test runner remains available.

Root `tests/installer.test.mjs` exercises the JavaScript updater with simulated MIDI. `tests/installer-ui.test.cjs` exercises installer UI state using LinkeDOM. These do not communicate with a device. See the report for sanitizer flags and exclusions.

## Installation and storage

The live HTTPS installer in `dist/install/` uses Web MIDI SysEx and user initiation. It verifies package size, SHA-256 and identity before writing. It supports the running FM-1 application and the compatible MIDI OTA loader, checks the final returned identity, and provides AMB-1 1.2 and a locally selected, hash-checked stock V15 package as rollback targets. An intermediate reset to AMB-1 is not required for compatible AMB/Wolf/Felucca update implementations. Owner-reported RC2 operation has passed. The owner completed the normal original-firmware round trip; physical music-backup/restore remains a separate acceptance check.

AMB/Wolf musical projects are not migrated. Bunker uses four Felucca project slots and the existing autosave mechanisms, which may replace old data. Its 114-parameter projects are not accepted by Felucca 1.6.1. Firmware rollback is not a backup. Do not erase protected boot/calibration sectors. UBOOT is distinct from the MIDI OTA loader and may require manufacturer tools; there is no unconditional Chrome recovery promise.

RC2 preserves the exact RC1 project layout (114 parameters). The tested RC1 package and its complete source remain downloadable, and the installer has an explicit RC1 rollback target. The loader model passed both directions.

GROW: K1 root, K2 scale, K3 TUNE/SOFT/FLOW/NEW, K4 selected track or all. OCT+ applies, OCT- restores one previous GROW step if no newer phrase edit invalidated it. Target phrases are locked for audition. Drums and slices are skipped. Pending choices and undo history are not serialized; the resulting notes and scales are. The new renderer uses integer geometry, the existing canvas, and at most ~8 Hz. ANIM OFF disables decorative motion.

The English user-facing guide is `dist/manual.html`. On the new HOME page SELECT/HOME chooses WORLD/GROW/SOUND/MOTIF/EVOLVE/MIX; ALGORITHM selects track; KNOB 1–4 matches the display; WORLD/MOTIF are applied by OCT+; EVOLVE OCT− locks the phrase. Existing EDIT, ENV, LFO, FX, SCL, ARP, SEQ and SAVE pages operate on those same tracks.

## Distribution layout

- `firmware/`: complete fork and assets; generated build output is excluded from source distribution.
- `dist/`: static site sources, guide, installer and simulator source-facing files. Live deployment also contains binaries and audio previews.
- `docs/test-results/`: recorded final checks and manifest.
- `docs/research/`: documented research snapshots.

The package checksum in `dist/install/manifest.json` identifies the exact device image tested and published. Compiler/SDK binaries and third-party tool installations are not copied into the source archive.

## English release website and film

Live site: https://bunker-evolved.sugarypixie10.chatgpt.site

The `dist/` website has the installer, recovery guide, tutorial, browser instrument and an English narrated film. The host-only `firmware/tests/bunker_video.c` captures actual firmware UI and DSP; `tools/make_video.py` composes the film with supplied narration. Scripts and voice assets are included; generated movie, firmware, WASM and listening previews are distributed separately. The source archive excludes those generated binaries.

The new installer provides complete Bunker music backup and restore through the existing Felucca archive protocol. Only Bunker RC1/RC2 identities are accepted for music transfers. Explicit consent, complete archive validation and an exclusive tab lock precede restore. This does not export original/AMB projects.

The main firmware, browser code and generators are GPL-3.0-only with original component notices retained. The release film and narration are supplied for this project's publication, separately from the software license. `docs/video/` has the narration, chapters, upload description and media provenance.

Published on 17 October 2026:

- English film: https://www.youtube.com/watch?v=NJ5UFi62s_0
- GitHub release: https://github.com/kidbee85-beep/bunker-evolved/releases/tag/v1.0.0
- Website: https://bunker-evolved.sugarypixie10.chatgpt.site

On 18 October 2026 the complete corresponding source from the published `Bunker-Evolved-Source.zip` was added to `main`, including the vendored engine files. Clone this repository or download the archive of the source-publication commit to obtain the full tree. The original source ZIP remains available as a release asset. The existing `v1.0.0` tag was preserved: its automatically generated GitHub source archives still refer to the original README-only commit, so use the source-publication commit or the attached full source ZIP instead. This source publication does not change or rebuild the released firmware. The old German AMB-1 Evolved video belongs to a separate project and was not modified.

## Chrome editor and feature provenance

[Editor](https://bunker-evolved.sugarypixie10.chatgpt.site/editor/) · [Comparison with Felucca and other firmware](https://bunker-evolved.sugarypixie10.chatgpt.site/comparison.html)

Editor 0.1 is in `dist/editor/`, with independent music/protocol/device modules and an interchangeable MIDI adapter. See `docs/release/editor-architecture.md` and `feature-provenance.md`. The editor has actual C-handler host tests and DOM interaction tests; physical USB acceptance remains pending. Native apps and plug-ins are future work. The firmware binary remains unchanged.
