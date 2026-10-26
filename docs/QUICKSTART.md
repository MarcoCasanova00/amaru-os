# AMARU OS quick start

AMARU OS is a generative FM-1 firmware (GPL-3.0) built on Bunker Evolved. It runs
in the browser at https://marcocasanova00.github.io/amaru-os/ — click **Einschalten**.

The hardware front panel and keyboard work the same as Bunker Evolved. Quick reference:

## The generator (GEN)
HOME → rotate the SELECT encoder to the **GEN** page (K1–K4 show genre parameters):

| Knob | Role |
|------|------|
| **K1** | GENRE (tribe / mental / hardtekk / acid core / jungle / drum&bass / breakbeat / techno / psytrance) |
| **K2** | SEED (any number 0–9999 — gives you a stable, repeatable song) |
| **K3** | ENERGY (0–100: full sets add extra hats/ghosts, empty strips them) |
| **K4** | FORM — `LOOP` (1 bar) / `PARTS` (2 bars) / `SONG` (4 bars) |

Buttons:
- **OCT+** → advance the seed by one and play a new song. Keep pressing it to cycle through takes of the same genre/energy/form.
- **OCT-** → reset the seed to the base value and play it.
- **PLAY** → run/stop the sequencer. The LED goes green.
- **REC** → record a take to flash (saved in your browser).

The track sound name shows the recipe's source at a glance:
`HYPOGEUM CHANT`, `ORACLE CHOIR`, `SILBURY WAIL`, `SILBURY MOUND`, `SKARA HEARTH`,
`MYC PUNCHY`, `MYC BLOWN` — the Carnac / Mycelium-inspired timbres.

## Genre character notes (from JIANT theory / r/tekno references)

- **TRIBE / MENTAL / HARDTEKK / ACIDCORE** — drums mapped to tribal / mental / hard / acid patterns (JIANT drum map family). The dark groove comes from off-beat hats (`..x...x...x...x.`) and the skank-style bass (`BASS_TRIBE`: off-beat root + fifth). Mental tekno uses `PAD_NOISE` (Carnac's dark noise hearth) + `LEAD_SIREN` for its hypnotic atmosphere.
- **JUNGLE / DnB / BREAKBEAT** — broken drum grammar (not four-on-the-floor). JIANT `DM_DNB` map (`X.........8.....` BD + `6.5.6.5.6.5.6.5.` CH) gives the off-beat skitter; the snare's anticipation ghost (`X..3`) creates the roll feel. The bass is a two-step reese (`BASS_SUB` with `DNB` rule: root at 0 and off-beat at 10).
- **TECHNO / PSYTRANCE** — 4-on-the-floor kicks with driven/stabbed/chorded layers.

The generator is deterministic: same SEED + same settings produces the exact same 64-step plan (4 bars in SONG form). The build-system's static checks (HAL-only access, RAM/flash margins) pass; real-hardware validation is still pending.

## Themes
The screen is strictly monochrome. MENU → DISPLAY → STYLE cycles:
- **1** green on black (default)
- **2** black on white
- **3** amber on black

## Navigation
- HOME shows WORLD / GROW / SOUND / MOTIF / EVOLVE / MIX / GEN.
- SELECT (encoder 2) / ALGORITHM (encoder 8) move the cursor on the keys.
- The 12 front buttons are the drum kit rows + FX held layer (see MENU → HELP for hints).
- Computer keys: `Z`–`'`, `A`–`0` play/track; `[` `]` OCT; `1`–`0` the top row.

## Saving
- SAVE → SONGS writes the current session; WORLD names your take.
- In the browser, takes are kept in `localStorage` for the session.

## Identity
AMARU OS 0.1a — splash, themes and artwork are AMARU's; the Bunker Evolved workflow and
its GROW/EVOLVE are preserved and extended, not removed. Original artwork (logos/, splash)
is © Marco Casanova; the underlying firmware is GPL-3.0 (see LICENSE).
