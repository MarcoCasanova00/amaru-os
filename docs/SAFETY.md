# AMARU OS — Flash safety & storage

This document records why AMARU OS is safe to flash on a real M-VAVE FM-1,
how its persistent storage is laid out, and what was verified. It is the
engineering record behind the "can it brick?" question.

---

## 1. Why AMARU cannot hard-brick an FM-1

The FM-1 (JieLi AC791N) keeps its recovery in an **unflashable ROM bootloader**.
This is the platform guarantee every custom firmware relies on:

> **USB Rescue Boot:** built-in safe recovery mode — **OCT− held on power-on, or
> two crash loops in a row** — boots a minimal updater reporting as `FM-1_000`.
> This guarantees web flashing and recovery without bricking.

Because it lives in ROM, it runs *before* any flashed app and cannot be disabled
by firmware. Even a fully corrupted app is recoverable via OCT−.

### What our firmware provably does not do

| Concern | Evidence / guard |
|---|---|
| Overwrite the bootloader head `[0, 0x4000)` | `ldr_test`: **"flash head [0,0x4000) untouched"** |
| Write outside the OTA staging | `ota_test`: **"nothing written outside 0xE0000..0xE4FFF"**; foreign-size / aborted sessions leave no commit |
| Write outside the storage areas | `storage_hw.c` gates every erase/program via `FL_STORE_OK` |
| Accept a foreign/damaged package | `ldr_test`: **"foreign key / damaged app head refused, nothing erased"** |
| Disable rescue | the OCT−/OCT+ power-on and UBOOT entry paths are preserved (`main.c`) |

### Install path

AMARU is installed as a validated **`.fwsc`** package built with the proper update
loader (`fm1pkg_make`). Raw `.bin` is never flashed. The M-UPGRADE handshake
verifies identity before any write.

The published installer (`install.html`) is generated from `firmware/web/fm1pkg.js`
and `firmware/web/fm1ota.js` by `tools/build_pages.py`, and it pins the identity,
size and SHA-256 of the exact released package. The device **pulls** the image with
`0x30` read requests; the page answers them. It never streams raw data at the
device, which is what a 0.4 page once did and why a flash could appear to do
nothing while the device waited in update mode. `tests/installer.test.mjs` and
`tests/installer.page.test.mjs` replay the whole exchange against a simulated
FM-1 and compare every block with the released `.fwsc`.

### The only "true brick" scenarios

A permanent brick requires physically damaging the ROM/flash controller, or an
*external* tool (not this firmware) writing the bootloader region. The worst
realistic outcome of a bad flash is a simple re-flash via OCT− rescue.

---

## 2. Persistent storage

Verified against the `release` build (image 501,340 B, flash-fit passes; app-area cap 602,112 B).
The CPU budget (`tests/target_budget.py`) is clean: 0 over-budget render targets. The audio engine
set is ANALOG/DIGITAL/DRUM/FM6 — the retired engines (incl. SAMPLE) stay compiled out, so the
render loops are the verified, in-budget ones. Imported TEKNO EPROM dumps are preserved under
`firmware/assets/tekno-import/` but deliberately not compiled into flash (see its README).

| Resource | Availability |
|---|---|
| **User presets** | `UP_SLOTS = 32` (2 banks, `0xDC000..0xDFFFF`) |
| **Projects** | **4 slots** (`0x97000..0x9EFFF`) + an **autosave** (`0xE5000`) |
| **Settings** | `0xFC000` |
| **User sample slots** | `0xA0000..0xDBFFF` |

Every object uses an **A/B sector pair** with CRC + commit records, so a power
cut never loses both copies.

- Each **project save is exactly 3840 B** (`PROJ_STORE_SIZE == ST_PAYLOAD_MAX`)
  — a full 4‑track × 64-step record. The format is frozen (`_Static_assert`), so
  it always fits by construction, but there is no slack to grow a record.
- "Songs" are stored inside projects (track/pattern/song-chain data is part of
  the project record).

### Runtime headroom (`release` build)

| Resource | Used | Available |
|---|---|---|
| RAM static (`.data+.bss`) | 53,044 B | 73,728 B (28% free) |
| Pool (voices/patterns) | 328,924 B | 368,640 B (11% free) |

---

## 3. Backup & recovery

- **Editor backup/restore:** `BACKUP_GET/PUT` backs up settings, projects, user
  presets, FM6 patches and samples over USB. Do this before flashing.
- **Rescue:** OCT− at power-on → update mode; `fm1_rescue.py` (Linux);
  `MvaveFM1Unbricker` (Windows). Stock V15/V16 can always be restored.
- AMARU must **never** disable the OCT− power-on handler (the rescue guarantee).

---

## 4. Optimization verdict

The firmware already fits with margin; no risky trim is warranted. The only
candidate is the ~126 KB factory sample sets (PIANO/FLUTE/SAX/PERC/TRANH), unused
since SLICE/SAMPLE are retired — but these are kept deliberately (project
direction is to keep/import samples, and they back editor slice features). The
build is already optimized via engine retirement + `gc-sections`.

---

## 5. Verification checklist (host)

- `amaru_gen_test` — all genres/forms valid, deterministic (~PASS)
- `installer.test.mjs` — installer/verifier pins + M-UPGRADE read protocol (~PASS)
- `installer.page.test.mjs` — install.html end to end against a simulated FM-1 (~PASS)
- `ldr_test` — loader update safety (~PASS)
- `ota_test` — OTA staging windows (~PASS)
- `storage_test`, `project_test`, `upreset_test` — persistence (~PASS)
- `fm4_test` — DIGITAL→FM6 conversion (~PASS)
- native emulator builds + renders (705,600 B) and matches the WASM bit-for-bit
