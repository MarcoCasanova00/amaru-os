# AMARU OS / Based on Bunker Advanced

Amaru OS, the revolution of hard genres for live performances

> "It's still a rough and rugged firmware" 

<p align="center">
  <img src="firmware/web/emu/logos/logo-splash.png" alt="AMARU OS splash — green monochrome, your logo and “by Marco Casanova”">
</p>

**AMARU OS** is an open-source custom firmware for the M-VAVE FM-1 synthesiser. It is
a **four-track** generative engine for hard genres (tekno, acid, hardcore, jungle,
drum & bass, psytrance) — in continuous evolution, with wavetable, FM, granular and
analog synthesis available per track.

Built by musicians, for musicians — the pursuit of trap groove, true dark atmosphere
and in-pocket bounces. Tested on real FM-1 hardware. `AMARU OS 0.4` is a green/black
terminal session running on top of AMARU's original 96×96 monochrome splash logo.

---

## At a glance

- **4 tracks**: AIR (pad/chorus), MELODY (lead), ROOT (bass), SPARK (drums)
- **9 first-class genres**: tribe tekno, mental tekno, hardtekk, acidcore, jungle, drum & bass, breakbeat tekno, techno, psytrance (DnB/breakbeats are **not** four-on-the-floor)
- **Deterministic generator (GEN page)**: GROW/EVOLVE-style presses → OCT+ advances SEED and plays a new take; OCT- resets. Full one-bar punch at LOOP, or PARTS or SONG.
- **Synthesis engines** (four active): ANALOG, DIGITAL (FM4-style), FM6 and DRUM; the other engines are retired slots
- **Three strict monochrome palettes**: `GRN/BLK`, `WHT/BLK`, `AMB/BLK` + accent REC LED
- **Timbres**: Native Carnac- / Mycelium-style textures recreated strictly by parameter values (no binaries imported)
- **License**: GPL-3.0-only (derivative of Bunker Evolved 1.0)

## Try it now — no download

Open https://marcocasanova00.github.io/amaru-os/ → click `Einschalten` → connect the FM-1 over USB for the live version. Browser works on Chrome/Edge; no driver install.

## What you'll hear

Two-voice synthesis per track, clocked on the GEN page — `GENRE SEED ENERGY FORM` on
K1–K4. The output pushes dark techno and acidcore energy, and keeps the swing cornerstones
of breaks, jungle and DnB. `ENERGY` thickens hats beyond 60 and ghosts below 34; `FORM`
runs either a loop, structured parts, or a full 64-step song phrase.

The track slots let you slot any of the four engines on any track (`ANALOG → AIR`,
`DRUM → SPARK`, `FM6 → ROOT`, `DIGITAL → MELODY`). End the take with one press if
you need the original.

## Tips

- **OCT+** for a new take from the same parameters.
- **OCT-** resets the seed.
- **Gen bank** info page shows the recipe source: AMARU ECHO, MAYA PLUCK, AMARU BASS,
  GREEN KIT, TUPAC SHADOW, AZTEC SIREN, etc.
- See `docs/QUICKSTART.md` for the full GEN control legend, theme cycling, and
  long-press shortcuts.

## License

GPL-3.0-only (Bunker 1.0, Felucca MIT, sample CC0, fonts OFL). See
`docs/ATTRIBUTION.md` and `LICENSE`.

## Emergency recovery (universal FM-1 rescue)
Hold **OCT−** while switching the FM-1 on — the device enters update mode and listens for rescue. Use `fm1_rescue.py` on Linux or `MvaveFM1Unbricker` on Windows. See [`docs/RESCUE.md`](docs/RESCUE.md). AMARU must **never** disable this handler.

- Built on **Bunker Evolved 1.0** (Hügelton Instruments) — GPL-3.0-only
- Genre concept taken from **JIANT** (GPL-3.0-only); the generator page/grammar
  took practical rules/structure from JIANT
- Timbral parameter shapes adapted from **Carnac OS** (GPL‑3.0) and **Mycelium FM‑1** (GPL-3.0-only) — numbers only
- All artwork in this repo is original by the maintainer (logo splash, monotone
  16×16 AMARU icon set).

## Pages

- Emulator: https://marcocasanova00.github.io/amaru-os/
- Song editor: https://marcocasanova00.github.io/amaru-os/editor/ (create/modify songs, JSON import/export)
- Installer: https://marcocasanova00.github.io/amaru-os/install.html
  (WebMIDI, Chrome/Edge: installs the pinned release package over USB)
- Package verifier: https://marcocasanova00.github.io/amaru-os/verify.html
  (identity/size/SHA-256 check only, no flash actions)

`install.html` and `verify.html` are **generated** — do not edit them by hand. They inline
`firmware/web/fm1pkg.js` + `firmware/web/fm1ota.js`, so the page always speaks the same
M-UPGRADE protocol as the firmware (the device pulls the package with `0x30` read requests;
the host never pushes raw data). Rebuild after a firmware change with:

```
python3 tools/build_pages.py            # pins identity, size and SHA-256 of build/felucca.fwsc
node tests/installer.test.mjs           # protocol + pins
node tests/installer.page.test.mjs      # the page, end to end, against a simulated FM-1
```
- Releases: https://github.com/MarcoCasanova00/amaru-os/releases
- Firmware Hub: https://thefirmwarehub.com (submission planned post-release)
- Flash safety & storage: `docs/SAFETY.md`

---

*AMARU OS — by Marco Casanova. GPL-3.0-only.*
