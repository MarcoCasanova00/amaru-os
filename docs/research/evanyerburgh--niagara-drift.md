# Niagara Drift

**A drone synth for the M-VAVE FM-1.**

Niagara Drift is open-source firmware that turns the M-VAVE
FM-1 into a drone synthesizer: two always-on drones you tune, shape and move rather than play note by
note. It's inspired by the Stylophone CPM DS-2, the Moog Subharmonicon and the Soma Lyra-8.

![The FM-1 with what every control does in Niagara Drift written on it](docs/img/panel.png)

*Your FM-1 with Niagara Drift installed: what each knob, button and key now does. Amber is drone I,
teal drone II, lilac the LFO. The black keys are switches (lit = on; on the LFO pages they set the
LFO's routing, and X-mod lets drone I move drone II's filter); the white keys hold 16 scenes.*

**Status: 0.1 beta.** It runs on the FM-1 (tested on one unit so far) and in the browser:
[try it](https://evanyerburgh.github.io/niagara-drift/) without an FM-1, the same code with sound.

## What it does

- **Two drones**, each a main oscillator that blends sine → triangle → saw → square (X-WAVE), a
  partner oscillator at an interval with detune (equal or just tuning), a sub one or two octaves down
  or a subharmonic (⅓, ⅕ of the pitch), and a white or pink noise, into its own self-oscillating
  filter (low-, band- or high-pass) with drive. Pitch is free between notes, from 27 Hz to 1.6 kHz.
- **Links** between them: pitch link, hard sync, and drone I's sound moving drone II's cutoff.
- **The Ultra LFO**, after the DS-2's: one rate, two waves of eight shapes each (one of them truly
  random), routed to each drone's pitch or X-WAVE and cutoff or resonance, alone or summed. It can
  move its own rate and lock to the Swell.
- **Movement** (OCT− / OCT+): all the LFO's depths down or up together, from still to double.
- **The Swell**: both drones breathing in time, from a slow soft swell to a hard rhythmic gate, with swing.
- **Effects**: a dark PT2399-style delay (the LFO can warble its time) into a stereo reverb (Room,
  Plate, Hall, Chorus hall).
- **Feedback** (the finished sound fed back into the drones) and **Character** (clean to vintage).
- **Drift**: the drones are never quite still; pitch, cutoff, timbre, level and detune slowly wander.
- **Morph**: buttons and keys never jump: the sound moves to a new setting over a time you set.
- **16 scenes** on the white keys, recalled with a morph; a **motion loop** that records your knob
  moves.

## Installing

1. Open the [web installer](https://evanyerburgh.github.io/niagara-drift/install/) in **Chrome or Edge**
   (they can talk to the FM-1 over USB; Safari and Firefox can't).
2. Connect the FM-1 straight to the computer with a USB **data** cable (no hub) and switch it on.
3. Press **Install** and allow MIDI when the browser asks. It takes about two minutes; don't touch the cable
   or the FM-1 until it says Done. Then press PLAY.

Or from a terminal, with the file from the [releases](https://github.com/evanyerburgh/niagara-drift/releases):
`pip3 install mido python-rtmidi`, then `python3 tools/fm1_install.py niagara-drift-0.1-beta.fwsc`.

**What it replaces:** Niagara Drift takes the place of M-VAVE's firmware, so the FM-1's own sounds and
sequencer are gone until you go back (below).

**Updating:** when there's a new version, run the installer again the same way. Your sound and scenes are
kept, unless that version's notes in the [changelog](CHANGELOG.md) say otherwise.

**Going back:** the installer's "Back to M-VAVE's firmware" writes M-VAVE's own FM-1 V15 back over the
same cable (or use M-VAVE's updater, M-UPGRADE, from [m-vave.com/download](https://www.m-vave.com/download)).
If it ever fails to start twice in a row it comes up in **safe mode** (no sound, USB on), where the installer
still works. If an FM-1 won't start at all, a Mac can still put the stock firmware back: see
[BUILDING.md](BUILDING.md#install). Installing firmware is at your own risk; it's a beta.

## Playing it

**KNOB 1–2 are always drone I, KNOB 3–4 always drone II.** The page buttons change what they do;
the screen shows each page's settings as a picture, and the value of the knob you're turning.

| | |
|---|---|
| **PLAY** | fade the drones in or out; **SEL + PLAY** fades drone II alone out or back in |
| **OCT− / OCT+** | **Movement**: all the LFO's movement down or up together, 10% a tap (hold to keep going), from still (0%) to double (200%); lit while it isn't 100% |
| **SAVE** | save the sound, at once. While the drone plays, the sound dips out and back over about 50 ms around the write; the dot by the page name means not yet written |
| **SEL** (hold) | fine: a pitch knob moves in single cents, cutoff and resonance in tenths |

### Pages

Tap a page button and the knobs stay on that page. Hold it to peek: turn a knob, let go, and you're
back. Tap it again for its second page; dots by the page's name show which of its pages you're on.
Each page's screen shows a picture of what its knobs do, and the four values below it. Every knob
glides: a slow turn is a smooth sweep, never steps; turn faster to go further.

#### Play · HOME

<img src="docs/img/page-play.png" width="488" alt="The Play 1 and Play 2 pages: each drone's waveform; each drone's filter curve and level">

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| Play 1 | I pitch | I cutoff | II pitch | II cutoff |
| Play 2 | I resonance | I level | II resonance | II level |

- **MASTER**: past about four-fifths of its turn it drives into overdrive, at about the same loudness.

#### Tone · ENV

<img src="docs/img/page-tone.png" width="488" alt="The Tone 1 and Tone 2 pages: each drone's sound through its drive; each drone's noise through its filter">

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| Tone 1 | I drive | I osc mix | II drive | II osc mix |
| Tone 2 | I noise | I colour | II noise | II colour |

- **Osc mix** goes from the oscillators to the sub.
- **Noise** is the noise's level beside the oscillators, as in a synth's mixer: they stay as they are.
  It's heard all along the knob and never louder than them (100 is 6 dB under, 50 −12 dB, 25 −18). It
  goes through the drone's filter with them, so the cutoff, resonance, LFO and Drift shape it.
  **Colour** is white or pink (turn right for pink, left for white; it crossfades over the Morph time).

#### Osc · EDIT

<img src="docs/img/page-osc.png" width="244" alt="The Osc page: each drone's waveform on the sine to square scale, and its pulse width">

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| Osc | I X-Wave | I PW (pulse width) | II X-Wave | II PW |

- **X-Wave** blends sine → triangle → saw → square; **PW** is the square's pulse width.

#### Partner · SEQ

<img src="docs/img/page-partner.png" width="244" alt="The Partner page: each drone's partner oscillator under its main one, and how fast they beat">

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| Partner | I interval | I detune | II interval | II detune |

- The partner oscillator sits an interval above or below each drone; **detune** makes them beat.
  With **Tuning: just** (Setup 2) the intervals are pure ratios.

#### LFO · LFO

<img src="docs/img/page-lfo.png" width="488" alt="The LFO depths and LFO mods pages: the two waves and where they go; the rate modulation, cross-mod and delay LFO">

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| LFO depths | I osc depth | I filter depth | II osc depth | II filter depth |
| LFO mods | rate mod | mod by (wave) | I → II cutoff amount | delay LFO depth |

- **Depths** and **rate mod** are centred: the middle is none.
- The LFO's rate and waves are on the top-left knobs; see [The LFO](#the-lfo) below.

#### Swell · ARP

<img src="docs/img/page-swell.png" width="244" alt="The Swell page: the swell's shape over two bars">

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| Swell | rate | depth | shape | swing |

- Tap ARP to start or stop the Swell; tap it twice to see this page.
- **The Swell** runs from a 33-second breath to a buzz (it starts at about one breath every two seconds).
- **Depth** is how far it dips, in dB (down to −30; all the way, silence). **Shape** firms the breath up into
  a pulse, and only at the top into a hard gate.

#### Effects · FX

<img src="docs/img/page-effects.png" width="488" alt="The Effects 1 and Effects 2 pages: the delay's echoes and the reverb's tail; the reverb type and the feedback">

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| Effects 1 | delay time | repeats | delay mix | reverb mix |
| Effects 2 | reverb type | speaker (low cut) | feedback | character |

- **Speaker** cuts the deep bass a small speaker can't play (40–240 Hz); leave it off on headphones.
- **Feedback** sends the finished sound (after the delay and reverb) back into the drones: a resonant,
  thickening colour that grows with the knob (more with the delay up), rough at the top but never louder than
  the sound without it; it fades with the drones when you stop.
- **Character** takes the oscillators from clean to vintage: softer edges, a bent saw, a little pitch
  jitter, the two drones slightly apart, a whisper of noise.

#### Setup · GLO

<img src="docs/img/page-setup.png" width="488" alt="The Setup 1 and Setup 2 pages: glide, fade, drift and stereo; the Morph time and the tuning">

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| Setup 1 | glide | fade | drift | stereo |
| Setup 2 | morph | tune | scene switch | tuning (equal / just) |

- **Glide** is how long a drone takes to reach a new pitch (up to 10 s; off by default). **Stereo**
  spreads the drones from mono to drone I left, drone II right.
- **Morph** is the switch time for buttons and keys: how long they take to move the sound to its new
  setting (0.02–30 s); **fade** is how long PLAY takes (up to 60 s); **scene switch** how long the switch
  to a scene takes (0.1 s to 5 minutes).
- **Drift** keeps the drones from ever being quite still: besides a little pitch wander, each drone's
  cutoff, timbre (X-Wave), level and partner detune slowly wander on their own, around where you've set
  them, never repeating. Gentle by default; 0 holds everything exactly where you put it.
- **Tuning: just** makes the partner's intervals pure ratios (a fifth is exactly 3/2), so they don't
  beat; detune still adds beating on purpose.

### The LFO

| Knob | Sets |
|---|---|
| SELECT | the rate, from one cycle in 50 seconds to 70 Hz (below 0.1 Hz the screen shows a cycle's length, "21s") |
| PRESETS | Wave I's shape (or off) |
| ALGORITHM | Wave II's shape (or off) |

One clock, two waves; the two lights by PLAY show them moving. The last shape on each is random.

### Black keys: switches

Lit means on. They follow the page: on the LFO pages they set the LFO's routing. Normally they're grouped
by meaning: drone I (keys 1–3), the two global keys (4–5), drone II (6–8), the links (9–11).

| Key | Normally | On the LFO pages |
|---|---|---|
| 1 | I partner | I osc wave (I, II, both) |
| 2 | I sub: −1 oct, −2 oct, ⅓, ⅕ | I osc LFO → X-Wave (instead of pitch) |
| 3 | I filter: LP, BP, HP | I filter LFO → resonance (instead of cutoff) |
| 4 | pitch lock (the pitch knobs do nothing) | I filter wave (I, II, both) |
| 5 | LFO reset | LFO reset |
| 6 | II partner | II osc wave (I, II, both) |
| 7 | II sub: −1 oct, −2 oct, ⅓, ⅕ | II osc LFO → X-Wave |
| 8 | II filter: LP, BP, HP | II filter LFO → resonance |
| 9 | pitch link (II follows I) | II filter wave (I, II, both) |
| 10 | hard sync (II to I) | delay wave (I, II, both) |
| 11 | I → II cutoff | I → II cutoff |

- A **wave key** goes round with each tap: Wave I (dark), Wave II (lit), both (winking).
- **LFO reset:** tap to restart the LFO. Hold it half a second to lock the LFO to the Swell (it then
  restarts every bar; the key stays lit). Hold again to free it.
- With **pitch link** on, KNOB 3 sets the gap between the drones.
- A **filter key** goes round with each tap: low-pass (dark), band-pass (winking), high-pass (lit).
- A **sub key** goes round with each tap: −1 octave (dark), −2 octaves (lit), then the subharmonics ⅓
  and ⅕ of the pitch (winking).

### White keys, ARP and REC

| | |
|---|---|
| **White key** | tap: morph to the scene stored there, over the scene switch time, and stay (turn a knob to stop part way). **Hold** to visit: let go and it morphs back to where you were. A scene keeps the sound, not the room: Speaker, Stereo, Tune, Pitch lock, Fade, Morph and Scene switch stay as they are |
| **SAVE, then a white key** | hold SAVE first, then tap the key: the sound is stored there (a key pressed first goes to its scene instead; lit keys hold a scene; the key of the scene you're on blinks slowly, until you turn a knob or flip a switch; while playing, the sound dips for a moment as it's written) |
| **ARP** (tap) | the Swell on or off: both drones chopped in time. Hold ARP to peek at its page; tap twice quickly to stay on it |
| **REC** | tap to record your knob moves, tap to loop them, tap to stop (up to 5 minutes, the time shown by REC in the top bar; all four knobs turning fast for minutes fills it sooner, and it starts looping there, "FULL"; each pass glides back to the start) |

### MIDI

Over USB or the TRS jack, a note on **channel 1** tunes drone I to it, a note on **channel 2** drone II
(with Pitch link on, a channel 2 note sets the interval instead). It tunes; it doesn't start or stop the drones.

### Trying it in the browser

[Play it on the website](https://evanyerburgh.github.io/niagara-drift/): the firmware itself, compiled for the
browser, with sound. SAVE there keeps your sound in that browser. Working on the code, `./emulator.command`
builds the same page from your checkout, serves it at http://localhost:8765 and opens it.

| Do | Mouse | Keyboard |
|---|---|---|
| Turn a knob | drag up or down, or scroll | ↑ ↓ LFO rate, ← → Wave I |
| Hold a button | Shift-click or right-click (click again to let go) | |
| PLAY · SAVE (hold) · SEL (hold) | | Space · Enter · C |
| HOME · EDIT · Movement − · + | | Esc · V · Z · X |
| Black keys 1–11 | click | 1 … 0, − |
| Scenes 1–16 | click | Q … I, A … K |

## Problems and ideas

It's a beta, tested on one FM-1 so far. If something goes wrong, sounds wrong or could be better, please
[open an issue](https://github.com/evanyerburgh/niagara-drift/issues): say what you did, what happened, and
what the startup screen says the version is.

## Building and testing

See [BUILDING.md](BUILDING.md) for the toolchain, including the extra steps on a Mac.

```
tests/run_tests.sh      # the instrument, the platform pieces, the app in a simulator
./emulator.command      # the same code in the browser: builds it, serves it, opens it
./build.sh              # build/omni.fwsc, the installable firmware
```

`build/host/omni_host SCRIPT OUTDIR` runs the whole app on a computer from a script
(`tests/scenarios/*.omni`), writing audio and screenshots. `python3 tools/make_screens.py` remakes the pictures of the pages
above (`docs/img/page-*.png`) from `tests/scenarios/pages.omni`.

Installing firmware is at your own risk. Before installing a development build, make sure you can
put M-VAVE's firmware back (see BUILDING.md).

## Credits

Niagara Drift is the next link in a chain of FM-1 firmwares:

- [Felucca](https://github.com/hugelton/Felucca) by Leo Kuroshita (Hügelton Instruments): the
  platform (hardware layer, USB, update loader, storage, installer), and a reference for the
  sound engine.
- [X0X](https://github.com/charlesvestal/fm1-x0x) and [FoMni](https://github.com/charlesvestal/fm1-fomni)
  by Charles Vestal: the platform reshaped for a single instrument, the host simulator and the
  browser emulator. Niagara Drift started from FoMni's code.

GPL-3.0-only; see [LICENSING.md](LICENSING.md).

Niagara Drift isn't affiliated with or endorsed by M-VAVE, Hügelton Instruments or the makers of any
instrument that inspired it.
