# AMARU OS — handoff2

- **Date:** 2026-10-26
- **Read after:** `HANDOFF.md` (the original GEN brief)
- **Branch / HEAD:** `main` / `a5c7130` (`docs: add GEN rework handoff for next session`)
- **Working tree:** dirty; this session's changes are **not committed**
- **Session archive:** `session-2026-10-26/` (inside this repo; full session plus a copy of this handoff)

This note describes where the engine-retirement / four-engine session stopped. The original `HANDOFF.md` still describes the GEN rework that was planned before this session; the GEN work is **not finished**.

## TL;DR

The firmware now exposes four active engines:

| Engine | Index | Notes |
|---|---:|---|
| ANALOG | 0 | active |
| DIGITAL | 1 | permanently active (`FELUCCA_FM4=1`) |
| DRUM | 10 | active |
| FM6 | 12 | fully retained |

Every other engine index is a silent retired placeholder. Migration paths for old projects and user presets are implemented. Host `project_test` and `upreset_test` pass. Host `ui_test` is down to **2 failures, both in AMARU GEN**; one was already failing at baseline, the other is because GEN still targets the old engine set.

Do not assume the ARM firmware builds. There is no `arm-none-eabi-gcc` on this machine, so flash/RAM/size were not measured.

## What was requested this session

Reorient AMARU for tekno:

- keep **ANALOG, DIGITAL, DRUM, FM6**;
- retire PHASE, LOFI, SAMPLE, VOICE, TRIO, WHEEL, GRAIN, PHYS, NOISE, SLICE;
- keep stored engine indices append-only and migrate old projects/user presets;
- add the agreed analog/digital/drum preset sets;
- update firmware, tests, web/editor protocol and docs;
- write a handoff for the next session.

The preset implementation, source exclusion, web/docs updates and GEN rework were **not done**.

## Current host test status

Run from the repository's `firmware/` directory. Set `HOSTASM` to the directory containing the packaged host assembler wrapper (the wrapper must be named `as`):

```sh
HOSTASM=/path/to/amaru-session-2026-10-26/tools/hostasm
OUT=/tmp/amaru-host
mkdir -p "$OUT"

cc -B"$HOSTASM" -O1 -w -Ibuild/gen -Ifirmware/src \
  -o "$OUT/ui_test" tests/ui_test.c -lm
"$OUT/ui_test"

cc -B"$HOSTASM" -O1 -w -Ibuild/gen -Ifirmware/src \
  -o "$OUT/project_test" tests/project_test.c -lm
"$OUT/project_test"

cc -B"$HOSTASM" -O1 -w -Ibuild/gen -Ifirmware/src \
  -o "$OUT/upreset_test" tests/upreset_test.c -lm
"$OUT/upreset_test"
```

Latest results:

- `project_test`: **pass**
- `upreset_test`: **pass**
- `ui_test`: **2 FAILED**
  - `AMARU GEN: message names the genre and form` — also fails on clean HEAD; pre-existing.
  - `AMARU GEN: engines, presets, root, scale and every step match the planner` — new; GEN tables/defaults still put retired engines into the plan.

Logs are in the session archive under `test-logs/`.

Clean-HEAD baseline was checked in a separate worktree: `ui_test` had only the pre-existing missing-`JUNGLE PARTS` message failure. The assembler wrapper itself is not the cause of the two failures.

### Host assembler wrapper

The firmware sources contain RISC-V `csync` / `ssync` / `cli` / `sti` / `flushinv` asm that the host assembler does not know. Use:

```sh
cc -B"$HOSTASM" ...
```

The wrapper defines those as no-ops. It is in the session archive at:

```text
tools/hostasm/as
```

The packaged wrapper is self-contained and uses a temporary directory under `${TMPDIR:-/tmp}`.

### Generated headers

Generated headers are in `firmware/build/gen` in the source snapshot. They are included in the session archive's `repo/` snapshot so the host tests can run without regenerating fonts/icons.

If regenerating them, install fontTools in the local Python environment first.

## What changed in the source

### `firmware/firmware/src/core.h`

- `NENGINES` is now fixed at `14u`; all old indices keep their numbers.
- `NENG_SHOWN` is `4u`.
- Added `ENGI_ANALOG`, kept `ENGI_DIGITAL=1`, `ENGI_PHYS=9`, `ENGI_DRUM=10`, `ENGI_FM6=12`.
- Added `ENG_ACTIVE_MASK`, `eng_active(e)` and `eng_fallback(e)`.
- `FELUCCA_SLICE` defaults to `0`.
- `FELUCCA_FM4` defaults to `1`.
- Moved `ENGI_PHYS` / `ENGI_DRUM` before `ENG_ACTIVE_MASK`; they had been undeclared there, which broke compilation.

Fallback mapping:

- WHEEL (7), GRAIN (8) → DIGITAL (1)
- SLICE (13) → DRUM (10)
- everything else retired → ANALOG (0)
- PHYS MODEL DRUM and SAMPLE PERC migrate specially first.

### `firmware/firmware/src/engines.c`

- Added `ENG_RETIRED` with no-op note/render callbacks and an empty preset/edit table.
- Retired engine slots point at `ENG_RETIRED`.
- `ENGINE_ORDER` is `{ANALOG, DIGITAL, FM6, DRUM}`.
- `eng_ok()` now calls `eng_active()`.
- `eng_rank()` falls back through `eng_fallback()`.
- `TRK_DEF` is now:
  - T1 ANALOG ACID
  - T2 DIGITAL FM BASS
  - T3 FM6 PAD
  - T4 DRUM KIT
- Retired sources are still included by the translation unit; they are just not reachable through the engine table.

### `firmware/firmware/src/project.c`

Added `proj_retired()`:

- preserves steps/notes/hits/accents/ratchets/probability/nudge/levels/pans/sends/LFO/ARP/chord/song data;
- applies PHYS/PERC drum migrations where needed;
- resets generic retired engine parameters (`P_E0..P_E7`);
- drops incompatible motion events for retired engine IDs;
- runs after `proj_fm4` / `proj_perc` in `proj_import`, direct runtime restore and runtime restore.

### `firmware/firmware/src/upreset.c`

Added `up_migrate()` coverage for retired engines after the PHYS/PERC cases. Under `UP_HOST`, `param_desc_of` is guarded and generic retired engines reset E values to `0`; firmware uses the real engine default.

### `firmware/firmware/src/ui.c`

- `apply_preset_to()` guards retired engines / falls back to an active engine.
- `set_engine_of()` maps retired requests through `eng_fallback()`.

### `firmware/firmware/src/params.c`, `category.c`, `fx.c`, `midi_control.c`

Updated name tables, DIGITAL category label, engine VCOST table and MIDI CC engine table for the new four-engine layout. Retired names are `"-"`.

## Test changes already made

### `firmware/tests/project_test.c`

- added `old_target`;
- made `track_ok` / `track_v3_ok` migration-aware;
- updated FUN2/FUN5/FUN6 expectations;
- updated PHYS/SAMPLE/PERC migration expectations;
- adjusted motion-event expectations.

**Passes.**

### `firmware/tests/upreset_test.c`

- initial engine is ANALOG;
- retired PHYS/SAMPLE expectations now fall to ANALOG with host E defaults `0`;
- re-import is idempotent.

**Passes.**

### `firmware/tests/ui_test.c`

Fixed during this session:

- removed the obsolete SAMPLE factory-browsing block;
- skipped `test_sample_alert` when SAMPLE is retired;
- reduced the browse threshold from 200 to 100 loads (four active engines have fewer presets);
- lowered the GREY page-count floor from 200 to 50;
- updated the category BASS count floor from 6 to 5;
- updated `SONG_PROBE_GOLDEN` to `0x9662420Du` for the new power-on defaults;
- fixed `test_fm6_ops` to select track 3 (`song.sel=2`) and expect `UNDO/REDO T3`;
- fixed the SAMPLE fallback check to test `eng_req`, not the stale running `engine` field;
- updated EDIT-layer key tests for the new order (DIGITAL second, FM6 third, DRUM last).

### `firmware/tests/song_probe.h`

Golden hash changed to `0x9662420Du`, still 11026 blocks, because power-on track defaults changed. The old cross-version “before lanes” comparison no longer applies literally.

## Remaining failures / work

### 1. AMARU GEN (highest priority)

The original `HANDOFF.md` GEN brief is still valid, but the engine set changed underneath it. `amaru_gen.c` / `bunker_amaru.inc` still reference retired engines. The two `ui_test` failures should be handled together:

- update GEN genre sound recipes to active engines/presets;
- make `amaru_plan` target the four active engines;
- regenerate/update expected planner data in tests;
- decide whether the missing `JUNGLE PARTS` message is a real baseline bug or a host-only artifact.

### 2. Presets not implemented

Still to add:

- 5 ANALOG: `PSY BASS`, `HI BASS`, `PSY KICK`, `GOA ACID`, `SWAMP SQ`
- 16 DIGITAL: `ROOT BASS`, `SPORE TALK`, `HYPHA BLIP`, `BARK GROWL`, `MOSS DRONE`, `GILL METAL`, `FUNGAL DROP`, `NIGHT SPORE`, `ACID FOAM`, `SWAMP CHIRP`, `METAL HIT`, `RUST IMPACT`, `HITECH ZAP`, `LASER FALL`, `LIQUID FM`, `SPORE BURST`
- 3 procedural drum kits: `PSY KICK`, `FOREST`, `HITECH`

Append after existing factory presets. Extend `PRESET_CAT`, `N_DRUM_KIT`, `DK_COUNT`, `DV_NKIT`, `DK_PLAYS`, `DRUM_PRESETS` without reordering stored values.

### 3. Source exclusion still pending

Retired sources are still compiled into `engines.c`. Still to remove from the firmware build:

- `eng_sample.c`
- `eng_slice.c`
- `ui_slice.c`
- `slice_store.c`
- sample/SLICE UI and backup paths
- unused `SMP_USER_SLOTS` / `smp_user_scan` references

Keep the source files in the tree.

### 4. Web/editor docs

Still need 14-index protocol/UI updates:

- `firmware/web/test_web.mjs`
- `firmware/web/editor.html`
- `firmware/web/EDITOR_PROTOCOL.md`
- README/docs engine lists
- `docs/AMARU-ANALYSIS.md`
- `docs/ATTRIBUTION.md`

Mycelium reference (`https://github.com/megagott/mycelium-fm1`) is GPL-3.0-only. Attribution is required. Do **not** import sample banks.

### 5. ARM build / push

Blocked:

- no `arm-none-eabi-gcc`;
- `git push` fails because GitHub credentials are unavailable.

Do not claim a firmware binary, size or flash headroom until a real ARM build is made. Previous budget numbers were about 534 KB app size, about 47 KB raw headroom with a 40 KB working ceiling, and RAM `61,156 / 73,728 B`.

## Invariants / traps for the next agent

- **Engine indices are append-only.** Never reorder or renumber.
- **FM6 is fully retained.** Do not migrate FM6 to DIGITAL.
- **DIGITAL is active.** `FELUCCA_FM4` must stay `1` for AMARU.
- **SLICE is retired.** Keep `FELUCCA_SLICE=0`.
- Migration must stay one-way and idempotent.
- Preserve sequencer/mix data even when replacing an engine.
- `track_t.engine` is the currently running engine, not necessarily `eng_req`; tests that need it must let the mixer run (`gm_hits_play` / `mix_block`) or check `eng_req`.
- `eng_rank(e)` takes an engine index, **not** a display position. The EDIT-layer white keys use positions (`white(0..NENG_SHOWN-1)`).
- UI tests assume no host debug prints; remove temporary instrumentation before packaging.
- `drumfuzz_test.c` has a pre-existing `rnd` symbol conflict with `amaru_gen.c`.
- `uac_test.c` needs `-DHALF_FRAMES=...` and `-DT_CDC=0/1`.
- `run_tests.sh` does not yet know about the host assembler wrapper.

## Session archive

The directory `session-2026-10-26/` in this repo contains:

- `README.md` — package map
- `handoff2.md` — copy of this file
- `HANDOFF.md` — copy of the original GEN handoff with the next-session note
- `SESSION.md` — structured session record, chronology, decisions and context
- `TRANSCRIPT.md` — readable full transcript of every message, reasoning block and tool call/output, with local absolute paths replaced by neutral placeholders
- `transcript.jsonl` — machine-readable full transcript (one part per line)
- `messages.jsonl` — message metadata plus parts
- `changes.patch` — current tracked source/test diff
- `repo/` — full working-tree snapshot at packaging time (no `.git`, no `node_modules`, no session directory)
- `tools/hostasm/as` — self-contained host assembler wrapper
- `test-logs/ui_test.log`
- `test-logs/project_test.log`
- `test-logs/upreset_test.log`

The full session, transcript, logs and source snapshot are kept together in `session-2026-10-26/` for the next agent. The transcript is a packaging-time snapshot. Object files, host binaries and `.git` are not included.

## Suggested first steps

1. Read this file, then the original `HANDOFF.md` GEN brief.
2. Run the three host tests with the wrapper to confirm the documented state.
3. Handle the two AMARU GEN failures and update GEN to the four active engines.
4. Continue with presets, source exclusion, web/docs.
5. Only then attempt an ARM build on a machine that has the toolchain.
