/* SPDX-License-Identifier: GPL-3.0-only */
/* Host test of the pure AMARU planner (no Bunker/Felucca globals). */
#include <stdio.h>
#include <string.h>
#include "amaru_gen.h"

static int failures;
#define CHECK(c, ...) do { if (!(c)) { failures++; printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); } } while (0)

static int lane_count(const amaru_plan_t *p, int trk, int lane)
{
    int n = 0;
    for (int i = 0; i < p->steps; i++) if (p->step[trk][i].hit & lane) n++;
    return n;
}
static int note_count(const amaru_plan_t *p, int trk)
{
    int n = 0;
    for (int i = 0; i < p->steps; i++) if (p->step[trk][i].n) n++;
    return n;
}
static int kick_off_quarter(const amaru_plan_t *p)
{
    int n = 0;
    for (int i = 0; i < p->steps; i++)
        if ((p->step[AMARU_SPARK][i].hit & AMARU_L_KICK) && (i % 4))
            n++;
    return n;
}
static int kick_on_quarter_bar0(const amaru_plan_t *p)
{
    int n = 0;
    for (int i = 0; i < 16; i++)
        if ((p->step[AMARU_SPARK][i].hit & AMARU_L_KICK) && !(i % 4))
            n++;
    return n;
}

static uint32_t hsh(uint32_t h, uint32_t v) { h ^= v; h *= 16777619u; return h; }

/* A plan's identity, part by part: a hash of the first two bars of each role. Two genres
 * must differ in at least two of these, over a seed sweep: drums, bass, harmony, lead. */
static uint32_t sig_drums(const amaru_plan_t *p)
{
    uint32_t h = 2166136261u;
    for (int i = 0; i < 32; i++)
        h = hsh(h, (uint32_t)p->step[AMARU_SPARK][i].hit | ((uint32_t)p->step[AMARU_SPARK][i].acc << 8));
    return h;
}
static uint32_t sig_bass(const amaru_plan_t *p)
{
    uint32_t h = 2166136261u;
    for (int i = 0; i < 32; i++) {
        const amaru_step_t *s = &p->step[AMARU_ROOT][i];
        h = hsh(h, s->time == AMARU_ST_REST ? 0xFFFFFFFFu : (uint32_t)s->note[0] + 1u);
    }
    return h;
}
static uint32_t sig_pad(const amaru_plan_t *p)
{
    uint32_t h = 2166136261u;
    for (int i = 0; i < 32; i++) {
        const amaru_step_t *s = &p->step[AMARU_AIR][i];
        h = hsh(h, ((uint32_t)s->n << 8) | (s->n ? s->note[0] : 0u));
    }
    return h;
}
static uint32_t sig_lead(const amaru_plan_t *p)
{
    uint32_t h = 2166136261u;
    for (int i = 0; i < 32; i++) {
        const amaru_step_t *s = &p->step[AMARU_MELODY][i];
        h = hsh(h, s->n ? (uint32_t)s->note[0] + 1u : 0u);
    }
    return h;
}
/* a bass note is in the mode, or a semitone beside a mode tone (the legal approach) */
static int in_mode(int root, int scale, int note)
{
    static const int8_t ST[6][7] = {
        {0, 2, 3, 5, 7, 8, 10}, {0, 1, 3, 5, 7, 8, 10}, {0, 2, 3, 5, 7, 9, 10},
        {0, 2, 3, 5, 7, 8, 11}, {0, 2, 4, 5, 7, 9, 11}, {0, 3, 5, 7, 10, 12, 14}
    };
    static const uint8_t LEN[6] = {7, 7, 7, 7, 7, 5};
    int m;
    switch (scale) {
    case 8: m = 1; break;
    case 3: m = 2; break;
    case 7: m = 3; break;
    case 1: m = 4; break;
    case 6: m = 5; break;
    default: m = 0; break;
    }
    for (int oct = -3; oct <= 8; oct++)
        for (int k = 0; k < LEN[m]; k++) {
            int t = root + oct * 12 + ST[m][k];
            if (note == t || note == t - 1 || note == t + 1)
                return 1;
        }
    return 0;
}

static void genre_identity(void)
{
    enum { SEEDS = 64 };
    static uint32_t drums[AMARU_NGENRES][SEEDS], bass[AMARU_NGENRES][SEEDS];
    static uint32_t pads[AMARU_NGENRES][SEEDS], lead[AMARU_NGENRES][SEEDS];

    for (int g = 0; g < AMARU_NGENRES; g++)
        for (int s = 0; s < SEEDS; s++) {
            amaru_plan_t p;
            amaru_plan(&p, (uint8_t)g, (uint16_t)(s * 7919 + 3), 70, AMARU_PARTS);
            drums[g][s] = sig_drums(&p);
            bass[g][s] = sig_bass(&p);
            pads[g][s] = sig_pad(&p);
            lead[g][s] = sig_lead(&p);
        }

    for (int a = 0; a < AMARU_NGENRES; a++)
        for (int b = a + 1; b < AMARU_NGENRES; b++) {
            int dd = 0, bd = 0, pd = 0, ld = 0, diff = 0;
            for (int s = 0; s < SEEDS; s++) {
                dd += drums[a][s] != drums[b][s];
                bd += bass[a][s] != bass[b][s];
                pd += pads[a][s] != pads[b][s];
                ld += lead[a][s] != lead[b][s];
            }
            diff = (dd >= 48) + (bd >= 48) + (pd >= 48) + (ld >= 48);
            CHECK(diff >= 2, "genres %u and %u share their grammar (d%d b%d p%d l%d)",
                  a, b, dd, bd, pd, ld);
            if (a == AMARU_MENTAL && b == AMARU_PSY)
                CHECK(diff >= 2, "MENTAL and PSY are the same piece");
        }

    /* the breakbeat rows are broken, the techno rows four on the floor, every seed */
    for (int g = 0; g < AMARU_NGENRES; g++)
        for (int s = 0; s < SEEDS; s++) {
            amaru_plan_t p;
            int q = 0, off = 0, down = 0;
            amaru_plan(&p, (uint8_t)g, (uint16_t)(s * 104729 + 11), 70, AMARU_LOOP);
            for (int i = 0; i < 16; i++) {
                int kick = (p.step[AMARU_SPARK][i].hit & AMARU_L_KICK) != 0;
                if (kick && !(i % 4)) q++;
                if (kick && (i % 4)) off++;
                if (p.step[AMARU_SPARK][i].hit) down |= 1;
            }
            if (g == AMARU_JUNGLE || g == AMARU_DNB || g == AMARU_BREAKBEAT) {
                CHECK(q < 4 && off > 0, "broken genre %d seed %d is four-on-the-floor", g, s);
            } else {
                CHECK(q == 4, "four-on-floor genre %d seed %d lost a quarter kick", g, s);
            }
            CHECK(down, "genre %d seed %d has an empty bar", g, s);
        }

    /* the melody and bass stay in the mode, the harmony moves */
    for (int g = 0; g < AMARU_NGENRES; g++) {
        int moved = 0;
        for (int s = 0; s < 8; s++) {
            amaru_plan_t p;
            uint32_t pc = 0;
            amaru_plan(&p, (uint8_t)g, (uint16_t)(s * 31 + 5), 80, AMARU_SONG);
            for (int i = 0; i < 64; i++) {
                const amaru_step_t *b = &p.step[AMARU_ROOT][i];
                const amaru_step_t *l = &p.step[AMARU_MELODY][i];
                const amaru_step_t *a = &p.step[AMARU_AIR][i];
                if (b->n) CHECK(in_mode(p.root, p.scale, b->note[0]), "bass out of mode genre %d seed %d", g, s);
                if (l->n) CHECK(in_mode(p.root, p.scale, l->note[0]), "lead out of mode genre %d seed %d", g, s);
                if (a->n) pc |= 1u << (a->note[0] % 12);
            }
            moved |= (pc & (pc - 1u)) != 0;   /* at least two distinct chord roots in the plan */
        }
        CHECK(moved, "harmony never moves genre %d", g);
    }
}

/* SONG is four bars: bar 3 a break (no kick, the backbeat kept), bar 4 a build with the roll into the loop point */
static void song_form(void)
{
    for (int g = 0; g < AMARU_NGENRES; g++) {
        amaru_plan_t p, q, r;
        int i, kick_break = 0, backbeat = 0, kick_build = 0, roll = 0, broken;
        broken = g == AMARU_JUNGLE || g == AMARU_DNB || g == AMARU_BREAKBEAT;
        amaru_plan(&p, (uint8_t)g, 4242, 70, AMARU_SONG);
        amaru_plan(&q, (uint8_t)g, 4242, 70, AMARU_PARTS);
        amaru_plan(&r, (uint8_t)g, 4242, 70, AMARU_LOOP);
        CHECK(p.steps == 64 && q.steps == 32 && r.steps == 16, "form lengths genre %d", g);
        {   /* the four bars are not one bar repeated: a phrase, not a longer loop */
            uint32_t h[4];
            int same = 0;
            for (int b = 0; b < 4; b++) {
                h[b] = 2166136261u;
                for (int k = 0; k < 16; k++)
                    h[b] = hsh(hsh(h[b], p.step[AMARU_SPARK][b * 16 + k].hit),
                               (uint32_t)p.step[AMARU_SPARK][b * 16 + k].vel);
            }
            for (int b = 1; b < 4; b++)
                same += h[b] == h[0];
            CHECK(same < 2, "SONG repeats one bar genre %d", g);
        }
        for (i = 32; i < 48; i++)
            kick_break += (p.step[AMARU_SPARK][i].hit & AMARU_L_KICK) != 0;
        for (i = 0; i < 16; i++)
            backbeat += (p.step[AMARU_SPARK][32 + i].hit & AMARU_L_SNARE) != 0;
        for (i = 48; i < 64; i++)
            kick_build += (p.step[AMARU_SPARK][i].hit & AMARU_L_KICK) != 0;
        for (i = 60; i < 64; i++)
            roll += (p.step[AMARU_SPARK][i].hit & AMARU_L_SNARE) != 0;
        CHECK(kick_break == 0, "SONG break keeps the kick genre %d (%d)", g, kick_break);
        CHECK(backbeat > 0, "SONG break lost the backbeat genre %d", g);
        CHECK(roll == 4, "SONG build has no roll into the loop genre %d", g);
        CHECK(broken ? kick_build > 0 : kick_build >= 3, "SONG build lost its kick genre %d (%d)", g, kick_build);
        if (broken) {                                   /* the broken rows stay broken in every bar */
            for (int b = 0; b < 4; b++) {
                int qk = 0;
                for (i = 0; i < 16; i++)
                    qk += (p.step[AMARU_SPARK][b * 16 + i].hit & AMARU_L_KICK) && !(i % 4);
                CHECK(qk < 4, "SONG bar %d of a broken genre is four-on-the-floor genre %d", b + 1, g);
            }
        }
        CHECK(note_count(&p, AMARU_AIR) > 0 && note_count(&p, AMARU_MELODY) > 0,
              "SONG lost a melodic part genre %d", g);
    }
}

int main(void)
{
    const uint8_t forms[] = {AMARU_LOOP, AMARU_PARTS, AMARU_SONG};
    for (uint8_t g = 0; g < AMARU_NGENRES; g++) {
        CHECK(amaru_genre_name(g) && amaru_genre_name(g)[0], "genre %u name", g);
        for (unsigned fi = 0; fi < sizeof(forms); fi++) {
            uint8_t f = forms[fi];
            amaru_plan_t a, b;
            amaru_plan(&a, g, 0x1234u, 70, f);
            amaru_plan(&b, g, 0x1234u, 70, f);
            CHECK(memcmp(&a, &b, sizeof a) == 0, "determinism genre %u form %u", g, f);
            CHECK(a.steps == (f == AMARU_LOOP ? 16 : f == AMARU_PARTS ? 32 : 64), "form length genre %u form %u", g, f);
            CHECK(a.bpm >= 30 && a.bpm <= 180, "bpm genre %u: %u", g, a.bpm);
            CHECK(a.root < 12, "root genre %u", g);
            CHECK(a.scale < 32, "scale genre %u", g);
            CHECK(a.sound[AMARU_SPARK].engine == 10, "drums engine genre %u", g);
            for (int t = 0; t < AMARU_TRACKS; t++) {
                CHECK(a.sound[t].name && a.sound[t].name[0], "sound name genre %u track %d", g, t);
                CHECK(a.sound[t].engine < 14, "engine genre %u track %d", g, t);
                CHECK(a.sound[t].tone <= 127, "tone genre %u track %d", g, t);
            }
            for (int t = 0; t < AMARU_TRACKS; t++)
                for (int i = 0; i < a.steps; i++) {
                    const amaru_step_t *s = &a.step[t][i];
                    CHECK(s->time <= AMARU_ST_REST, "time genre %u trk %d step %d", g, t, i);
                    CHECK(s->n <= 4, "n genre %u trk %d step %d", g, t, i);
                    for (int k = 0; k < s->n; k++)
                        CHECK(s->note[k] >= 12 && s->note[k] <= 108, "note genre %u trk %d step %d: %u", g, t, i, s->note[k]);
                    CHECK(s->probability == 0 || (s->probability >= 1 && s->probability <= 101), "prob genre %u", g);
                    CHECK(!(s->time == AMARU_ST_REST && (s->n || s->hit)), "rest carries data genre %u trk %d step %d", g, t, i);
                    CHECK(!(t == AMARU_SPARK && s->n), "drum track has notes genre %u step %d", g, i);
                    if (s->time == AMARU_ST_NOTE && t != AMARU_SPARK)
                        CHECK(s->n > 0, "melodic note without note genre %u trk %d step %d", g, t, i);
                }
            CHECK(note_count(&a, AMARU_AIR) > 0, "empty AIR genre %u form %u", g, f);
            CHECK(note_count(&a, AMARU_MELODY) > 0, "empty MELODY genre %u form %u", g, f);
            CHECK(note_count(&a, AMARU_ROOT) > 0, "empty ROOT genre %u form %u", g, f);
            CHECK(lane_count(&a, AMARU_SPARK, AMARU_L_KICK) > 0, "no kick genre %u form %u", g, f);
            CHECK(lane_count(&a, AMARU_SPARK, AMARU_L_SNARE) > 0, "no snare genre %u form %u", g, f);
            CHECK(lane_count(&a, AMARU_SPARK, AMARU_L_HAT) > 0, "no hat genre %u form %u", g, f);
        }

        amaru_plan_t low, high;
        amaru_plan(&low, g, 7, 5, AMARU_PARTS);
        amaru_plan(&high, g, 7, 100, AMARU_PARTS);
        CHECK(lane_count(&high, AMARU_SPARK, AMARU_L_HAT) >= lane_count(&low, AMARU_SPARK, AMARU_L_HAT),
              "energy did not add hats genre %u", g);

        amaru_plan_t s1, s2;
        amaru_plan(&s1, g, 1, 70, AMARU_PARTS);
        amaru_plan(&s2, g, 2, 70, AMARU_PARTS);
        CHECK(memcmp(&s1, &s2, sizeof s1) != 0, "seed insensitive genre %u", g);

        amaru_plan_t bar;
        amaru_plan(&bar, g, 99, 70, AMARU_LOOP);
        if (g == AMARU_JUNGLE || g == AMARU_DNB || g == AMARU_BREAKBEAT) {
            CHECK(kick_on_quarter_bar0(&bar) < 4, "broken genre %u is four-on-the-floor", g);
            CHECK(kick_off_quarter(&bar) > 0, "broken genre %u has no off-quarter kick", g);
        } else {
            CHECK(kick_on_quarter_bar0(&bar) == 4, "four-on-floor genre %u lost a quarter kick", g);
        }
    }
    genre_identity();
    song_form();
    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("amaru_gen_test: all genres/forms valid and deterministic\n");
    return 0;
}
