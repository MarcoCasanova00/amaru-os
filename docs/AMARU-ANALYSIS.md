# AMARU OS / BUNKER ADVANCED — source analysis and build plan

Status: Step 1 analysis. Baseline imported from Bunker Evolved 1.0 (`c9bb0b1`).
This document is the engineering record for the fusion. It describes what each source
actually does, what will be reused, what will be rebuilt, what will not be imported,
and how the result can be validated without risking the FM-1.

---

## 1. Product definition

**AMARU OS** is the product name of the firmware; **BUNKER ADVANCED** is the
instrument/UI identity inside it.

It is a new FM-1 firmware built on the Bunker Evolved tree, with:

- Bunker Evolved navigation, four tracks, 64-step sequencer, eight shared voices,
  GROW and EVOLVE preserved.
- A deterministic generator with `GENRE`, `SEED`, `ENERGY` and `FORM`
  (`LOOP` / `PARTS` / `SONG`), at least:
  tribe tekno, mental tekno, hardtekk, acidcore, jungle, drum & bass,
  breakbeat tekno, techno, psytrance.
- Genre-specific drum grammar, bass behaviour and arrangement. Jungle, drum & bass
  and breakbeat must not be four-on-the-floor.
- Bunker-native sound recipes for the requested tekno/psy/ambient palette:
  mental pads, tomb drones, sirens, wavetable ghosts, ritual choirs, metal blooms,
  cave pads, psy swarms, acid mists, dark textures, tribe/tekno basses, hardtekk
  stabs, acidcore voices, psy rolling basses/leads, jungle/DnB sub basses and
  breakbeat textures.
- An original monochrome retro-terminal visual layer with three themes:
  green/black, black/white and amber/black.
- No Carnac, Mycelium, JIANT, Bunker, Fallout or Classic Mac binaries/brand assets
  copied in. Character is rebuilt from Bunker's own engines, recipes and drawing
  primitives.

Non-negotiable constraints:

1. Do not brick the FM-1. Keep a tested stock rollback path.
2. Preserve Bunker's workflow. Extend; do not remove functions.
3. Generated material must remain editable by GROW/EVOLVE and by the normal
   step/sound editors.
4. Seed must be deterministic and must survive regeneration.
5. All source licences must remain compatible and attributed.

---

## 2. Sources and provenance

| Source | Local revision | Licence | Role |
|---|---|---|---|
| Bunker Evolved 1.0 | `c9bb0b1` | GPL-3.0-only | Base firmware, UI, sequencer, engines, storage, recovery |
| JIANT tekno fork | `2f16bec` (fork of JIANT `dc8d025` base) | GPL-3.0-only | Genre generator ideas, tekno recipes, tests |
| Carnac OS 0.9.4 | `32369d3` | GPL-3.0 | Wavetable/FX/voice/UI reference; concepts only |
| Mycelium FM-1 1.0 | `750eb12` | GPL-3.0-only | Textures, punch FX, arranger, recovery reference |
| Classic Mac icons | `thomasareed/Classic-Mac-icons` `main` | **No licence found** | **Rejected** (see §7) |

All four firmware sources are GPL-3.0(-only), so a GPL fusion is licence-compatible.
Attribution and licence texts are kept in the repository root and in
`docs/ATTRIBUTION.md`.

Bunker already vendors Felucca-Engines (MIT), msfa (Apache-2.0), CC0 samples and
OFL fonts. Those notices stay with the Bunker tree and must not be removed.

---

## 3. Bunker Evolved — the base

### 3.1 What it is

- Four parts/tracks (`NTRK = NPART = 4`), each with its own engine, preset,
  parameters, eight voices and a 64-step pattern. The eight voices are a shared
  budget.
- 14 engine slots: ANALOG, reserved DIGITAL, PHASE, LOFI, SAMPLE, VOICE/FORMANT,
  TRIO, WHEEL, GRAIN, PHYS, DRUM, NOISE, FM6, SLICE. Engine numbers are store/protocol
  indices and are append-only.
- Four project slots, 114 parameters per project, A/B autosave, song chain,
  GROW/EVOLVE, worlds/sounds/motifs.
- Bunker home pages: `WORLD`, `GROW`, `SOUND`, `MOTIF`, `EVOLVE`, `MIX`, with four
  encoders and OCT+/OCT- actions.
- Existing UI palettes include `GREEN`, `AMBER` and `MONO` (black and white).
  `GREEN` and `AMBER` still use a contrasting accent, so they are not strictly
  monochrome today.

### 3.2 Why it is the base

- It already has the four-track workflow the user wants to keep.
- Its sequencer is the actual audio data model (GROW/EVOLVE edit `step_t`), so a
  generator that writes normal steps is automatically compatible.
- Its storage, bootguard and OTA path are mature and tested.
- Its engines already cover analog, FM6, phase distortion, formant/choir, grain,
  physical modelling, noise, drum synthesis and slicing. That is enough to rebuild
  the Carnac/Mycelium character natively.

### 3.3 Baseline build (verified locally)

Command (one core, as recommended for Bunker):

```sh
python3 firmware/tools/gen_bunker_library.py
cd firmware
JIELI_TOOLCHAIN=/home/maltomare/.jieli/toolchain \
AC79_SDK=/home/maltomare/fw-AC79_AIoT_SDK \
FELUCCA_CPU2=0 sh ./build.sh
```

Observed:

| Metric | Value |
|---|---|
| App image | 527,692 B |
| Loader | 6,934 B |
| Package | 610,090 B, identity `FM-1_7100002` |
| RAM `.data + .bss` | 61,156 B of 73,728 B (12,572 B free) |
| Pool | 328,924 B of 368,640 B (39,716 B free) |
| XIP region | `0x8DFBC` = 581,564 B |
| App-image headroom | ≈ 53,872 B |

The exact package headroom depends on the installer/update-loader envelope. Treat
**40 KB of app-image growth as the working ceiling** for AMARU, not 53 KB. RAM and
pool are tighter and must be measured after each milestone.

### 3.4 Storage, update and recovery — what must be preserved

- Updates use the M-UPGRADE SysEx path. The updater checks the UFW header CRCs,
  board flash length (`OTA_FL_LEN = 0x93000`), the update loader, stages the
  loader outside the store area, verifies the staged image by read-back, and only
  commits after the host's explicit success message. A refused package is rejected
  before any erase.
- `bootguard` is RAM-only: two failed boots within the first 30 s fall back to
  UBOOT; it cannot rescue an image that fails before the application starts.
- OCT- + OCT+ held for 5 s enters UBOOT; OCT- alone at power-on enters the
  application-level rescue path (USB installer, audio off).
- Four project slots; autosave may replace old working data. AMB/Wolf projects
  are not migrated. Users must save before installing a different firmware.
- Stock V15 rollback remains the external installer/UBOOT path. AMARU must ship
  with the stock image and checksum documented, and must never erase the
  bootloader.

**Rule for this project:** no hardware flashing until host build, native tests,
emulator boot, update-package verification and a rollback rehearsal are complete.

---

## 4. JIANT tekno fork — generator analysis

### 4.1 What was added by the user

Compared with upstream JIANT base `dc8d025`, the fork changes:

- `firmware/src/gen.c`
- `firmware/src/gen_tables.c`
- `firmware/src/eng_analog.c`
- `firmware/src/ui_draw.c`
- `firmware/src/ui_menu.c`
- `tests/gen_test.c`
- `web/editor.html`

It adds six tekno-family genres in the slots of removed pop/jazz genres, several
new ANALOG presets, and a tekno articulation/envelope pass.

Current genre table (0-based index):

```
26 TRIBE TEKNO   30 BREAKS          31 DRUM & BASS
33 TEKNO         34 MENTAL TEKNO    35 ACID CORE
36 HARDTEKK      42 PSY TRANCE
```

The fork has **no genre named JUNGLE**. Upstream also had no JUNGLE genre; it only
had a `BA_JUNGLE` bass archetype. AMARU must add a first-class JUNGLE profile and a
first-class BREAKBEAT TEKNO profile (BREAKS exists but is not named/parameterised
as breakbeat tekno).

### 4.2 Verified runtime reachability problems

These are real defects in the current fork, not style opinions.

1. **Candidate cap hides the last ANALOG preset.**
   `gen_sound()` collects matching presets into `any[32]`, but
   `ANALOG_PRESETS` now has **33** entries. Index 32 (`MENTAL RING`) is never
   considered. It is one of the newly added tekno presets.
   Consequence: `SR_MENTAL` and `SR_MENTAL_BASS` cannot reach their ANALOG
   `MENTAL RING` branch.

2. **Several recipe words do not exist in the engine the recipe routes to.**
   The selector picks an engine from the recipe's three weighted choices, then
   searches that engine's preset names. If no word matches, it silently falls back
   to any preset. The result still gets the tekno envelope, so the current tests
   pass even though the intended sound was never used.

   | Recipe | Intended words / engines | Broken branches |
   |---|---|---|
   | `SR_ACIDCORE` | CORE 303 / RESO / FM PLUCK, engines A,A,PH | `RESO` not in ANALOG; `FM PLUCK` not in PHASE |
   | `SR_TEKK_BASS` | TEKK BASS / HARD BASS / GROWL, A,A,A | `HARD BASS`, `GROWL` not in ANALOG |
   | `SR_PSY_LEAD` | PSY LEAD / PD LEAD / FM PLUCK, A,A,PH | `PD LEAD` not in ANALOG; `FM PLUCK` not in PHASE |
   | `SR_TEKNO_BASS` | TEKNO BASS / FM BASS / SYN BASS, A,A,A | `FM BASS`, `SYN BASS` not in ANALOG |
   | `SR_MENTAL_BASS` | FM BASS / MENTAL RING / FB BASS, FM,A,PH | ANALOG `MENTAL RING` capped; only FM/PH branches resolve |
   | `SR_MENTAL` | MENTAL SIREN / MENTAL RING / ICE BELL, A,FM,PH | FM `MENTAL RING` missing; PH `ICE BELL` missing |
   | `SR_PSY_BASS` | PSY BASS / REESE / FB BASS, A,A,A | `FB BASS` not in ANALOG |

   `SR_TRIBE_BASS`, `SR_TEKK_STAB`, `SR_TEK_ARP` and `SR_PSY_ARP` resolve
   correctly.

3. **Tests do not catch the above.**
   The new tests check envelope releases, 4/4 kick density and absence of empty
   downbeats. They do not assert that the recipe's named preset was actually
   selected. `GEN_NEVER` does not exclude the new names, and "MENTAL" is not
   accidentally matched by "METAL".

4. **Tests cover only the six tekno genres.** `GG = {27,34,35,36,37,43}` maps to
   `TRIBE TEKNO, TEKNO, MENTAL TEKNO, ACID CORE, HARDTEKK, PSY TRANCE`.
   JUNGLE, DRUM & BASS and BREAKS have no dedicated non-4/4 regression test.

5. **JIANT has six tracks; Bunker has four.** A direct port is impossible without
   changing Bunker's identity, which the brief forbids. The JIANT role model
   (drums, bass, chords/pad, arp, lead, texture) must be compressed to four layers.

### 4.3 Decision for AMARU

- Reuse the **genre concepts, drum/bass grammar, articulation ideas and test
  intent**, not the six-track engine.
- Fix reachability at the source of truth (candidate cap and recipe word/engine
  agreement) if any JIANT code is ported. Do not paper over it in tests.
- Map four Bunker layers explicitly:

  | Bunker layer | AMARU generator role |
  |---|---|
  | `AIR` | pad / chord / texture |
  | `MELODY` | lead / arp / rave stab |
  | `ROOT` | bass / sub |
  | `SPARK` | drums / percussion |

  This keeps the visible four-layer workflow and gives every generated piece a
  complete rhythm section. A future optional mode can put an arp on `SPARK` and
  drums elsewhere, but the default must remain four tracks.

- Add JUNGLE and BREAKBEAT TEKNO as first-class profiles. Keep DRUM & BASS and
  BREAKS, reparameterised for AMARU.
- Every generated step must be a normal `step_t`, so GROW/EVOLVE work unchanged.

---

## 5. Carnac OS — what to learn, what not to import

Carnac is a wavetable synthesizer. Relevant facts from its source/docs:

- 15 wavetable banks × 8 morph frames (the header reserves 16 bank slots).
- 512-sample Q15 cycles, seven band-limited mip levels per frame.
- One gain per bank; morphing does not change level.
- The bank image is one image in Felucca store flash (`NUM_BANKS × 16 KiB` =
  256 KiB for 16 slots), uploaded separately over USB-MIDI and read in place.
- Seven voice architectures including STRIKE, four filter types, three modulation
  slots, spring/tomb/tank reverbs, tape and bucket-brigade delays, two layers,
  step sequencer, arpeggiator.

**Why AMARU will not import Carnac binaries:**

- The store-flash image format and upload path are Carnac-specific.
- 256 KiB is a large, separate flash commitment and cannot be justified before a
  measured AMARU budget and a recovery test.
- Copying the wavetable bank as a binary would also copy its authored character
  too closely.

**What AMARU rebuilds natively:** the *character* of moving spectra and haunted
timbres using Bunker's PHASE, FM6, GRAIN, FORMANT, NOISE, PHYS and ANALOG engines,
with slow LFO/filter motion, chorus/delay/reverb sends and purpose-built presets.
Names like "wavetable ghost" describe a recipe, not a Carnac asset.

---

## 6. Mycelium FM-1 — what to learn, what not to import

Mycelium contributes:

- Six 32-step grooves: FOREST JAM 156, PSY ROLL 148, PSY GALLOP 146,
  GOA ACID 144, HITECH 196, TEKK 170.
- Four 240-bar procedural factory songs; arranger scenes; scene snapshots in RAM.
- PUNCH-IN FX: 16 held-key effects (loops, stutter, reverse, tape stop, half speed,
  filters, crush, gate, echo, wobble), beat-synced and crossfaded.
- A/B sector-pair storage with CRC and commit-header-last; application-level
  recovery (`SLOOP USB RESCUE`) and a RAM-only bootguard.
- A soundbank JSON that names patches by role (bass/keys/organ/pluck/stab/FX).

**Not imported:** Mycelium brand assets, samples or binaries. Some legacy sample
and brand rights were not verified for public redistribution. The soundbank names
are useful as a role taxonomy only.

**What AMARU rebuilds natively:** the organic/psy texture vocabulary through
Bunker recipes (grain clouds, formant choirs, noise beds, physical metal, FM
blooms), plus a small beat-synced performance FX layer if the CPU/pool budget
allows. The arranger concept can be expressed with Bunker's existing song chain
rather than copying Mycelium's scene engine.

---

## 7. Visual identity and the icon licence problem

Requirements: monochrome retro-terminal; green/black, black/white, amber/black;
icons adapted from `thomasareed/Classic-Mac-icons` **after a licence check**.

Finding: that repository has **no licence metadata**, no `LICENSE` file
(`raw.githubusercontent.com/.../LICENSE` returns 404), and its README contains no
grant of rights. "Enjoy!" is not a licence. Redistributing adapted pixels in a
GPL firmware would be a copyright risk.

**Decision:** do not use or adapt any pixel from that repository. AMARU will use an
original icon set drawn for the FM-1, in the same classic-GUI spirit (folder, disk,
wave, skull, seed, bolt, etc.), generated from AMARU's own vector/pixel source.
This is recorded as a deliberate rejection, not an oversight.

Themes:

- `MONO` (existing Bunker black/white) satisfies the B&W requirement.
- Add strict monochrome `TERM GREEN` and `TERM AMBER` palettes: every UI token is a
  tint of one hue, including the accent, unlike Bunker's current colourful
  GREEN/AMBER palettes.
- Keep palette indices append-only. Saved palette ids are indices.

---

## 8. AMARU architecture

### 8.1 Layers

```
UI (Bunker home pages + GEN page)
  └─ generator planner (pure, host-testable)
       └─ apply layer (writes Bunker step_t / track_t)
            ├─ GROW / EVOLVE / SEQ / SOUND / PROJECT (unchanged)
            └─ engines + FX (Bunker-native)
```

### 8.2 Generator

New files:

- `firmware/firmware/src/amaru_gen.h` — plan types and genre constants.
- `firmware/firmware/src/amaru_gen.c` — pure planner: deterministic, no hardware
  calls, no allocation, no Bunker globals.
- `firmware/firmware/src/bunker_amaru.c` — Bunker adapter: applies a plan to the
  four tracks and the song.

Plan output:

- genre, seed, energy, form, BPM, root, scale, step length;
- four tracks in Bunker layer order `AIR, MELODY, ROOT, SPARK`;
- per-track steps (`n`, `note[4]`, `time`, `flags`, `vel`, `hit`, `acc`,
  `probability`);
- per-track sound recipe (engine, preset, tone, attack, release, chorus, delay,
  reverb, drive, voice).

Determinism:

- 32-bit xorshift seeded from `seed ^ genre ^ form ^ energy`, fixed constants.
- No `rand()`, no time, no audio RNG, no uninitialised memory.
- Same inputs ⇒ byte-identical plan.
- Regeneration keeps the seed unless the user explicitly changes it.

Form:

- `LOOP`: one 16-step bar (track length 16).
- `PARTS`: 32 steps = A/B.
- `SONG`: 64 steps = A/B/A/C with genre-appropriate fills.

This is a bounded in-pattern arrangement, not a chain editor. It keeps the
generator inside Bunker's four-track/64-step data model and therefore inside
GROW/EVOLVE. A later milestone may write `chain_config_t` sections for a longer
song, but that is not required for the first generator.

### 8.3 Genre grammar

Each profile defines:

- tempo range, root, scale, default energy, swing;
- 16-step masks for kick, snare, clap, closed hat, open hat, percussion;
- kick mode (four-on-floor, two-step, broken, interlock);
- bass mode (sub, offbeat, roll, acid, stab);
- pad/lead behaviour and sound choices;
- fill/ghost rules.

Jungle/DnB/breakbeat profiles are explicitly **not** four-on-the-floor:

- Jungle: broken kick (0, 6, 10 in a 16-step bar), ghost snares, sub bass.
- Drum & bass: two-step kick (0, 10), snare on 4/12, 16th hats, long sub.
- Breakbeat tekno: syncopated kick (0, 3, 6, 10, 13), broken snares, rave stab.

### 8.4 Sound bank

First generator pass uses Bunker's existing factories (ANALOG, FM6, PHASE,
FORMANT, GRAIN, PHYS, NOISE, DRUM) plus tone/A/R/FX edits. A later pass adds new
Bunker-native presets for the named palette. No new engine and no wavetable image
in the first pass.

### 8.5 UI

- Add a `GEN` home page after `MIX`.
- Four encoders: `GENRE`, `SEED`, `ENERGY`, `FORM`.
- `OCT+` = generate; `OCT-` = new seed. Both refuse while playing/recording.
- Keep every existing page and action.
- Show the active genre and form; show deterministic seed.
- Add three themes; keep `MONO` as the B&W theme.

### 8.6 Failure/undo policy

- Generation is a whole-instrument action, like loading a world.
- It clears the one-track undo rather than leaving a misleading partial undo.
- It does not write projects or user presets.
- It refuses while transport/recording/song chain is active.
- It calls `sync_reload` so the editor re-reads the new state.

---

## 9. Implementation milestones

**M0 — analysis and repository (this commit)**
- Import baseline, document sources, findings, licences, budgets, safety.

**M1 — deterministic generator core**
- Pure planner + host tests.
- Nine genre profiles, four-layer mapping, three forms.
- Tests: determinism, all profiles valid, non-4/4 for jungle/DnB/breakbeat,
  energy monotonicity, seed sensitivity, GROW/EVOLVE compatibility.

**M2 — Bunker integration and UI**
- Apply layer, `GEN` page, OCT+/OCT-.
- Build; native checks; emulator boot; verify 4 tracks and eight-voice budget.

**M3 — Bunker-native sound bank**
- Add AMARU recipes using existing engines/presets; extend presets only where a
  name is needed and only append-only.
- Metal blooms, tomb drones, ritual choirs, cave pads, psy swarms, acid mists,
  wavetable ghosts, sirens, hardtekk stabs, acidcore voices, sub basses.

**M4 — visuals and character**
- Strict monochrome themes and original icon set.
- Motion/FX pass for Mycelium/Carnac character within CPU/pool budget.

**M5 — release gate**
- Full test matrix, emulator/video evidence, package verification, rollback
  rehearsal, ROM/stock image and checksums, install guide.

---

## 10. Verification plan

Host and firmware checks:

- `python3 tests/run_bunker_checks.py`
- `build/host/bunker_runtime smoke|stress 3600`
- `build/host/bunker_grow_test`
- `python3 tests/bunker_audio_bank.py`
- New `tests/amaru_gen_test.c` (pure planner): determinism, validity, genre rules,
  four-layer coverage, form lengths.
- Build with the documented toolchain; record app/RAM/pool deltas.
- Emulator: `web/emu/build.sh`, native check, WASM test.
- Update package: size/SHA-256/identity; install refusing wrong board; stage and
  rollback in the emulator/host model.
- Hardware: only after all of the above, with the stock image and installer ready.

Required 9-question release gate (must be answered explicitly before saying the
work is complete):

1. Does it build? **Yes.** Baseline BunkEEvolved `c9bb0b1` app **527,692 B** vs AMARU
   fused app **534,188 B** (+6,496 B); loader 6,934 B; package 610,090 B,
   identity `FM-1_7100002`; RAM `.data + .bss` **61,156 B** of 73,728 B;
   pool 328,924 B of 368,640 B. Growth measured after the generator +
   terminal + GEN-page slice; still inside the **40 KB working ceiling**
   (baseline app-image headroom ≈ 53,872 B).
2. Do all tests pass? **Yes** at this slice: `tests/run_bunker_checks.py` (19
   suites), new `tests/amaru_gen_test.c`, `tests/theme_test.c`, `tests/ui_test.c`
   (incl. GEN application, non-4/4 JUNGLE, GROW/EVOLVE on generated data), 
   `build/host/bunker_runtime smoke` / `stress 3600`, `build/host/bunker_grow_test`,
   `python3 tests/bunker_audio_bank.py`, and the native emulator check
   (`build/emu/native_check build/emu/native.f32`) all green. Only known failures
   are the pre-existing `ui_render` splash *expectation* mismatch (the emulator's
   `ui_render.c` still expects upstream Felucca splash text) and the WASM emulator
   build (`emcc` not installed in this host).
3. Does every genre produce valid, genre-appropriate output? **Yes for this slice.**
   The pure planner is deterministic and validates all 9 AMARU genres and all 3
   forms (LOOP/PARTS/SONG), four layers, energy-bounded output; generated drums
   are genre-appropriate (JUNGLE/DNB/BREAKBEAT broken-kick masks verified
   off-quarter/4th-density).
4. Is the UI navigable and are old functions reachable? **Yes for the added path.**
   GEN page wired through K1-K4 + OCT+; GROW/EVOLVE verified against generated
   tracks (GROW applies/undo, EVOLVE mutates). Old pages untouched.
5. Does the emulator boot and play? **Partially.** Native emulator check boots the
   real `felucca_web.c` and plays the heavy test song: green (705,600 B output).
   WASM build deferred (no Emscripten toolchain on this host).
6. Is recovery/update safety verified? **Not re-run for the new package** (deferred to
   hardware phase). OTA/bootguard code paths were not touched; stock V15 remains the
   rollback target.
7. Is rollback verified? **Not re-run for the new package** (deferred). No new binary
   was flashed; hardware phase untouched.
8. Are flash/RAM/CPU budgets within limits? **Yes.** Flash +6,496 B app, RAM/pool
   unchanged; generator is UI-time only (no audio-ISR cost). Stress run: voices
   <= 8, all outputs finite, peak 8,675, RMS 713.65 over 3600 s.
9. Is brick risk acceptably low, with a recovery path? **Yes for this change set.**
   No new engines, no DSP, no binary imports, no flash writes added; deterministic
   planner; stock rollback intact.

Until all hardware/validation prerequisites are closed, nothing is declared
release-ready; the FM-1 must not be flashed here.

---

## 11. Risks and mitigations

| Risk | Mitigation |
|---|---|
| Flash growth | 40 KB working ceiling; measure every milestone; no wavetable image |
| RAM/pool growth | Pure planner has no allocation; plan struct is small; measure |
| Audio CPU | Generator is UI-time only; no audio-ISR work; no new engines |
| Six→four track loss | Explicit role mapping; tests require all four layers active |
| Recipe fallback | Test preset identity, not only envelope; keep names in the right engine |
| Seed drift | Pure deterministic planner; same-input byte comparison test |
| GROW/EVOLVE breakage | Generator writes ordinary steps; run GROW/EVOLVE tests on generated data |
| UI regression | Additive page; existing pages untouched; emulator + 9-question gate |
| Project overwrite | Generator never saves projects; warn/stop during play/record |
| Icon licence | Rejected unlicensed set; original icons only |
| Brick | No hardware flash until build/emulator/rollback evidence; stock V15 rollback |

---

## 12. What is explicitly not being done

- No Carnac wavetable image or PCM binaries.
- No Mycelium samples, logos, brand assets or groove binaries.
- No JIANT six-track engine forced into Bunker's four tracks.
- No Classic Mac icon pixels.
- No copying of Fallout/Bunker/Carnac/Mycelium visual identity.
- No hardware flashing in this phase.
- No new engine or real-time DSP in the audio ISR.

---

## 13. Attribution

AMARU OS is a GPL-3.0-only derivative. It must keep:

- Bunker Evolved / Felucca copyright and GPL notices.
- Felucca-Engines MIT notice.
- msfa Apache-2.0 notice.
- CC0 sample and OFL font notices.
- JIANT copyright/GPL notice for reused generator concepts and tables.
- Carnac OS and Mycelium FM-1 copyright/GPL notices for referenced source
  concepts, even where no code or binary is copied.

See `docs/ATTRIBUTION.md` for the per-source statement.
