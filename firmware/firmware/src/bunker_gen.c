/* SPDX-License-Identifier: GPL-3.0-only
 * Bunker Evolved composition layer. Fixed-size, allocation-free, bounded work.
 * Included by seq.c after scale helpers. It edits the actual step sequencer.
 * Each track keeps its current notes: FREEZE therefore preserves an audible phrase.
 */
static uint32_t bunker_rng = 0x42554e4bu;
static uint8_t bunker_loops[NTRK];
static uint32_t bunker_changes[NTRK];
static uint32_t bunker_rand(void)
{
    uint32_t x = bunker_rng;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return bunker_rng = x ? x : 1u;
}
static int bunker_degree(const track_t *t, int degree, int base)
{
    int oct = 0, d[12], n = 0;
    uint32_t mask = scale_mask(t);
    for (int j = 0; j < 12; j++) if (mask & (1u << j)) d[n++] = j;
    if (!n) return base;
    while (degree < 0) { degree += n; oct--; }
    oct += degree / n;
    return clamp(base + t->p[P_ROOT] + 12 * oct + d[degree % n], 24, 108);
}
static int bunker_neighbor(const track_t *t, int note, int direction)
{
    int n = note;
    for (int j = 0; j < 12; j++) {
        n += direction;
        if (n < 28 || n > 100) return note;
        if (scale_mask(t) & (1u << ((n - t->p[P_ROOT] + 120) % 12))) return n;
    }
    return note;
}
static void bunker_mutate(track_t *t, uint32_t force)
{
    uint32_t k = (uint32_t)(t - trk), mode = (uint32_t)t->p[P_BMODE];
    uint32_t amount = (uint32_t)clamp(t->p[P_BAMOUNT], 0, 100);
    uint32_t len = (uint32_t)clamp(t->p[P_SLEN], 1, NSTEP);
    if (k >= NTRK || !mode || mode > 7u || (!force && !amount) || t->eng_req == ENGI_DRUM) return;
    if (!force && bunker_rand() % 100u >= amount) return;
    uint32_t at = bunker_rand() % len, other = (at + 1u + bunker_rand() % len) % len;
    step_t *s = &t->step[at], *o = &t->step[other];
    if (mode == 3u) { /* ORBIT: rotate the musical pattern, preserving every note */
        step_t last = t->step[len - 1u];
        for (uint32_t j = len - 1u; j; j--) t->step[j] = t->step[j - 1u];
        t->step[0] = last;
    } else if (mode == 4u) { /* MIRROR: exchange two complete steps */
        step_t tmp = *s; *s = *o; *o = tmp;
    } else if (mode == 6u) { /* BREATH: velocity and chance, never pitch */
        for (uint32_t j = 0; j < len; j++) if (t->step[j].n && t->step[j].time == ST_NOTE) {
            int d = (int)(bunker_rand() % 17u) - 8;
            t->step[j].vel = (uint8_t)clamp((int)t->step[j].vel + d, 45, 110);
            step_set_chance(&t->step[j], (uint32_t)clamp((int)step_chance(&t->step[j]) + d, 45, 100));
        }
    } else {
        /* Find one existing note, at most one lap. Empty patterns stay empty. */
        for (uint32_t j = 0; j < len && (!s->n || s->time != ST_NOTE); j++) {
            at = (at + 1u) % len; s = &t->step[at];
        }
        if (!s->n || s->time != ST_NOTE) return;
        if (mode == 1u) { /* WALK: one scale step, all chord tones together */
            int dir = bunker_rand() & 1u ? 1 : -1;
            for (uint32_t j = 0; j < s->n && j < 4u; j++) s->note[j] = (uint8_t)bunker_neighbor(t, s->note[j], dir);
        } else if (mode == 2u) { /* ANSWER: a note borrowed from this phrase */
            if (o->n && o->time == ST_NOTE) {
                memcpy(s->note, o->note, 4); s->n = o->n;
            } else s->note[0] = (uint8_t)bunker_neighbor(t, s->note[0], -1);
        } else if (mode == 5u) { /* BLOOM: move a note into a rest (constant density) */
            if (o->time == ST_REST || !o->n) { *o = *s; memset(s, 0, sizeof *s); s->time = ST_REST; }
            else s->note[0] = (uint8_t)bunker_neighbor(t, s->note[0], 1);
        } else { /* VOICING: octave inversion, preserving pitch classes */
            uint32_t j = bunker_rand() % s->n;
            s->note[j] = (uint8_t)(s->note[j] < 66 ? s->note[j] + 12 : s->note[j] - 12);
        }
    }
    bunker_changes[k]++;
}
static void bunker_cycle(track_t *t, uint32_t top)
{
    uint32_t k = (uint32_t)(t - trk);
    if (top) { bunker_loops[k] = 0; return; }
    /* Recording and song chains own their steps. No competing generative writer. */
    if (song.rec || chain.running || !t->p[P_BMODE]) return;
    uint32_t period = (uint32_t)clamp(t->p[P_BPERIOD], 1, 32);
    if (++bunker_loops[k] < period) return;
    bunker_loops[k] = 0;
    bunker_mutate(t, 0);
}
