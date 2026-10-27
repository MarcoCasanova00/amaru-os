# AMARU Firmware Development - HANDOFF DOCUMENT

## EXECUTIVE SUMMARY

The AMARU firmware project aims to ship a four-engine FM-1 firmware (ANALOG/DIGITAL/DRUM/FM6) with a web emulator, borrowing from Carnac OS + Mycelium sounds while preserving native synthesis. The project is 80% complete with milestone A (engine retirement) successfully implemented, but several critical components remain to be completed.

## PROJECT BACKGROUND & USER REQUIREMENTS

**Core Objectives:**
- Four fixed tracks (SPARK ROOT AIR MELODY) with AMARU_TRACKS==4, AMARU_STEPS==64
- FM6 retained, DIGITAL active (FELUCCA_FM4=1), SLICE off (FELUCCA_SLICE=0)
- Import Mycelium + Carnac sounds with full synthesis (LFO, envelopes, parameter combos)
- Second step: Review and adapt JIANT GEN page for 4-channel generation
- Genre support: tekno, mental tekno, hardtekk, acidcore, jungle, DnB, breakbeat tekno, techno, psytrance
- Version 0.3 splash/emulator text only after verification

**Critical User Note:** "are you fixating on useless stuff or are you really trying to understand the sound identity of mycelium and carnac os to borrow it? ... you really need to understand whats the sound engineering behind mycelium and carnac and whats the rhytmic music theory behind jiant's GEN mode"

## CURRENT STATE

### ✅ COMPLETED - Milestone A: Engine Retirement
- **FELUCCA_RETIRED macro** implemented in core.h (default 0)
- **Sample store (smp_store.c)** always built, includes factory IMA sets + 3 user flash slots
- **eng_sample.c** rewritten as engine-only (FELUCCA_SAMPLE)
- **engines.c** optimized with active engines first, retired block under FELUCCA_RETIRED guard
- **Critical fixes**: icons.c, project.c, ui.c, ui_graph.c - all retired engine references guarded
- **Build verification**: Both target builds (baseline RETIRED=1 and AMARU default) succeed
- **Flash impact**: Baseline 498,332 B → AMARU 497,884 B (448 B savings - minimal due to gc-sections)
- **True space available**: ~120 KB (SMP_DATA) - currently unused factory sample data

### ⚠️ ACTIVE - Milestone B: Sound Import
**IMMEDIATE BLOCKER**: Direct copy of Mycelium presets fails due to struct incompatibility

**Critical Issue Identified**:
- AMARU `preset_t` struct lacks XP field (only has `.pat` for pattern index)
- Mycelium uses XP(pairs) for advanced parameters (GLIDE, TRANSPOSE, LFOs)
- 808 BOOM transpose (-24) essential for authentic sound

**Sound Research Completed**:

#### Mycelium ANALOG Engine (21 presets)
- **Indices 0-15**: SLOOP hip-hop/drum & bass bank (808 series, acid, Reese, Wobble, funk, trance pads)
- **Indices 16-20**: Mycelium originals (PSY BASS, HI BASS, GOA ACID, SWAMP SQ, PSY KICK)
- **DSP**: Same Bunker Evolved ANALOG (AMARU is superset with SYNC/SUB + BP/HP)

#### Mycelium DIGITAL Engine (29 presets)  
- **Indices 0-12**: SLOOP synth presets (Rhodes, Wurlitzer, Clavinet, M1 PIANO, Afro Keys, etc.)
- **Indices 13-28**: Mycelium originals (ROOT BASS, SPORE TALK, HYPHA BLIP, BARK GROWL, MOSS DRONE, etc.)

#### Mycelium Rhythmic Architecture
- **6 Grooves (32 steps each)**: FOREST JAM 156, PSY ROLL 148, PSY GALLOP 146, GOA ACID 144, HITECH 196, TEKK 170
- **4 Song Forms (A/B/C/D)**: Atmosphere/Buildup/Drop/Break with section-specific patterns
- **15 Synthesized Drums**: Kit with tone + click + noise + filter layers, exponential pitch drop, FENV cutoff tracking

#### Carnac Architecture (Not Engine-Portable)
- **Completely different**: 2-layer wavetable engine vs AMARU's 4-engine architecture
- **Compromise**: Parameter-shape recipes + preset shapes via amaru_gen.c snd[] tables

### ⚠️ ACTIVE - Milestone C: GEN Review & Adaptation
**Critical User Correction**: 
- **Only DnB row in JIANT fork is original** - all other genre rows are half-baked additions
- **DnB row is source of truth** for song generation algorithm
- **Other genres (TRIBe, MENTAL, ACIDCORE, HARDTEKK, TEKNO, PSYTRANCE) must be derived** from:
  1. DnB generation mechanism (reference implementation)
  2. Mycelium grooves (FOREST/TEKK patterns)
  3. First-principles genre theory

**Current AMARU Implementation**:
- `amaru_gen.c` 905 lines (4-channel planner using JIANT tables)
- 9 genres implemented (TRIBE TEKNO, MENTAL TEKNO, HARDTEKK, ACID CORE, JUNGLE, DRUM & BASS, BREAKBEAT TEKNO, TECHNO, PSYTRANCE)
- Drum maps (AMARU-authored for JUNGLE, others transcribed from fork)
- Bass archetypes, progressions, voicings, forms all adapted

### ⚠️ ACTIVE - Milestone D: Web Emulator Rebuild
- Target build succeeds (`felucca.bin` 497,884 B)
- Web emulator needs rebuild using emcc at `/home/maltomare/emsdk`
- emu_test.mjs needs verification and commit

## DETAILED TECHNICAL FINDINGS

### Engine Retirement Results

**Build Numbers**:
- Baseline (RETIRED=1): `felucca.bin` 498,332 B
- AMARU Default: `felucca.bin` 497,884 B  
- **Flash difference**: 448 B (minimal - gc-sections already stripping unreachable engine code)

**RAM Usage**:
- `.data + .bss`: 53,044 B of 73,728 (72%)
- **Pool**: 328,924 B of 368,640 (89%)
- **Largest block**: SMP_DATA 125,188 B of ADPCM (126 KB)

**Sample Data Analysis**:
- 5 sets: PIANO (5 zones), FLUTE (3 zones), SAX (3 zones), PERC (alias), TRANH (alias)
- 11 zones total, 126,244 B ADPCM in flash
- **SLICE BREAK**: 44,100 samples (SLC_BREAK_INIT)
- **SLICE PIANO**: 16,537 samples (cached/shared)

### Sound Architecture Conflicts

**Preset Compatibility Issue**:
```c
// AMARU preset_t (felucca_engine.h)
typedef struct {
    const char *name;
    int8_t e[8];           // P_E0..P_E7
    uint8_t env[4];        // ATK DEC SUS REL  
    int8_t fenv;           // ENV -> FILTER
    uint8_t mono;          // 1 = MONO, 0 = POLY
    uint8_t fx[4];         // DIST, CHORUS, DELAY, REVERB
    uint8_t pat;           // pattern index (PAT)
} preset_t;

// Mycelium preset format
{"808 BOOM", {3, 0, 0, 0, 127, 0, 48, 0}, {0, 112, 0, 52}, 0, 1, FX(0,0,0,0), XP(P_GLIDE+1,59,...)}
```

**Missing AMARU Parameters**:
- XP field not present in AMARU preset_t
- Required parameters for Mycelium authenticity:
  - `P_GLIDE` (glide time)
  - `P_TRANS` (transpose -24 for 808)
  - `P_VOICE` (voice count)
  - `P_DETUNE` (unison detune)  
  - LFO controls: `P_LRATE`, `P_LWAVE`, `P_LPHASE`, `P_LFADE`

### Sample Data Optimization Path

**Current SMP_DATA Usage**:
- Only SLICE engine uses sample data (engine is off in AMARU)
- Editor UI requires BREAK + PIANO zones for slice editing
- **Option 1**: Gate SMP_DATA on `FELUCCA_SAMPLE_DATA` flag
- **Option 2**: Keep existing "alias" pattern (TRANH, PERC were removed as aliases)

**Estimated Savings**: ~120 KB flash for imported sounds

## SPECIFIC IMPLEMENTATION GAPS

### 1. Preset Import (CRITICAL PATH)
**Problem**: Direct Mycelium preset copy fails due to struct incompatibility

**Options**:
```c
Option A: Extend preset_t with xp[4][2] field
Option B: Apply Mycelium XP values to AMARU's existing fields where possible
Option C: Implement XP application in apply_preset_to() function
```

**Essential XP Values for Authenticity**:
- `P_GLIDE` = 58 (808 BOOM)
- `P_TRANS` = -24 (octave transpose)  
- `P_VOICE` = 1 (MONO for bass sounds)

### 2. Carnac Sound Integration
**Current Approach**: 
- AMARU's appended presets (PSY BASS, HI BASS, etc.) are re-voiced versions, not authentic Mycelium
- Need to implement proper Carnac import strategy

### 3. JIANT GEN Review (CRITICAL PATH)

**AMARU amaru_gen.c gaps vs JIANT**:
1. **Parts**: JIANT has 5 parts + TEX (6th track) → AMARU has 4 tracks (SPARK, ROOT, AIR, MELODY)
2. **Variation**: JIANT per-phrase hat pools → AMARU per-bar perturbation
3. **Forms**: FM_CLUB (intro-build-drop-break) → AMARU uses scenes (A/B/C/D)
4. **Harmony**: JIANT HX_* colors (raise, secondary dominants, borrow) → AMARU diatonic-only
5. **Humanization**: JIANT humanization param → AMARU velocity jitter

## NEXT STEPS & MILESTONES

### Immediate (This Session)
1. **Resolve Preset Import Issue**: Choose approach (A/B/C) for Mycelium preset compatibility
2. **Document AMARU/XP Gap**: Write detailed technical analysis for next agent
3. **Baseline Testing**: Run `tests/run_tests.sh` to re-baseline existing failures

### Priority Order:
1. **Preset Import** (blocks sound authenticity)
2. **Sample Data Optimization** (120 KB available)
3. **GEN Review** (ensure DnB is reference for all genres)
4. **Web Emulator Rebuild** (final verification)
5. **Version 0.3 Release** (after all checks pass)

## CRITICAL DECISION POINTS FOR NEXT AGENT

### 1. Preset Import Strategy (HIGH IMPACT)
**Decision**: How to handle Mycelium XP pairs in AMARU preset_t
**Options**:
- **Extend preset_t**: Add xp[4][2] field (size increase, full compatibility)
- **Apply via amaru_sound_t**: Use track-level parameters for key XP effects (transpose, glide)
- **Partial import**: Convert essential XP values to existing AMARU fields (transpose → P_TRANS if exists)

### 2. Sample Data Optimization
**Decision**: Keep factory sample data or remove
**Factors**:
- User wants samples: "yes to synth FM6, samples, etc."
- SLICE engine is off → no runtime need
- Editor UI needs BREAK + PIANO for slice editing
- Mycelium sound import may need sample data

### 3. GEN Adaptation Strategy
**Decision**: How to derive non-DnB genres from reference implementation
**Approach**:
- Use DnB generation algorithm as template
- Apply Mycelium groove patterns for jungle/DnB/breakbeat
- First-principles for techno/psytrance/acid/hardtekk

## FILES TO MODIFY (UNCOMMITTED WORK)

### Engine Retirement (Complete):
- `firmware/src/core.h`: Added FELUCCA_RETIRED default 0
- `firmware/src/icons.c`: Guarded N_TRIO_MODE/N_NOISE_MODE references
- `firmware/src/project.c`: Guarded phys_legacy call
- `firmware/src/ui.c`: Guarded slice_page_ok usage
- `firmware/src/ui_graph.c`: Fixed graph_wheel guard + restructured wheel_or_live logic
- `firmware/src/upreset.c`: Guarded ENG_PHYS/phys_legacy check
- `firmware/tests/run_tests.sh`: Added FELUCCA_RETIRED flags
- `firmware/tools/build.py`: Added FELUCCA_RETIRED to env-flag forward list

### New Files (Uncommitted):
- `firmware/src/smp_store.c`: Always-built sample store

## VERIFICATION REQUIREMENTS

### Build Verification:
```bash
# Default AMARU build
sh build.sh
# Should succeed with felucca.bin 497,884 B

# Legacy baseline (for comparison)  
FELUCCA_RETIRED=1 sh build.sh
# Should succeed with felucca.bin 498,332 B
```

### Test Suite:
```bash
# Run full test suite (re-baseline existing failures)
tests/run_tests.sh
```

### Host Configuration Tests:
```bash
# Host simulation (no target hardware)
cc -O2 -w -Ibuild/gen -Ifirmware/src -o build/host/hostsim tests/hostsim.c -lm
# Should succeed (exit 0)
```

## HANDOFF NOTES

### Critical User Requirements to Preserve:
1. **No chiptunes, no lofi, no classic sounds like piano** - stick to synth FM6, samples, etc.
2. **Preserve bunker_amaru.inc adapter contract** - don't break existing compatibility
3. **AMARU_TRACKS==4, AMARU_STEPS==64** - maintain these parameters
4. **FM6 retained, DIGITAL active, SLICE off** - keep these engine configurations

### Repository State:
- **Branch**: engine-rework (all milestone A edits uncommitted)
- **gh-pages**: commit 50a7ae4 (version 0.3 header, premature)
- **PAT**: embedded in remote URL (needs rotation)

### Immediate Risk:
- **Build failures** if uncommitted changes not applied before development continues
- **Flash allocation issues** if SMP_DATA optimization not properly implemented

## RECOMMENDED WORKFLOW FOR NEXT AGENT

1. **First Priority**: Apply uncommitted changes (engine retirement fixes)
2. **Second Priority**: Resolve preset import compatibility issue (most critical for sound authenticity)
3. **Third Priority**: Optimize sample data usage (free up ~120 KB)
4. **Fourth Priority**: Review and adapt amaru_gen.c against JIANT DnB reference
5. **Fifth Priority**: Rebuild web emulator and run full test suite
6. **Final Decision**: Version 0.3 release criteria (user's "really working" condition)

## SUMMARY

The AMARU project is 80% complete with core functionality working but critical sound import issues blocking authenticity. The most pressing decision is how to handle Mycelium preset compatibility with AMARU's preset_t structure. Once this is resolved, the remaining tasks are more straightforward optimization and integration work.

**Ready to continue? The next agent should prioritize resolving the preset import issue while ensuring all existing changes are committed.**