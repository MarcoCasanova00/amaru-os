# AMARU OS — Handoff Note

- **Date:** 2026-10-26
- **State:** Alpha 0.1a, release-gate *hardware* still pending formal sign-off.
- **Current source:** AMARU `2c441ff` (docs/recipe placeholders clarification); JIANT reference `2f16bec`.
- **Immediate next session:** rework AMARU GEN into a JIANT-style music-theory generator on the existing four tracks, without changing sounds.

## 1. Goal for the next session

Make AMARU’s GEN produce JIANT-like random songs using only the existing four AMARU layers:

| AMARU layer | Role | JIANT role used as source |
|---|---|---|
| `SPARK` | drums / percussion | `RL_DRUM` |
| `ROOT` | bass | `RL_BASS` |
| `AIR` | pad / chord / texture | `RL_PAD` |
| `MELODY` | lead / melodic motif | `RL_LEAD` |

Do **not** add the JIANT `ARP` or `TEX` tracks. Do **not** change the Bunker engines, sound recipes, track count, or the public `amaru_plan_t` four-track shape.

The outcome must be:

- Drums have genre-specific grammar, lane accents, ghosts, fills and ratchets instead of one fixed bar with light probability.
- Bass follows the chord plan, kick interlock and genre-specific bass archetypes instead of six hard-coded degree patterns.
- Pads/leads sit on real chord progressions and voicings, so the harmony has a flow.
- A fixed genre still sounds like that genre across seeds; genres stop generating each other’s identities.
- All generated steps remain ordinary `step_t` data so GROW/EVOLVE, editing, undo and playback keep working.

## 2. Why the current generator bleeds genres

Current data flow:

```text
amaru_plan()
  -> GENRES[genre]                 fixed masks + bass/pad/lead mode + sound
  -> one global xorshift RNG       seeded by seed/genre/form/energy
  -> make_drums()                  fixed 16-step masks, light random hats/ghosts/fill
  -> make_bass()                   fixed degree arrays per BASS_* mode
  -> make_pad()                    root-position triad, almost no progression
  -> make_lead()                   fixed HOOK/ARP/ACID/STAB figures
  -> amaru_apply_plan()            copies to step_t, applies sounds
```

The problem is structural, not a bad seed:

1. `genre_def_t` (`amaru_gen.c:39`) has only `kick/snare/clap/hat/ohat/perc`, `bass/pad/lead` mode enums, one `root`, one `scale`, tempo/swing and `snd[4]`. It has no chord progression, no bass-archetype pool, no drum-map pool, no form/human defaults, and no per-genre part amounts.
2. Six of the nine AMARU genres share the exact same kick and snare masks:
   `TRIBE TEKNO`, `MENTAL TEKNO`, `HARDTEKK`, `ACID CORE`, `TECHNO`, `PSYTRANCE`.
   All use:
   - kick `x...x...x...x...`
   - snare `....x.......x...`
3. Four pairs share kick + snare + clap + hat exactly:
   - `TRIBE = MENTAL = PSY`
   - `HARDTEKK = TECHNO`
4. `MENTAL TEKNO` and `PSYTRANCE` also share root `9`, `SC_PHRY`, `BASS_ROLL`, the same kick/snare/hat, and no clap. They differ only in percussion, pad mode, lead mode, BPM, energy and sound recipe. That is why one can generate the other.
5. `TECHNO` also uses `BASS_ROLL`; `JUNGLE`/`DRUM & BASS` both use `BASS_SUB` and `SC_MIN`; `HARDTEKK`/`BREAKBEAT TEKNO` both use `BASS_OFF`. The mode enums are too coarse to carry genre identity.
6. `make_pad()` always builds `deg, deg+2, deg+4` from one key. There is no `PR_*` progression, no chord quality, no voice leading, so the bass/lead cannot follow changing harmony.
7. `make_bass()` never reads a chord. `BASS_ROLL` is just `deg = (pos * 3 + bar) & 7` and `BASS_ACID` is another fixed pattern. Neither approaches the next chord.
8. `amaru_plan()` uses one RNG stream for all parts. Any change in drum probability also changes bass/pad/lead decisions; there is no per-part determinism.
9. AMARU has no microtiming field in `step_t` (`core.h:156`); its flags are only `SF_ACCENT`, `SF_SLIDE`, `SF_RATCH` (bits 3–4). JIANT has `SF_MFINE` (bits 5–6), `cwhen`, components and lane modes. Those are real runtime differences, not generator settings.

Exact current signature table (bar 0):

| Genre | Kick | Snare | Hat | Bass | Pad | Lead | Root/scale |
|---|---|---|---|---|---|---|---|
| TRIBE TEKNO | 4/4 | 2/4 | offbeat 16th | TRIBE | CHOIR | STAB | 2 DOR |
| MENTAL TEKNO | 4/4 | 2/4 | offbeat 16th | ROLL | NOISE | SIREN | 9 PHRY |
| HARDTEKK | 4/4 | 2/4 | offbeat 16th | OFF | STAB | STAB | 5 MIN |
| ACID CORE | 4/4 | 2/4 | 16ths | ACID | STAB | ACID | 3 PHRY |
| JUNGLE | broken | broken | 16ths | SUB | CHOIR | SPARSE | 2 MIN |
| DRUM & BASS | broken | 2/4 | offbeat 16th | SUB | DRONE | ARP | 7 MIN |
| BREAKBEAT TEKNO | broken | broken | 16ths | OFF | STAB | STAB | 5 DOR |
| TECHNO | 4/4 | 2/4 | offbeat 16th | ROLL | DRONE | HOOK | 7 MIN |
| PSYTRANCE | 4/4 | 2/4 | offbeat 16th | ROLL | CHOIR | ARP | 9 PHRY |

## 3. How JIANT avoids the bleed

JIANT’s genre row is a complete musical grammar, not just masks and sounds. `ggenre_t` (`gen_tables.c:440`) contains:

- `scale[4]` — four possible modes per genre.
- `drum`, `drum_pct`, `kit`, `ghosts`, `hats`, `fills`, `sd_micro` — a named drum map plus ghost/hat/fill rules and snare placement.
- `bass[3]`, `bass_lo`, `kick_mode`, `bass_gate` — three bass archetypes, register, kick handover rule, gate.
- `prog[3]`, `cbars`, `pads`, `voicing` — three progressions, chord length, pad playing style, voicing.
- `arp[2]`, `lead` — arp and lead styles.
- `amt[GP_N]`, `form`, `human`, `hx` — default part amounts, song form, humanization, harmony family.

The flow is:

```text
GENRE row
  -> gen_pick(): root + scale[seed % 4] + progression[seed % 3] + bass[seed % 3]
                 + voicing + amounts + form + human + harmony
  -> gplan(): chord starts/lengths/qualities/voicings
  -> per-part RNG streams: GS_DRUMS, GS_BASS, GS_CHORDS, GS_LEAD, GS_FORM, GS_MIX
  -> gen_drums() reads the drum map and kick; gen_bass() reads the chord plan and kick;
     gen_pads() reads the chord plan and voicing; gen_lead() reads the bass/chord plan
  -> gen_parts()/gen_post(): section order and arrangement
```

That is the “music theory is the flow” part AMARU is missing. JIANT also has 67 bass archetypes (`gen_tables.c:130`), ~46 drum maps and many progressions. AMARU currently has 6 bass modes, 4 pad modes, 6 lead modes and 9 genre rows.

Important JIANT references:

- `gen_pick()` `gen.c:2280`; `gen_make()` `gen.c:2367`; `gen_parts()` `gen.c:1719`; `gform_order()` `gen.c:1687`.
- `gen_drums()` `gen.c:425`; `gen_bass()` `gen.c:686`; `gen_acid()` `gen.c:626`; `gen_pads()` `gen.c:809`; `gen_lead()` `gen.c:1083`.
- `gen_post()` `gen.c:2189`.
- Per-part seeds: `gseed()` `gen.c:41`; stream enum `gen.c:64`.
- JIANT genre rows for the six tekno-family genres: `gen_tables.c:573-641`; `BREAKS` at `:589`, `DRUM & BASS` at `:593`.
- JIANT has no genre named `JUNGLE`. AMARU’s `JUNGLE` must be defined from JIANT `DM_BREAKS`/`DM_DNB` plus `BA_JUNGLE`, not copied as a nonexistent row.

## 4. Target AMARU design

### 4.1 Keep the public plan and sounds

Keep `amaru_plan_t`, `AMARU_TRACKS == 4`, `AMARU_STEPS == 64` and the existing adapter `bunker_amaru.inc`.

Preserve the existing `snd[4]` values per AMARU genre exactly. Do **not** port JIANT `KIT[]`, `SR_*` recipes, engine numbers or `P_*` parameters. “Fix GEN” means change the notes/rhythms, not the voices.

The only sound-adjacent change allowed is the existing tone jitter in `amaru_plan()`, unless a later decision says otherwise.

### 4.2 Replace `genre_def_t` with an internal JIANT-like row

Suggested internal shape (not ABI; `amaru_gen.c` only):

```c
typedef struct {
    const char *name;
    uint8_t lo, hi, swing;
    uint8_t scale[4];
    uint8_t drum, drum_pct, ghosts, hats, fills;
    int8_t  sd_micro;            /* later; first pass may leave 0 */
    uint8_t bass[3], bass_lo, kick_mode, bass_gate;
    uint8_t prog[3], cbars, pads, voicing;
    uint8_t lead;
    uint8_t form, human, hx;
    uint8_t amt[AMARU_TRACKS];         /* role density; keep all four nonzero */
    amaru_sound_t snd[AMARU_TRACKS];   /* unchanged AMARU sounds */
} amaru_genre_t;
```

Only port the subsets needed by the nine AMARU rows:

- Drum maps: `DM_TRIBE`, `DM_MENTAL`, `DM_ACIDCORE`, `DM_HARDTEKK`, `DM_TEKNO`, `DM_PSY`, plus `DM_BREAKS`, `DM_DNB`, and a JUNGLE-oriented break map. Do not port all 46 maps.
- Bass archetypes: the ~13 used by those rows, e.g. `BA_TRIBE`, `BA_BOUNCE`, `BA_PSY`, `BA_MENTAL`, `BA_ROLL16`, `BA_ACID`, `BA_ACIDCORE`, `BA_HARDTEKK`, `BA_PUMP8`, `BA_DNB`, `BA_JUNGLE`, `BA_WOBBLE`, `BA_AROUND`. Do not port all 67 unless later needed.
- Progressions: `PR_ONE`, `PR_PHRY`, `PR_I_VII`, `PR_I_VI`, `PR_DEEP4`, `PR_TRANCE`, `PR_ANDAL`, and the minor/PHRY fallbacks. Do not port the whole `PROG` table initially.
- Chord/scale helpers: `gnd`, `gtone`, `gchord`, `gchord_starts`, `gbass_tone`, `gvoice` (or AMARU equivalents). These are small and essential.
- Drum helpers: `GHOST_RANK`, `HAT_CELL`, `FILLS`; map JIANT lane levels to AMARU’s `hit` + `acc` + step `vel`.
- Form helpers: port `gform_order()` logic only enough to vary bars within AMARU’s one 64-step plan; do not port `section_store`/`section_load` in the first pass.

### 4.3 Source-row mapping

Use JIANT’s musical fields (including BPM and swing), keep AMARU’s row name and sound block:

| AMARU row | JIANT source for music-theory fields |
|---|---|
| `TRIBE TEKNO` | `TRIBE TEKNO` |
| `MENTAL TEKNO` | `MENTAL TEKNO` |
| `HARDTEKK` | `HARDTEKK` |
| `ACID CORE` | `ACID CORE` |
| `JUNGLE` | `DM_BREAKS` + `BA_JUNGLE` + breakbeat form; no direct JIANT row |
| `DRUM & BASS` | `DRUM & BASS` |
| `BREAKBEAT TEKNO` | `BREAKS` |
| `TECHNO` | `TEKNO` |
| `PSYTRANCE` | `PSY TRANCE` |

Map JIANT `amt[]` by role, not by index: `GP_DRUMS -> SPARK`, `GP_BASS -> ROOT`, `GP_CHORDS -> AIR`, `GP_LEAD -> MELODY`; skip `GP_ARP`. Keep every AMARU amount nonzero so all four visible tracks stay populated.

Do not add JIANT’s 45% random “mutant” genre mixing in the first pass. AMARU has only nine named genres; mixing them is exactly the bleed the user reported. Within-genre variation should come from `scale[4]`, `prog[3]`, `bass[3]`, drum-map alt choices and per-part seeds, not from swapping another genre’s identity.

### 4.4 Generation pipeline

Replace the current four `make_*` calls with:

1. `pick`: derived from seed/genre/form/energy, choose root 0..11, one of four modes, one progression, one bass archetype, pad/lead variants, human/swing.
2. `plan_chords`: build 1–4 bar chord starts/qualities/voicings. Store only internal `gch_t`-style data, not in `amaru_plan_t`.
3. `make_drums`: read drum map; apply density, ghosts, hats, fills, bar variation, step-level ratchets. Set `hit`, `acc`, `vel`, `time`.
4. `make_bass`: read chord plan and kick. Support at least `INTERLOCK`, `SUSTAIN`, `DOUBLE`, `ANSWER`; generate approaches/slides/dead notes; no fixed `DEG[16]` for every genre.
5. `make_pad`: read chord plan; use pad style (`STAB`, `DRONE`, `CHOIR`, `TRANCE`, etc.) and voicing; no always-root triad.
6. `make_lead`: read chord plan and bass; use a composed motif/ostinato/sparse line. JIANT’s arp material may be folded into `MELODY` only as a lead variant; never add a fifth track.
7. `apply_form`: use form to vary bars/phrase endings within the existing 16/32/64-step plan. True multi-section storage is out of scope for the first pass.

Use per-part RNG streams derived from the master seed, e.g.:

```text
stream_seed = mix(seed, genre, form, energy, part_id)
```

This keeps one part stable when another part’s generator changes, and makes tests reproducible.

### 4.5 AMARU constraints to respect

- Max four notes per step (`note[4]`); chords must fit.
- Drum-touching steps use `hit`, `acc`; no notes on `SPARK`.
- Every generated step must have a valid `time`; rests carry no note/hit data.
- Note range must stay `12..108`.
- `probability` should remain 0 for generated notes unless a deliberate humanization feature is added later.
- `AMARU_SF_ACCENT` / `AMARU_SF_SLIDE` only today. Step-level ratchets are available via Bunker’s `SF_RATCH` bits 3–4; add matching `AMARU_SF_RATCH` constants in `amaru_gen.h` because the adapter copies `flags` unchanged. Lane-level ratchets are not supported.
- AMARU has per-lane **accent** but not per-lane **velocity**: JIANT’s lane level must be approximated as hit vs accented hit plus step velocity. Do not claim exact JIANT velocity.
- Do not add microtiming in this pass. AMARU has no `cwhen` or timing offset in `step_t`. Real microtiming requires a core/sequencer/project-format change (see §7).

## 5. Implementation order

1. Snapshot current output/tests so the sound blocks and public API can be compared.
2. Add the compressed AMARU genre table, drum maps, bass archetypes, progressions and music-theory helpers in `amaru_gen.c` (or a new `amaru_gen_tables.c` if build lists are updated).
3. Port `gen_drums` first. Validate kick/snare/ghost/fill differences before touching bass.
4. Port `gen_bass` + `gen_acid`; make bass read the chord plan and kick.
5. Port `gen_pads`; then `gen_lead`/motif logic.
6. Add per-part RNG streams and bar-level form variation.
7. Add genre-signature tests and update docs.
8. Run host tests and inspect flash/RAM size before merging.

Do not start by copying JIANT `gen.c` wholesale. It is included into `ui_input.c` after `song_dice.c` (`gen.c:5`) and depends on UI/global state (`gp`, `gx`, `trk`, `dice_*`, `load_begin`, `section_store`, `project_used`, `gen_lbl`). It also writes JIANT-only sound parameters (`P_FCUT`, `P_FRES`, `P_DTYPE`), assumes 15 engines, and calls helpers such as `dhold`/`dlevel`/`euclid`. Extracting the musical core into AMARU’s pure planner is the safe path.

Do not port JIANT’s role allocator unchanged either. With `NTRK=4`, its priority order picks drums, bass, pad and arp and drops lead. AMARU’s fixed `SPARK/ROOT/AIR/MELODY` mapping is deliberate; there is no ARP track to fill.

## 6. Tests to add

Extend `firmware/tests/amaru_gen_test.c`; keep the existing determinism/form/bounds checks.

### Genre identity

- Compute a signature per plan: first-bar drum hits by lane, bass onset/degree sequence, chord starts/qualities, lead motif intervals.
- For each genre, test at least 64 seeds. For every pair of genres, require at least two of these to differ:
  drum-map family, bass archetype set, progression set, pad style, lead style, form/human.
- Explicitly fail the current collision cases:
  - `MENTAL` vs `PSY`: must not have identical drum bar + root/scale + bass pattern.
  - `TRIBE` vs `MENTAL` vs `PSY`: must not share all four fixed masks.
  - `HARDTEKK` vs `TECHNO`: must not share all four fixed masks.
  - `JUNGLE` vs `DRUM & BASS`: must remain distinguishable by drum grammar and bass archetype, not only BPM.
- Do not use one golden pattern per genre; test invariants and minimum distances instead.

### Drums

- `JUNGLE`, `DRUM & BASS`, `BREAKBEAT TEKNO` have broken kicks, not four-on-the-floor.
- Techno family has four-on-the-floor kick; kick density changes with energy but stays in the genre’s range.
- Ghost/fill events appear at the expected rates over a seed sweep.
- No empty downbeat; no notes on `SPARK`.

### Bass

- Every note is in the current chord/scale (or a legal chromatic approach immediately resolving to it).
- `ROLL/INTERLOCK` bass avoids the kick hit where the archetype says so.
- Bass changes when the underlying chord changes; no 16-step fixed motif across all bars.
- Slides/approaches are limited and resolve.

### Pads/leads

- Chords use at least two different roots/qualities across a 4-bar `SONG` plan.
- Chord tones are in scale; voicings fit `note[4]`.
- Lead material follows the chord/bass; no always-empty or always-identical lead.
- All four tracks remain populated as before.

### Adapter/compatibility

- `amaru_apply_plan` still maps `SPARK/ROOT/AIR/MELODY` correctly.
- Sound blocks are byte-identical to the pre-rework sound assignments (apart from existing tone jitter) for every genre.
- GROW/EVOLVE still operate on generated steps; no new step type or malformed rest appears.

## 7. Deferred: microtiming, lane modes, components

These are JIANT behaviors AMARU cannot represent today:

- **Microtiming:** JIANT stores `ST_MICRO` in `cwhen` + `SF_MFINE` (bits 5–6), range −32..+31 64ths (`core.h:451`). AMARU `step_t` has no `cwhen`; its flags bits 5–6 are unused, but putting JIANT’s `SF_MFINE` there would only carry the sub-sixteenth remainder, not the whole-sixteenth part. Real microtiming needs a new timing field or a `cwhen` equivalent, plus `seq.c` early/late handling and project-format migration.
- **Lane modes/ratchets:** JIANT has `step_set_lmode` and per-lane ratchet logic. AMARU only has one step-level ratchet (`SF_RATCH`).
- **Components:** JIANT’s `comp/carg/comp2/carg2/cwhen/ctime/lm[]` have no AMARU equivalent.

Do not fake these with probability or global swing. Either implement them as a separate core task after the genre rework, or explicitly ship the groove without microtiming. The user’s four-track/sound constraints remain intact either way.

## 8. Risks / open decisions

- **Flash/RAM:** current AMARU gate is RAM .data+.bss 61 KB and flash image 535,612 B. Measure after adding tables. Keep the ported subset small; do not import all of JIANT’s generated tables blindly.
- **Lane velocity:** AMARU cannot reproduce JIANT’s 16-level per-lane velocity without a `step_t`/save-format change. Map levels to hit/acc and accept approximation, or scope a separate core change.
- **Sections:** AMARU has one plan, not JIANT’s stored sections. First pass varies bars within 16/32/64 steps; true `A/B/A/C` section storage is a larger runtime feature.
- **Genre names:** JIANT has no `JUNGLE`; AMARU’s JUNGLE row must be authored from breakbeat/DnB elements. Do not claim byte parity with a JIANT row that does not exist.
- **Scale coverage:** AMARU’s planner maps only six scale tables. JIANT rows using `SC_LYD`, `SC_BLUE`, etc. must be filtered or AMARU’s scale table extended first.
- **UI:** AMARU GEN is one page with four controls; JIANT GEN is three pages. Do not port the three-page UI in this pass. The generator behavior is the priority; HOME already reaches AMARU GEN.
- **Seed behavior:** keep AMARU’s current OCT+ new-take / OCT− reset behavior unless a separate decision changes it.

## 9. Key files

- `firmware/firmware/src/amaru_gen.c` — planner; `GENRES:55`, `make_drums:210`, `make_bass:255`, `make_pad:331`, `make_lead:353`, `amaru_plan:408`.
- `firmware/firmware/src/amaru_gen.h` — public 4-track plan; `AMARU_AIR/MELODY/ROOT/SPARK`.
- `firmware/firmware/src/bunker_amaru.inc` — adapter; `amaru_apply_plan:37`, `amaru_generate:98`, `amaru_new_seed:119`.
- `firmware/firmware/src/bunker_ui.c` — GEN controls and OCT+/OCT− seed behavior `:220-248`.
- `firmware/firmware/src/core.h` — `step_t:156`, `track_t:250`, `SF_RATCH`; no microtiming field.
- `firmware/firmware/src/seq.c` — timing, ratchets, swing; relevant only if microtiming is later added.
- `firmware/tests/amaru_gen_test.c` — host generator tests.
- `firmware/tests/ui_test.c` — `test_amaru_gen` around `:9840`.
- `/tmp/opencode/jiant/firmware/src/gen.c` — reference; `gen_drums:425`, `gen_bass:686`, `gen_acid:626`, `gen_pads:809`, `gen_lead:1083`, `gen_pick:2280`, `gen_make:2367`, `gen_parts:1719`, `gen_post:2189`.
- `/tmp/opencode/jiant/firmware/src/gen_tables.c` — `BASS_ARCH:130`, `ggenre_t:440`, `GENRE[]:461`, six tekno rows `:573-641`.
- `/tmp/opencode/jiant/firmware/src/song_dice.c` — JIANT helpers (`dhold`, `dlevel`, `euclid`, roles) if a helper must be extracted.
- `/tmp/opencode/jiant/docs/TEKNO-REWORK.md` — measured genre/bass/envelope issues and regression approach.
- `docs/QUICKSTART.md`, `docs/AMARU-ANALYSIS.md` — update genre descriptions after the rework.

## 10. Previous shipped state

- **Firmware (AMARU OS):** derivative of Bunker Evolved 1.0 (GPL-3.0).
- **Generator (`amaru_gen.c`):** 9 genres, 4 tracks (AIR/MELODY/ROOT/SPARK), 3 forms (LOOP/PARTS/SONG), deterministic seed, GROW/EVOL integration.
- **Sound recipes:** Carnac/Mycelium-inspired presets ported as numbers, not imported binaries. Plus cosmetic AMARY/MAYA/TUPAC names.
- **Theme:** strict monochrome palettes GRN/BLK, BLK/WHITE, AMB/BLK; accidental REC red → accent.
- **Web UI:** AMARU branding, 1-bit logo splash, AMARU 0.1a version.
- **Web emulator (gh-pages):** latest `felucca.wasm`; preset alias returns clean factory names.
- **Sound alias layer (`amaru_sound_alias.c`):** curated name overlays + factory fallback.
- **Documentation:** README hardware status/flashing, QUICKSTART, ATTRIBUTION, release-gate questions with test evidence.

## 11. Brick-safety posture

- Firmware flashes via Jieli’s stock staging+commit loader; a package whose flash size does not match `OTA_FL_LEN` is refused before commit.
- Build gates pass: `hal/` register access only, single core, RAM .data+.bss 61 KB, flash image 535,612 B.
- Hardware runtime (SPL handoff, clock tree, flash write timing) remains unverified; alpha should stay alpha until formal hardware sign-off.

## 12. Other future work

1. Hardware validation: rehearse V15 rollback, flash one FM-1, confirm no brick, sign off the 9-question gate → beta.
2. Full factory-name catalog in `amaru_sound_alias.c`.
3. Optional true JIANT-style microtiming/lane modes after the GEN theory rework.
4. Visual identity pass on `ui_draw.c` track chrome.
5. Hardware rollback test harness.
6. Optional Carnac/Mycelium texture work and user-bank cloning.

Built on Bunker Evolved 1.0 (Hügelton Instruments), GPL-3.0-only. Engine tools/modules retained. Release-gate doc: `docs/AMARU-ANALYSIS.md`.
