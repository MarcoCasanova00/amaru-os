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
    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("amaru_gen_test: all genres/forms valid and deterministic\n");
    return 0;
}
