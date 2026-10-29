# AMARU OS

**Hard-tekno generative firmware for the M-VAVE FM-1.** Four-track song generator, Bunker HOME identity, and a USB installer that never touches the boot block.

<p align="center">
  <img src="firmware/web/emu/logos/logo-splash.png" alt="AMARU OS splash — monochromatic Bunker HOME identity with canary-yellow Mesoamerican accents">
</p>

[![License: GPL-3.0-only](https://img.shields.io/badge/license-GPL--3.0--only-blue.svg)](LICENSE)
[![Emulator](https://img.shields.io/badge/try%20it-in%20the%20browser-ff7a00.svg)](https://marcocasanova00.github.io/amaru-os/)

AMARU OS turns your FM-1 into a live hard-tekno groove box. It is a **deterministic, seed-driven four-track generative engine** — pick a genre, dial a SEED, and the GEN page renders a complete 64-step song across AIR (pad/chorus), MELODY (lead), ROOT (bass), and SPARK (drums). Same seed, same settings, same song, every time.

Built on Bunker Evolved 1.0. Licensed GPL-3.0-only.

<p align="center">
  <a href="https://marcocasanova00.github.io/amaru-os/"><strong>Play in the browser →</strong></a> &nbsp;|&nbsp;
  <a href="https://marcocasanova00.github.io/amaru-os/install.html"><strong>Install to FM-1 →</strong></a> &nbsp;|&nbsp;
  <a href="https://marcocasanova00.github.io/amaru-os/editor/"><strong>Song editor →</strong></a> &nbsp;|&nbsp;
  <a href="https://marcocasanova00.github.io/amaru-os/verify.html"><strong>Verify package →</strong></a>
</p>

---

## The sound package

**Drums (SPARK track)** — thirteen kits spanning hard and classic electronic percussion:

- **TEKNO** — dry, percussive, brutal: hard kick, cracking snare, tight clap, metallic hats, dry tom, hard rim, short bell
- **GABBER** — heavy saturated kick, cracking noisy snare, metal hats
- **HARDTEKK / HITECH** — short driven kick, cluster hats
- **DEEP / TIGHT / PERC** — model-kit percussion from the Felucca pieces
- **80 / 10 / 66 / 55 / 77** — the classic box kits
- **PSY / FOREST** — AMARU-appended model kits

**808 presets** — `808 BASS BOOM`, `808 BASS DIRTY`, `808 BASS SLIDE`.

**Synthesis engines** (four active): **ANALOG**, **DIGITAL** (FM4-style), **FM6**, and **DRUM**. Retired engines stay compiled out so the render loops stay tight.

**Genres** — tribe tekno, mental tekno, hardtekk, acidcore, jungle, drum & bass, breakbeat tekno, techno, psytrance. DnB and breakbeats are not four-on-the-floor.

**32 hard-tekno worlds** — curated at the GEN recipe layer, replacing the 112 ambient worlds.

---

## GEN page — one seed, one song

| Knob | Role |
|---|---|
| **GENRE** | tribe / mental / hardtekk / acid core / jungle / drum&bass / breakbeat / techno / psytrance |
| **SEED** | any number 0–9999 — a stable, repeatable song |
| **ENERGY** | 0–100 — full adds extra hats and ghosts, empty strips them back |
| **FORM** | `LOOP` (1 bar) / `PARTS` (2 bars) / `SONG` (4 bars) |

**OCT+** advances the seed and plays a new take. **OCT-** resets to the base value. **PLAY** runs or stops the sequencer. **REC** records a take.

The track sound name shows the recipe's source at a glance: `MYC PUNCHY`, `MYC BLOWN`, `GREEN KIT`, `TEKNO KIT`, `HYPOGEUM CHANT`, `ORACLE CHOIR`, `SILBURY WAIL`, and more.

---

## Installing (at your own risk)

1. **Coming from another custom firmware (Felucca, Baud Girl, or any other)? Return to official V15 first**, then install AMARU. Writing straight over another custom firmware has bricked an FM-1.
2. Open **[install.html](https://marcocasanova00.github.io/amaru-os/install.html)** in Chrome or Edge, with the FM-1 connected directly by a **data** USB cable. Close other MIDI apps.
3. Press **Install**, allow MIDI (SysEx), and wait for **Done** (about a minute). The status stays armed the whole write and disarms only at Done.
4. **Do not unplug while writing.** If interrupted, replug and press Install again — it resumes from the start.

The installer checks the FM-1 first and writes only over official V15 or AMARU. It offers the **approved** version only: every change is checked in the [emulator](https://marcocasanova00.github.io/amaru-os/) first, and only then reaches the installer.

Offline: `python3 tools/fm1_install.py build/felucca.fwsc`.

**Recovery:** hold **OCT−** while switching the FM-1 on — the device enters update mode and listens for rescue. See [`docs/RESCUE.md`](docs/RESCUE.md) and [`docs/SAFETY.md`](docs/SAFETY.md). AMARU must **never** disable this handler.

---

## Changelog

### 0.4
- TEKNO drum kit and TEKNO KIT preset added
- Monochrome Bunker HOME rework — canary accents in the bottom corners, splash untouched
- 32 hard-tekno worlds (replacing 112 ambient), curated at the GEN recipe layer
- 808 presets renamed `808 BASS BOOM / DIRTY / SLIDE`
- Flash-fit verified; CPU budget clean; brick-safety gates re-verified (`ota_test`, `ldr_test`)
- Song editor librarian verified on FM-1 hardware: backup export/restore round-trip (`BACKUP_LIST`/`GET`/`PUT`)

### Earlier
- Built on Bunker Evolved 1.0; genre concept from JIANT; timbral shapes from Carnac OS / Mycelium FM-1 (numbers only)

---

## About

AMARU OS is an alternative firmware for the M-VAVE FM-1, made for playing live and building whole songs without a computer.

- **Emulator**: https://marcocasanova00.github.io/amaru-os/
- **Installer**: https://marcocasanova00.github.io/amaru-os/install.html
- **Editor**: https://marcocasanova00.github.io/amaru-os/editor/
- **Verifier**: https://marcocasanova00.github.io/amaru-os/verify.html
- **Flash safety**: `docs/SAFETY.md`
- **Rescue**: `docs/RESCUE.md`
- **Releases**: https://github.com/MarcoCasanova00/amaru-os/releases

AMARU OS is an independent community firmware. It is not affiliated with or endorsed by M-VAVE, Bunker Evolved, JIANT, Carnac OS, or Mycelium FM-1. Installing custom firmware is at the user's risk; keep the stock image and a verified recovery path.

See [`docs/ATTRIBUTION.md`](docs/ATTRIBUTION.md) and `LICENSE` for full attribution.
