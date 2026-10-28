# AMARU FIRMWARE PROJECT - AGENT PROMPT (read this first)

This file is the **single, self-contained contract** for any agent continuing the AMARU firmware
rework. Read it before touching anything else in the repository. (Consolidated from the prior
`agent_prompt.md` + `prompt.md` so a fresh agent only needs this one file.)

---
## 🎯 MISSION (in one line)
Build 4-engine FM-1 firmware (ANALOG / DIGITAL / DRUM / FM6) + web emulator that
**borrows Carnac OS + Mycelium sounds (authentic DSP behavior, not files) while
keeping native synthesis**, in 4 channels (AIR, MELODY, ROOT, SPARK). Ship when
worked end-to-end with real hardware flash-fit verification. **Sound identity (sounds,
drums, synthesis, JIANT FM GEN) comes first. Tests come last (they take time/tokens).**

---
## 📊 CURRENT STATUS
- **Release commit:** `release` branch (identical to `main`) — sound import (Mycelium
  ANALOG / DIGITAL verbatim), DRUM kit presets, GEN's four-bar SONG form **and the finished
  GEN port** (arp lead for psy/techno, pad harmony colour, per-genre humanise, tribal tom
  ghosts), web emulator rebuilt, release version **AMARU 0.4**.
- **Branches:** `release` (app snapshot, same code as `main`), `main` (everything), `gh-pages`
  (web emulator only), `alpha` (old, ignore)
- **Safety/storage:** flash safety (no hard-brick path) and storage capacity are verified —
  see `docs/SAFETY.md`.
- `engine-rework` has been deleted (its changes merged into main); do not recreate
- **Milestone A (engine retirement):** complete. AMARU build 500,172 B (`build/felucca.bin`);
  RETIRED=1 baseline 498,332 B. Flash-fit is checked by `utilities/target_budget.py`; there is no
  FM-1 hardware here, so no on-device verification yet (do not claim one).
- **Milestone B (sound import):** ANALOG 32 presets (Mycelium's 21 verbatim, XP extras applied),
  DIGITAL 36 presets (Mycelium's 29 verbatim -- MARIMBA skipped, slot 6 already is it), DRUM 15 kit
  presets (808 909 606 CR-78 707 + DEEP TIGHT PERC + PSY FOREST HITECH + JUNGLE TECHNO HARDTEKK),
  FM6 8 factory patches + the DIGITAL conversion (`fm4_convert.c`, `fm4_nearest` weighs the ENV so
  CLAV does not become FUNK KEY).
- **Milestone C (GEN):** 9 genres, JIANT's drum maps / bass archetypes / progressions adapted to the
  four tracks; SONG is a four-bar phrase (break, then the build back); jungle / DnB / breakbeat stay
  off the quarters. `tests/amaru_gen_test.c` holds these invariants.
- **Milestone D (web emulator):** rebuilt (Emscripten) — `build/emu/felucca.wasm`
  764,907 B, AMARU 0.4; verified bit-for-bit against native via `emu_test.mjs`; published
  on `gh-pages`.
- **Power-on sounds (TRK_DEF):** fixed to the four roles -- AIR SOFT PAD (ANALOG 4), MELODY BRASS
  (DIGITAL 3 -> FM6 BRASS), ROOT FM BASS (FM6 2), SPARK DRUM KIT. The four-engine patch had left
  DIGITAL BASS on MELODY and FM6 PAD on ROOT (no lead at power-on); the hostsim TRACKS chord test
  caught it (a mono preset cannot record two keys).
- **Tests after the retirement:** built with FELUCCA_RETIRED=0 as it ships. `tests/golden.txt` was
  re-baselined (the four-engine preset sets; the retired engines' renders are gone);
  `ui_test` and `regress` no longer call SAMPLE's `sample_next` / `sample_render` when
  `FELUCCA_SAMPLE=0` (the sample-wave check and the one-shot interpolation check are skipped);
  `regress`'s voice-budget checks use FM6 (cap 6) instead of the retired VOICE engine (cap 4);
  `project_test` PM_SYMP -> `3`.

---
## 🔑 PRIORITIES (READ FIRST)
1. **Sound engine FIRST** — Carnac sounds, Mycelium sounds + drums, synthesis (LFO, envelopes),
   native parameter combos **fully baked into our firmware**, and JIANT FM GEN functionality ported.
   This is the core identity of the product. Slight downtime on tests is acceptable, not on sound rework.
2. **Tests later** — verification is important but takes token/time. Re-baseline after sound
   engine substantially stable.
3. **Web emulator last** — last-mile verification only after sounds + GEN are correct.

---
## 🛠️ KEY CONSTRAINTS (don't break these)
- `AMARU_TRACKS==4`, `AMARU_STEPS==64`
- `FELUCCA_SLICE=0`, `FELUCCA_FM4=1`, `FELUCCA_RETIRED=0`
- Engines: 4 active (ANALOG, DIGITAL, DRUM, FM6; retired slots point at `&ENG_RETIRED`)
- Preserve the `bunker_amaru.inc` adapter contract (see `HANDOFF.md`)
- FM-1 hardware: FELUCCA / etiny; real flash-fit verification matters
- **No chiptunes, no lofi, no classical sounds like piano only** — synth FM6 + samples + GEN / Mycelium-style
- **Hardware / family-specific constraints**: keep smooth and DSP-friendly

---
## 🔧 ENVIRONMENT (MACHINE-SPECIFIC)
- **Toolchain:** `~/.jieli/toolchain/pi32v2/bin/clang` (confirmed present **ON THIS MACHINE ONLY**);
  AC79 SDK at `~/fw-AC79_AIoT_SDK`. *If you are on a different machine*, the build will NOT run
  without jieli's pi32v2 toolchain + SDK installed — install jieli's pi32v2 toolchain or build in a
  known-working container. Any `clang` on PATH is NOT equivalent.
- **Build command:** `sh build.sh` (build.sh has no +x; invoke with sh)
- **Output:** `firmware/build/felucca.bin` (~498 KB baseline), flash-fits via `utilities/target_budget.py`
- **Web emulator:** emcc at `/home/maltomare/emsdk`
- **Reference source trees** (LOCAL PATHS - may NOT exist on your machine):
  - Mycelium: `/home/maltomare/amaru-research/mycelium-fm1/src/mycelium-fm1/firmware/src/`
  - Carnac: `/home/maltomare/amaru-research/carnac-os/src/`
  - JIANT fork: `/home/maltomare/JIANT-fork/firmware/src/`

  ⚠️ **If these directories don't exist on your machine**, search the web for the public source
  repositories via **thefirmwarehub.com** (search e.g. "thefirmwarehub.com amaru mycelium carnac").
  The community source repos are typically published there. The essential artifacts needed:
    - Mycelium's ANALOG + DIGITAL preset tables (with `XP` extras) in `eng_analog.c` / `eng_digital.c`;
    - Mycelium's drum kit definitions in `drum_synth.c` + `felucca_drumkits.h`;
    - JIANT's GEN result: `gen_tables.c` (GENRE[] at ~461) + `gen.c` (~2869 lines);
    - Carnac parameter shapes in `PresetStructures.h`.

  The AMARU documentation (this directory) annotates verbatim excerpts taken from those repos, so
  you do **not** strictly need them: `HANDOFF.md`, `docs/RESCUE.md`, `docs/AMARU-ANALYSIS.md`.

---
## 🚧 TASK LIST (ordered — do in sequence)

### Task 1 — Preset Import (DONE — kept as the record)
**Problem (was):** Mycelium presets use `XP(pairs)`; AMARU's `preset_t` had no XP field.
**What landed:** `preset_t.x[8]` + `XP()` in `engines/src/felucca_engine.h`; `preset_extras()` in
`src/params.c` (Mycelium's `id = x[i] - 1`, clamp to `TP[id]`), called by `ui.c apply_preset_to()`
after the sends. ANALOG: 21 Mycelium presets verbatim at 0..12 + 15 appended (32). DIGITAL:
`src/fm4_convert.c DIGITAL_PRESETS` 36 (Mycelium's 29 verbatim at 8..23 and 24..35; MARIMBA at
24.. was skipped because slot 6 already is that preset), `FM4_NPRESETS 36`, `FM4_TO_FM6[36]` with
the appended ones on `FM6_OWN`. `fm4_nearest` weighs ENV / ENV->FLT too (CLAV and FUNK KEY share
every EDIT value). Verified by `tests/fm4_test.c` (presets, routing, pitch, centroid, level,
envelope) and `tests/fm4_div0_test.c` (UBSan; built with `-DFELUCCA_FM4=0`).

**Files:** `firmware/firmware/src/ui.c` (`apply_preset_to`), `src/params.c` (`preset_extras`),
      `src/fm4_convert.c`, `engines/src/felucca_engine.h`, `engines/src/eng_analog.c`.

### Task 2 — Drum Engine (DONE for the kits; verify on device later)
DRUM has the five machines the model table is made of (KIT 80 10 66 55 77 = 808 909 606 CR-78
707), the 1.5.6 combos (DEEP TIGHT PERC) and Mycelium's three originals (PSY FOREST HITECH), now
all offered as named presets, plus JUNGLE TECHNO HARDTEKK shaped with Mycelium's own numbers
(`tools/gen_drumkits.py`: JUNGLE kick drive 50, TECHNO drive 96). `tests/drum_test.c` covers every
lane of every kit, TONE / DECAY / TUNE / SNAP, the corners, the choke, the GM map, lane LEVEL, and
now that every preset applies and sounds. Parameters live in `t->p[]` (P_E0..P_E7) and projects /
user presets store P_COUNT values, so they round-trip by construction. Mycelium's 35 kits are not
ported as data: AMARU re-authors them as piece-combos on its own 8-lane engine (a choice made
before this task list; do not replace the engine without a plan).

### Task 3 — Sample Data (Optional for now)
Only SLICE engine uses SMP_DATA (5 sets ~126KB, currently unused because SLICE off in AMARU).
Options: gate on FELUCCA_SAMPLE_DATA, or use "alias" pattern — user wants samples though, so
only trim if needed to free flash budget. Bias TOWARD importing Mycelium samples vs. trimming.

### Task 4 — JIANT GEN Review + 4-channel Port (largely DONE; review further by ear)
JIANT's GEN (gen_tables.c + gen.c · 2869 lines / 688-line tables) produces good coherent songs per genre.
AMARU's `src/amaru_gen.c` ports the parts that fit four tracks: JIANT's drum maps (verbatim for
TRIBE MENTAL ACIDCORE HARDTEKK TEKNO PSY BREAKS DNB; JUNGLE authored), hat cells, ghost ranks,
fills, bass archetypes, progressions, pad / lead styles, the per-genre amounts and sound recipes.
SONG (64 steps = 4 bars) now shapes the phrase: break in bar 3, build with the roll in bar 4;
jungle / DnB / breakbeat never fall on four quarters (`tests/amaru_gen_test.c`).
Still open: JIANT's arp / harmony-family / humanise layers, its 32-bar FM_CLUB arc (AMARU has four
bars), and a listen-through of every genre's throw.
Every target genre must have its own drum signature:
  - JUNGLE / DnB / BREAKBEAT TEKNO: NON-square patterns (no 4-on-4) — held by the test
  - others: genre-appropriate

### Task 5 (later) — Test Re-baseline
`tests/run_tests.sh`. Note pre-existing failures observed: phys_test 2, noise_test 10, mod_test 3,
sound14_test 2, insert_test 2, editor_test_c1 build error (from broken HEAD — re-verify, some may have been
compile fallout of the broken tree; and project_test had PM_SYMP undefined — fixed to 3 in session 2).

### Task 6 (later) — Web Emulator
Rebuild emu with emcc from `/home/maltomare/emsdk`, run emu_test.mjs.

### Task 7 (later, hygiene) — PAT Rotation
The PAT is NOT committed (verified grep of source tree). It lives only in local
`.git/config`'s `remote.origin.url`. To avoid token leakage from backups/sync tools, rotate it:
GitHub → Developer settings → revoke old token → create scoped `contents:write` token →
`git remote set-url origin https://x-access-token:<NEW>@github.com/MarcoCasanova00/amaru-os.git`
on every clone. Then confirm `git push --dry-run origin main` succeeds.

---
## 🔍 TECHNICAL FINDINGS (verified)

### Engine Retirement Results (Milestone A)
- **Baseline RETIRED=1**: felucca.bin **498,332 B**
- **AMARU Default**: felucca.bin **497,884 B**
- **Flash difference:** only 448 B from gc-sections savings; true save available is SMP_DATA (~120KB).

### Preset Compatibility
```c
typedef struct {
    const char *name;
    int8_t  e[8];      /* P_E0..P_E7 */
    uint8_t env[4];    /* ATK DEC SUS REL */
    int8_t  fenv;      /* ENV -> FILTER */
    uint8_t mono;      /* 1 = MONO, 0 = POLY */
    uint8_t fx[4];     /* DIST, CHORUS, DELAY, REVERB */
    uint8_t pat;       /* pattern hint */
} preset_t;            /* AMARU — no XP field */

// Mycelium preset (XP pairs)
{"808 BOOM", {3,0,0,0,127,0,48,0}, {0,112,0,52}, 0, 1, FX(0,0,0,0),
 XP(P_GLIDE+1,59, P_GLMODE+1,1, P_ED_PIT+1,18, P_TRANS+1,-24)}
```

Mycelium has full DSP behavioral parity with AMARU's ANALOG on indices 0..4
(SAW, SQR, TRI, SIN, PWM → index 0..4). Mycelium's ANALOG has 7 waves (incl SYNC/SUB).
Mycelium's DIGITAL uses the same 4-op FM as AMARU FELUCCA FM4 (`ENG_DIGITAL`).

### Sample Data (SMP_DATA)
5 sets: PIANO (5 zones), FLUTE (3), SAX (3), PERC (alias), TRANH (alias)
Total **126,244 B ADPCM in flash** (SLICE BREAK 44,100 samples, PIANO SHARED 16,537 samples).
Runtime used: SLICE engine only — currently OFF → unused in AMARU default.

### Rate / engine-rate limits
- `.data + .bss`: 53,044 B of 73,728 (72%)
- Pool: 328,924 B of 368,640 (89%) ← monitor headroom

### Test Baseline
Old loud-test failures: phys_test 2, noise_test 10, mod_test 3, sound14_test 2, insert_test 2,
editor_test_c1 build error. Re-baseline properly. `project_test` build error (PM_SYMP undefined)
fixed to `3` in session 2.

### PRESET HANDLES
AMARU also uses `FX(…)`, `PAT(n)`, `V_POLY/V_MONO/V_LEGATO/V_UNISON`, `F_ENGSEL`, and the engine
defaults per engine. See `upreset.c` and `params.c` for the value handles used.

---
## 💡 KEY INSIGHTS FOR SUCCESS
1. **Sound authenticity matters most** — 100% listenability, don't compromise.
2. **DnB row is reference** — all genre derivation from it (other fork rows are half-baked).
3. **Backward compat.** — Don't break existing project / preset compatibility.
4. **Flash efficiency** — Optimize SMP_DATA but preserve sample functionality users want.
5. **AMARU 4-channel design** — Respect that while adapting.
6. **Tests late** — verification is important; not at the start of today's mission.
7. **Modular architecture** — AMARU's engines (ANALOG/DIGITAL/DRUM/FM6), no full port of Carnac.
8. **Bunker adapter** — See `bunker_amaru.inc`. Holds the contract between pure planner and
   live `result_t`. Touching it carelessly will silently break imports.

---
## 🚫 WHAT NOT TO DO
- Don't touch `bunker_amaru.inc` adapter contract without explicit reason
- Don't break `AMARU_TRACKS==4, AMARU_STEPS==64`
- Don't run full test suite before sound identity is settled (wastes tokens)
- Don't commit unreviewed changes; prefer smaller commits after `git status` check
- Don't start with web emulator — that's last-mile

---
## 🔧 BACK-UP COMMANDS (the next agent will need these)
- Build: `sh build.sh` (from firmware/)
- Flash-fits: `python3 tests/target_budget.py build/felucca.dis tests/target_budget.txt`
- Hostsim: `cc -O2 -w -Ibuild/gen -Ifirmware/src -o build/host/hostsim tests/hostsim.c -lm`
- Hostsim run: `./build/host/hostsim`

---
## ✅ VERIFICATION WHEN DONE
1. All 9 target genres generate coherent songs with genre-appropriate drum signatures
2. Mycelium sounds (PSY BASS, 808 BOOM etc.) are findable/playable as factory presets
3. Builds succeed: `sh build.sh` (hostsim optional)
4. With tests last (low priority): `tests/run_tests.sh` re-baselined

---
*This file is committed to `main` as `0b97e9e`. Clone the repo and read this file first, nothing else.*