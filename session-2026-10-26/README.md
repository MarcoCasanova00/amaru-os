# AMARU OS session package — 2026-10-26

Session archive for the AMARU OS engine-retirement / four-engine session.

This directory is inside the repository so the next agent has the full context in one place. It contains local session history, logs and a source snapshot.

Start with `handoff2.md`. `SESSION.md` is the structured record. `TRANSCRIPT.md` is the readable full transcript; `transcript.jsonl` and `messages.jsonl` are machine-readable exports.

Included:

- `handoff2.md` / `HANDOFF.md`
- `SESSION.md`
- `TRANSCRIPT.md`
- `transcript.jsonl`
- `messages.jsonl`
- `changes.patch` (apply to HEAD `a5c7130`)
- `repo/` (working-tree snapshot; no `.git`, no `node_modules`, includes generated headers)
- `tools/hostasm/as` (self-contained host assembler wrapper)
- `test-logs/`
