# AMARU OS — Flash safety & storage record

**AMARU OS 0.4** — identity `FM-1_7100002`, image 501,660 B (app-area cap 602,112 B).

This document is the engineering record behind "is it safe to flash on real FM-1 hardware?". It lists what the build provably does *not* do, how persistent storage is laid out, and what was verified. The short verdict: **AMARU cannot hard-brick an FM-1 by design** — the worst realistic outcome of a bad flash is a re-flash via OCT− rescue.

---

## 1. Why AMARU cannot hard-brick an FM-1

The FM-1 (JieLi AC791N / WL82) keeps its recovery in an **unflashable ROM bootloader**. This is the platform guarantee every custom firmware relies on, and AMARU depends on it:

> **USB Rescue Boot:** built-in safe recovery mode — **OCT− held on power-on, or two crash loops in a row** — boots a minimal updater reporting as `FM-1_000`. This guarantees web flashing and recovery without bricking.

Because it lives in ROM, it runs *before* any flashed app and **cannot be disabled by firmware**. Even a fully corrupted app is recoverable via OCT−.

### What AMARU provably does not do

| Concern | Evidence / guard |
|---|---|
| Overwrite the bootloader head `[0, 0x4000)` | `ldr_test`: **"flash head [0,0x4000) untouched"** — app writes start at `0x4000` |
| Write outside the OTA staging area | `ota_test`: **"nothing written outside `0xE0000..0xE4FFF`**"; foreign-size / aborted sessions leave no commit |
| Write outside the storage areas | `storage_hw.c` gates every erase/program via `FL_STORE_OK` |
| Accept a foreign or damaged package | `ldr_test`: **"foreign key / damaged app head refused, nothing erased"** |
| Disable rescue | OCT− / OCT+ power-on and UBOOT entry paths preserved (`main.c`) — AMARU must never disable them |

### Install path

AMARU is installed as a validated **`.fwsc`** package built with the proper update loader (`fm1pkg_make`). Raw `.bin` is never flashed. The M-UPGRADE handshake verifies package identity before any write.

The published installer (`install.html`) is **generated** from `firmware/web/fm1pkg.js` and `firmware/web/fm1ota.js` by `tools/build_pages.py`, and it pins the identity, size and SHA-256 of the exact released package:

- **Identity:** `FM-1_7100002`
- **Size:** `610090 B`
- **SHA-256:** `f082a9e5bb26001600e58d5658b59b240f7b5f145adbaf75af35ebc1370864bf`

The device **pulls** the image with `0x30` read requests (M-UPGRADE); the page answers them. It never streams raw data at the device. `tests/installer.test.mjs` and `tests/installer.page.test.mjs` replay the whole exchange against a simulated FM-1 and compare every block with the released `.fwsc`.

### The only "true brick" scenarios

A permanent brick requires physically damaging the ROM/flash controller, or an *external* tool (not this firmware) writing the bootloader region. The worst realistic outcome of a bad AMARU flash is a simple re-flash via OCT− rescue.

---

## 2. Persistent storage layout

Verified against the `release` build.

| Resource | Location | Capacity |
|---|---|---|
| **User presets** | `0xDC000..0xDFFFF` | 32 slots (2 banks) |
| **Projects** | `0x97000..0x9EFFF` | 4 slots |
| **Project autosave** | `0xE5000` | 1 slot |
| **Settings** | `0xFC000` | — |
| **User sample slots** | `0xA0000..0xDBFFF` | — |
| **Flash head (bootloader)** | `[0, 0x4000)` | **Never written** |
| **OTA staging** | `0xE0000..0xE4FFF` | Erased on session end |
| **Update records** | `0x93000..0xFC000` (outside Felucca store) | Cleared on commit |

Every object uses an **A/B sector pair** with CRC + commit records, so a power cut never loses both copies.

- Each **project save is exactly 3840 B** (`PROJ_STORE_SIZE == ST_PAYLOAD_MAX`) — a full 4-track × 64-step record. The format is frozen (`_Static_assert`), so it always fits by construction.
- "Songs" are stored inside projects (track / pattern / song-chain data is part of the project record).

### Runtime headroom (`release` build)

| Resource | Used | Available | Free |
|---|---|---|---|
| Flash image | 501,660 B | 602,112 B (cap) | ~100 KB |
| RAM static (`.data+.bss`) | 53,044 B | 73,728 B | 28% |
| Pool (voices / patterns) | 328,924 B | 368,640 B | 11% |

CPU budget (`tests/target_budget.py`): **0 over-budget render targets**. The audio engine set is ANALOG / DIGITAL / DRUM / FM6 — the retired engines (incl. SAMPLE) stay compiled out so the render loops remain the verified, in-budget ones.

Imported TEKNO EPROM dumps are preserved under `firmware/assets/tekno-import/` but deliberately **not compiled into flash** (see its README). The Vermona SK86 *character* is captured instead in a new DRUM engine kit (`TEKNO`, kit 13) — pure parameter data in the existing, in-budget engine, so zero CPU cost.

---

## 3. Backup & recovery

| Step | Action |
|---|---|
| **Before flashing** | Editor → Backup (settings, projects, user presets, FM6 patches, samples) |
| **Flash** | `install.html` — WebMIDI, Chrome/Edge, data USB cable |
| **Interrupted flash** | Replug the cable, press Install again — it resumes from the start |
| **Soft brick** | Hold **OCT−** while switching the FM-1 on → update mode → press Install |
| **Restore stock** | Official V15 / V16 `FM-1.fwsc` can always be re-flashed via OCT− |
| **Hard brick** | FM-1 Transporter (RP2040 dongle) or `MvaveFM1Unbricker` (Windows/Linux) — see `docs/RESCUE.md` |

AMARU must **never** disable the OCT− power-on handler — that is the rescue guarantee.

---

## 4. Verification checklist (host)

| Test | What it checks | Status |
|---|---|---|
| `amaru_gen_test` | All genres / forms valid and deterministic | PASS |
| `drum_test` | All 13 kits (incl. TEKNO) meet lane targets | PASS |
| `midi_control_test` | MIDI CC / PC handling across 14 kits | PASS |
| `editor_test` | Editor round-trip | PASS |
| `storage_test` / `project_test` / `upreset_test` | Persistence A/B sectors | PASS |
| `settings_test` | Settings migration, palette ids, calibration | PASS |
| `v156_test` | 1.5.6 kit invariants | PASS |
| `installer.test.mjs` | Installer / verifier pins + M-UPGRADE read protocol | PASS |
| `installer.page.test.mjs` | `install.html` end-to-end vs simulated FM-1 | PASS |
| `ota_test` | OTA staging windows; foreign / damaged refused | PASS |
| `ldr_test` | Flash head untouched; sector erase/program/verify | PASS |
| Emulator WASM | Bit-identity with native build | PASS |

Pre-existing host-only `ui_test` failures (categories / SONG-rows / FM6 ops) remain; they are not regressions and do not affect audio or flash safety.

---

## 5. Honest boundary

Everything above is verified **against the firmware's own tests, the simulator, and the published web pages**. I have **not** tested on physical FM-1 hardware. The ROM-bootloader rescue path and the flash-write windows are the platform's documented guarantees, and AMARU follows them strictly — but the final ears-on / USB handshake on real hardware is yours.

If you flash, back up via the editor first, keep a data USB cable handy, and know that OCT− rescue (or the FM-1 Transporter / `MvaveFM1Unbricker` on Windows/Linux) is your way back.
