/* SPDX-License-Identifier: GPL-3.0-only
 * Bunker Evolved's front panel. The existing EDIT / ENV / LFO / FX / SCL /
 * ARP / SEQ / SAVE pages continue to edit the same tracks and same DSP.
 */
#include "bunker_library.inc"
static void bunker_ui_boot(void)
{
    /* Reserved settings byte: migrate the display once, keep later choices. */
    if (favorites.factory[15][13] != 0xB1u) {
        favorites.factory[15][13] = 0xB1u;
        ui_home_view = HV_BUNKER; settings.palette = UI_BUNKER_INDEX; palette_apply();
    }
}

static uint8_t bunker_world_pick;
static uint8_t bunker_sound_pick[4], bunker_motif_pick[4];
static uint8_t bunker_density[4] = {4,7,2,3};
static const char *const B_ROLES[] = {"AIR", "MELODY", "ROOT", "SPARK"};
static const char *const B_PAGES[] = {"WORLD", "GROW", "SOUND", "MOTIF", "EVOLVE", "MIX"};
static uint32_t bunker_tone_id(const track_t *t)
{
    static const uint8_t ids[NENGINES] = {4,2,1,2,3,1,4,1,7,2,2,2,2
#if FELUCCA_SLICE
        ,7
#endif
    };
    return P_E0 + ids[t->eng_req % NENGINES];
}
static const char *bunker_sound_name(uint32_t k)
{
    const track_t *t = &trk[k];
    for (uint32_t i=0;i<16u;i++) {
        const bunker_sound_t *s=&B_SOUNDS[k][i];
        if(t->eng_req!=s->engine || t->preset!=s->preset) continue;
        uint32_t id=bunker_tone_id(t);
        const param_desc_t *d=&ENGINES[t->eng_req]->edit[id-P_E0];
        int tone=d->min+(int)s->tone*(d->max-d->min)/127;
        if(t->p[id]==tone && t->p[P_ATK]==s->attack && t->p[P_REL]==s->release &&
           t->p[P_CHOR]==s->chorus && t->p[P_DLY]==s->delay && t->p[P_REV]==s->reverb) {
            bunker_sound_pick[k]=(uint8_t)i; return s->name;
        }
    }
    return "EDITED SOUND";
}
static void bunker_colour(track_t *t, uint32_t tone)
{
    uint32_t id = bunker_tone_id(t);
    const param_desc_t *d = &ENGINES[t->eng_req % NENGINES]->edit[id - P_E0];
    t->p[id] = (int16_t)(d->min + (int32_t)tone * (d->max - d->min) / 127);
}
static void bunker_sound(uint32_t k, uint32_t index)
{
    track_t *t = &trk[k % 4u];
    const bunker_sound_t *s = &B_SOUNDS[k % 4u][index % 16u];
    bunker_sound_pick[k % 4u] = (uint8_t)(index % 16u);
    load_begin(t, UNDO_SOUND);
    set_engine_of(t, s->engine);
    apply_preset_to(t, s->preset);
    fm1_irq_off();
    bunker_colour(t, s->tone);
    t->p[P_ATK] = s->attack; t->p[P_REL] = s->release;
    t->p[P_CHOR] = s->chorus; t->p[P_DLY] = s->delay; t->p[P_REV] = s->reverb;
    t->p[P_VOICE] = k == 2u ? V_MONO : V_POLY;
    t->p[P_LRATE] = 16; t->p[P_LWAVE] = 0;
    t->p[P_LTRIG] = 1;
    t->p[P_LD_FLT] = k == 0u ? 6 : 0;
    t->p[P_SPRD] = k == 0u ? 30 : k == 2u ? 0 : 14;
    fm1_irq_on();
    load_end(t);
    ui.force = 1;
}
static void bunker_pattern(uint32_t k, uint32_t motif, uint32_t density)
{
    track_t *t = &trk[k % 4u];
    const bunker_motif_t *m = &B_MOTIFS[motif % NELEM(B_MOTIFS)];
    uint32_t len = (uint32_t)clamp(t->p[P_SLEN], 1, NSTEP), hit = 0;
    step_t pattern[NSTEP];
    memset(pattern, 0, sizeof pattern);
    density = (uint32_t)clamp((int32_t)density, 1, 16);
    for (uint32_t i = 0; i < NSTEP; i++) {
        step_t *s = &pattern[i];
        s->time = ST_REST;
        if (i >= len || (i * density % 16u) >= density) continue;
        int degree = m->degree[hit++ & 7u];
        int base = k == 2u ? 36 : k == 0u ? 48 : k == 1u ? 60 : 72;
        /* Every role uses the chosen editable motif, in a shared key. */
        s->n = (uint8_t)(k == 0u ? 2 : 1);
        s->note[0] = (uint8_t)bunker_degree(t, degree, base);
        if (s->n == 2u) s->note[1] = (uint8_t)bunker_degree(t, degree + 4, base);
        s->time = ST_NOTE;
        s->vel = (uint8_t)(k == 0u ? 65 : k == 2u ? 76 : 68 + (i * 7u % 24u));
        if (k == 3u) step_set_chance(s, 80);
    }
    load_begin(t, UNDO_PAT);
    fm1_irq_off();
    memcpy(t->step, pattern, sizeof pattern);
    t->seq_active = 1;
    bunker_loops[k] = 0;
    fm1_irq_on();
    bunker_motif_pick[k] = (uint8_t)(motif % NELEM(B_MOTIFS));
    bunker_density[k] = (uint8_t)density;
    load_end(t);
    ui.force = 1;
}
static void bunker_harmony(int root, int scale)
{
    fm1_irq_off();
    for (uint32_t k = 0; k < NTRK; k++) {
        track_t *t = &trk[k];
        int delta = root - t->p[P_ROOT];
        t->p[P_ROOT] = (int16_t)root; t->p[P_SCALE] = (int16_t)scale; t->p[P_QUANT] = 3;
        for (uint32_t i = 0; i < NSTEP; i++) for (uint32_t j = 0; j < t->step[i].n && j < 4u; j++)
            t->step[i].note[j] = (uint8_t)scale_snap(t, clamp((int)t->step[i].note[j] + delta, 24, 108));
    }
    fm1_irq_on();
    ui.force = 1;
}
static void bunker_scene(uint32_t index)
{
    if (song.playing || chain_busy()) { ui_message("STOP TO LOAD WORLD"); return; }
    const bunker_world_t *w = &B_WORLDS[index % NELEM(B_WORLDS)];
    static const uint8_t level[4] = {94,100,96,91}, modes[4] = {7,1,0,5};
    static const int8_t pan[4] = {-12,12,0,22};
    bunker_world_pick = (uint8_t)(index % NELEM(B_WORLDS));
    bunker_rng = 0x42554e4bu ^ (index * 2654435761u);
    chain_defaults(&chain_config);
    song.rec = 0;
    for (uint32_t i = 0; i < G_COUNT; i++) song.g[i] = GP[i].def;
    song.g[G_BPM] = w->bpm;
    song.g[G_RTYPE] = 2; song.g[G_RSIZE] = 84; song.g[G_RDAMP] = 85;
    song.g[G_DTIME] = 10; song.g[G_DFDBK] = 49; song.g[G_DCOLOR] = 50;
    for (uint32_t k = 0; k < NTRK; k++) {
        track_t *t = &trk[k];
        fm1_irq_off();
        track_defaults(t);
        fm1_irq_on();
        t->p[P_SLEN] = 32; t->p[P_SDIV] = 1; t->p[P_SGATE] = k == 0 ? 120 : 76;
        t->p[P_LEVEL] = level[k]; t->p[P_PAN] = pan[k];
        t->p[P_ROOT] = w->root; t->p[P_SCALE] = w->scale; t->p[P_QUANT] = 3;
        t->p[P_BMODE] = modes[k]; t->p[P_BAMOUNT] = k == 0 ? 30 : 45; t->p[P_BPERIOD] = k == 0 ? 4 : 2;
        bunker_sound(k, w->sound[k]);
        bunker_pattern(k, w->motif[k], w->density[k]);
        bunker_changes[k] = 0;
    }
    str_cpy(proj_name, w->name, sizeof proj_name);
    proj_cur = PROJ_NO_SLOT;
    undo.keep = 0; /* a whole world is not a one-track undo */
    sync_reload = 1;
    ui_message("WORLD READY - PLAY");
    ui.force = 1;
}
#include "bunker_grow.c"
static void bunker_home_input(uint32_t oct)
{
    int32_t s;
    uint32_t k = song.sel;
    track_t *t = TSEL;
    if ((s = panel_enc(EN_ALGO)) != 0) {
        track_select((uint32_t)clamp((int32_t)song.sel + (s > 0 ? 1 : -1), 0, 3));
        k = song.sel; t = TSEL;
    }
    if ((s = panel_enc(EN_SELECT)) != 0) {
        bunker_page = (uint8_t)clamp((int32_t)bunker_page + (s > 0 ? 1 : -1), 0, BP_COUNT-1);
        ui.force = 1;
    }
    bunker_grow_sync();
    if ((s = panel_enc(EN_PRESET)) != 0) {
        if (bunker_page == BP_WORLD) bunker_world_pick = (uint8_t)clamp(bunker_world_pick + s, 0, NELEM(B_WORLDS)-1);
        else if (bunker_page == BP_MOTIF) bunker_motif_pick[k] = (uint8_t)clamp(bunker_motif_pick[k] + s, 0, NELEM(B_MOTIFS)-1);
        else if (bunker_page == BP_GROW) grow.style=(uint8_t)clamp(grow.style+s,0,3);
        else bunker_sound(k, (uint32_t)clamp(bunker_sound_pick[k] + s, 0, 15));
        ui.force = 1;
    }
    for (uint32_t c = 0; c < 4u; c++) {
        if (!(s = panel_enc(EN_K1+c))) continue;
        ui.force = 1;
        if (bunker_page == BP_WORLD) {
            if (c == 0) bunker_world_pick = (uint8_t)clamp(bunker_world_pick + s, 0, NELEM(B_WORLDS)-1);
            if (c == 1) bunker_harmony(clamp(t->p[P_ROOT]+s,0,11), t->p[P_SCALE]);
            if (c == 2) bunker_harmony(t->p[P_ROOT], clamp(t->p[P_SCALE]+s,0,NELEM(N_SCALE)-1));
            if (c == 3) song.g[G_BPM] = (int16_t)clamp(song.g[G_BPM]+s,30,180);
        } else if (bunker_page == BP_GROW) {
            if(c==0)grow.root=(uint8_t)clamp(grow.root+s,0,11);
            if(c==1)grow.scale=(uint8_t)clamp(grow.scale+s,0,31);
            if(c==2)grow.style=(uint8_t)clamp(grow.style+s,0,3);
            if(c==3)grow.all=(uint8_t)clamp(grow.all+s,0,1);
        } else if (bunker_page == BP_SOUND) {
            if (c == 0) bunker_sound(k, (uint32_t)clamp(bunker_sound_pick[k]+s,0,15));
            else {
                uint32_t id = c == 1 ? bunker_tone_id(t) : c == 2 ? P_REL : P_REV;
                const param_desc_t *d = id >= P_E0 ? &ENGINES[t->eng_req]->edit[id-P_E0] : &TP[id];
                t->p[id] = (int16_t)clamp(t->p[id]+s,d->min,d->max);
                motion_capture(t,id,t->p[id]); ml_knob(t,id);
            }
        } else if (bunker_page == BP_MOTIF) {
            if (c == 0) bunker_motif_pick[k] = (uint8_t)clamp(bunker_motif_pick[k]+s,0,NELEM(B_MOTIFS)-1);
            if (c == 1) bunker_density[k] = (uint8_t)clamp(bunker_density[k]+s,1,16);
            if (c == 2) t->p[P_SLEN] = (int16_t)clamp(t->p[P_SLEN]+s,1,64);
            if (c == 3) t->p[P_SDIV] = (int16_t)clamp(t->p[P_SDIV]+s,0,NELEM(N_DIV)-1);
        } else if (bunker_page == BP_EVOLVE) {
            static const uint8_t ids[4] = {P_BMODE,P_BAMOUNT,P_BPERIOD,P_TRANS};
            uint32_t id = ids[c]; t->p[id] = (int16_t)clamp(t->p[id]+s,TP[id].min,TP[id].max);
            motion_capture(t,id,t->p[id]); ml_knob(t,id);
        } else { trk[c].p[P_LEVEL] = (int16_t)clamp(trk[c].p[P_LEVEL]+s,0,127);
            motion_capture(&trk[c],P_LEVEL,trk[c].p[P_LEVEL]); ml_knob(&trk[c],P_LEVEL); }
    }
    if (oct & 2u) {
        if (bunker_page == BP_WORLD) bunker_scene(bunker_world_pick);
        else if (bunker_page == BP_GROW) bunker_grow_apply();
        else if (bunker_page == BP_MOTIF) {
            if (song.rec || chain_busy()) ui_message("STOP REC / SONG FIRST");
            else { bunker_pattern(k,bunker_motif_pick[k],bunker_density[k]); ui_message("MOTIF APPLIED"); }
        } else if (bunker_page == BP_EVOLVE) {
            if (song.rec || chain_busy()) ui_message("STOP REC / SONG FIRST");
            else { load_begin(t,UNDO_PAT); fm1_irq_off(); bunker_mutate(t,1); fm1_irq_on(); load_end(t); ui.force=1; }
        } else { t->p[P_MUTE] ^= 1; ui.force=1; }
    }
    if (oct & 1u) {
        if (bunker_page == BP_GROW) bunker_grow_undo();
        else if (bunker_page == BP_EVOLVE) { t->p[P_BMODE]=0; ui_message("MOTIF LOCKED"); }
        else { t->p[P_MUTE] ^= 1; ui_message(t->p[P_MUTE] ? "TRACK MUTED" : "TRACK ON"); }
        ui.force=1;
    }
}
#include "bunker_draw.c"
