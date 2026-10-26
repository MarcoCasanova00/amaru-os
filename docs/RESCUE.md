# AMARU OS — Hardware rescue & recovery

**Universal FM-1 rescue (must stay intact long-term)**  
*M-VAVE FM-1 recovery*, per https://thefirmwarehub.com/kb/#rescue.

1. **Power-on recovery**  
   Hold **OCT− (Octave minus)** while switching the FM-1 on. The device enters update mode and listens for a package. Press **Install** in the hub/built-in installer to continue.

2. **USB rescue (Linux)**  
   ```bash
   sudo python3 fm1_rescue.py amaru  # or sloop, v15, etc.
   ```  
   The hub compares the package byte-for-byte (SHA-256 pinned), writes only the firmware area, and reads every sector back.  
   See https://thefirmwarehub.com/kb/#rescue for the full `rescue/fm1_rescue.py` walk-through.

3. **Transporter rescue** (Windows/offline)  
   Use https://github.com/Quixotic7/MvaveFM1Unbricker (MIT) to read the flash twice, keep a backup, and write into the firmware area.

Because the hub documents this recovery, **AMARU must never disable the OCT− on power-on handler** — removing it would brick any FM-1 user if rescue becomes unavailable.

---

## Portability checklist

- **Identity kept as** `F -1_7100002` (no collision maps across the hub table—the hub reads it automatically).
- **Manifest hooks**: AMARU is GPL-3.0-only, derivative of Bunker Evolved. Source code mirrors unchanged.
- **(Optional) fmhub.json in root** — card text metadata:
  {
    "what": "A 4-track generative AMARU OS — Carnac/Mycelium textures blended with JIANT-style techno breeds.",
    "kinds": ["groove"],
    "play": "https://marcocasanova00.github.io/amaru-os/"
  }
  Keys: `what` (20..300 chars, no HTML), `kinds` (groove/synth/drums/chords/tracker/other), `play`, `editor`.