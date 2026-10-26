# AMARU OS — Handoff Note

**Date:** 2026-10-26  
**State:** Alpha 0.1a, release-gate *hardware* still pending formal sign-off. Source at commit `17e?` (see `git log`).

## What we shipped today

- **Firmware (AMARU OS)**: derivative of Bunker Evolved 1.0 (GPL-3.0).  
- **Generator (`amaru_gen.c`)**: 9 genres (tribe/mental/hardtekk/acidcore/jungle/dnb/breakbeat/techno/psytrance), 4 tracks (AIR/MELODY/ROOT/SPARK), 3 forms (LOOP/PARTS/SONG). Deterministic seed, GROW/EVOL integration.  
- **Sound recipes**: Carnac/Mycelium-inspired presets (HYPOGEUM CHANT, SKARA HEARTH, SILBURY WAIL, SIBURY MOUND, ORACLE CHOIR, MYC PUNCHY, MYC BLOWN) ported as **numbers** (params), not imported binaries. Plus cosmetic AMARY/MAYA/TUPAC names.  
- **Theme**: strict monochrome palettes GRN/BLK, BLK/WHITE, AMB/BLK; accidental REC red → accent.  
- **Web UI**: AMARU branding; logo splash as 1-bit monochrome bitmap; AMARU 0.1a version.  
- **Web emulator (gh-pages)**: latest `felucca.wasm` deployed; preset alias now returns clean factory names (no PXX staging names).  
- **Sound alias layer (`amaru_sound_alias.c`)**: curated name overlays + fallback to original factory name.  
- **Documentation**: README.md hardware-status & flashing, QUICKSTART, ATTRIBUTION, and release-gate questions answered with test evidence.

## Brick-safety posture today

- Firmware flashes via **Jieli’s stock staging+commit loader**, same as Bunker — a package whose flash size doesn’t match `OTA_FL_LEN` is refused **before** anything is committed.
- Build-system gates pass: `hal/` only register access, single core, RAM .data+.bss 61 KB, flash image 535,612 B.
- **Hardware runtime** (SPL handoff, clock tree, flash write timing) **unverified** — alpha should remain alpha until a formal hardware-test sign-off.

## Future advancements (suggested)

1. **Hardware validation phase**: rehearse V15 rollback, flash one FM-1, confirm no brick, sign off 9-question gate → release **beta**.
2. **Full factory-name catalog** (extend `amaru_sound_alias.c` table beyond 17 curated entries), covering every factory preset of every engine.
3. **Genre-deepening pass**: reduce 16-step text masks to layered pattern generators (sub-step noise + ratchets) for DnB/jungle breaks.
4. **Visual identity pass**: skin `ui_draw.c` track chrome with tiles/leaves (low-level `cv_rect`/`cv_pixel` only, no new engines), e.g. 1-bit vine themes per genre.
5. **Hardware rollback test harness**: drop `scripts/rehearse_rollback.sh` exercising `FELUCCA_ID → newer V15`.
6. **Optional features**: Carnac/Mycelium deep-dive textures (brown-breath `NOISE` layer), psy/swarm timbre model; user-bank cloning tool.

## Attribs maintained

Built on Bunker Evolved 1.0 (Hügelton Instruments), GPL-3.0-only. Engine tools/modules retained. Release-gate doc: `docs/AMARU-ANALYSIS.md`.