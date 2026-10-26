# Bunker Evolved 1.0 — feature provenance

Audit: 17 October 2026. Comparison target: Felucca 1.6.1, pinned at
5738617968a22b5d4c0d3470cfcc95559052f446, not a claim about every future release.
Bunker is a GPL-3.0-only derivative, not an independently built operating system.

| Area | What Bunker contributes | What already exists |
| --- | --- | --- |
| Ambient front panel | WORLD / GROW / SOUND / MOTIF / EVOLVE / MIX, AIR / MELODY / ROOT / SPARK roles, colour palette and note animation. See bunker_ui.c and bunker_draw.c. | Felucca supplies the physical input, graphics infrastructure and underlying editing pages. Four-layer ambient instruments already exist in AMB-1 and MAKU. M8 inspired the colour direction, not code or artwork. |
| Sound library | 64 role-specific recipes and 24 curated worlds, defined in bunker_library.inc and applied in bunker_ui.c. Each recipe selects an existing preset and adjusts tone, envelope, effects and voice behaviour. | Synthesis engines, factory presets, filters, modulation and effects come from Felucca and its dependencies. These are not 64 new synthesis engines or a newly invented reverb. |
| Motifs | 24 named scale-degree templates, role-specific register, harmony, density and velocity mapping. They write editable sequencer notes. | Step sequencing, chords, Euclidean spacing and scale degrees are established ideas. Felucca already has patterns and sequence tools; Sloop also offers extensive sequencing and scales. |
| Scales | The pinned base has 16 N_SCALE entries. Bunker appends 16, bringing the menu to 32, and supplies descriptive names in GROW. See params.c, seq.c, bunker_grow.c. | All scales themselves are established musical material. The first 16 and the original quantisation machinery are inherited. |
| EVOLVE | bunker_gen.c: seven bounded processes (WALK, ANSWER, ORBIT, MIRROR, BLOOM, BREATH, VOICE), per-track chance/loop interval, LOCK and guards for recording/song-chain ownership. The implementation edits the stored sequence. | Random walks, note borrowing, rotation, swapping, velocity variation and octave voicing are common techniques. Felucca already has randomness, arp WALK, probability, conditions and modulation. Automatic ambient evolution also exists in UnholyUnion. |
| GROW | bunker_grow.c: TUNE / SOFT / FLOW / NEW in a chosen key and scale, selected/all pitched tracks on hardware, sparse seeding, locking, guarded one-step undo. | Quantisation, mutation, undo and one-button generative arrangement are not new inventions. AMB-1 has grow/freeze gestures; UnholyUnion has evolution and undo. Bunker's particular workflow and implementation are the contribution. |
| Browser editor | New modular UI, portable phrase model, validated schema, release-specific device service and Web MIDI adapter in dist/editor/. Recipes and motifs are checked against Bunker's native C implementation. | Felucca already has a substantial web editor and defines the MIDI/SysEx protocol. Bunker does not claim to invent a browser editor. The browser instrument's infrastructure derives through Felucca from X0X. |
| Reliability and installer | Release packaging, identity/hash checks, documented results, Bunker-specific tests and this project's owner acceptance report. | USB, updater, boot recovery, watchdog and persistence infrastructure are principally inherited. Their presence is not a guarantee against all failures. |

The defensible differentiator is the combination: curated ambient starting points,
simple and reversible scale-based composition, plus Felucca's retained deep sound editing.
It is not a proven world-first generative technique, new DSP engine, or claim of superior
sound quality/safety over the upstream projects. No meaningful ownership percentage is
asserted: line counts would conflate imported engines, generated assets and our UI work.

## Concrete neighbours

- Felucca 1.6.1: multi-engine synthesizer, four tracks/eight shared voices, engine editing,
  effects, modulation, sequencer, scale tools, presets, projects, MIDI, editor, installer.
- AMB-1: Eno-inspired generative ambient, pad/bells/bass/pulse, unequal loops, pieces,
  selectable notes, grow/rest and freeze, tape effects and plate reverb. These are its own
  musical design; Bunker's engine is not AMB-1's tape-loop or plate-reverb code.
- Sloop: three synth tracks and a drum track, scales/chords, probability, ratchets,
  nudge and parameter locks. Bunker is not claiming these as exclusive.
- MAKU: four roles, randomly chosen in-key ambience, macros and an evolving preference
  system. Its documentation explicitly removes Felucca's deep sequencer/preset/project
  editing. Bunker retains those editing paths; it does not implement MAKU's liking system.
- UnholyUnion 0.2.3: shared-key ambient arrangement, timed harmony/modulation evolution,
  checkpoints/undo and a browser editor. This disproves a broad claim that editable,
  evolving four-track ambient on the FM-1 is unique to Bunker.

## Primary sources

- https://github.com/hugelton/Felucca/tree/5738617968a22b5d4c0d3470cfcc95559052f446
- https://github.com/hugelton/Felucca/blob/5738617968a22b5d4c0d3470cfcc95559052f446/firmware/src/params.c
- https://github.com/charlesvestal/fm1-amb
- https://github.com/isod89/sloop-fm1/blob/main/GUIDE.md
- https://github.com/okumurari2/maku
- https://github.com/actuallysparky/FM-1-Unholy-Union-generative-drone-firmware

Upstream README snapshots are retained under docs/research. Bunker's corresponding
source and component licences are in Bunker-Evolved-Source.zip. Main-branch upstream
pages can change; specific Bunker claims are tied to the pinned source above.
