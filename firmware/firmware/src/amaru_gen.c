/* SPDX-License-Identifier: GPL-3.0-only
 * AMARU OS deterministic four-layer generator (pure planner).
 *
 * The grammar is deliberately explicit rather than genre-name-driven: a genre
 * owns a kick/snare/hat/percussion mask, a bass mode, a pad mode and a lead
 * mode. Jungle, drum & bass and breakbeat use broken kick masks; they are not
 * four-on-the-floor. Every note is written as an ordinary step so Bunker's
 * GROW/EVOLVE and editors keep working on generated material.
 */
#include "amaru_gen.h"

/* --------------------------------------------------------------- modes --- */
enum { BASS_SUB, BASS_OFF, BASS_ROLL, BASS_ACID, BASS_STAB, BASS_TRIBE };
enum { PAD_DRONE, PAD_STAB, PAD_CHOIR, PAD_NOISE };
enum { LEAD_HOOK, LEAD_ARP, LEAD_ACID, LEAD_SPARSE, LEAD_SIREN, LEAD_STAB };

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

/* engine numbers (Bunker is append-only; these are the stable indices) */
enum { E_ANALOG = 0, E_PHASE = 2, E_LOFI = 3, E_FORMANT = 5, E_GRAIN = 8,
       E_PHYS = 9, E_DRUM = 10, E_NOISE = 11, E_FM6 = 12 };
/* Bunker AMV_POLY / AMV_MONO / AMV_LEGATO / AMV_UNISON */
enum { AMV_POLY = 0, AMV_MONO = 1, AMV_LEGATO = 2, AMV_UNISON = 3 };

#define ASND(nm, en, pr, tn, at, rl, ch, dl, rv, ds, vc) \
    { nm, en, pr, tn, at, rl, ch, dl, rv, ds, vc }

typedef struct {
    const char *name;
    uint8_t lo, hi, root, scale, energy, swing;
    const char *kick, *snare, *clap, *hat, *ohat, *perc;
    uint8_t bass, pad, lead, broken;
    amaru_sound_t snd[AMARU_TRACKS];
} genre_def_t;

/* The masks are one 16-step bar; x/a = hit, A = accent, . = rest. */
static const genre_def_t GENRES[AMARU_NGENRES] = {
    /* AIR, MELODY, ROOT, SPARK */
    { "TRIBE TEKNO", 150, 166, 2, SC_DOR, 70, 6,
      "x...x...x...x...", "....x.......x...", "................", "..x...x...x...x.", "................", "..x..x....x..x..",
      BASS_TRIBE, PAD_CHOIR, LEAD_STAB, 0,
      { ASND("TRIBE CHOIR", E_FORMANT, 0, 64, 55, 105, 25, 30, 90, 0, AMV_POLY),
        ASND("TRIBE PLUCK", E_ANALOG, 8, 70, 0, 55, 10, 40, 55, 0, AMV_POLY),
        ASND("TRIBE BASS", E_ANALOG, 2, 44, 0, 35, 0, 10, 20, 12, AMV_MONO),
        ASND("TRIBE DRUMS", E_DRUM, 3, 64, 0, 30, 0, 12, 22, 0, AMV_POLY) } },

    { "MENTAL TEKNO", 158, 178, 9, SC_PHRY, 64, 0,
      "x...x...x...x...", "........x.......", "................", "..x...x...x...x.", "......x.......x.", "..x.....x.....x.",
      BASS_ROLL, PAD_NOISE, LEAD_SIREN, 0,
      { ASND("MENTAL PAD", E_GRAIN, 0, 58, 70, 112, 35, 35, 100, 0, AMV_POLY),
        ASND("MENTAL SIREN", E_PHASE, 3, 82, 8, 78, 20, 45, 60, 18, AMV_MONO),
        ASND("MENTAL SUB", E_ANALOG, 7, 36, 0, 60, 0, 12, 25, 0, AMV_MONO),
        ASND("MENTAL DRUMS", E_DRUM, 0, 64, 0, 30, 0, 14, 28, 0, AMV_POLY) } },

    { "HARDTEKK", 170, 180, 5, SC_MIN, 86, 0,
      "x...x...x...x...", "....x.......x...", "....x.......x...", "..x...x...x...x.", "..x...x...x...x.", "................",
      BASS_OFF, PAD_STAB, LEAD_STAB, 0,
      { ASND("HARDTEKK PAD", E_ANALOG, 3, 66, 12, 55, 20, 25, 45, 25, AMV_POLY),
        ASND("HARDTEKK STAB", E_ANALOG, 6, 76, 0, 32, 8, 20, 35, 40, AMV_POLY),
        ASND("HARDTEKK BASS", E_ANALOG, 2, 52, 0, 26, 0, 5, 12, 35, AMV_MONO),
        ASND("HARDTEKK DRUMS", E_DRUM, 2, 64, 0, 24, 0, 10, 18, 0, AMV_POLY) } },

    { "ACID CORE", 165, 180, 3, SC_PHRY, 82, 0,
      "x...x...x...x...", "....x.......x...", "................", "xxxxxxxxxxxxxxxx", "......x.......x.", "..x.....x.....x.",
      BASS_ACID, PAD_STAB, LEAD_ACID, 0,
      { ASND("ACID MIST", E_NOISE, 0, 72, 35, 90, 20, 28, 80, 0, AMV_POLY),
        ASND("CORE 303", E_ANALOG, 4, 88, 0, 34, 20, 30, 35, 45, AMV_MONO),
        ASND("CORE BASS", E_ANALOG, 4, 58, 0, 34, 12, 20, 22, 35, AMV_MONO),
        ASND("CORE DRUMS", E_DRUM, 0, 64, 0, 26, 0, 12, 20, 0, AMV_POLY) } },

    { "JUNGLE", 160, 170, 2, SC_MIN, 76, 8,
      "x.....x...x.....", "....x..x....x..x", "................", "x.x.x.x.x.x.x.x.", "......x.......x.", "..x..x....x..x..",
      BASS_SUB, PAD_CHOIR, LEAD_SPARSE, 1,
      { ASND("JUNGLE CHOIR", E_FORMANT, 0, 55, 60, 105, 30, 32, 92, 0, AMV_POLY),
        ASND("JUNGLE STAB", E_ANALOG, 8, 74, 0, 46, 12, 38, 55, 0, AMV_POLY),
        ASND("JUNGLE SUB", E_ANALOG, 7, 30, 0, 70, 0, 8, 18, 0, AMV_MONO),
        ASND("JUNGLE DRUMS", E_DRUM, 0, 64, 0, 30, 0, 14, 28, 0, AMV_POLY) } },

    { "DRUM & BASS", 172, 178, 7, SC_MIN, 78, 0,
      "x.........x.....", "....x.......x...", "................", "x.x.x.x.x.x.x.x.", "......x.......x.", "..x.....x.....x.",
      BASS_SUB, PAD_DRONE, LEAD_ARP, 1,
      { ASND("DNB PAD", E_FM6, 4, 60, 55, 110, 28, 30, 88, 0, AMV_POLY),
        ASND("DNB REESE", E_PHASE, 3, 80, 0, 55, 18, 36, 45, 18, AMV_MONO),
        ASND("DNB SUB", E_ANALOG, 7, 28, 0, 80, 0, 6, 15, 0, AMV_MONO),
        ASND("DNB DRUMS", E_DRUM, 0, 64, 0, 30, 0, 14, 28, 0, AMV_POLY) } },

    { "BREAKBEAT TEKNO", 140, 150, 5, SC_DOR, 78, 4,
      "x..x..x...x..x..", "....x..x..x.x...", "................", "x.x.x.x.x.x.x.x.", "......x.......x.", "..x..x....x..x..",
      BASS_OFF, PAD_STAB, LEAD_STAB, 1,
      { ASND("BREAK PAD", E_ANALOG, 11, 58, 45, 95, 30, 28, 78, 0, AMV_POLY),
        ASND("BREAK STAB", E_ANALOG, 6, 78, 0, 40, 10, 30, 45, 18, AMV_POLY),
        ASND("BREAK BASS", E_ANALOG, 2, 46, 0, 38, 0, 8, 18, 18, AMV_MONO),
        ASND("BREAK DRUMS", E_DRUM, 0, 64, 0, 28, 0, 14, 26, 0, AMV_POLY) } },

    { "TECHNO", 140, 155, 7, SC_MIN, 72, 0,
      "x...x...x...x...", "....x.......x...", "....x.......x...", "..x...x...x...x.", "......x.......x.", "................",
      BASS_ROLL, PAD_DRONE, LEAD_HOOK, 0,
      { ASND("TECHNO PAD", E_FM6, 4, 62, 50, 100, 25, 30, 80, 0, AMV_POLY),
        ASND("TECHNO LEAD", E_ANALOG, 0, 82, 0, 50, 20, 40, 45, 10, AMV_MONO),
        ASND("TECHNO BASS", E_ANALOG, 4, 48, 0, 42, 8, 20, 20, 22, AMV_MONO),
        ASND("TECHNO DRUMS", E_DRUM, 0, 64, 0, 30, 0, 12, 24, 0, AMV_POLY) } },

    { "PSYTRANCE", 140, 148, 9, SC_PHRY, 82, 0,
      "x...x...x...x...", "....x.......x...", "................", "..x...x...x...x.", "......x.......x.", "..x.....x.....x.",
      BASS_ROLL, PAD_CHOIR, LEAD_ARP, 0,
      { ASND("PSY CHOIR", E_FORMANT, 0, 66, 55, 110, 30, 32, 92, 0, AMV_POLY),
        ASND("PSY LEAD", E_ANALOG, 0, 92, 0, 55, 30, 50, 55, 12, AMV_UNISON),
        ASND("PSY ROLL", E_ANALOG, 4, 52, 0, 30, 10, 22, 22, 18, AMV_MONO),
        ASND("PSY DRUMS", E_DRUM, 0, 64, 0, 28, 0, 12, 24, 0, AMV_POLY) } }
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
static int chance(uint32_t pct) { return pct && rnd() % 100u < pct; }
static int rr(int lo, int hi) { return hi <= lo ? lo : lo + (int)(rnd() % (uint32_t)(hi - lo + 1)); }

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
static int mask_hit(const char *m, int i)
{
    char c = i >= 0 && i < 16 ? m[i] : '.';
    return c == 'x' || c == 'X' || c == 'a' || c == 'A';
}
static int mask_acc(const char *m, int i)
{
    char c = i >= 0 && i < 16 ? m[i] : '.';
    return c == 'A';
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

/* ---------------------------------------------------------------- drums --- */
static void make_drums(amaru_plan_t *p, const genre_def_t *g)
{
    for (int i = 0; i < p->steps; i++) {
        int pos = i % 16, bar = i / 16;
        uint32_t hit = 0, acc = 0;
        if (mask_hit(g->kick, pos)) { hit |= AMARU_L_KICK; if (mask_acc(g->kick, pos) || pos == 0) acc |= AMARU_L_KICK; }
        if (mask_hit(g->snare, pos)) { hit |= AMARU_L_SNARE; if (mask_acc(g->snare, pos) || pos == 4 || pos == 12) acc |= AMARU_L_SNARE; }
        if (mask_hit(g->clap, pos)) { hit |= AMARU_L_CLAP; if (mask_acc(g->clap, pos) || pos == 4 || pos == 12) acc |= AMARU_L_CLAP; }
        if (mask_hit(g->hat, pos)) hit |= AMARU_L_HAT;
        if (mask_hit(g->ohat, pos)) hit |= AMARU_L_OHAT;
        if (mask_hit(g->perc, pos)) hit |= AMARU_L_TOM;

        /* ENERGY: hats thicken above 60, thin out below 34; ghost snares arrive with it. */
        if (p->energy >= 34 && (pos & 1) == 0 && !(hit & (AMARU_L_KICK | AMARU_L_SNARE)) && chance(40 + p->energy / 3))
            hit |= AMARU_L_HAT;
        if (p->energy >= 62 && (pos & 3) == 2 && chance(45 + p->energy / 4))
            hit |= AMARU_L_HAT;
        if (p->energy >= 80 && (pos & 3) == 1 && chance(30))
            hit |= AMARU_L_HAT;
        if (p->energy < 34) {
            if ((pos & 3) == 2) hit &= ~AMARU_L_HAT;
            hit &= ~(AMARU_L_OHAT | AMARU_L_TOM | AMARU_L_BELL);
        }
        if (g->broken && (pos & 3) == 3 && chance(12 + p->energy / 4))
            hit |= AMARU_L_SNARE;                 /* a ghost, not an accent */

        /* bar variation and a last-bar fill; LOOP stays one bar. */
        if (bar && chance(18)) {
            if (pos == 7) hit |= AMARU_L_RIM;
            if (pos == 15) hit |= AMARU_L_OHAT;
        }
        if (p->form == AMARU_SONG && bar == 3 && pos >= 12 && chance(55))
            hit |= AMARU_L_TOM;
        if (p->form == AMARU_SONG && bar == 2 && pos == 14 && chance(40))
            hit |= AMARU_L_BELL;

        amaru_step_t *s = &p->step[AMARU_SPARK][i];
        s->hit = (uint8_t)hit;
        s->acc = (uint8_t)(acc & hit);
        s->time = hit ? AMARU_ST_NOTE : AMARU_ST_REST;
        s->vel = (uint8_t)(hit ? (acc ? 112 : 88) : 0);
    }
}

/* ----------------------------------------------------------------- bass --- */
static void make_bass(amaru_plan_t *p, const genre_def_t *g)
{
    static const int8_t DEG[16] = {0, 0, 4, 2, 0, 4, 6, 4, 0, 0, 4, 2, 7, 4, 2, 0};
    int mode = g->bass;
    for (int i = 0; i < p->steps; i++) {
        int pos = i % 16, bar = i / 16;
        int on = 0, deg = DEG[pos], slide = 0, accent = 0;
        switch (mode) {
        case BASS_SUB:
            /* long sub: DnB two-step, jungle broken, otherwise one root and one fifth. */
            if (p->genre == AMARU_DNB) {
                on = pos == 0 || pos == 10;
                if (pos >= 1 && pos <= 5) { p->step[AMARU_ROOT][i].time = AMARU_ST_TIE; continue; }
                if (pos >= 11) { p->step[AMARU_ROOT][i].time = AMARU_ST_TIE; continue; }
            } else if (p->genre == AMARU_JUNGLE) {
                on = pos == 0 || pos == 6 || pos == 10;
                if ((pos >= 1 && pos <= 5) || (pos >= 7 && pos <= 9) || pos >= 11) {
                    p->step[AMARU_ROOT][i].time = AMARU_ST_TIE; continue;
                }
            } else {
                on = pos == 0 || pos == 8;
                if (pos >= 1 && pos <= 7) { p->step[AMARU_ROOT][i].time = AMARU_ST_TIE; continue; }
                if (pos >= 9) { p->step[AMARU_ROOT][i].time = AMARU_ST_TIE; continue; }
            }
            if (pos == 6 || pos == 10) deg = 4;
            break;
        case BASS_OFF:
            on = pos == 2 || pos == 6 || pos == 10 || pos == 14;
            deg = pos == 6 ? 4 : pos == 10 ? 7 : pos == 14 ? 2 : 0;
            break;
        case BASS_ROLL:
            on = !(mask_hit(g->kick, pos));
            deg = (pos * 3 + bar) & 7;
            if (deg > 4) deg = 4;
            break;
        case BASS_ACID:
            on = pos == 0 || pos == 1 || pos == 2 || pos == 4 || pos == 6 || pos == 7 ||
                 pos == 8 || pos == 10 || pos == 11 || pos == 12 || pos == 14 || pos == 15;
            deg = (pos * 5 + bar * 2) & 7;
            if (deg > 4) deg = 4;
            slide = chance(22);
            accent = pos == 0 || pos == 8;
            break;
        case BASS_STAB:
            on = pos == 0 || pos == 6 || pos == 10 || pos == 14;
            deg = pos == 6 ? 4 : pos == 10 ? 2 : 0;
            accent = pos == 0;
            break;
        default: /* BASS_TRIBE */
            on = pos == 2 || pos == 6 || pos == 10 || pos == 14;
            deg = pos == 2 ? 0 : pos == 6 ? 4 : pos == 10 ? 0 : 2;
            accent = pos == 6;
            break;
        }
        if (!on) continue;
        amaru_step_t *s = &p->step[AMARU_ROOT][i];
        s->n = 1;
        s->note[0] = (uint8_t)note_at(p, deg, 2);
        s->time = AMARU_ST_NOTE;
        s->flags = (uint8_t)((accent ? AMARU_SF_ACCENT : 0) | (slide ? AMARU_SF_SLIDE : 0));
        s->vel = (uint8_t)(accent ? 112 : 88);
    }
}

/* ------------------------------------------------------------------ pad --- */
static void put_chord(amaru_plan_t *p, int t, int i, int deg, int vel, int accent)
{
    amaru_step_t *s = &p->step[t][i];
    s->n = 3;
    s->note[0] = (uint8_t)note_at(p, deg, 3);
    s->note[1] = (uint8_t)note_at(p, deg + 2, 3);
    s->note[2] = (uint8_t)note_at(p, deg + 4, 3);
    s->time = AMARU_ST_NOTE;
    s->flags = (uint8_t)(accent ? AMARU_SF_ACCENT : 0);
    s->vel = (uint8_t)vel;
}
static void make_pad(amaru_plan_t *p, const genre_def_t *g)
{
    for (int i = 0; i < p->steps; i++) {
        int pos = i % 16, bar = i / 16;
        int mode = g->pad;
        if (mode == PAD_DRONE) {
            if (pos == 0) put_chord(p, AMARU_AIR, i, (bar == 2 ? 4 : 0), 70, 0);
            else p->step[AMARU_AIR][i].time = AMARU_ST_TIE;
        } else if (mode == PAD_CHOIR) {
            if (pos == 0 || (p->form != AMARU_LOOP && pos == 8 && bar != 1))
                put_chord(p, AMARU_AIR, i, (bar & 1) ? 2 : 0, 66, 0);
            else
                p->step[AMARU_AIR][i].time = AMARU_ST_TIE;
        } else if (mode == PAD_NOISE) {
            if (pos == 0 || pos == 8) put_chord(p, AMARU_AIR, i, (bar + pos / 8) & 1 ? 2 : 0, 60, 0);
        } else { /* PAD_STAB */
            if (pos == 0 || pos == 8) put_chord(p, AMARU_AIR, i, (bar & 1) ? 4 : 0, 84, pos == 0);
        }
    }
}

/* ----------------------------------------------------------------- lead --- */
static void make_lead(amaru_plan_t *p, const genre_def_t *g)
{
    static const int8_t HOOK[4] = {0, 4, 2, 6};
    static const int8_t ARP[6] = {0, 2, 4, 7, 4, 2};
    for (int i = 0; i < p->steps; i++) {
        int pos = i % 16, bar = i / 16;
        int on = 0, deg = 0, slide = 0, accent = 0;
        switch (g->lead) {
        case LEAD_HOOK:
            on = (pos & 3) == 0;
            deg = HOOK[(pos / 4 + bar) & 3];
            accent = pos == 0;
            break;
        case LEAD_ARP:
            on = (p->energy > 70 ? 1 : 2) == 1 ? 1 : (pos & 1) == 0;
            deg = ARP[(pos + bar * 2) % 6];
            accent = pos == 0;
            break;
        case LEAD_ACID:
            on = pos == 0 || pos == 1 || pos == 2 || pos == 3 || pos == 5 || pos == 6 ||
                 pos == 7 || pos == 9 || pos == 10 || pos == 11 || pos == 13 || pos == 14 || pos == 15;
            deg = ARP[(pos * 2 + bar) % 6];
            slide = chance(28);
            accent = pos == 0 || pos == 8;
            break;
        case LEAD_SPARSE:
            on = pos == 0 || pos == 6 || pos == 10;
            deg = pos == 6 ? 4 : pos == 10 ? 2 : 0;
            break;
        case LEAD_SIREN:
            if (pos == 0 || pos == 8) { on = 1; deg = (pos == 8 ? 4 : 0); }
            else if ((pos >= 1 && pos <= 7) || pos >= 9) {
                p->step[AMARU_MELODY][i].time = AMARU_ST_TIE;
                continue;
            }
            break;
        default: /* LEAD_STAB */
            on = pos == 2 || pos == 6 || pos == 10 || pos == 14;
            deg = pos == 2 ? 0 : pos == 6 ? 4 : pos == 10 ? 2 : 6;
            accent = pos == 6;
            break;
        }
        if (!on) continue;
        amaru_step_t *s = &p->step[AMARU_MELODY][i];
        s->n = 1;
        s->note[0] = (uint8_t)note_at(p, deg, 5);
        s->time = AMARU_ST_NOTE;
        s->flags = (uint8_t)((accent ? AMARU_SF_ACCENT : 0) | (slide ? AMARU_SF_SLIDE : 0));
        s->vel = (uint8_t)(accent ? 116 : 92);
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
    const genre_def_t *g = &GENRES[genre];
    for (int i = 0; i < (int)sizeof(*out); i++) ((uint8_t *)out)[i] = 0;
    out->genre = genre;
    out->form = form;
    out->energy = energy;
    out->seed = seed;
    out->root = g->root;
    out->scale = g->scale;
    out->swing = g->swing;
    out->steps = (uint8_t)(form == AMARU_LOOP ? 16 : form == AMARU_PARTS ? 32 : 64);
    rng_state = 0x9E3779B9u ^ (uint32_t)seed ^ ((uint32_t)genre << 8) ^
                ((uint32_t)form << 16) ^ ((uint32_t)energy << 24);
    if (!rng_state) rng_state = 0x42554E4Bu;
    out->bpm = (uint8_t)rr(g->lo, g->hi);
    /* add a little deterministic life to the key while keeping the profile's mode */
    out->root = (uint8_t)((g->root + (rr(0, 3) == 0 ? rr(0, 1) : 0)) % 12);
    for (int t = 0; t < AMARU_TRACKS; t++) {
        out->sound[t] = g->snd[t];
        int tone = (int)g->snd[t].tone + rr(-4, 4);
        if (tone < 0) tone = 0;
        if (tone > 127) tone = 127;
        out->sound[t].tone = (uint8_t)tone;
    }
    clear_steps(out);
    make_drums(out, g);
    make_bass(out, g);
    make_pad(out, g);
    make_lead(out, g);
}
