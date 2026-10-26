# AMARU OS — next plans (license-safe only)

These are the planned items for the next slice. Anything marked "LICENSE CHECK REQUIRED"
or "no import" is deliberately excluded from the import column in
`docs/AMARU-ANALYSIS.md` (§7, §11): Classic-Mac icons, Carnac wavetable/sample
binaries, and Mycelium samples/grooves are **not** being copied because their
redistribution rights are not verified and their binary footprint is too large.

## ✅ Done (alpha)
- Deterministic 9-genre / 4-layer generator (`amaru_gen.c`, GEN page).
- Three strict monochrome terminal themes (green/black, black/white, amber/black).
- `GEN` page wired to K1–K4 + OCT+/OCT-; GROW/EVOLVE proven on generated tracks.
- Host tests green; native WASM matches wasm bit-for-bit.
- `main`, `alpha`, `gh-pages` pushed; GitHub Pages enabled.

## ⏭ Next

- [ ] **Macintosh-style icons (AMARU-original, monochrome)**
  - Design a small set of 1980s-Mac-style glyphs ourselves in the AMARU monochrome
    style (no import of `thomasareed/Classic-Mac-icons`, and **no import of
    `therenegar/retro-icons`** either — that repo ships 3,798 Windows-Icons
    extracted from old software and has **no LICENSE file / no SPDX grant**, the
    same licensing gap that rejected Classic-Mac). Use the icon sources only as
    monochrome shape reference, then redraw in AMARU style. Reuse Bunker's icon
    drawing primitives; add a `B_` → `AM_` icon table.
  - Gate: pass strict-terminal contrast checks.

- [ ] **More variety on the GEN page**
  - Add **TARGET** / **layer** selection so each role can be regenerated
    independently (sounds vs sequence vs song) without losing the seed.
  - Add named tekno/psy/ambient recipes and more Mycelium-/Carnac-inspired
    **native** textures (grain clouds, formant choirs, noise beds, FM blooms,
    physical metal) through Bunker's own engines — no external binaries.

- [ ] **Independent regeneration controls**
  - Per-layer OCT+ regenerate; a shared SEED that survives partial regens.

- [ ] **Full release gate**
  - Run on hardware with stock V15 rollback rehearsed.
  - Complete the 9-question safety gate (recovery/update + hardware questions
    are currently "deferred" in `docs/AMARU-ANALYSIS.md`).

## ❌ Blocked / not doing (by design)
- Copying Carnac `wavetable` PCM/binaries (256 KiB flash risk, unverified rights).
- Copying Mycelium samples, grooves, or brand assets (unverified rights, GPL
  source is fine for concepts only).
- Importing Classic-Mac icons (no license metadata).
- Direct port of JIANT's six-track engine (Bunker is four tracks by design).
