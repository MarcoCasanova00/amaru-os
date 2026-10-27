/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* SAMPLE: IMA ADPCM sample sets. The store (the factory sets, the user slots, IMA_STEP) is smp_store.c,
 * included before this; the engine is built with FELUCCA_SAMPLE=1 only (engines.c). */
static const char *const N_ONOFF_E[] = {"OFF", "ON"};

/* voice: ph[0] position (samples), ph[1] fraction Q16, s[0] predictor, s[1] step
 * index, s[2] previous sample, s[3] current sample, s[4] zone, s[5] step Q16 */
static inline int32_t sample_next(const smp_zone_t *z, voice_t *v, int loop)
{
    uint32_t pos = v->ph[0], b = SMP_DATA[z->off + (pos >> 1)];
    uint32_t code = (pos & 1u) ? (b >> 4) : (b & 15u);
    int32_t step = IMA_STEP[(uint32_t)v->s[1] <= 88u ? v->s[1] : 88], vd = step >> 3;
    if (code & 4u)
        vd += step;
    if (code & 2u)
        vd += step >> 1;
    if (code & 1u)
        vd += step >> 2;
    v->s[0] = clamp(v->s[0] + ((code & 8u) ? -vd : vd), -32768, 32767);
    v->s[1] = clamp(v->s[1] + IMA_IDX[code & 7u], 0, 88);
    pos++;
    if (pos > z->le && z->looped && loop) {
        pos = z->ls;
        v->s[0] = z->pred;
        v->s[1] = z->idx;
    }
    v->ph[0] = pos;
    return v->s[0];
}

static void sample_note_on(track_t *t, voice_t *v)
{
    uint32_t si = (uint32_t)t->p[P_E0] % SMP_NALL, i, zi = 0xFFFFu, width = 128u;
    if (si < SMP_NSETS) {                           /* a built-in set: its zones split the keyboard */
        const smp_set_t *set = &SMP_SETS[si];
        for (i = 0; i < set->nz; i++)
            if (v->note >= SMP_ZONES[set->z0 + i].lo && v->note <= SMP_ZONES[set->z0 + i].hi) {
                zi = set->z0 + i;
                break;
            }
        v->s[4] = (int32_t)(zi == 0xFFFFu ? set->z0 : zi);
    } else {                                        /* user slot: silent if empty; the narrowest zone plays */
        uint32_t k = si - SMP_NSETS;
        for (i = 0; i < usr_nz[k]; i++) {
            const smp_zone_t *z = &usr_zone[k][i];
            if (v->note >= z->lo && v->note <= z->hi && z->hi - z->lo <= width) {
                zi = 0x8000u | k << 5 | i;
                width = z->hi - z->lo;
            }
        }
        v->s[4] = (int32_t)(zi == 0xFFFFu ? 0 : zi);
    }
    v->ph[0] = 0;
    v->ph[1] = 0;
    v->s[0] = 0;
    v->s[1] = 0;
    v->s[2] = v->s[3] = 0;
    v->s[6] = zi == 0xFFFFu;     /* 1 = sample ended (one-shot); a kit key with no sound stays silent */
    v->s[7] = 0;                 /* lo-pass state */
    if (smp_set_missing(si))
        v->s[6] = 2;             /* 2 = no sample data: the sine (smp_sine; ph[0] its phase) */
}

static void sample_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    const smp_zone_t *z = smp_zone((uint32_t)v->s[4]);
    uint32_t i, frac = v->ph[1];
    int32_t d16 = clamp(m->pitch16 + p[P_E1] * 16 - z->root16, -1536, 576);   /* <= 3 octaves up: bounded decode load */
    uint32_t r = pow2_q16(d16), stepq = (r >> 8) * (z->rate >> 8);   /* Q16 samples per output */
    int32_t bits = p[P_E2], lp = 4000 + ((clamp((p[P_E4] << 8) + m->cutoff, 0, 127 << 8) * 28767) >> 15);
    int32_t drv = p[P_E6], sh = bits / 10;
    if (v->s[6] == 2) {                               /* no sample data: the sine at the note's pitch + TUNE */
        smp_sine(out, n, m, p[P_E1] * 16, lp, &v->ph[0], &v->s[7]);
        return;
    }
    if (v->s[6] || !z->n) {
        v->active = 0;
        return;
    }
    if (v->ph[0] == 0 && v->ph[1] == 0 && v->s[3] == 0)
        v->s[3] = sample_next(z, v, p[P_E3]);         /* prime the interpolator */
    for (i = 0; i < n; i++) {
        int32_t s;
        frac += stepq;
        while (frac >= 65536u) {
            frac -= 65536u;
            v->s[2] = v->s[3];
            if (v->ph[0] >= z->n) {                   /* one-shot reached its end */
                v->s[6] = 1;
                v->s[3] = 0;
                if (frac >= 65536u) {                /* past the last sample-to-zero interval */
                    v->s[2] = 0;
                    frac &= 65535u;
                }
                break;
            }
            v->s[3] = sample_next(z, v, p[P_E3]);
        }
        s = v->s[2] + (((v->s[3] - v->s[2]) * (int32_t)(frac >> 1)) >> 15);
        if (sh)                                       /* BITS: 0 = clean, up to 12 bits removed */
            s = (s >> sh) << sh;
        if (drv)
            s = softclip(s + (((s >> 2) * (drv * 150)) >> 11));   /* = s * drv * 600 / 32768, no overflow */
        v->s[7] += mulq15(s - v->s[7], lp);           /* gentle tone control (CUT) */
        out[i] += voice_amp(v->s[7], m, i) << 1;
        if (v->s[6])
            break;
    }
    v->ph[1] = frac;
}

static const engine_t ENG_SAMPLE = {
    .name = "SAMPLE",
    .page_title = {"SET", "TONE"},
    .edit = {
        {"SET", F_ENUM, 0, SMP_NALL - 1, 0, SMP_ALL_NAMES, 0},
        {"TUNE", F_SEMI, -24, 24, 0, 0, 0},
        {"BITS", F_INT, 0, 127, 0, 0, 0},
        {"LOOP", F_ENUM, 0, 1, 1, N_ONOFF_E, 0},
        {"CUT", F_INT, 0, 127, 127, 0, 0},
        {"-", F_INT, 0, 0, 0, 0, 0},
        {"DRV", F_PCT, 0, 127, 0, 0, 0},
        {"-", F_INT, 0, 0, 0, 0, 0},
    },
    .presets = SMP_PRESET_TABLE,
    .npresets = SMP_NPRESETS,
    .note_on = sample_note_on,
    .render = sample_render,
    .knob = {P_E0, P_E4, P_ATK, P_REL},
    .sampled = 1,
};
