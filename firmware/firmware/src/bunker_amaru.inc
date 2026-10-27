/* SPDX-License-Identifier: GPL-3.0-only
 * Bunker adapter for the pure AMARU planner. Included by bunker_ui.c, where
 * Bunker's static track/song/editor state is in scope.
 */
static uint8_t amaru_genre = AMARU_TECHNO;
static uint16_t amaru_seed = 0x4D41u;
static uint8_t amaru_energy = 70;
static uint8_t amaru_form = AMARU_PARTS;
static uint8_t amaru_bpm = 140;

static void amaru_sound_apply(track_t *t, const amaru_sound_t *s)
{
    set_engine_of(t, s->engine);
    apply_preset_to(t, s->preset);
    fm1_irq_off();
    if (s->engine != E_DRUM)
        bunker_colour(t, s->tone);
    t->p[P_ATK] = s->atk;
    t->p[P_DEC] = 80;
    t->p[P_SUS] = 100;
    t->p[P_REL] = s->rel;
    t->p[P_CHOR] = s->chor;
    t->p[P_DLY] = s->dly;
    t->p[P_REV] = s->rev;
    t->p[P_DIST] = s->dist;
    t->p[P_VOICE] = s->voice;
    t->p[P_LRATE] = 16;
    t->p[P_LWAVE] = 0;
    t->p[P_LTRIG] = 1;
    t->p[P_LD_FLT] = 0;
    t->p[P_SPRD] = s->engine == E_DRUM ? 0 : 14;
    fm1_irq_on();
}

static void amaru_apply_plan(const amaru_plan_t *p)
{
    static const uint8_t level[AMARU_TRACKS] = {94, 100, 96, 91};
    static const int8_t pan[AMARU_TRACKS] = {-12, 12, 0, 22};
    if (song.playing || song.rec || chain_busy()) {
        ui_message("STOP PLAY / REC / SONG");
        return;
    }
    song.rec = 0;
    song.g[G_BPM] = p->bpm;
    song.g[G_SWING] = p->swing;
    for (uint32_t k = 0; k < NTRK; k++) {
        track_t *t = &trk[k];
        fm1_irq_off();
        track_defaults(t);
        fm1_irq_on();
        amaru_sound_apply(t, &p->sound[k]);
        amaru_last[k] = p->sound[k];    /* the SOUND page shows the recipe's name until it is edited */
        fm1_irq_off();
        t->p[P_SLEN] = p->steps;
        t->p[P_SDIV] = 2;                 /* 1/16 */
        t->p[P_SSWING] = 0;
        t->p[P_SGATE] = k == AMARU_AIR ? 120 : k == AMARU_MELODY ? 82 :
                        k == AMARU_ROOT ? (p->genre == AMARU_JUNGLE || p->genre == AMARU_DNB ? 120 :
                                           p->genre == AMARU_PSY || p->genre == AMARU_TECHNO ? 55 : 72) : 0;
        t->p[P_ROOT] = p->root;
        t->p[P_SCALE] = p->scale;
        t->p[P_QUANT] = 3;
        t->p[P_TRANS] = 0;
        t->p[P_LEVEL] = level[k];
        t->p[P_PAN] = pan[k];
        t->p[P_MUTE] = 0;
        t->p[P_BMODE] = (int16_t)(k == AMARU_AIR ? 7 : k == AMARU_MELODY ? 1 : k == AMARU_ROOT ? 5 : 0);
        t->p[P_BAMOUNT] = 0;              /* explicit EVOLVE (OCT+) still works; no surprise auto-mutation */
        t->p[P_BPERIOD] = 4;
        t->seq_active = 1;
        memset(t->step, 0, sizeof t->step);
        for (uint32_t i = 0; i < p->steps && i < NSTEP; i++) {
            const amaru_step_t *s = &p->step[k][i];
            step_t *d = &t->step[i];
            d->n = s->n;
            for (uint32_t j = 0; j < s->n && j < 4u; j++)
                d->note[j] = s->note[j];
            d->time = s->time;
            d->flags = s->flags;
            d->vel = s->vel;
            d->hit = s->hit;
            d->acc = s->acc;
            d->probability = 0;
        }
        fm1_irq_on();
        bunker_loops[k] = 0;
        bunker_changes[k] = 0;
        bunker_sound_pick[k] = 0;
    }
    undo.keep = 0;                        /* a whole generated piece is not a one-track undo */
    sync_reload = 1;
    proj_cur = PROJ_NO_SLOT;
    ui.force = 1;
}

static void amaru_generate(void)
{
    amaru_plan_t p;
    amaru_plan(&p, amaru_genre, amaru_seed, amaru_energy, amaru_form);
    amaru_bpm = p.bpm;
    amaru_apply_plan(&p);
    if (!song.playing && !song.rec && !chain_busy()) {
        char b[48];
        str_cpy(b, amaru_genre_name(p.genre), sizeof b);
        uint32_t n = (uint32_t)str_len(b);
        b[n++] = ' ';
        str_cpy(b + n, amaru_form_name(p.form), sizeof b - n);
        n = (uint32_t)str_len(b);
        b[n++] = ' '; str_cpy(b + n, "SEED", sizeof b - n);
        n = (uint32_t)str_len(b);
        b[n++] = ' ';
        fmt_int(b + n, p.seed);
        ui_message(b);
    }
}

static void amaru_new_seed(void)
{
    amaru_seed = (uint16_t)(amaru_seed * 1665u + 10139u + amaru_genre * 257u);
    /* the caller's message carries the new take / seed reset feedback */
}
