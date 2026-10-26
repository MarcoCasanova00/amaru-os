/* SPDX-License-Identifier: GPL-3.0-only
 * RC2: four small constellations driven by actual notes and playheads.
 * Integer geometry, existing canvas only, <=8 Hz, no audio RNG consumption.
 */
static uint16_t bunker_ink(uint32_t k)
{
    static const uint16_t c[4]={RGB(184,147,255),RGB(150,245,160),RGB(110,208,255),RGB(249,157,209)};
    return settings.palette==UI_BUNKER_INDEX ? c[k&3u] : k==song.sel ? T_ACCENT : T_THEME;
}
static void bunker_diamond(int x,int y,int r,uint16_t c)
{
    cv_line(x,y-r,x+r,y,c);cv_line(x+r,y,x,y+r,c);
    cv_line(x,y+r,x-r,y,c);cv_line(x-r,y,x,y-r,c);
}
static void bunker_garden(void)
{
    static const int8_t wave[32]={0,1,2,3,4,5,5,6,6,6,5,5,4,3,2,1,0,-1,-2,-3,-4,-5,-5,-6,-6,-6,-5,-5,-4,-3,-2,-1};
    uint32_t phase=(ui_prefs&PREF_ANIM_OFF) ? 0 : ui.frame/8u;
    /* A quiet star field, bounded to the middle band. */
    for(uint32_t i=0;i<18u;i++)cv_rect(25+(int)(i*47u%202u),35+(int)(i*29u%70u),1,1,T_LINE);
    for(uint32_t k=0;k<NTRK;k++) {
        track_t *t=&trk[k];uint32_t len=(uint32_t)clamp(t->p[P_SLEN],1,NSTEP);
        uint16_t c=t->p[P_MUTE] ? T_DIM : bunker_ink(k);
        int base=45+(int)k*18,oldx=25,oldy=base;
        char b[2]={(char)('1'+k),0};
        if(k==song.sel){cv_rect(2,base-8,17,16,T_SURF);cv_rect(2,base-8,2,16,c);}
        cv_text(7,base-7,&AF_S,b,c);
        for(uint32_t j=0;j<16u;j++) {
            uint32_t start=j*len/16u,end=(j+1u)*len/16u;
            if(end<=start)end=start+1u;
            const step_t *st=&t->step[start];uint32_t note=0,chord=0;
            for(uint32_t p=start;p<end && p<len;p++)if(t->step[p].n && t->step[p].time==ST_NOTE) {
                st=&t->step[p];note=st->note[0];chord=st->n;break;
            }
            int x=28+(int)j*13,y=base+(wave[(phase+j*2u+k*5u)&31u]/3);
            if(chord)y-=1+(int)(note%12u)/3;
            cv_line(oldx,oldy,x,y,t->p[P_MUTE] ? T_LINE : c);
            if(chord){cv_rect(x-1,y-1,3,3,c);if(chord>1)cv_rect(x,y+3,1,2,T_MID);}
            if(song.playing && t->seq_idx>=start && t->seq_idx<end && !t->p[P_MUTE]) {
                bunker_diamond(x,y,4,c);cv_rect(x-1,y-1,3,3,T_TEXT);
                cv_rect(x,base+5,1,2,c);
            }
            oldx=x;oldy=y;
        }
    }
    uint32_t root=bunker_page==BP_GROW ? grow.root : (uint32_t)TSEL->p[P_ROOT];
    uint32_t scale=bunker_page==BP_GROW ? grow.scale : (uint32_t)clamp(TSEL->p[P_SCALE],0,31);
    for(uint32_t n=0;n<12u;n++) {
        int on=!!(SCALE_MASK[scale]&(1u<<((n+12u-root)%12u)));
        int x=7+(int)n*19;
        cv_rect(x,113,15,3,on ? T_THEME : T_LINE);
        if(n==root)cv_rect(x,112,15,5,T_ACCENT);
    }
}
static void bunker_draw(void)
{
    static uint32_t last_frame;
    uint32_t k=song.sel;track_t *t=TSEL;
    char val[4][24],b[48];const char *labels[4];
    if(!ui.force && ui.frame-last_frame<8u)return;
    if(!ui.force && (ui_prefs&PREF_ANIM_OFF) && !song.playing && !ui.msg_t)return;
    last_frame=ui.frame;bunker_grow_sync();
    if(bunker_page==BP_WORLD) {
        labels[0]="WORLD";labels[1]="KEY";labels[2]="SCALE";labels[3]="BPM";
        fmt_int(val[0],bunker_world_pick+1);str_cpy(val[1],N_NOTE[t->p[P_ROOT]],24);
        str_cpy(val[2],N_SCALE[t->p[P_SCALE]],24);fmt_int(val[3],song.g[G_BPM]);
    } else if(bunker_page==BP_GROW) {
        labels[0]="KEY";labels[1]="SCALE";labels[2]="MOVE";labels[3]="TARGET";
        str_cpy(val[0],N_NOTE[grow.root],24);str_cpy(val[1],N_SCALE[grow.scale],24);
        str_cpy(val[2],B_GROW_STYLE[grow.style],24);str_cpy(val[3],grow.all ? "ALL" : B_ROLES[k],24);
    } else if(bunker_page==BP_SOUND) {
        (void)bunker_sound_name(k);
        labels[0]="SOUND";labels[1]="COLOUR";labels[2]="RELEASE";labels[3]="SPACE";
        fmt_int(val[0],bunker_sound_pick[k]+1);fmt_int(val[1],t->p[bunker_tone_id(t)]);
        fmt_int(val[2],t->p[P_REL]);fmt_int(val[3],t->p[P_REV]);
    } else if(bunker_page==BP_MOTIF) {
        labels[0]="MOTIF";labels[1]="HITS/16";labels[2]="LENGTH";labels[3]="RATE";
        fmt_int(val[0],bunker_motif_pick[k]+1);fmt_int(val[1],bunker_density[k]);
        fmt_int(val[2],t->p[P_SLEN]);str_cpy(val[3],N_DIV[t->p[P_SDIV]],24);
    } else if(bunker_page==BP_EVOLVE) {
        labels[0]="MODE";labels[1]="CHANCE";labels[2]="LOOPS";labels[3]="TRANS";
        str_cpy(val[0],N_BMODE[t->p[P_BMODE]],24);fmt_int(val[1],t->p[P_BAMOUNT]);
        fmt_int(val[2],t->p[P_BPERIOD]);fmt_int(val[3],t->p[P_TRANS]);
    } else for(uint32_t c=0;c<4u;c++){labels[c]=B_ROLES[c];fmt_int(val[c],trk[c].p[P_LEVEL]);}
    cv_begin(240,26,T_BG);
    cv_text(6,3,&AF_S,"BUNKER",T_THEME);cv_text(74,3,&AF_S,B_PAGES[bunker_page],T_TEXT);
    for(uint32_t p=0;p<BP_COUNT;p++)cv_rect(145+(int)p*7,7,p==bunker_page ? 4 : 2,p==bunker_page ? 4 : 2,p==bunker_page ? T_ACCENT : T_DIM);
    cv_text(198,3,&AF_S,song.playing ? "PLAY" : "STOP",song.playing ? T_ACCENT : T_MID);
    cv_rect(0,24,240,1,T_LINE);cv_blit(0,0);
    cv_begin(240,54,T_BG);
    for(uint32_t c=0;c<4u;c++) {
        int x=(int)c*60;uint16_t ink=bunker_ink(c);
        cv_rect(x+2,2,56,48,T_SURF);cv_rect(x+2,2,56,2,ink);
        cv_text_on(x+6,7,&AF_S,labels[c],T_MID,T_SURF);cv_text_fit(x+6,25,&AF_M,val[c],ink,T_SURF,48);
        cv_rect(x+6,46,48,1,T_LINE);
    }
    cv_blit(0,26);
    cv_begin(240,120,T_BG);
    if(bunker_page==BP_GROW) {
        str_cpy(b,N_NOTE[grow.root],48);uint32_t n=(uint32_t)str_len(b);b[n++]=' ';str_cpy(b+n,B_SCALE_LONG[grow.scale],48-n);
        cv_text_fit(6,0,&AF_M,b,T_TEXT,T_BG,228);
        const char *caption=grow.style==0 ? "FIT NOTES TO YOUR SCALE" : grow.style==1 ? "SMALL STEPS, SAME RHYTHM" : grow.style==2 ? "LET THE MELODY WANDER" : "A FRESH MELODIC PHRASE";
        cv_text_fit(6,21,&AF_S,caption,T_MID,T_BG,228);
    } else {
        const char *title=bunker_page==BP_WORLD ? B_WORLDS[bunker_world_pick].name : bunker_page==BP_SOUND ? bunker_sound_name(k) : bunker_page==BP_MOTIF ? B_MOTIFS[bunker_motif_pick[k]].name : proj_name;
        cv_text_fit(6,0,&AF_M,title,T_TEXT,T_BG,228);
        str_cpy(b,N_NOTE[t->p[P_ROOT]],48);uint32_t n=(uint32_t)str_len(b);b[n++]=' ';str_cpy(b+n,B_SCALE_LONG[clamp(t->p[P_SCALE],0,31)],48-n);
        cv_text_fit(6,21,&AF_S,b,T_MID,T_BG,170);
        cv_text_r(233,21,&AF_S,N_BMODE[t->p[P_BMODE]],T_ACCENT,T_BG);
    }
    bunker_garden();cv_blit(0,80);
    cv_begin(240,40,T_BG);
    if(ui.msg_t)cv_text_fit(6,0,&AF_S,ui.msg,T_TEXT,T_BG,228);
    else {
        cv_text(6,0,&AF_S,B_ROLES[k],bunker_ink(k));
        cv_text(77,0,&AF_S,ENGINES[t->eng_req]->name,T_MID);
        if(bunker_page==BP_GROW){fmt_int(b,grow.serial);cv_text_r(232,0,&AF_S,b,T_DIM,T_BG);}
    }
    if(bunker_page==BP_GROW) {
        cv_rect(3,19,113,19,T_ACCENT);cv_rect(120,19,117,19,T_SURF);
        cv_text_on(12,21,&AF_S,"OCT+ GROW",T_INK,T_ACCENT);
        cv_text_on(129,21,&AF_S,"OCT- UNDO",grow.valid ? T_TEXT : T_DIM,T_SURF);
    } else {
        const char *hint=bunker_page==BP_WORLD ? "OCT+ LOAD  /  HOME: GROW" : bunker_page==BP_SOUND ? "EDIT: SYNTH / OCT-: MUTE" : bunker_page==BP_MOTIF ? "OCT+ APPLY / SEQ: EDIT" : bunker_page==BP_EVOLVE ? "OCT+ MUTATE / OCT- LOCK" : "K1-4 LEVEL / OCT- MUTE";
        cv_text_fit(6,21,&AF_S,hint,T_TEXT,T_BG,228);
    }
    cv_blit(0,200);
    if(ui.msg_t)ui.msg_t=(uint8_t)(ui.msg_t>8 ? ui.msg_t-8 : 0);
    ui.force=0;
}
