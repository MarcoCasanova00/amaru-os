# AMARU OS 0.4

**Hard-tekno generative firmware for the M-VAVE FM-1.** Four-track song generator, monochrome Bunker HOME identity with Mesoamerican accents, and a hardened USB installer that never touches the boot block.

<p align="center">
  <img src="firmware/web/emu/logos/logo-splash.png" alt="AMARU OS splash — canary-yellow suns, temple-step marks, green monochrome garden">
</p>

**AMARU OS** is an open-source, deterministic generative firmware for the M-VAVE FM-1 synthesiser (JieLi AC791N). It turns the FM-1 into a live hard-tekno groove box: four tracks (AIR / MELODY / ROOT / SPARK), nine first-class genres (tribe tekno, mental tekno, hardtekk, acidcore, jungle, drum & bass, breakbeat tekno, techno, psytrance), and a GEN page that renders full 64-step songs from one SEED value.

Built on Bunker Evolved 1.0. Licensed GPL-3.0-only.

<p align="center">
  <a href="https://marcocasanova00.github.io/amaru-os/"><strong>Play in the browser →</strong></a> &nbsp;|&nbsp;
  <a href="https://marcocasanova00.github.io/amaru-os/install.html"><strong>Install to FM-1 →</strong></a> &nbsp;|&nbsp;
  <a href="https://marcocasanova00.github.io/amaru-os/editor/"><strong>Song editor →</strong></a> &nbsp;|&nbsp;
  <a href="https://marcocasanova00.github.io/amaru-os/verify.html"><strong>Verify package →</strong></a>
</p>

---

## Specifications

| Category | Detail |
|---|---|
| **Platform** | M-VAVE FM-1 (JieLi AC791N / WL82) |
| **Version** | AMARU OS 0.4 — identity `FM-1_7100002` |
| **Tracks** | 4: AIR (pad/chorus), MELODY (lead), ROOT (bass), SPARK (drums) |
| **Genres** | tribe tekno, mental tekno, hardtekk, acidcore, jungle, drum & bass, breakbeat tekno, techno, psytrance |
| **DRUM kit** | TEKNO (kit 13): Vermona SK86 character — dry, percussive, brutal; hard kick, cracking snare, tight clap, metallic hats, dry tom, hard rim, short bell |
| **Synthesis engines** | ANALOG, DIGITAL (FM4-style), FM6, DRUM — four active engines; retired engines stay compiled out |
| **Display** | Strictly monochrome: GRN/BLK, WHT/BLK, AMB/BLK palettes + canary-yellow Bunker HOME garden accents (bottom corners, clear of top-band text) |
| **Splash screen** | Unchanged AMARU original 96×96 monochrome splash |
| **Flash image** | 501,660 B (app-area cap 602,112 B — ~100 KB free) |
| **CPU budget** | 0 over-budget render targets (verified) |
| **RAM static** | 53,044 B / 73,728 B |
| **Pool (voices/patterns)** | 328,924 B / 368,640 B |
| **User presets** | 32 slots (2 banks) |
| **Projects** | 4 slots + autosave |
| **License** | GPL-3.0-only |

---

## What makes AMARU different

**Deterministic generative engine (GEN page).**  
One SEED (0–9999) + GENRE + ENERGY + FORM produces an exact, repeatable 64-step, four-bar song. OCT+ advances the seed and plays a new take; OCT- resets to the base value. Same seed, same settings — same song, every time.

**TEKNO DRUM kit, in-budget.**  
The Vermona SK86 EPROM character (hard kick, cracking snare, tight clap, metallic hats, dry tom, hard rim, short bell) is captured as a pure-parameter DRUM engine kit — zero new render code, zero CPU cost. No samples compiled into flash; the raw dumps stay preserved under `firmware/assets/tekno-import/`.

**Monochrome Bunker HOME identity.**  
Green/black terminal garden with two canary-yellow suns and temple-step marks in the bottom corners. The accents never overlap the world names; the splash screen is untouched. Monochrome palette break is allowed *only* for these marks.

**No functionality removed.**  
The 112 ambient worlds were replaced by 32 hard-tekno worlds at the GEN recipe layer, but nothing was taken away — the four engines, the four palettes, the GROW/EVOLVE workflow, and the Bunker HOME identity are all preserved and extended.

**Flash safety by design.**  
- The FM-1's recovery lives in an **unflashable ROM bootloader**: OCT− held on power-on (or two crash loops) boots a minimal updater reporting as `FM-1_000`. It runs before any flashed app and cannot be disabled by firmware.
- AMARU writes **only** the app area `[0x4000, 0x93000)`. The bootloader head `[0, 0x4000)` is provably untouched (`ldr_test`).
- Packages are validated by **identity + size + SHA-256** before any transfer. Foreign or damaged packages are refused before any write.
- The device **pulls** the image with `0x30` read requests (M-UPGRADE); the host never pushes raw data. Nothing is committed until the device confirms `success`.
- Every flash sector is erased, programmed, read back, and compared (retry up to 2×) before the next sector.

**Zero-cost optimization.**  
The build fits with margin via engine retirement + `gc-sections` — no risky trimming. The retired SAMPLE engine stays compiled out specifically to keep the CPU budget clean.

---

## Try it now (no download)

Open [https://marcocasanova00.github.io/amaru-os/](https://marcocasanova00.github.io/amaru-os/) in Chrome or Edge, click **Einschalten**, and connect the FM-1 over a data USB cable for the live version. No driver install.

**Web ecosystem** — all cross-linked, all showing the AMARU logo:

| Page | Purpose |
|---|---|
| `/` | Emulator — full FM-1 simulation in the browser |
| `/install.html` | Installer — WebMIDI USB flashing with progress bar and "DO NOT UNPLUG" guard |
| `/editor/` | Song editor — create/modify songs, JSON import/export, backup/restore |
| `/verify.html` | Package verifier — identity/size/SHA-256 check only, no flash actions |

---

## Install to FM-1

1. **Back up your music first** — editor → Backup (settings, projects, presets, FM6 patches, samples).
2. Open **install.html** in Chrome/Edge. Hard-reload so it picks up the pinned package (`FM-1_7100002`, SHA-256 `f082a9e5…`).
3. Connect the FM-1 directly by a **data** USB cable. Close other MIDI apps.
4. Press **Install**, allow MIDI (SysEx), and wait for **Done** (about a minute). The status stays armed/pulsing the whole write and disarms only at Done.
5. **Do not unplug while writing.** If interrupted, replug and press Install again — it resumes from the start.

**Recovery:** hold **OCT−** while switching the FM-1 on → update mode → press Install again. See [`docs/RESCUE.md`](docs/RESCUE.md) and [`docs/SAFETY.md`](docs/SAFETY.md) for the full flash-safety record.

---

## What's new in 0.4

- **TEKNO DRUM kit** (kit 13) — Vermona SK86 character, pure parameter data, in-budget
- **TEKNO KIT** factory preset + `TEKNO KIT` genre recipe
- Monochrome Bunker HOME rework: canary-yellow suns + temple-step marks (bottom corners)
- 32 hard-tekno worlds (replacing 112 ambient), curated at the GEN recipe layer
- 808 presets renamed `808 BASS BOOM / DIRTY / SLIDE`
- Flash-fit verified (501,660 B), CPU budget clean (0 over-budget targets)
- Brick-safety gates re-verified: `ota_test` PASS, `ldr_test` PASS, head `[0,0x4000)` untouched

---

## Credits

- **AMARU OS** — Marco Casanova. Original artwork (logos, splash, 16×16 AMARU icon set).
- **Bunker Evolved 1.0** — Hügelton Instruments (GPL-3.0-only): foundation, GROW/EVOLVE workflow, four-track architecture.
- **JIANT** (GPL-3.0-only): genre concept and generator grammar rules.
- **Carnac OS / Mycelium FM‑1** (GPL‑3.0): timbral parameter shapes — numbers only, no binaries imported.
- **Felucca** (MIT): WebMIDI M-UPGRADE installer protocol (`fm1pkg.js`, `fm1ota.js`).
- **M-VAVE** and **FM-1** are trademarks of their respective owners.

See [`docs/ATTRIBUTION.md`](docs/ATTRIBUTION.md) and `LICENSE` for full attribution.

---

*AMARU OS 0.4 — GPL-3.0-only. Flash safety record: `docs/SAFETY.md`. Hardware rescue: `docs/RESCUE.md`.*
