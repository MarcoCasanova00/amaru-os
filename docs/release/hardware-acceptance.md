# Bunker Evolved 1.0 — owner hardware acceptance

Date: 17 October 2026, Europe/Zurich.
Device: the project owner's M-VAVE FM-1.

The owner confirmed completion of the requested sequence:

1. Bunker build RC2 → original FM-1 firmware: succeeded.
2. Original FM-1 firmware → Bunker build RC2: completed.
3. Bunker build RC2 → original FM-1 firmware: completed.

The owner reported that audio and controls worked, and approved promoting the
website to the release version. This is the owner's hardware report; the agent
did not operate USB hardware. No installer log or photograph was supplied.
The owner transcribed the displayed label as `PE1.0-RC2`; the source-defined
label is `BE 1.0-RC2`, with package identity `FM-1_7100002`.
The exact original firmware version was not separately recorded. The site's
verified stock download and documented baseline is V15.

## Released artifact

Release name: Bunker Evolved 1.0.
Embedded identity: FM-1_7100002 / BE 1.0-RC2.
File: bunker-evolved-rc2.fwsc (610090 bytes).
SHA-256: db0b50d0ae9c3bfccee76e3b93de09fffa55389169278a0c5899dad9c79e6bb8.

This is the same tested binary, promoted without recompiling or changing its
internal version. Existing RC2 users already have the released firmware.

## Scope

Normal updates, audio and controls on this device were accepted by its owner.
This does not establish interrupted-transfer recovery, deliberate power-loss
recovery, other board revisions, long-duration on-chip CPU headroom, or the
physical browser music-backup/restore workflow. Existing automated evidence
and recovery limitations remain in test-status.txt.
