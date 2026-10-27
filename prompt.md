# AMARU FIRMWARE PROJECT - AGENT PROMPT (read this first)

This file is the contract for any agent continuing the AMARU firmware rework.
Read it before touching anything else in the repository.

---
## 🎯 MISSION (in one line)
Build 4-engine FM-1 firmware (ANALOG / DIGITAL / DRUM / FM6) + web emulator that
**borrows Carnac OS + Mycelium sounds (authentic DSP behavior, not files) while
keeping native synthesis**, in 4 channels (AIR, MELODY, ROOT, SPARK). Ship when
worked end-to-end with real hardware flash-fit verification.

---
## 📊 CURRENT STATUS
- **Main commit:** `9f2ca52` (latest) — Cherry-picked milestone A + documentation
- **Branches:** `main` (everything), `gh-pages` (web emulator only), `alpha` (old, ignore)
- `engine-rework` has been deleted (its changes merged into main); do not recreate
- **Milestone A (engine retirement):** complete. Both builds 497,884 B default and 498,332 B baseline.
- **Milestones B, C, D (sound engine import + GEN port + web rebuild):** remaining work

---
## 🔑 PRIORITIES (READ FIRST)
1. **Sound engine FIRST** — Carnac sounds, Mycelium sounds + drums, synthesis (LFO, envelopes),
   native parameter combos **fully baked into our firmware**, and JIANT FM GEN functionality ported.
   This is the core identity of the product. Slight downtime on tests is acceptable, not on sound rework.
2. **Tests later** — verification is important but takes token/time. Re-baseline after sound
   engine substantially stable.

---
## 🛠️ KEY CONSTRAINTS (don't break these)
- `AMARU_TRACKS==4`, `AMARU_STEPS==64`
- `FELUCCA_SLICE=0`, `FELUCCA_FM4=1`, `FELUCCA_RETIRED=0`
- Engines: 4 active (ANALOG, DIGITAL, DRUM, FM6; retired slots point at `&ENG_RETIRED`)
- Preserve the `bunker_amaru.inc` adapter contract
- FM-1 hardware: FELUCCA / etiny; real flash-fit verification matters (build-fits only goes so far)
- **No chiptunes, no lofi, no classical sounds like piano only** — synth FM6 + samples + GEN / Mycelium-style
- **Hardware / family-specific constraints**: keep smooth and DSP-friendly

---
## 🔧 ENVIRONMENT READY
- **Build command:** `sh build.sh` (build.sh has no +x; invoke with sh)
- **Output:** `firmware/build/felucca.bin` (~498 KB baseline), flash-fits via `utilities/target_budget.py`
- **Toolchain:** `~/.jieli/toolchain/pi32v2/bin/clang` (confirmed present); AC79 SDK at `~/fw-AC79_AIoT_SDK`
- **Web emulator:** emcc at `/home/maltomare/emsdk`
- **Reference source trees** (single source of truth, LOCAL PATHS - may NOT be on your machine):
  - Mycelium: `/home/maltomare/amaru-research/mycelium-fm1/src/mycelium-fm1/firmware/src/`
  - Carnac: `/home/maltomare/amaru-research/carnac-os/src/`
  - JIANT fork: `/home/maltomare/JIANT-fork/firmware/src/`

  ⚠️ **If these directories don't exist on your machine**, search the web for the public source
  repositories via **firmawarehub.com** (search "firmawarehub.com amaru" or "firmawarehub Carnac"
  etc.). The community source repos are typically published there. The essential things needed:
    - Mycelium's ANALOG + DIGITAL preset tables (with XP extras) – they are `static const preset_t
      ANALOG_PRESETS[]` and `DIGITAL_PRESETS[]` in `eng_analog.c` / `eng_digital.c`.
    - Mycelium's drum kit definitions – `drum_synth.c` + `felucca_drumkits.h`.
    - JIANT's GEN tables – `gen_tables.c` (GENRE[] at ~461) + `gen.c`.
    - Carnac parameter shapes – `PresetStructures.h` documents its structural layout.

  The AMARU documentation (`HANDOFF.md`, `docs/RESCUE.md`, `docs/AMARU-ANALYSIS.md`)
  annotates verbatim excerpts of these reference tables so you don't strictly need them."""

---
## 🚧 TASK LIST (ordered — do in sequence)

### Task 1 — Preset Import (Most Urgent)
**Problem:** Mycelium presets use `XP(pairs)`; AMARU's `preset_t` has no `XP` field.
**Solution:** Either (A) extend `preset_t`; (B) apply via `amaru_sound_t`; (C) partial import.
**Goal:** ALL Mycelium ANALOG (21) + DIGITAL (29) presets land verbatim, with GLIDE, TRANSPOSE,
ENV→PITCH, LFO rate / LFO→filter / LFO→pitch / LFO→shape faithfully applying. Carnac presets may be
re-authored as parameter-shape recipes (parameter combinations + LFO + envelopes).

Files: `firmware/firmware/src/ui.c` (`apply_preset_to` land line 1347),
      `firmware/firmware/src/core.h` (FELUCCA_* macros), `firmware/firmware/src/eng_analog.c`,
      `firmware/firmware/src/eng_digital.c` (vendor's `engines/src/...` vs Mycelium's `firmware/src/...`).

### Task 2 — Drum Engine Flesh-out (sound engine essential)
Mycelium has 15 synthesized drums; verify all kit definitions exist in AMARU DRUM engine.
Ensure drift / tone / decay / drive parameters all survive edit/upload round-trip.
Verify `DW` etc. regressions avoided.

### Task 3 — Sample Data (Optional for now)
Only SLICE engine uses SMP_DATA (5 sets ~126KB, currently unused because SLICE off in AMARU).
Options: gate on FELUCCA_SAMPLE_DATA, or use "alias" pattern — user wants samples though,
so only trim if needed to free flash budget.

### Task 4 — JIANT GEN Review + 4-channel Port
JIANT's GEN (gen_tables.c + gen.c · 2869 lines / 688-line tables) produces good coherent songs per genre.
Review the drum patterns, parameter values, and form. Only D-n… "wrote good reference" — some top-up;
Drum & Bass (`DM_DNB` map) is the reference template. Genre table at gen_tables.c:461.
Port to AMARU's 4-track (SPARK, ROOT, AIR, MELODY) 4-engine (ANALOG/DIGITAL/DRUM/FM6).
Every target genre must have its own drum signature:
  - JUNGLE / DnB / BREAKBEAT TEKNO: NON-square patterns (no 4-on-4)
  - others: genre-appropriate

AMARU's `amaru_gen.c` ships with 9 genres already implemented and AMARU-shaped; review its gaps
(a fuller review point is laid out in `HANDOFF.md` §SPECIFIC IMPLEMENTATION GAPS (3)).

### Task 5 (later) — Test Re-baseline
`tests/run_tests.sh`. Note pre-existing failures observed: phys_test 2, noise_test 10, mod_test 3,
sound14_test 2, insert_test 2, editor_test_c1 build error (from broken HEAD — re-verify, some may have been
compile fallout of the broken tree; and project_test had PM_SYMP undefined — fixed to 3 in session 2).

### Task 6 (later) — Web Emulator
Rebuild emu with emcc from `/home/maltomare/emsdk`, run emu_test.mjs.

### Task 7 (later) — PAT Rotation
Rotate embedded ghp PAT in origin URL (`ghp_RZFTOK...`).

---
## 🔍 TECHNICAL FINDINGS (READ - these are verified)

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
Pre-existing test failures: phys_test 2, noise_test 10, mod_test 3, sound14_test 2, insert_test 2, editor_test_c1 build error.
project_test build error = PM_SYMP undefined — fixed to `3` in session 2. Re-baseline properly.

### PRESET HANDLES
AMARU also uses `FX(…)`, `PAT(n)`, `V_POLY/V_MONO/V_LEGATO/V_UNISON`, `F_ENGSEL`, and the engine defaults per engine. See `upreset.c` and `params.c` for the value handles used.

---
## 💡 KEY INSIGHTS FOR SUCCESS
1. **Sound authenticity matters most** — 100% listenability, don't compromise.
2. **DnB row is reference** — all genre derivation from it (other fork rows are half-baked).
3. **Backward compat.** — Don't break existing project / preset compatibility.
4. **Flash efficiency** — Optimize SMP_DATA but preserve sample functionality users want.
5. **AMARU 4-channel design** — Respect that while adapting.
6. **Tests late** — verification is important; not at the start of today's mission.
7. **Modular architecture** — AMARU's engines (ANALOG/DIGITAL/DRUM/FM6), no full port of Carnac.

---
## 🚫 WHAT NOT TO DO
- **Don't touch `bunker_amaru.inc` adapter contract** without explicit reason
- **Don't break `AMARU_TRACKS==4, AMARU_STEPS==64`**
- **Don't run full test suite before sound identity is settled** (wastes tokens)
- **Don't commit unreviewed changes; prefer smaller commits after `git status` check**
- **Don't start with web emulator** — that's last-mile

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
3. Builds succeed: `sh build.sh` (unit-test host: `hostsim`)
4. With tests last (low priority): `tests/run_tests.sh` re-baselined

---
*This file is committed to `main` as `9f2ca52..9f2ca52`. Clone the repo and read this file first, nothing else.*