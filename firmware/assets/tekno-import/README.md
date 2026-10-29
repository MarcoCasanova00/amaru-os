# TEKNO — imported hard-percussion EPROM dumps (preserved, not compiled)

These 10 one-shot percussion hits are decoded from the **Vermona SK86** sound EPROMs
(DBWBP synth-EPROM archive, `Vermona SK86 roms.zip`, "bottom" ROMs: 8-bit PCM,
22050 Hz, decoded to float then re-quantised to 16-bit WAV).

**Status: preserved, not part of the firmware build.**
They are intentionally kept out of the compiled flash image. The SAMPLE engine
(which would play them) was re-enabled in an experiment, but its render loop ran
**over its CPU budget** (+32% vs the +10% limit) on the AC79 chip, which risks
audio dropouts under dense playback. For stability, that engine re-enable was
reverted; these samples remain here as a clean, ready source.

### To actually use them (safe path, recommended)
Run them through the firmware's own **DRUM engine** (already compiled, in-budget,
brick-safe) rather than re-enabling SAMPLE. The DRUM engine is analog-synthesis,
so this means mapping each hit's *character* into a drum-voice kit (kick/snare/
hat/tom bodies, noise tails) or, if a sampled playback path is ever added to
DRUM, importing these WAVs via `tools/gen_samples.py`.

For reference, the generation pipeline is:
`python3 firmware/tools/gen_samples.py firmware/build/gen/felucca_samples.h`
(with these WAVs placed as a `kit`-kind CC0 set under `firmware/assets/samples-cc0/TEKNO/`).
Note that would raise `SMP_NSETS` 5→6 and change the sample bank — a real change
to verify before shipping.

### License note
Vermona still exists as a company; these are manufacturer ROM dumps shared on
DBWBP for preservation/repair, not under an open license. The project owner
stated they have a team handling the legal side. Until that is resolved,
keep them out of the distributed GPL firmware.
