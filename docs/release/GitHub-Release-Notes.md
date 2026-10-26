# Bunker Evolved 1.0

Free generative ambient firmware for the M-VAVE FM-1. Four editable layers, 64 sound recipes, 24 worlds, 24 motifs, 32 scales, GROW with undo, seven EVOLVE processes and thirteen synthesis engines.

- [Website, tutorial and installer](https://bunker-evolved.sugarypixie10.chatgpt.site)
- [Chrome editor](https://bunker-evolved.sugarypixie10.chatgpt.site/editor/)
- [Feature comparison and provenance](https://bunker-evolved.sugarypixie10.chatgpt.site/comparison.html)
- [New English introduction](https://www.youtube.com/watch?v=NJ5UFi62s_0)
- [Recovery guide](https://bunker-evolved.sugarypixie10.chatgpt.site/rescue.html)

## Release approval

On 17 October 2026, the project owner completed Bunker → original FM-1 firmware → Bunker → original, and confirmed working audio and controls. Release 1.0 promotes that exact tested firmware without recompiling it. Its internal label remains `BE 1.0-RC2` and its device identity is `FM-1_7100002`. Existing users of this build do not need to flash again.

This is owner acceptance on one FM-1. It does not establish power-loss recovery or compatibility with every board revision. The exact stock version string was not separately recorded; Original V15 remains the documented installer baseline.

## Downloads

- `bunker-evolved-rc2.fwsc` — the unchanged, hardware-tested Release 1.0 firmware.
- `Bunker-Evolved-Source.zip` — complete corresponding source, assets, build instructions, tests, licenses and updated documentation. **Use this attached archive. GitHub's automatically generated source archives do not contain the full source tree.**
- `Bunker-Evolved-Publication-Kit.zip` — firmware, source, English film, captions, thumbnail and release records.
- `Bunker-Evolved-Introduction.mp4` — the English walkthrough; actual C-rendered firmware screens and audio, an illustrated panel and calm AI narration.
- `hardware-acceptance.md` and `test-status.txt` — hardware report and automated test evidence.
- `SHA256SUMS.txt` — download integrity checks.

Firmware: 610,090 bytes.
SHA-256: `db0b50d0ae9c3bfccee76e3b93de09fffa55389169278a0c5899dad9c79e6bb8`.

## Installation and recovery

Install directly from compatible original FM-1 firmware; AMB-1 is not an intermediate requirement. Back up your music before switching. Other firmware project formats are not converted, and firmware rollback does not restore overwritten music. UBOOT is a separate chip mode, not guaranteed Chrome rescue. Keep power and USB connected until completion.

The unchanged firmware retains the recorded 19-suite regression, GROW, audio/WASM, loader-model and simulated stress results. These are separate from the owner's hardware report. The release website includes an English guide and music backup/restore for compatible Bunker builds; physical music-backup/restore and interrupted-power recovery remain outside this hardware acceptance report.

Based on Felucca 1.6.1 by Leo Kuroshita / Hügelton Instruments and contributors. GPL-3.0-only with retained MIT / Apache / CC0 / font notices. Complete credits are in the attached source archive.

## New website component: Editor 0.1

A modular Chrome editor adds four-track sound/parameter editing, 64 Bunker recipes, factory presets, 24 motifs, 32 scales, selected-track GROW, detailed note editing, phrase import/export and guarded phrase undo. Firmware 1.0 is unchanged.

49,337 host assertions include real requests to the C firmware handler, native parity for every sound recipe and motif, write/readback, stale undo and timeout handling. DOM interaction checks also pass. This is not a completed physical-USB or browser-layout test. The offline sketchpad is silent. Native iOS/iPadOS/macOS apps and VST/AU wrappers are future work; the modules and architecture are included in the source.
