/* SPDX-License-Identifier: GPL-3.0-only
 * AMARU OS deterministic four-layer generator.
 *
 * This header is deliberately free of Bunker/Felucca types: the planner in
 * amaru_gen.c can be compiled and tested on the host, and the Bunker adapter
 * (bunker_amaru.c) converts a plan into step_t/track_t. No allocation, no I/O,
 * no hardware access, no time or random source.
 */
#ifndef AMARU_GEN_H
#define AMARU_GEN_H
#include <stdint.h>

#define AMARU_TRACKS 4
#define AMARU_STEPS 64

/* Bunker layer order: keep the visible AIR / MELODY / ROOT / SPARK roles. */
enum { AMARU_AIR, AMARU_MELODY, AMARU_ROOT, AMARU_SPARK };

/* step time, same values as Bunker's ST_NOTE / ST_TIE / ST_REST */
enum { AMARU_ST_NOTE, AMARU_ST_TIE, AMARU_ST_REST };
#define AMARU_SF_ACCENT 1u
#define AMARU_SF_SLIDE 2u
/* Bunker's step-level ratchet (SF_RATCH, flags bits 3..4): 1..3 = x2..x4 hits of the step */
#define AMARU_SF_RATCH2 8u
#define AMARU_SF_RATCH4 24u

/* drum lanes, same bits as Bunker's step_t.hit */
#define AMARU_L_KICK 0x01u
#define AMARU_L_SNARE 0x02u
#define AMARU_L_CLAP 0x04u
#define AMARU_L_HAT 0x08u
#define AMARU_L_OHAT 0x10u
#define AMARU_L_TOM 0x20u
#define AMARU_L_RIM 0x40u
#define AMARU_L_BELL 0x80u

enum {
    AMARU_TRIBE,
    AMARU_MENTAL,
    AMARU_HARDTEKK,
    AMARU_ACIDCORE,
    AMARU_JUNGLE,
    AMARU_DNB,
    AMARU_BREAKBEAT,
    AMARU_TECHNO,
    AMARU_PSY,
    AMARU_NGENRES
};

enum { AMARU_LOOP, AMARU_PARTS, AMARU_SONG, AMARU_NFORMS };

typedef struct {
    uint8_t n;
    uint8_t note[4];
    uint8_t time;          /* AMARU_ST_* */
    uint8_t flags;         /* AMARU_SF_* */
    uint8_t vel;
    uint8_t hit;           /* AMARU_L_* */
    uint8_t acc;
    uint8_t probability;   /* 0 = 100 %, 1..100, 101 = silent */
} amaru_step_t;

typedef struct {
    const char *name;
    uint8_t engine;        /* Bunker engine number */
    uint8_t preset;        /* factory preset index in that engine */
    uint8_t tone;          /* 0..127, the engine's colour parameter */
    uint8_t atk, rel, chor, dly, rev, dist;
    uint8_t voice;         /* Bunker V_POLY / V_MONO / V_LEGATO / V_UNISON */
} amaru_sound_t;

typedef struct {
    uint8_t genre, form, energy;
    uint16_t seed;
    uint8_t bpm, root, scale, steps, swing;
    amaru_sound_t sound[AMARU_TRACKS];
    amaru_step_t step[AMARU_TRACKS][AMARU_STEPS];
} amaru_plan_t;

const char *amaru_genre_name(uint8_t genre);
const char *amaru_form_name(uint8_t form);
void amaru_plan(amaru_plan_t *out, uint8_t genre, uint16_t seed,
                uint8_t energy, uint8_t form);

#endif
