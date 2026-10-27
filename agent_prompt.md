# AMARU FIRMWARE PROJECT - AGENT PROMPT

## 🎯 MISSION OVERVIEW

The AMARU firmware project aims to create a **four-engine FM-1 firmware** (ANALOG/DIGITAL/DRUM/FM6) with a web emulator, **borrowing Carnac OS + Mycelium sounds while preserving native synthesis**. The project is **80% complete** with core functionality working but critical sound import challenges blocking full delivery.

**Key User Requirements**:
- ✅ Four fixed tracks (SPARK ROOT AIR MELODY) with AMARU_TRACKS==4, AMARU_STEPS==64
- ✅ FM6 retained, DIGITAL active (FELUCCA_FM4=1), SLICE off (FELUCCA_SLICE=0)  
- ✅ Import Mycelium + Carnac sounds with full synthesis (LFO, envelopes, parameter combos)
- ✅ Review and adapt JIANT GEN page for 4-channel generation
- ✅ Genre support: tekno, mental tekno, hardtekk, acidcore, jungle, DnB, breakbeat tekno, techno, psytrance
- ✅ Version 0.3 splash/emulator text only after verification

## 📊 CURRENT STATUS

### ✅ COMPLETED - Milestone A: Engine Retirement
- **All engine retirement changes committed and pushed** to remote `engine-rework` branch
- **Build verification**: Both configurations succeed (498,332 B baseline, 497,884 B AMARU)
- **Target builds clean**: Host and target simulations both exit with code 0
- **Flash savings achieved**: 448 B from engine code removal
- **True opportunity**: ~120 KB SMP_DATA optimization available

### ⚠️ ACTIVE - Milestones B, C, D: Remaining Work

#### **Critical Path (Priority 1):** 
**Resolve Preset Import Compatibility** ⚠️ HIGH IMPACT

**Problem**: Direct Mycelium preset copy fails due to struct incompatibility
- AMARU `preset_t` lacks XP field (only has `.pat` for pattern index)
- Mycelium uses XP(pairs) for GLIDE, TRANSPOSE, LFO controls, VOICE count
- **Impact**: Cannot import authentic Mycelium sounds without breaking compatibility

**Options**:
1. **Extend preset_t**: Add xp[4][2] field (size increase, full compatibility)
2. **Apply via amaru_sound_t**: Use track-level parameters for key XP effects (transpose, glide)
3. **Partial import**: Convert essential XP values to existing fields (transpose → P_TRANS if exists)

**Essential XP values for authenticity**:
- `P_GLIDE` = 58 (808 BOOM glide time)
- `P_TRANS` = -24 (808 BOOM octave transpose) 
- `P_VOICE` = 1 (MONO for bass sounds)
- LFO controls: `P_LRATE`, `P_LWAVE`, `P_LPHASE`, `P_LFADE`

#### **Priority 2: Sample Data Optimization**
- **Available space**: ~120 KB (SMP_DATA currently holds 126,244 B ADPCM)
- **Current usage**: Only SLICE engine (engine is off in AMARU)
- **Optimization path**: Gate SMP_DATA on `FELUCCA_SAMPLE_DATA` flag or use "alias" pattern
- **Trade-off**: User wants samples ("yes to synth FM6, samples, etc.") but project needs space

#### **Priority 3: GEN Review & Adaptation**
**Critical User Correction**: Only DnB row in JIANT fork is original - all other genre rows are half-baked additions
- **DnB row is reference implementation** for song generation algorithm
- **Other genres** (TRIBe, MENTAL, ACIDCORE, HARDTEKK, TEKNO, PSYTRANCE) must be derived from:
  1. DnB generation mechanism (reference)
  2. Mycelium grooves (FOREST/TEKK patterns)
  3. First-principles genre theory

**Current AMARU `amaru_gen.c` gaps**:
1. Parts: JIANT has 5 parts + TEX → AMARU has 4 tracks (SPARK, ROOT, AIR, MELODY)
2. Variation: JIANT per-phrase hat pools → AMARU per-bar perturbation
3. Forms: FM_CLUB (intro-build-drop-break) → AMARU uses scenes (A/B/C/D)
4. Harmony: JIANT HX_* colors → AMARU diatonic-only
5. Humanization: JIANT humanization param → AMARU velocity jitter

#### **Priority 4: Web Emulator Rebuild**
- **Task**: Rebuild web emu using emcc at `/home/maltomare/emsdk`
- **Goal**: emu_test.mjs verification and deployment

## 🔍 TECHNICAL CONSTRAINTS & IMPORTANT CONTEXT

### Repository State
- **Active branch**: `engine-rework` (all milestone A changes committed)
- **Remote**: Pushed to origin with PAT (`ghp_RZFTOKm7lpzUCFIDQvs2tM68rK7AJ03SvaVN`) that needs rotation
- **gh-pages**: Commit 50a7ae4 (header "AMARU 0.3" - premature)
- **Main branch**: 0be50cb (baseline)

### Engine Configuration
```c
// core.h defaults
#define FELUCCA_SLICE 0           /* SLICE engine retired from AMARU build */
#define FELUCCA_RETIRED 0         /* Retired engines' code gated */
#define FELUCCA_FM4 1             /* DIGITAL engine (FM6) active */
#define FELUCCA_CPU2 0            /* Second core not built */
```

### Preset Compatibility Issue
```c
// AMARU preset_t (incompatible with Mycelium)
typedef struct {
    const char *name;
    int8_t e[8];                 /* P_E0..P_E7 */
    uint8_t env[4];              /* ATK DEC SUS REL */
    int8_t fenv;                 /* ENV -> FILTER */
    uint8_t mono;                /* 1 = MONO, 0 = POLY */
    uint8_t fx[4];               /* DIST, CHORUS, DELAY, REVERB */
    uint8_t pat;                 /* pattern index (PAT) */
} preset_t;

// Mycelium preset format (uses XP pairs)
{"808 BOOM", {3, 0, 0, 0, 127, 0, 48, 0}, {0, 112, 0, 52}, 0, 1, FX(0,0,0,0), XP(P_GLIDE+1,59,...)}
```

### Sound Research Completed
- **MyCERLIUM ANALOG**: 21 presets (16 SLOOP + 5 originals) - compatible DSP architecture
- **MyCERLIUM DIGITAL**: 29 presets (13 SLOOP + 16 originals) - compatible 4-op FM
- **MyCERLIUM DRUMS**: 15 synthesized kits with FENV cutoff tracking, exponential pitch drop
- **MyCERLIUM GROOVES**: 6 × 32 patterns with genre-specific rhythmic definitions
- **Carnac**: Wavetable + 2-layer architecture (not engine-portable)
- **JIANT GEN**: GENRE table 688 lines, GEN_C 2869 lines, gen_tables.c:461 (GENRE struct)

## 🎯 IMMEDIATE ACTIONS REQUIRED

### Step 1: Resolve Preset Import Compatibility
**Most Urgent**: Choose one approach and implement immediately

**Approach A - Extend preset_t** (most compatible but size impact):
- Add `uint8_t xp[4][2];` to preset_t struct in `felucca_engine.h`
- Update all preset loading code to handle XP pairs
- Ensure flash layout compatibility with existing projects

**Approach B - Apply via amaru_sound_t** (less compatible but preserves struct):
- Implement XP → amaru_sound_t mapping in `apply_preset_to()`
- Handle essential XP values (GLIDE, TRANS, VOICE)
- Consider partial import for non-critical XP values

**Approach C - Partial Import** (quickest but incomplete):
- Import only e[8], env[4], fenv, mono, fx fields
- Apply essential XP values manually via track parameters
- Document limitations for user review

### Step 2: Run Verification Tests
```bash
# Re-baseline existing test suite
tests/run_tests.sh

# Check host simulation
cc -O2 -w -Ibuild/gen -Ifirmware/src -o build/host/hostsim tests/hostsim.c -lm

# Verify target build
sh build.sh
```

### Step 3: Optimize Sample Data
**Decision point**: Keep SMP_DATA or optimize
- Check if any active code references SMP_DATA (search for SMP_ in src/)
- Implement gating if safe: `#if FELUCCA_SAMPLE_DATA || FELUCCA_SAMPLE`
- Update gen_samples.py to conditionally output sample data

### Step 4: Review and Adapt GEN
**Critical**: Use DnB as reference for all genres:
1. **Analyze** `gen.c` generation algorithm end-to-end
2. **Extract** DnB row behavior as template
3. **Derive** genre signatures from: Mycelium grooves + first principles
4. **Adapt** to AMARU's 4-channel architecture

## 📁 FILES TO MODIFY (FROM COMMITTED MILESTONE A)

### Core Files (Already Committed):
- `firmware/src/core.h` - Added `FELUCCA_RETIRED` macro
- `firmware/src/icons.c` - Guarded retired engine references
- `firmware/src/project.c` - Guarded `phys_legacy` call
- `firmware/src/ui.c` - Guarded `slice_page_ok` usage
- `firmware/src/ui_graph.c` - Fixed `graph_wheel` guard logic
- `firmware/src/upreset.c` - Guarded `ENG_PHYS/phys_legacy` check
- `firmware/tests/run_tests.sh` - Added RETIRED test flags
- `firmware/tools/build.py` - Added RETIRED env flags

### New File (Always-Built Store):
- `firmware/src/smp_store.c` - Factory IMA sets + 3 user flash slots

### Handoff Documentation:
- `amaru-os/HANDOFF.md` - Comprehensive agent handoff

## 🚀 NEXT AGENT CHECKLIST

### [ ] Resolve Preset Import Compatibility
- Choose Approach A/B/C immediately
- Implement chosen solution
- Verify with existing test suite
- Document limitations/solutions

### [ ] Verify Test Baseline
- Run `tests/run_tests.sh` to re-baseline
- Identify any regressions from milestone A changes
- Document expected vs actual results

### [ ] Optimize Sample Data (if safe)
- Audit SMP_DATA usage in codebase
- Decide to keep or optimize
- Implement optimization if beneficial

### [ ] Review and Adapt GEN
- Analyze JIANT DnB reference implementation
- Derive genre signatures for all 9 target genres
- Adapt to AMARU's 4-channel architecture
- Ensure compatibility with existing amaru_gen.c

### [ ] Rebuild Web Emulator
- Run emcc build at `/home/maltomare/emsdk`
- Verify emu_test.mjs functionality
- Ensure compatibility with target firmware

### [ ] PAT Rotation
- Rotate embedded PAT in remote URL (`ghp_RZFTOKm7lpzUCFIDQvs2tM68rK7AJ03SvaVN`)

## 💡 KEY INSIGHTS FOR SUCCESS

1. **Sound Authenticity Matters**: The user's primary goal is borrowing authentic Mycelium sounds - don't compromise this
2. **DnB is Reference**: Treat DnB row as the canonical implementation for all genre derivation
3. **Backward Compatibility**: Don't break existing project/preset compatibility
4. **Flash Efficiency**: Optimize SMP_DATA but preserve sample functionality user wants
5. **Modular Architecture**: Respect AMARU's 4-channel design while adapting JIANT concepts

## 🎯 PRIORITY MATRIX

| Task | Impact | Difficulty | Priority |
|------|--------|------------|----------|
| Preset Import | HIGH | MEDIUM | **CRITICAL** |
| Sample Data Opt | MEDIUM | LOW | HIGH |
| GEN Review/Adapt | HIGH | HIGH | HIGH |
| Web Emulator Rebuild | MEDIUM | LOW | MEDIUM |
| PAT Rotation | LOW | LOW | LOW |

---

**Next agent should start with Preset Import compatibility resolution** - this is the single highest impact, highest priority task that will determine whether the project can deliver authentic Mycelium sounds.