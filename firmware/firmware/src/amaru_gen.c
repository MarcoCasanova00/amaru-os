/* SPDX-License-Identifier: GPL-3.0-only
 * AMARU OS deterministic four-layer generator (pure planner).
 *
 * A genre is a musical grammar, not a mask: a scale set, a drum map of levels (after
 * Mutable Instruments' Grids), three bass archetypes written against the kick, three
 * chord progressions with a chord length, a pad style, a lead style, four part amounts
 * and a form. The grammar is AMARU's own re-authoring of the JIANT tables (GPL-3.0):
 * the same ideas sized down to four roles (AIR pad, MELODY lead, ROOT bass, SPARK
 * drums), four active engines (ANALOG, DIGITAL, DRUM, FM6) and 16..64 ordinary steps.
 * Nothing here touches the hardware, allocates, or reads a clock: the planner is a pure
 * function of (genre, seed, energy, form) so the host tests and the device agree.
 *
 * Every generated step is an ordinary step so GROW/EVOLVE, editing, undo and playback
 * keep working. LOOP is one bar, PARTS two, SONG four; the grammar varies the bars.
 */
#include "amaru_gen.h"

/* ------------------------------------------------------------------ modes --- */
/* Bunker scale indices: CHR MAJ MIN DOR MIX PEN MPEN HARM PHRY LYD ... */
enum { SC_MIN = 2, SC_DOR = 3, SC_MPEN = 6, SC_HARM = 7, SC_PHRY = 8, SC_MAJ = 1 };

static const int8_t SCALE_STEPS[6][7] = {
    {0, 2, 3, 5, 7, 8, 10},   /* natural minor */
    {0, 1, 3, 5, 7, 8, 10},   /* phrygian */
    {0, 2, 3, 5, 7, 9, 10},   /* dorian */
    {0, 2, 3, 5, 7, 8, 11},   /* harmonic minor */
    {0, 2, 4, 5, 7, 9, 11},   /* major */
    {0, 3, 5, 7, 10, 12, 14}  /* minor pentatonic */
};
static const uint8_t SCALE_LEN[6] = {7, 7, 7, 7, 7, 5};

/* AMARU's four active engines (Bunker indices are append-only; the rest are placeholders) */
enum { E_ANALOG = 0, E_DIGITAL = 1, E_DRUM = 10, E_FM6 = 12 };
/* Bunker V_POLY / V_MONO / V_LEGATO / V_UNISON */
enum { AMV_POLY = 0, AMV_MONO = 1, AMV_LEGATO = 2, AMV_UNISON = 3 };

#define ASND(nm, en, pr, tn, at, rl, ch, dl, rv, ds, vc) \
    { nm, en, pr, tn, at, rl, ch, dl, rv, ds, vc }

/* DRUM's preset numbers (src/eng_drum.c DRUM_PRESETS, in order): the kit a genre's drums play */
enum { DP_STD, DP_DEEP, DP_TIGHT, DP_PERC, DP_PSY, DP_FOREST, DP_HITECH,
       DP_808, DP_909, DP_606, DP_CR78, DP_707, DP_JUNGLE, DP_TECHNO, DP_HARDTEKK };

/* ------------------------------------------------------------- drum maps --- */
/* A lane's 16 steps as levels: '.' 0, '1'..'9' 26 a step (9 = 234), 'X' 255. A step
 * hits when its level clears the density; 'X' and '9' are anchors (always accented).
 * Lanes: BD SD CP CH OH TM RS CB (AMARU: KICK SNARE CLAP HAT OHAT TOM RIM BELL).
 * The breakbeat rows are AMARU's own reading of the breaks/DnB maps; JIANT has no
 * JUNGLE row, so that map is authored from the jungle break and its bass archetype. */
enum { DM_TRIBE, DM_MENTAL, DM_ACIDCORE, DM_HARDTEKK, DM_TEKNO, DM_PSY,
       DM_BREAKS, DM_DNB, DM_JUNGLE, DM_N };

static const char *const DRUM_MAP[DM_N][8] = {
    [DM_TRIBE]    = {"X...X...X...X...", "....X.......X...", "....4.......4...", "7.37.37.37.37.3.",
                     "..7...7...7...7.", "..5..5..5..5..5.", "...6..6..6..6..6", ".4..4.4...4..4.."},
    [DM_MENTAL]   = {"X...X...X...X...", "....X.......X...", "........4.......", "7373737373737373",
                     "..7...7...7...7.", "6..6..6..6..6..6", "5....5....5....5", ""},
    [DM_ACIDCORE] = {"X...X...X...X...", "....X.......X...", "....4.......4...", "7.7.7.7.7.7.7.7.",
                     "..7...7...7...7.", "..............4.", "", "......3.......3."},
    [DM_HARDTEKK] = {"X...X...X...X..8", "....X.......X...", "....4.......4...", "7.7.7.7.7.7.7.7.",
                     "..7...7...7...7.", "", "", "..3...3...3...3."},
    [DM_TEKNO]    = {"X...X...X..8X...", "....X.......X...", "....4.......4...", "7.7.7.7.7.7.7.7.",
                     "..7...7...7...7.", "......4.......4.", "5..5..5...5.5...", ""},
    [DM_PSY]      = {"X...X...X...X...", "....X.......X...", "....4.......4...", "7.7.7.7.7.7.7.7.",
                     "..7...7...7...7.", "", "..5...5...5...5.", ""},
    [DM_BREAKS]   = {"X.6.......8.....", "....X..3.3..X..3", "", "7.6.7.6.7.6.7.6.",
                     "......5.......5.", "", "", ""},
    [DM_DNB]      = {"X.........8.....", "....X.......X..3", "", "6.5.6.5.6.5.6.5.",
                     "", "", ".......3........", ""},
    [DM_JUNGLE]   = {"X..6..6...8.....", "....X..3..3.X..3", "", "7.5.7.5.7.5.7.5.",
                     "......4.......5.", "..5..5....5..5..", "", ""},
};

/* the hats' accent cells (percent of the hit's velocity on the four sixteenths of a beat) */
enum { HC_FLAT, HC_HOUSE, HC_FUNK, HC_MACHINE };
static const uint8_t HAT_CELL[4][4] = {{100, 100, 100, 100}, {100, 55, 80, 55},
                                       {90, 50, 70, 65}, {100, 70, 100, 70}};
/* the snare's ghost positions, the best first (the sixteenth before 2 and 4, then the "a"s) */
static const uint8_t GHOST_RANK[12] = {3, 7, 11, 15, 13, 5, 9, 1, 14, 6, 10, 2};

/* fills on the last bar of a multi-bar form */
enum { FL_NONE, FL_ROLL, FL_TOM, FL_DROP, FL_HAT, FL_LINEAR };
enum { KICK_INTERLOCK, KICK_ANSWER, KICK_SUSTAIN, KICK_DOUBLE };

/* ------------------------------------------------------- bass archetypes --- */
/* 16 steps a bar, a character a step: '.' rest, '=' the note before held on, 'R' root,
 * 'r' root soft, 'x' a dead note, '2' '3' '5' '6' '7' '9' chord degrees (in the scale),
 * 'L' the fifth below, '8' the octave, 'A' an approach a semitone under the next root,
 * 'B' from above, 'N' the next root early, 'S' a scale step towards it. Flags: 'a' accent,
 * 's' slide into the next note, 'g' ghost. bars: 1 or 2. */
enum { BA_TRIBE, BA_BOUNCE, BA_PSY, BA_MENTAL, BA_ROLL16, BA_ACID, BA_ACIDCORE,
       BA_HARDTEKK, BA_PUMP8, BA_DNB, BA_JUNGLE, BA_WOBBLE, BA_AROUND, BA_N };
typedef struct { const char *n, *f; uint8_t bars; } barch_t;
static const barch_t BASS_ARCH[BA_N] = {
    [BA_TRIBE]    = {"R..3..5.8.3..R.8", "a..s..a.s.s..a..", 1},
    [BA_BOUNCE]   = {"R.8.R.8.R.8.R.8A", "a..............s", 1},
    [BA_PSY]      = {".RRR.RRR.RRR.RRR", "", 1},
    [BA_MENTAL]   = {"R..2..3.R..5..R.", "a..s..s.a..s..a.", 1},
    [BA_ROLL16]   = {"RRRRRRRRRRRRRRRR", "..a...a...a...a.", 1},
    [BA_ACID]     = {0, 0, 0},                                  /* procedural, see make_bass */
    [BA_ACIDCORE] = {"RR8RR5RRRR8RR3RR", "a..a..a.a..a..a.", 1},
    [BA_HARDTEKK] = {"R.8.R.5.R.8.R.5.", "a...a...a...a...", 1},
    [BA_PUMP8]    = {"R=R=R=R=R=R=R=R=", "", 1},
    [BA_DNB]      = {"R=====..R==...7=", "..............s.", 1},
    [BA_JUNGLE]   = {"R..R..R...R.R...", "", 1},
    [BA_WOBBLE]   = {"R=========R===5=", "..............s.", 1},
    [BA_AROUND]   = {"R..x3.R.x5.5..NR", "...g....g.....s.", 1},
};

/* ----------------------------------------------------------- progressions --- */
/* Scale degrees (0 = the mode's tonic), one chord a bar: the triad stays diatonic, so
 * the mode decides the qualities and no chord can be out of key. */
enum { PR_ONE, PR_PHRY, PR_I_VII, PR_I_VI, PR_DEEP4, PR_TRANCE, PR_ANDAL,
       PR_I_IV, PR_MINOR4, PR_N };
typedef struct { uint8_t n, deg[4]; } prog_t;
static const prog_t PROG[PR_N] = {
    [PR_ONE]    = {1, {0}},
    [PR_PHRY]   = {2, {0, 1}},             /* i - II, the phrygian lift */
    [PR_I_VII]  = {2, {0, 6}},             /* i - VII */
    [PR_I_VI]   = {2, {0, 5}},             /* i - VI */
    [PR_DEEP4]  = {4, {0, 3, 5, 4}},       /* i - iv - VI - v */
    [PR_TRANCE] = {4, {0, 5, 2, 6}},       /* i - VI - III - VII */
    [PR_ANDAL]  = {4, {0, 6, 5, 4}},       /* the andalusian descent */
    [PR_I_IV]   = {2, {0, 3}},             /* i - iv */
    [PR_MINOR4] = {4, {0, 5, 3, 6}},       /* i - VI - iv - VII */
};

/* pad styles */
enum { PAD_STAB, PAD_DRONE, PAD_TRANCE, PAD_SPARSE, PAD_DUB, PAD_SWELL, PAD_COMP };
/* voicings */
enum { VM_PLAIN, VM_QUART, VM_RICH, VM_POWER, VM_PLANE };
/* lead styles */
enum { LD_HOOK, LD_OSTINATO, LD_MOTIF, LD_SPARSE, LD_MECH };

/* ---------------------------------------------------------------- genres --- */
typedef struct {
    const char *name;
    uint8_t lo, hi, swing;                 /* BPM range, G_SWING */
    uint8_t root;                          /* the key (0 = C) */
    uint8_t scale[4];                      /* the modes a throw picks from */
    uint8_t drum, drum_base, hats, ghosts, fills;
    uint8_t bass[3], bass_lo, kick_mode;
    uint8_t prog[3], cbars, pads, voicing;
    uint8_t lead;
    uint8_t amt[AMARU_TRACKS];             /* AIR MELODY ROOT SPARK density, all four live */
    amaru_sound_t snd[AMARU_TRACKS];
} amaru_genre_t;

/* The sounds are AMARU's recipe names (Carnac / Mycelium parameter shapes written as
 * numbers) on the four engines the firmware keeps: ANALOG, DIGITAL, DRUM, FM6. */
static const amaru_genre_t GENRES[AMARU_NGENRES] = {
    { "TRIBE TEKNO", 150, 166, 6, 2, {SC_DOR, SC_PHRY, SC_MIN, SC_MPEN},
      DM_TRIBE, 208, HC_FUNK, 6, FL_TOM,
      {BA_TRIBE, BA_BOUNCE, BA_PSY}, 33, KICK_INTERLOCK,
      {PR_ONE, PR_PHRY, PR_I_VII}, 4, PAD_STAB, VM_QUART, LD_HOOK,
      {40, 40, 90, 95},
      { ASND("AMARY QRIA", E_FM6, 4, 58, 110, 96, 40, 45, 90, 0, AMV_LEGATO),   /* Carnac 3 - Amazon green breath */
        ASND("MAYA PLUCK", E_ANALOG, 8, 70, 0, 55, 10, 40, 55, 0, AMV_POLY),
        ASND("AMARU BASS", E_ANALOG, 2, 44, 0, 35, 0, 10, 20, 12, AMV_MONO),
        ASND("GREEN KIT", E_DRUM, DP_909, 64, 0, 30, 0, 12, 14, 96, AMV_POLY) } },   /* Mycelium TEKNO: the 909, driven */

    { "MENTAL TEKNO", 155, 178, 4, 9, {SC_PHRY, SC_HARM, SC_MIN, SC_DOR},
      DM_MENTAL, 214, HC_MACHINE, 0, FL_NONE,
      {BA_MENTAL, BA_ROLL16, BA_PSY}, 33, KICK_SUSTAIN,
      {PR_ONE, PR_PHRY, PR_I_VII}, 4, PAD_DRONE, VM_PLANE, LD_OSTINATO,
      {20, 30, 85, 95},
      { ASND("TUPAC SHADOW", E_DIGITAL, 5, 58, 70, 112, 35, 35, 100, 0, AMV_POLY),  /* Carnac 4: the hearth's dark noise, far reverb */
        ASND("AZTEC SIREN", E_ANALOG, 6, 82, 8, 78, 20, 45, 60, 18, AMV_MONO),
        ASND("MENTAL SUB", E_ANALOG, 7, 36, 0, 60, 0, 12, 25, 0, AMV_MONO),
        ASND("MENTAL DRUMS", E_DRUM, DP_909, 64, 0, 30, 0, 14, 28, 0, AMV_POLY) } },

    { "HARDTEKK", 170, 180, 0, 5, {SC_MIN, SC_PHRY, SC_HARM, SC_DOR},
      DM_HARDTEKK, 220, HC_MACHINE, 0, FL_HAT,
      {BA_HARDTEKK, BA_PUMP8, BA_ROLL16}, 33, KICK_DOUBLE,
      {PR_ONE, PR_PHRY, PR_I_VII}, 4, PAD_STAB, VM_POWER, LD_SPARSE,
      {20, 25, 100, 100},
      { ASND("HARDTEKK PAD", E_ANALOG, 3, 66, 12, 55, 20, 25, 45, 25, AMV_POLY),
        ASND("HARDTEKK STAB", E_ANALOG, 6, 76, 0, 32, 8, 20, 35, 40, AMV_POLY),
        ASND("HARDTEKK BASS", E_ANALOG, 4, 52, 0, 26, 0, 5, 12, 35, AMV_MONO),
        ASND("MYC BLOWN", E_DRUM, DP_HARDTEKK, 60, 0, 24, 0, 10, 12, 105, AMV_POLY) } }, /* Mycelium BLOWN: the short driven kick, drive 72..105 */

    { "ACID CORE", 165, 180, 0, 3, {SC_PHRY, SC_MIN, SC_HARM, SC_MPEN},
      DM_ACIDCORE, 222, HC_MACHINE, 0, FL_DROP,
      {BA_ACID, BA_ACIDCORE, BA_ROLL16}, 33, KICK_ANSWER,
      {PR_ONE, PR_PHRY, PR_I_VII}, 4, PAD_STAB, VM_POWER, LD_MECH,
      {20, 25, 100, 100},
      { ASND("ACID MIST", E_DIGITAL, 5, 72, 35, 90, 20, 28, 80, 0, AMV_POLY),
        ASND("SILBURY WAIL", E_ANALOG, 4, 88, 0, 96, 20, 45, 20, 60, AMV_LEGATO),  /* Carnac 20: the wailing register, SH-2 drive 60 */
        ASND("SILBURY MOUND", E_ANALOG, 4, 44, 0, 30, 0, 10, 12, 60, AMV_LEGATO), /* Carnac 20: driven resonant SH-2, tied glide, an octave down */
        ASND("CORE DRUMS", E_DRUM, DP_TECHNO, 64, 0, 26, 0, 12, 20, 0, AMV_POLY) } },

    { "JUNGLE", 160, 170, 8, 2, {SC_MIN, SC_DOR, SC_PHRY, SC_MPEN},
      DM_JUNGLE, 206, HC_FUNK, 16, FL_LINEAR,
      {BA_JUNGLE, BA_DNB, BA_AROUND}, 31, KICK_INTERLOCK,
      {PR_I_VI, PR_MINOR4, PR_ONE}, 2, PAD_DUB, VM_RICH, LD_MOTIF,
      {45, 35, 95, 95},
      { ASND("JUNGLE CHOIR", E_FM6, 4, 55, 60, 105, 30, 32, 92, 0, AMV_POLY),
        ASND("JUNGLE STAB", E_ANALOG, 8, 74, 0, 46, 12, 38, 55, 0, AMV_POLY),
        ASND("JUNGLE SUB", E_ANALOG, 7, 30, 0, 70, 0, 8, 18, 0, AMV_MONO),
        ASND("JUNGLE DRUMS", E_DRUM, DP_JUNGLE, 64, 0, 30, 0, 14, 28, 0, AMV_POLY) } },

    { "DRUM & BASS", 170, 178, 0, 7, {SC_MIN, SC_DOR, SC_PHRY, SC_MPEN},
      DM_DNB, 210, HC_FLAT, 14, FL_LINEAR,
      {BA_DNB, BA_JUNGLE, BA_WOBBLE}, 29, KICK_DOUBLE,
      {PR_I_VI, PR_DEEP4, PR_ONE}, 2, PAD_SPARSE, VM_RICH, LD_SPARSE,
      {50, 30, 95, 95},
      { ASND("DNB PAD", E_FM6, 4, 60, 55, 110, 28, 30, 88, 0, AMV_POLY),
        ASND("DNB REESE", E_DIGITAL, 3, 80, 0, 55, 18, 36, 45, 18, AMV_MONO),
        ASND("DNB SUB", E_ANALOG, 7, 28, 0, 80, 0, 6, 15, 0, AMV_MONO),
        ASND("DNB DRUMS", E_DRUM, DP_JUNGLE, 64, 0, 30, 0, 14, 28, 0, AMV_POLY) } },

    { "BREAKBEAT TEKNO", 140, 150, 6, 5, {SC_MIN, SC_DOR, SC_PHRY, SC_MPEN},
      DM_BREAKS, 208, HC_FUNK, 18, FL_LINEAR,
      {BA_ACID, BA_JUNGLE, BA_AROUND}, 33, KICK_INTERLOCK,
      {PR_ONE, PR_I_IV, PR_I_VI}, 2, PAD_STAB, VM_RICH, LD_OSTINATO,
      {45, 40, 90, 95},
      { ASND("BREAK PAD", E_ANALOG, 11, 58, 45, 95, 30, 28, 78, 0, AMV_POLY),
        ASND("BREAK STAB", E_ANALOG, 8, 78, 0, 40, 10, 30, 45, 18, AMV_POLY),
        ASND("BREAK BASS", E_ANALOG, 2, 46, 0, 38, 0, 8, 18, 18, AMV_MONO),
        ASND("BREAK DRUMS", E_DRUM, DP_707, 64, 0, 28, 0, 14, 26, 0, AMV_POLY) } },

    { "TECHNO", 140, 155, 4, 7, {SC_MIN, SC_PHRY, SC_DOR, SC_HARM},
      DM_TEKNO, 212, HC_MACHINE, 4, FL_DROP,
      {BA_ROLL16, BA_PSY, BA_BOUNCE}, 33, KICK_SUSTAIN,
      {PR_ONE, PR_PHRY, PR_I_VII}, 4, PAD_DRONE, VM_PLANE, LD_OSTINATO,
      {30, 30, 85, 95},
      { ASND("TECHNO PAD", E_FM6, 4, 62, 50, 100, 25, 30, 80, 0, AMV_POLY),
        ASND("TECHNO LEAD", E_ANALOG, 0, 82, 0, 50, 20, 40, 45, 10, AMV_MONO),
        ASND("TECHNO BASS", E_DIGITAL, 2, 48, 0, 42, 8, 20, 20, 22, AMV_MONO),
        ASND("TECHNO DRUMS", E_DRUM, DP_TECHNO, 64, 0, 30, 0, 12, 24, 0, AMV_POLY) } },

    { "PSYTRANCE", 140, 148, 0, 9, {SC_PHRY, SC_MIN, SC_HARM, SC_DOR},
      DM_PSY, 216, HC_MACHINE, 0, FL_ROLL,
      {BA_PSY, BA_ROLL16, BA_PUMP8}, 33, KICK_SUSTAIN,
      {PR_PHRY, PR_TRANCE, PR_ANDAL}, 4, PAD_TRANCE, VM_RICH, LD_MOTIF,
      {100, 45, 100, 100},
      { ASND("ORACLE CHOIR", E_FM6, 4, 66, 90, 110, 35, 40, 92, 0, AMV_POLY),      /* Carnac 3: the menhir-thud strike, the vowel opens */
        ASND("PSY LEAD", E_ANALOG, 0, 92, 0, 55, 30, 50, 55, 12, AMV_UNISON),
        ASND("PSY ROLL", E_ANALOG, 4, 52, 0, 30, 10, 22, 22, 18, AMV_MONO),
        ASND("PSY DRUMS", E_DRUM, DP_PSY, 64, 0, 28, 0, 12, 24, 0, AMV_POLY) } }
};

const char *amaru_genre_name(uint8_t genre)
{
    return genre < AMARU_NGENRES ? GENRES[genre].name : "TECHNO";
}

const char *amaru_form_name(uint8_t form)
{
    static const char *const n[AMARU_NFORMS] = {"LOOP", "PARTS", "SONG"};
    return form < AMARU_NFORMS ? n[form] : "LOOP";
}

/* ---------------------------------------------------------------- random --- */
/* One stream a part: a part does not change because another part's generator changed. */
static uint32_t rng_state;
static uint32_t rnd(void)
{
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    if (!x)
        x = 0x9E3779B9u;
    rng_state = x;
    return x;
}
static void seed_part(uint16_t seed, uint8_t genre, uint8_t form, uint8_t energy, uint8_t part)
{
    uint32_t x = 0x9E3779B9u ^ ((uint32_t)seed * 0x2545F491u);
    x ^= (uint32_t)genre * 0x85EBCA6Bu;
    x ^= (uint32_t)form * 0xC2B2AE35u;
    x ^= (uint32_t)energy * 0x27D4EB2Fu;
    x ^= ((uint32_t)part + 1u) * 0x165667B1u;
    x ^= x >> 15;
    x *= 0x2545F491u;
    x ^= x >> 13;
    rng_state = x ? x : 0x42554E4Bu;
}
static int chance(uint32_t pct) { return pct && rnd() % 100u < pct; }
static int rr(int lo, int hi) { return hi <= lo ? lo : lo + (int)(rnd() % (uint32_t)(hi - lo + 1)); }
static int pick_idx(int n) { return n > 0 ? (int)(rnd() % (uint32_t)n) : 0; }
static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

/* --------------------------------------------------------------- helpers --- */
static int scale_mode(uint8_t bunker_scale)
{
    switch (bunker_scale) {
    case SC_PHRY: return 1;
    case SC_DOR:  return 2;
    case SC_HARM: return 3;
    case SC_MAJ:  return 4;
    case SC_MPEN: return 5;
    default:      return 0;   /* natural minor */
    }
}
static int scale_deg(const amaru_plan_t *p, int deg)
{
    int m = scale_mode(p->scale);
    int n = SCALE_LEN[m];
    int oct = 0;
    while (deg < 0) { deg += n; oct--; }
    oct += deg / n;
    return oct * 12 + SCALE_STEPS[m][deg % n];
}
static int note_at(const amaru_plan_t *p, int deg, int octave)
{
    int n = p->root + octave * 12 + scale_deg(p, deg);
    while (n < 12) n += 12;
    while (n > 108) n -= 12;
    return n;
}
static uint8_t lvl_of(char c)
{
    if (c == 'X' || c == 'x')
        return 255;
    if (c >= '1' && c <= '9')
        return (uint8_t)((c - '0') * 26);
    return 0;
}
static void clear_steps(amaru_plan_t *p)
{
    for (int t = 0; t < AMARU_TRACKS; t++)
        for (int i = 0; i < AMARU_STEPS; i++) {
            p->step[t][i].n = 0;
            p->step[t][i].time = AMARU_ST_REST;
            p->step[t][i].vel = 0;
            p->step[t][i].hit = 0;
            p->step[t][i].acc = 0;
            p->step[t][i].flags = 0;
            p->step[t][i].probability = 0;
        }
}

/* The chord plan: one diatonic triad a bar (or a chord every `cbars` bars). */
static uint8_t chord_deg[AMARU_STEPS];
static uint8_t chord_at[AMARU_STEPS];
static uint8_t kick_at[AMARU_STEPS];

static void build_chords(amaru_plan_t *p, const amaru_genre_t *g, uint8_t pi, uint8_t cbars)
{
    const prog_t *pr = &PROG[g->prog[pi]];
    int bars = p->steps / 16;
    if (cbars < 1) cbars = 1;
    if (bars >= 4 && cbars > 2) cbars = 2;                  /* a four-bar plan still changes chord */
    if (cbars > bars) cbars = (uint8_t)(bars ? bars : 1);
    if (p->form == AMARU_LOOP && pr->n > 1) cbars = 1;      /* a loop still moves */
    for (int i = 0; i < p->steps; i++) {
        int b = i / 16;
        int k = (b / cbars) % pr->n;
        chord_deg[i] = pr->deg[k];
        chord_at[i] = (uint8_t)((i % 16) == 0);
    }
}

/* ---------------------------------------------------------------- drums --- */
static void d_hit(amaru_plan_t *p, int i, int lane, int vel, int acc)
{
    if (i < 0 || i >= p->steps)
        return;
    amaru_step_t *s = &p->step[AMARU_SPARK][i];
    if (s->time != AMARU_ST_NOTE || !s->hit) {
        s->time = AMARU_ST_NOTE;
        s->vel = (uint8_t)clampi(vel, 28, 127);
        s->flags = 0;
    } else if (!acc) {
        s->vel = (uint8_t)((s->vel + clampi(vel, 28, 127)) / 2);
    }
    s->hit |= (uint8_t)(1u << lane);
    if (acc)
        s->acc |= (uint8_t)(1u << lane);
    else
        s->acc &= (uint8_t)~(1u << lane);
}

static void make_drums(amaru_plan_t *p, const amaru_genre_t *g)
{
    static const int8_t OFF[8] = {40, 20, 20, 0, 0, -20, -20, -20};
    int dens = 40 + g->amt[AMARU_SPARK] * 2 + ((int)p->energy - 50) / 2;
    const uint8_t *cell = HAT_CELL[g->hats];
    int bars = p->steps / 16;
    int ghosts = (g->ghosts * g->amt[AMARU_SPARK] + 50) / 100;
    if (!bars) bars = 1;
    dens = clampi(dens, 20, 255);

    for (int i = 0; i < p->steps; i++) {
        int b = i / 16, pos = i % 16;
        int hit = 0, acc = 0, lv_sum = 0, lv_n = 0, ghost_lv = 0, hat_only = 1;

        for (int l = 0; l < 8; l++) {
            const char *m = DRUM_MAP[g->drum][l];
            int lv = (m && m[pos]) ? lvl_of(m[pos]) : 0, lim;
            if (!lv)
                continue;
            if (lv < 230 && b) {                      /* Grids: the first bar pure, later bars perturbed */
                int k = (b % 4 == 3) ? 6 + p->energy / 8 : (b & 1) ? 3 + p->energy / 16 : 0;
                if (k)
                    lv += (int)(rnd() % (uint32_t)(k + 1));
            }
            lim = clampi(dens + OFF[l], 0, 255);
            if (lv <= 255 - lim)
                continue;
            if (l >= 3 && lv < 230 && g->amt[AMARU_SPARK] < 100 &&
                (int)(rnd() % 100u) >= 55 + g->amt[AMARU_SPARK] * 45 / 100)
                continue;                             /* the top end thinned by the part's amount */
            hit |= 1 << l;
            if (lv >= 230)
                acc |= 1 << l;
            else {
                lv_sum += lv;
                lv_n++;
            }
            if (l < 3 || l > 4)
                hat_only = 0;
            if (lv < 90 && l < 3)
                ghost_lv = lv;
        }
        if (!hit)
            continue;

        {
            int vel = lv_n ? 56 + lv_sum / lv_n * 58 / 255 : 100;
            if (ghost_lv)
                vel = 36 + ghost_lv * 20 / 90;
            if (hat_only)
                vel = vel * cell[pos % 4] / 100;
            if (!acc)
                vel = vel * (88 + (int)p->energy / 4) / 100;
            if (g->amt[AMARU_SPARK] > 60)
                vel += rr(-6, 6);                     /* a band, not a machine */
            amaru_step_t *s = &p->step[AMARU_SPARK][i];
            s->time = AMARU_ST_NOTE;
            s->hit = (uint8_t)hit;
            s->acc = (uint8_t)(acc & hit);
            s->vel = (uint8_t)clampi(vel, 30, 127);
            if ((hit & AMARU_L_KICK))
                kick_at[i] = 1;
        }
    }

    if (p->energy > 66 && DRUM_MAP[g->drum][3][0]) {  /* the hats driven: the empty sixteenth after a closed hat */
        uint8_t ch[AMARU_STEPS];
        for (int i = 0; i < p->steps; i++)
            ch[i] = (uint8_t)(p->step[AMARU_SPARK][i].time == AMARU_ST_NOTE &&
                              (p->step[AMARU_SPARK][i].hit & AMARU_L_HAT));
        for (int i = 1; i < p->steps; i++) {
            amaru_step_t *s = &p->step[AMARU_SPARK][i];
            if ((s->time == AMARU_ST_NOTE && (s->hit & (AMARU_L_HAT | AMARU_L_OHAT))) ||
                !ch[i - 1] || !chance((uint32_t)(55 + p->energy / 4)))
                continue;
            d_hit(p, i, 3, 44 + (int)p->energy / 5, 0);
        }
    } else if (p->energy < 34) {                      /* the room: half the open hats and the percussion gone */
        for (int i = 0; i < p->steps; i++) {
            amaru_step_t *s = &p->step[AMARU_SPARK][i];
            if (s->time != AMARU_ST_NOTE || !(s->hit & (AMARU_L_OHAT | AMARU_L_RIM | AMARU_L_BELL)) || !chance(50))
                continue;
            s->hit &= (uint8_t)~(AMARU_L_OHAT | AMARU_L_RIM | AMARU_L_BELL);
            s->acc &= s->hit;
            if (!s->hit)
                s->time = AMARU_ST_REST;
        }
    }

    for (int b = 0; b < bars && ghosts > 0; b++) {    /* the snare's ghosts: the best positions first */
        int placed = 0;
        for (int k = 0; k < 12 && placed < ghosts; k++) {
            int pos = GHOST_RANK[k], i = b * 16 + pos;
            amaru_step_t *s = &p->step[AMARU_SPARK][i];
            if (i >= p->steps || (s->time == AMARU_ST_NOTE && (s->hit & (AMARU_L_SNARE | AMARU_L_CLAP))))
                continue;
            if (!chance(55 + p->energy / 3))
                continue;
            d_hit(p, i, 1, rr(34, 50), 0);
            placed++;
        }
    }

    if (p->energy > 45) {                             /* a variation on the even bars: a weak kick, a moved rim */
        for (int i = 16; i < p->steps; i++) {
            int b = i / 16, pos = i % 16;
            amaru_step_t *s = &p->step[AMARU_SPARK][i];
            if (!(b & 1) || (pos & 1))
                continue;
            if (s->time != AMARU_ST_NOTE && chance((uint32_t)(8 + p->energy / 8)) &&
                (pos == 7 || pos == 10 || pos == 11 || pos == 14) && g->kick_mode != KICK_ANSWER)
                d_hit(p, i, 0, 80, 0);
            else if (s->time == AMARU_ST_NOTE && (s->hit & (AMARU_L_RIM | AMARU_L_BELL)) &&
                     chance((uint32_t)(6 + p->energy / 12))) {
                int rim = s->hit & AMARU_L_RIM;
                s->hit &= (uint8_t)~(AMARU_L_RIM | AMARU_L_BELL);
                s->hit |= (uint8_t)(rim ? AMARU_L_BELL : AMARU_L_RIM);
                s->acc &= s->hit;
            }
        }
    }

    {   /* the fill on the last bar of a multi-bar form; LOOP keeps its one bar */
        int last = bars - 1, base = last * 16, lane = DRUM_MAP[g->drum][1][0] ? 1 : 6;
        if (bars >= 2 && g->fills != FL_NONE && (bars >= 4 || chance(50))) {
            switch (g->fills) {
            case FL_ROLL:
                for (int k = 0; k < 4; k++) {
                    int i = base + 12 + k;
                    p->step[AMARU_SPARK][i].time = AMARU_ST_REST;
                    p->step[AMARU_SPARK][i].hit = 0;
                    d_hit(p, i, lane, 70 + k * 15, k == 3);
                    if (p->energy > 55 && k >= 2)
                        p->step[AMARU_SPARK][i].flags |= AMARU_SF_RATCH2;
                }
                break;
            case FL_TOM: {
                static const uint8_t TM[8] = {1, 0, 1, 1, 0, 1, 0, 1};
                for (int k = 0; k < 8; k++)
                    if (TM[k])
                        d_hit(p, base + 8 + k, 5, 88 + k * 3, k == 0 || k == 5);
                break;
            }
            case FL_DROP:
                if (base + 15 < p->steps)
                    d_hit(p, base + 15, 4, 100, 1);
                break;
            case FL_HAT:
                for (int k = 12; k < 16; k++) {
                    int i = base + k;
                    d_hit(p, i, 3, 70 + (k - 12) * 12, 0);
                    if (i < p->steps && k < 14)
                        p->step[AMARU_SPARK][i].flags |= AMARU_SF_RATCH2;
                }
                break;
            case FL_LINEAR: {
                static const uint8_t BD[8] = {1, 0, 1, 0, 1, 0, 0, 1}, SD[8] = {0, 1, 0, 1, 0, 1, 1, 0};
                for (int k = 0; k < 8; k++) {
                    if (BD[k])
                        d_hit(p, base + 8 + k, 0, 100, k == 0);
                    if (SD[k])
                        d_hit(p, base + 8 + k, 1, 84, 0);
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

/* ----------------------------------------------------------------- bass --- */
static int bass_root_of(const amaru_plan_t *p, int cd, int lo)
{
    int n = note_at(p, cd, 1);
    while (n < lo) n += 12;
    while (n >= lo + 12) n -= 12;
    return n;
}
static int tone_of(const amaru_plan_t *p, int cd, int root, int off)
{
    return root + (scale_deg(p, cd + off) - scale_deg(p, cd));
}
static int bass_off(char c)
{
    switch (c) {
    case '2': return 1;
    case '3': return 2;
    case '5': return 4;
    case '6': return 5;
    case '7': return 6;
    case '9': return 8;
    default:  return 0;
    }
}
/* A riff of the throw: the archetype with one to three steps moved, the same every bar. */
static void vary_riff(char *bc, int alen)
{
    int n = rr(1, 2), j = 0;
    for (int tries = 0; j < n && tries < 24; tries++) {
        int pos = 1 + pick_idx(alen - 1), next = (pos + 1) % alen;
        char c = bc[pos], prev = bc[pos - 1];
        int op = pick_idx(4);
        if (!(pos & 3) || bc[next] == '=')
            continue;                                 /* the beats kept, a held note kept whole */
        if (op == 0 && (c == '.' || c == 0) && (pos & 1) && (prev == 'R' || prev == '8' || prev == '5'))
            bc[pos] = chance(50) ? 'x' : 'R';
        else if (op == 1 && c == 'R')
            bc[pos] = chance(60) ? '9' : '6';
        else if (op == 2 && (c == 'R' || c == '6' || c == '9') && prev != '.' && prev != 0)
            bc[pos] = '.';
        else if (op == 3 && (next & 3) && ((c == '.' || c == 0) != (bc[next] == '.' || bc[next] == 0)) && c != '=')
            bc[pos] = bc[next];
        else
            continue;
        j++;
    }
}

static void make_bass(amaru_plan_t *p, const amaru_genre_t *g, uint8_t bi)
{
    const barch_t *a = &BASS_ARCH[g->bass[bi]];
    int lo = g->bass_lo;
    int held = 0;

    if (!a->n) {                                      /* BA_ACID: the 303 line, thrown against the chord */
        static const char D[6] = {'R', '8', '7', '5', '3', '2'};
        static const uint8_t W[6] = {55, 15, 10, 10, 5, 5};
        uint8_t on[16], pt[16], acc[16], sl[16];
        for (int i = 0; i < 16; i++) {
            on[i] = (uint8_t)((i % 4 == 0) || chance((uint32_t)(45 + p->energy / 3)));
            int k = pick_idx(6);
            (void)W;
            pt[i] = (uint8_t)k;
            acc[i] = sl[i] = 0;
        }
        for (int k = 0, tries = 0; k < rr(3, 5) && tries < 32; tries++) {
            int pos = pick_idx(16);
            if (on[pos] && !acc[pos]) { acc[pos] = 1; k++; }
        }
        for (int k = 0, tries = 0; k < rr(2, 3) && tries < 32; tries++) {
            int pos = pick_idx(15);
            if (on[pos] && on[pos + 1] && !sl[pos]) { sl[pos] = 1; k++; }
        }
        for (int i = 0; i < p->steps; i++) {
            int pos = i % 16, cd = chord_deg[i], root = bass_root_of(p, cd, lo), n;
            if (!on[pos])
                continue;
            n = chord_at[i] ? root : pt[pos] == 0 ? root + 12 : tone_of(p, cd, root, bass_off(D[pt[pos]]));
            if (n > lo + 26) n -= 12;
            amaru_step_t *s = &p->step[AMARU_ROOT][i];
            s->n = 1;
            s->note[0] = (uint8_t)clampi(n, 12, 108);
            s->time = AMARU_ST_NOTE;
            s->flags = (uint8_t)((acc[pos] ? AMARU_SF_ACCENT : 0) | (sl[pos] ? AMARU_SF_SLIDE : 0));
            s->vel = (uint8_t)(acc[pos] ? 112 : 90);
        }
        return;
    }

    {
        int alen = 16 * a->bars;
        char bc[AMARU_STEPS];
        for (int i = 0; i < alen; i++) {
            bc[i] = a->n[i];
            if (!bc[i])
                bc[i] = '.';
        }
        vary_riff(bc, alen);

        for (int i = 0; i < p->steps; i++) {
            char c = bc[i % alen], f = a->f ? a->f[i % alen] : '.';
            int cd = chord_deg[i], root, n, vel = 96, fl = 0;
            int chg = chord_at[i], nd = chord_deg[(i + 1 < p->steps) ? i + 1 : 0];

            if (chg && (i % 16) && (c == '.' || c == '='))
                c = 'R';                              /* a chord that comes in mid-bar: its root there */
            if (c == '.') { held = 0; continue; }
            if (c == '=') {
                if (held)
                    p->step[AMARU_ROOT][i].time = AMARU_ST_TIE;
                continue;
            }
            if (kick_at[i] && (i % 16) && !chg && c != 'x' &&
                (g->kick_mode == KICK_ANSWER || g->kick_mode == KICK_INTERLOCK))
                continue;                             /* the kick's step is the kick's */

            root = bass_root_of(p, cd, lo);
            switch (c) {
            case '8': n = root + 12; break;
            case 'L': n = tone_of(p, cd, root, 4) - 12; break;
            case 'A': n = (nd == cd) ? tone_of(p, cd, root, 4) : bass_root_of(p, nd, lo) - 1; break;
            case 'B': n = (nd == cd) ? tone_of(p, cd, root, 4) : bass_root_of(p, nd, lo) + 1; break;
            case 'N': n = bass_root_of(p, nd, lo); break;
            case 'S': {
                int r2 = bass_root_of(p, nd, lo);
                n = (nd == cd) ? tone_of(p, cd, root, 1) : r2 + (r2 > root ? -1 : 1);
                break;
            }
            case 'R': case 'r': case 'x': n = root; break;
            default: n = tone_of(p, cd, root, bass_off(c)); break;
            }
            if (chg && c != 'x')
                n = (c == '8') ? root + 12 : root;
            if (n > lo + 26) n -= 12;
            if (n < lo - 7) n += 12;
            n = clampi(n, 12, 108);

            if (c == 'r') vel = 70;
            if (c == 'x') vel = 42;
            if (f == 'g') vel = 50;
            if (f == 'a') { vel = 110; fl |= AMARU_SF_ACCENT; }
            if (f == 's') fl |= AMARU_SF_SLIDE;
            if (g->amt[AMARU_ROOT] > 70)
                vel = clampi(vel + rr(-6, 6), 28, 127);

            amaru_step_t *s = &p->step[AMARU_ROOT][i];
            s->n = 1;
            s->note[0] = (uint8_t)n;
            s->time = AMARU_ST_NOTE;
            s->flags = (uint8_t)fl;
            s->vel = (uint8_t)vel;
            held = 1;
        }
    }

    if (g->kick_mode == KICK_DOUBLE || g->kick_mode == KICK_INTERLOCK) {
        for (int i = 0; i < p->steps; i++) {
            int j = (g->kick_mode == KICK_DOUBLE) ? i : i + 1;
            if (!kick_at[i] || j >= p->steps || p->step[AMARU_ROOT][j].time != AMARU_ST_REST)
                continue;
            if (!chance((uint32_t)(g->kick_mode == KICK_DOUBLE ? 30 + g->amt[AMARU_ROOT] / 4 : 35)))
                continue;
            amaru_step_t *s = &p->step[AMARU_ROOT][j];
            s->n = 1;
            s->note[0] = (uint8_t)clampi(bass_root_of(p, chord_deg[j], lo), 12, 108);
            s->time = AMARU_ST_NOTE;
            s->vel = (uint8_t)(g->kick_mode == KICK_DOUBLE ? 92 : 72);
        }
    }

    if (g->amt[AMARU_ROOT] < 60) {                    /* a thinner bass: the weak notes dropped */
        for (int i = 0; i < p->steps; i++) {
            amaru_step_t *s = &p->step[AMARU_ROOT][i];
            if (s->time != AMARU_ST_NOTE || chord_at[i] || (s->flags & AMARU_SF_ACCENT))
                continue;
            if (chance(35)) {
                s->n = 0;
                s->time = AMARU_ST_REST;
                s->vel = 0;
                s->flags = 0;
            }
        }
    }
}

/* ------------------------------------------------------------------ pad --- */
static int chord_notes(const amaru_plan_t *p, int cd, int octave, int voicing, uint8_t *n)
{
    int cnt = 3, deg[4];
    switch (voicing) {
    case VM_QUART: deg[0] = cd;     deg[1] = cd + 3; deg[2] = cd + 5; break;
    case VM_RICH:  deg[0] = cd;     deg[1] = cd + 2; deg[2] = cd + 4; deg[3] = cd + 6; cnt = 4; break;
    case VM_POWER: deg[0] = cd;     deg[1] = cd + 4; deg[2] = cd + 7; break;
    case VM_PLANE: deg[0] = cd + 2; deg[1] = cd + 4; deg[2] = cd + 7; break;
    default:       deg[0] = cd;     deg[1] = cd + 2; deg[2] = cd + 4; break;
    }
    for (int i = 0; i < cnt; i++)
        n[i] = (uint8_t)note_at(p, deg[i], octave);
    return cnt;
}
static void put_chord(amaru_plan_t *p, int i, int cd, int voicing, int vel, int acc)
{
    uint8_t n[4];
    int cnt = chord_notes(p, cd, 3, voicing, n);
    amaru_step_t *s = &p->step[AMARU_AIR][i];
    s->n = (uint8_t)cnt;
    for (int k = 0; k < cnt; k++)
        s->note[k] = n[k];
    s->time = AMARU_ST_NOTE;
    s->flags = (uint8_t)(acc ? AMARU_SF_ACCENT : 0);
    s->vel = (uint8_t)clampi(vel, 30, 127);
}

static void make_pad(amaru_plan_t *p, const amaru_genre_t *g)
{
    uint8_t on[AMARU_STEPS];

    for (int i = 0; i < p->steps; i++) {
        int pos = i % 16;
        on[i] = chord_at[i];
        switch (g->pads) {
        case PAD_STAB:   if (pos == 8) on[i] = 1; break;
        case PAD_TRANCE: if (pos == 2 || pos == 6 || pos == 10 || pos == 14) on[i] = 1; break;
        case PAD_DUB:    if (pos == 2 || pos == 6 || pos == 10 || pos == 14) on[i] = 1; break;
        case PAD_COMP:   if (pos == 6 || pos == 10 || pos == 14) on[i] = 1; break;
        default: break;                            /* DRONE, SWELL, SPARSE: the changes only */
        }
    }

    for (int i = 0; i < p->steps; i++) {
        int hold = 0, vel, acc = 1;
        if (!on[i])
            continue;
        if (g->pads == PAD_DRONE || g->pads == PAD_SWELL)
            hold = p->steps - 1 - i;
        else if (g->pads == PAD_SPARSE)
            hold = 6;
        else if (g->pads == PAD_DUB)
            hold = 2;
        vel = (g->pads == PAD_SWELL) ? 72 : (i % 16) ? 78 : 88;
        acc = chord_at[i];
        if (g->amt[AMARU_AIR] > 70)
            vel = clampi(vel + rr(-5, 5), 30, 127);
        put_chord(p, i, chord_deg[i], g->voicing, vel, acc);
        for (int k = 1; k <= hold && i + k < p->steps; k++) {
            if (on[i + k] || chord_at[i + k])
                break;
            p->step[AMARU_AIR][i + k].time = AMARU_ST_TIE;
        }
    }
}

/* ----------------------------------------------------------------- lead --- */
static void make_lead(amaru_plan_t *p, const amaru_genre_t *g)
{
    static const int8_t CT[6] = {0, 2, 4, 7, 9, 11};        /* degrees over the chord's root */
    int8_t motif[8];
    int mlen = 4;

    for (int i = 0; i < 8; i++)                             /* the throw's motif: chord tones, a contour */
        motif[i] = CT[pick_idx(6)];
    if (g->lead == LD_OSTINATO || g->lead == LD_MECH)
        mlen = 3;
    else if (g->lead == LD_MOTIF)
        mlen = 5;

    for (int i = 0; i < p->steps; i++) {
        int pos = i % 16, bar = i / 16, on = 0, idx = 0, acc = 0, slide = 0;

        switch (g->lead) {
        case LD_HOOK:
            on = (pos == 0 || pos == 3 || pos == 6 || pos == 10 || pos == 14);
            idx = (pos / 2 + bar) % mlen;
            acc = pos == 0;
            break;
        case LD_OSTINATO:
            on = !(pos & 1);
            idx = (pos / 2 + bar) % mlen;
            acc = pos == 0;
            break;
        case LD_MOTIF:
            on = (pos == 0 || pos == 3 || pos == 6 || pos == 10 || pos == 12);
            idx = ((pos / 2) + (bar & 1) * 2) % mlen;
            acc = pos == 0;
            break;
        case LD_SPARSE:
            on = (pos == 0 || pos == 6 || pos == 10);
            idx = (pos / 4) % mlen;
            acc = pos == 0;
            break;
        default: /* LD_MECH */
            on = (pos != 3 && pos != 5 && pos != 9 && pos != 13);
            idx = (pos + bar) % mlen;
            acc = pos == 0 || pos == 8;
            slide = (pos % 4) == 2 && chance(30);
            break;
        }
        if (!on)
            continue;
        if (g->lead == LD_SPARSE && chance(25))
            continue;                                       /* air in the sparse line */

        {
            int deg = chord_deg[i] + motif[idx];
            int n = note_at(p, deg, 4);
            amaru_step_t *s = &p->step[AMARU_MELODY][i];
            s->n = 1;
            s->note[0] = (uint8_t)n;
            s->time = AMARU_ST_NOTE;
            s->flags = (uint8_t)((acc ? AMARU_SF_ACCENT : 0) | (slide ? AMARU_SF_SLIDE : 0));
            s->vel = (uint8_t)(acc ? 116 : 92);
        }
    }
}

#undef ASND

/* The public entry point. */
void amaru_plan(amaru_plan_t *out, uint8_t genre, uint16_t seed, uint8_t energy, uint8_t form)
{
    if (!out) return;
    if (genre >= AMARU_NGENRES) genre = AMARU_TECHNO;
    if (form >= AMARU_NFORMS) form = AMARU_LOOP;
    if (energy > 100) energy = 100;

    const amaru_genre_t *g = &GENRES[genre];
    for (int i = 0; i < (int)sizeof(*out); i++) ((uint8_t *)out)[i] = 0;

    out->genre = genre;
    out->form = form;
    out->energy = energy;
    out->seed = seed;
    out->steps = (uint8_t)(form == AMARU_LOOP ? 16 : form == AMARU_PARTS ? 32 : 64);

    /* the throw's own choices: a mode, a key, a progression, a bass archetype, a tempo */
    seed_part(seed, genre, form, energy, AMARU_TRACKS);
    out->scale = g->scale[pick_idx(4)];
    out->root = (uint8_t)((g->root + (chance(25) ? pick_idx(2) : 0)) % 12);
    out->bpm = (uint8_t)rr(g->lo, g->hi);
    out->swing = g->swing;
    {
        uint8_t pi = (uint8_t)pick_idx(3);
        uint8_t bi = (uint8_t)pick_idx(3);
        build_chords(out, g, pi, g->cbars);
        for (int t = 0; t < AMARU_TRACKS; t++) {
            out->sound[t] = g->snd[t];
            int tone = (int)g->snd[t].tone + rr(-4, 4);
            out->sound[t].tone = (uint8_t)clampi(tone, 0, 127);
        }
        clear_steps(out);
        for (int i = 0; i < AMARU_STEPS; i++) kick_at[i] = 0;

        seed_part(seed, genre, form, energy, AMARU_SPARK);
        make_drums(out, g);
        seed_part(seed, genre, form, energy, AMARU_ROOT);
        make_bass(out, g, bi);
        seed_part(seed, genre, form, energy, AMARU_AIR);
        make_pad(out, g);
        seed_part(seed, genre, form, energy, AMARU_MELODY);
        make_lead(out, g);
    }
}
