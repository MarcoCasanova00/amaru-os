# Loaf-Breaker Approach: Using IMA ADPCM Samples from Flash

## Context

The FLEXOR/R **loaf-breaker** project already ships a drum-kit import page (`import.html`) that reads **IMA ADPCM samples** from flash:

- Kit object lives in two A/B sectors (`0xDE000` = 8 KB, no Bluetooth).
- Data is IMA ADPCM read through `perc_set_sample()` / `sample_tick()` in `perc.c`.
- Each kit has three fields: snare, hi-hat, clap (OBJ_BLE objects), plus IMA ADPCM channels.
- The site exposes an **IMPORT tab** (`app/site/import.html`) driven by `kit_scan()`, `kit_ent()`, `kit_name()`, `kit_has()`.

We want to bring this pipeline into AMARU OS so SPARK track drums can use real samples instead of synthesis recipes.

## Why ADPCM?

AMARU OS deliberately keeps the **SAMPLE engine compiled out** (retired) to stay inside verified flash/CPU budgets. Instead we can reuse the **existing IMA ADPCM decode path** that the loaf-breaker uses, which gives us native 16-bit PCM on the fly without recompiling a new engine.

## What we need

1. **Decode IMA ADPCM** from flash (raw bytes at `0xDE000..0xDEFFF`, ~8 KB per kit).  
2. Map each kit's three fields (snare/hi-hat/clap) to decoded PCM values.  
3. Store decoded PCM in RAM or user presets (A/B sectors).  
4. Render via the existing DRUM engine (`eng_drum.c`) or FM6.  
5. Keep OTA window & foreign-package refusal intact.

## Existing pieces to reuse

| Piece | Location | Purpose |
|---|---|---|
| `perc_set_sample()` / `sample_tick()` | `perc.c` | Reads IMA ADPCM samples during playback. |
| `adpcm_at()` / `adpcm_read()` | `adcpm.c` | Decode IMA ADPCM → 16-bit PCM. |
| `kit_scan()`, `kit_ent()`, `kit_name()`, `kit_has()` | `app/site/import.html` / `tools/kit_frames.py` | Scans kit objects in store. |
| `OBJ_BLE` (no BT) | `app.h` / `kit_frames.py` | Confirms no Bluetooth writes. |

## Design notes (from CLAUDE.md)

- No double precision anywhere; 32-bit math only.
- Flash image cap 602,112 B; current AMARU image 501,628 B fits.
- CPU budget: render runs in audio interrupt, single-word setter calls.
- OTA handshake verifies identity/size/SHA-256 before any write.
- Kit layout: two A/B sectors (`0xDE000`, 8 KB); no external write.
- Import page uses SysEx zone 2 (`kit_sysex`), INFO/BEGIN/DATA/COMMIT/READ/DELETE.

## Proposed integration

### Step 1: Extract `.all` drum samples (optional)
If you have Electribe `.all` files containing drum PCM, run them through a **drum-machine extractor** (e.g., `smpldsnds/drum-machines` or `electribe-mockup`) to get raw WAV/PCM. This step is optional — the loaf-breaker uses IMA ADPCM directly.

### Step 2: Build a minimal IMA ADPCM decoder
Create a self-contained prototype `docs/research/adpcm_decode.c` that:
- Takes a raw byte array (flash sector `0xDE000..0xDEFFF`).
- Decodes IMA ADPCM (bit depth 10, sample rate 11025 Hz, 16-bit) → 16-bit PCM output.
- Returns a `uint16_t*` buffer sized to expected kit size (13 kits × 3 fields ≈ 39 samples).

### Step 3: Wire into the SPARK track
- In `eng_drum.c` / `eng_sample.c`, after reading a drum event, call the new decoder to fill the drum's PCM slots.
- Use the same A/B sector layout as loaf-breaker (`0xDE000` head, `0xDE010` tail, etc.).

### Step 4: Persist decoded PCM
- Write decoded PCM into user preset banks (`0xA0000..0xDBFFF`) or project records.
- Verify total stored bytes ≤ 602,112 B (current usage ~573 MB... wait, 610090 B total package; drums ~? keep within cap).

### Step 5: Emulator support
- Add a "Drum" sound source in the emulator WASM that plays the decoded PCM via Web Audio (WebMIDI fallback not used here).

## Files to create (branch `drum-samples-research`)

- `docs/research/drum-samples/loaf-breaker-adpcm-reference.md`
- `docs/research/drum-samples/adpcm_decode.c`
- `docs/research/drum-samples/README.md`

No changes to the verified build tree yet — all new files go under `docs/research/` and are excluded from the main `build.sh`/`fm1pkg_make.py` paths.

## Verification plan
- Run `tests/engine/run.sh` (build with `-Wdouble-promotion -Werror`) to confirm no double-precision.
- Run `tests/installer.test.mjs` against a simulated FM-1 (already exists) but point it at the new decoder path.
- Compare final image size vs 602,112 B cap.
- Ensure flash head `[0,0x4000)` untouched and OTA windows unchanged.

---
Created 2026-11-18
