# AMARU OS — session record, 2026-10-26

This is the working record for the engine-retirement / four-engine session. It is meant to be read with `handoff2.md` and the original `HANDOFF.md`.

## Starting point

- Repository: `<repo>`
- Branch: `main`
- HEAD: `a5c7130 docs: add GEN rework handoff for next session`
- Origin: `https://github.com/MarcoCasanova00/amaru-os`
- The original `HANDOFF.md` asked for a JIANT-style AMARU GEN rework using the existing four tracks.
- This session changed direction to the engine/sound reorientation first.
- No commits or pushes were made.
- `git push` is blocked: GitHub credentials are unavailable.

## Product / design decisions carried into this session

### Active engine set

Keep exactly:

| Engine | Index |
|---|---:|
| ANALOG | 0 |
| DIGITAL | 1 |
| DRUM | 10 |
| FM6 | 12 |

Retire:

`PHASE 2`, `LOFI 3`, `SAMPLE 4`, `VOICE 5`, `TRIO 6`, `WHEEL 7`, `GRAIN 8`, `PHYS 9`, `NOISE 11`, `SLICE 13`.

Keep all stored indices append-only. Retired slots remain in the engine table as silent placeholders so old projects and presets can be migrated safely.

### DIGITAL / FM6

- DIGITAL is permanently active for AMARU: `FELUCCA_FM4=1`.
- SLICE is off: `FELUCCA_SLICE=0`.
- FM6 is fully retained:
  - `up_fm6.c`
  - `fm6_vbank.c`
  - `fm6_core.c`
  - `editor_fm6.c`
  - `ui_fm6op.c`
  - project/favorites/patch-bank/operator UI
- Do not add FM6 → DIGITAL migration.

### Migration rules

Migration is one-way and idempotent.

Preserve:

- steps, notes, hits, accents, ratchets, probability, nudge
- level, pan, sends
- LFO, ARP, chord
- song/chain data

Reset generic retired engine-specific parameters. Drop incompatible engine-specific/FM6 operator motion where necessary.

Targets:

- WHEEL / GRAIN → DIGITAL
- SLICE → DRUM
- default retired engine → ANALOG
- PHYS MODEL DRUM → DRUM via `drum_from_phys`
- SAMPLE PERC → DRUM via `drum_from_perc`
- other PHYS / SAMPLE → ANALOG

### Presets approved but not implemented

5 ANALOG:

- `PSY BASS`
- `HI BASS`
- `PSY KICK`
- `GOA ACID`
- `SWAMP SQ`

16 DIGITAL:

- `ROOT BASS`
- `SPORE TALK`
- `HYPHA BLIP`
- `BARK GROWL`
- `MOSS DRONE`
- `GILL METAL`
- `FUNGAL DROP`
- `NIGHT SPORE`
- `ACID FOAM`
- `SWAMP CHIRP`
- `METAL HIT`
- `RUST IMPACT`
- `HITECH ZAP`
- `LASER FALL`
- `LIQUID FM`
- `SPORE BURST`

3 procedural drum kits:

- `PSY KICK`
- `FOREST`
- `HITECH`

No sample-bank import. Mycelium (`https://github.com/megagott/mycelium-fm1`) is GPL-3.0-only and needs attribution, not sample redistribution.

### GEN

The GEN rework in `HANDOFF.md` is still pending. `amaru_gen.c` / `bunker_amaru.inc` still reference retired engines. Preserve the four roles:

- SPARK = drums
- ROOT = bass
- MELODY = lead
- AIR = pad

### Budget

Previous numbers, not measured after these changes:

- app ≈ 534,188 B
- raw flash headroom ≈ 47.4 KB
- working ceiling 40 KB
- practical remaining ≈ 33.5 KB
- RAM: `61,156 / 73,728 B`
- pool: `328,924 / 368,640 B`

No ARM compiler was available, so no new size/RAM/CPU number exists.

## Chronology of this session

### 1. Source reorientation

Edited:

- `firmware/firmware/src/core.h`
- `firmware/firmware/src/engines.c`
- `firmware/firmware/src/params.c`
- `firmware/firmware/src/category.c`
- `firmware/firmware/src/fx.c`
- `firmware/firmware/src/midi_control.c`
- `firmware/firmware/src/project.c`
- `firmware/firmware/src/upreset.c`
- `firmware/firmware/src/ui.c`

The first source shape was:

- `NENGINES=14u`
- `NENG_SHOWN=4u`
- active mask for 0/1/10/12
- `ENGINE_ORDER={0,1,12,10}`
- `ENG_RETIRED` stub
- `TRK_DEF` = ANALOG ACID / DIGITAL FM BASS / FM6 PAD / DRUM KIT
- `proj_retired`
- `up_migrate` retired-engine handling
- UI fallback mapping

### 2. Compile blocker: ENGI_DRUM undeclared

Initial syntax checks failed because `ENG_ACTIVE_MASK` used `ENGI_DRUM` before its definition. The definitions for `ENGI_PHYS` / `ENGI_DRUM` were moved above `ENG_ACTIVE_MASK` in `core.h`. Syntax-only checks then passed.

### 3. Host test harness

The firmware's RISC-V synchronization asm prevented host compilation. A wrapper was created at:

```text
tools/hostasm/as
```

It defines `csync`, `ssync`, `cli`, `sti`, `flushinv` as no-ops and uses a temporary directory under `${TMPDIR:-/tmp}`. Host compiles use:

```sh
HOSTASM=/path/to/amaru-session-2026-10-26/tools/hostasm
cc -B"$HOSTASM" -O1 -w -Ibuild/gen -Ifirmware/src ...
```

A clean-HEAD baseline worktree was also checked. Its generated headers were read from `firmware/build/`. Baseline result: `ui_test` had one failure, `AMARU GEN: message names the genre and form`. This was useful to separate new failures from pre-existing ones.

### 4. `upreset_test`

`up_migrate()` needed a host guard because `param_desc_of` / engine defaults are not available in host test mode. Under `UP_HOST`, retired generic engines reset their E values to `0`.

The test was updated:

- initial engine ANALOG;
- PHYS / SAMPLE retire to ANALOG;
- host E defaults `0`;
- re-import idempotent.

Result: pass.

### 5. `project_test`

The test helpers were made migration-aware:

- added `old_target`;
- updated `track_ok` / `track_v3_ok`;
- FUN2 engine migration expectation;
- FUN5 / FUN6 moved to DIGITAL;
- PHYS / SAMPLE / PERC expectations;
- motion-event expectations, including the PERC → DRUM case.

Result: pass.

### 6. `ui_test` stabilization

Initial state after source changes:

- SAMPLE factory-browsing block crashed/segfaulted;
- FM6 test hit an FPE in `digital_render`;
- many tests assumed the old engine order/count/defaults.

Fixed:

1. **Segfault** — removed the obsolete SAMPLE factory-browsing block.
2. **FPE in FM6 ops** — `song.sel=1` now selected DIGITAL FM BASS, not FM6. Changed to `song.sel=2` for the FM6 PAD track and updated the expected undo message to `UNDO/REDO T3`.
3. **SAMPLE retired fallback** — the test checked `t->engine`, which was stale after `gm_hits_play` (the actual running engine only changes once the mixer runs). Changed the check to `eng_req`.
4. **SAMPLE alert tests** — skipped when SAMPLE is retired.
5. **Category count** — BASS factory count is now 5, not 6+.
6. **SONG hash** — `SONG_PROBE_GOLDEN` changed to `0x9662420Du` because power-on track defaults changed. Blocks stayed 11026.
7. **GREY test** — the failure was the “more than 200 pages” floor, not actual color. Four engines produce fewer pages; floor lowered to 50.
8. **Browse test** — four active engines have fewer presets; load threshold lowered from 200 to 100.
9. **EDIT-layer keys** — test now uses display positions (`white(2)`, `white(3)`), not `eng_rank(position)`. The new order is DIGITAL second, FM6 third, DRUM last.
10. **KNOB 2 engine test** — expected engine changed from hard-coded `3u` to `eng_vis(3)`.
11. **SAMPLE #124 block** — guarded out when SAMPLE is retired.
12. Removed all temporary `DBG` prints before packaging.

Result: `ui_test` now has only the two AMARU GEN failures. No crash.

## Current git state

Modified files:

```text
firmware/firmware/src/category.c
firmware/firmware/src/core.h
firmware/firmware/src/engines.c
firmware/firmware/src/fx.c
firmware/firmware/src/midi_control.c
firmware/firmware/src/params.c
firmware/firmware/src/project.c
firmware/firmware/src/ui.c
firmware/firmware/src/upreset.c
firmware/tests/project_test.c
firmware/tests/song_probe.h
firmware/tests/ui_test.c
firmware/tests/upreset_test.c
```

Also created by this packaging step:

```text
handoff2.md
amaru-session-2026-10-26/TRANSCRIPT.md
amaru-session-2026-10-26/transcript.jsonl
amaru-session-2026-10-26/messages.jsonl
amaru-session-2026-10-26/SESSION.md
amaru-session-2026-10-26/changes.patch
amaru-session-2026-10-26/tools/hostasm/as
amaru-session-2026-10-26/test-logs/ui_test.log
amaru-session-2026-10-26/test-logs/project_test.log
amaru-session-2026-10-26/test-logs/upreset_test.log
amaru-session-2026-10-26/handoff2.md
```

The full transcript is exported from OpenCode's local database session `ses_e862362b0ffemLRX9kU06CVCHs`. It is a snapshot taken at packaging time, so the final answer after the export is not in it.

Nothing is committed.

## Exact final test output

`project_test`:

```text
FUN4 with SAMPLE PERC: DRUM kit; other SAMPLE -> ANALOG      ok
project format test passed
```

`upreset_test`:

```text
category: a version 3 grid -> 5 with it, values kept; one that cannot be packed keeps none ok
user preset test passed
```

`ui_test`:

```text
ui: AMARU GEN: message names the genre and form                                FAIL
ui: AMARU GEN: BPM and swing follow the plan                                   ok
ui: AMARU GEN: engines, presets, root, scale and every step match the planner  FAIL
...
ui: 2 FAILED
```

The first GEN failure is pre-existing at clean HEAD. The second is new because GEN has not been updated for the four-engine set.

## Environment

- OS: Linux
- Shell: bash
- No `arm-none-eabi-gcc`.
- Available: `cc`, `node`, `python3`, `make`, `cmake`.
- No `~/.jieli`, no `/opt` ARM toolchain.
- Generated headers are present under `firmware/build/gen`.
- The generated headers are included in the `repo/` snapshot. Regeneration needs fontTools installed locally.

## Known test quirks

- `drumfuzz_test.c` has a pre-existing `rnd` symbol conflict with `amaru_gen.c`.
- `uac_test.c` needs `-DHALF_FRAMES=...` and `-DT_CDC=0/1`.
- `run_tests.sh` still needs host-wrapper awareness.
- The packaged host wrapper is self-contained.
- `track_t.engine` is the running engine; `eng_req` is the requested one.
- `eng_rank(e)` returns the display position of an engine **index**.
- EDIT-layer white keys use display positions 0..`NENG_SHOWN-1`.

## Conclusion / next session

The firmware-level four-engine change is mostly in place and host-tested. The immediate blocker is not a crash: it is finishing the test expectations and the AMARU GEN rework for the new engine set. After that, add presets, exclude retired sources, update web/editor protocol/docs, and do an ARM build on a machine with the toolchain.

Read `handoff2.md` first, then the original `HANDOFF.md` for the GEN brief.
