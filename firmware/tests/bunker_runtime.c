/* SPDX-License-Identifier: GPL-3.0-only
 * Actual UI, audio ISR, sequencer, synthesis and NOR persistence in one harness.
 * No substitute oscillator and no simulated parameter-to-sound mapping.
 */
#include "../web/emu/felucca_web.c"
#include <assert.h>
#include <stddef.h>
static unsigned checks;
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); exit(1); } checks++; } while(0)
static uint32_t audio_hash=2166136261u, samples, peak, max_voices;
static double energy;
static FILE *wav;
static void run_ms(uint32_t ms)
{
    uint32_t n=ms*441u/10u;
    while(n) {
        uint32_t m=n>128 ? 128 : n;
        web_render(m);
        unsigned voices=0;
        for(unsigned k=0;k<4;k++) for(unsigned j=0;j<NVOICE;j++) voices+=!!trk[k].v[j].active;
        if(voices>max_voices)max_voices=voices;
        CHECK(voices<=NVOICE);
        for(unsigned i=0;i<m;i++) {
            CHECK(isfinite(out_l[i]) && isfinite(out_r[i]));
            int l=(int)(out_l[i]*32768),r=(int)(out_r[i]*32768);
            CHECK(abs(l)<=32768 && abs(r)<=32768);
            uint32_t p=(uint32_t)(abs(l)>abs(r)?abs(l):abs(r));if(p>peak)peak=p;
            audio_hash=(audio_hash^(uint32_t)l)*16777619u;
            energy+=(double)l*l+(double)r*r;samples+=2;
            if(wav)wav_put(wav,l,r);
        }
        n-=m;
    }
}
static void tap(unsigned b) {web_buttons(1u<<b);run_ms(60);web_buttons(0);run_ms(60);}
static void turn(unsigned e,int n) {web_enc(e,n);run_ms(45);}
static void boot(void) {web_nor_erase();web_boot();run_ms(500);}
static void screen(const char *path) {
    FILE*f=fopen(path,"wb");CHECK(f);fprintf(f,"P6\n240 240\n255\n");
    for(unsigned i=0;i<240*240;i++) {uint16_t q=web_fb[i];q=(uint16_t)((q>>8)|(q<<8));
        uint8_t rgb[3]={(uint8_t)((q>>11)*255/31),(uint8_t)(((q>>5)&63)*255/63),(uint8_t)((q&31)*255/31)};fwrite(rgb,1,3,f);}
    fclose(f);
}
static uint32_t pattern_hash(unsigned k) {return fnv(2166136261u,trk[k].step,sizeof trk[k].step);}
static void check_fields(void) {
    for(unsigned k=0;k<4;k++) {
        CHECK(trk[k].eng_req<NENGINES && trk[k].eng_req!=ENGI_DIGITAL);
        for(unsigned p=0;p<P_E0;p++) CHECK(trk[k].p[p]>=TP[p].min && trk[k].p[p]<=TP[p].max);
        for(unsigned p=0;p<8;p++) {const param_desc_t*d=&ENGINES[trk[k].eng_req]->edit[p];CHECK(trk[k].p[P_E0+p]>=d->min&&trk[k].p[P_E0+p]<=d->max);}
        for(unsigned i=0;i<NSTEP;i++) {step_t*s=&trk[k].step[i];CHECK(s->n<=4 && s->time<=ST_REST);for(unsigned j=0;j<s->n;j++)CHECK(s->note[j]<=127);}
    }
}
static void smoke(void) {
    CHECK(P_COUNT==114);CHECK(PROJ_MIDI_OFF >= 68+4*(P_COUNT+2+64*9)+sizeof(chain_config)+sizeof(motion_store_t));
    CHECK(NELEM(SCALE_MASK)==32 && NELEM(N_SCALE)==32);
    for(unsigned w=0;w<NELEM(B_WORLDS);w++) {
        bunker_scene(w);check_fields();
        project_t a,b;project_store_t disk;
        project_capture(&a);CHECK(proj_pack(&disk,&a));CHECK(proj_import(&b,&disk,sizeof disk));
        CHECK(!memcmp(a.t,b.t,sizeof a.t));
        for(unsigned k=0;k<4;k++) for(unsigned q=0;q<16;q++) {bunker_sound(k,q);check_fields();}
    }
    bunker_scene(0);
    CHECK(project_save(0)==0);
    int old=trk[1].p[P_BAMOUNT];trk[1].p[P_BAMOUNT]=99;project_load(0);CHECK(trk[1].p[P_BAMOUNT]==old);
    /* Panel gestures, no direct parameter setter for these checks. */
    go_home();bunker_page=0;
    int root=trk[0].p[P_ROOT];turn(EN_K2,1);CHECK(trk[0].p[P_ROOT]==root+1 && trk[3].p[P_ROOT]==root+1);
    tap(B_HOME);CHECK(bunker_page==BP_GROW);
    tap(B_HOME);CHECK(bunker_page==BP_SOUND);
    unsigned before=trk[0].eng_req;turn(EN_PRESET,3);CHECK(bunker_sound_pick[0]==3 && trk[0].eng_req!=before);
    int colour=trk[0].p[bunker_tone_id(&trk[0])];turn(EN_K2,-10);CHECK(trk[0].p[bunker_tone_id(&trk[0])]<colour);
    tap(B_HOME);CHECK(bunker_page==BP_MOTIF);
    uint32_t h=pattern_hash(0);turn(EN_K1,4);tap(B_OCTUP);CHECK(pattern_hash(0)!=h);
    tap(B_HOME);CHECK(bunker_page==BP_EVOLVE);
    turn(EN_K1,-7);CHECK(trk[0].p[P_BMODE]==0);
    h=pattern_hash(0);for(unsigned i=0;i<1000;i++)bunker_cycle(&trk[0],0);CHECK(pattern_hash(0)==h);
    turn(EN_K1,1);turn(EN_K2,100);turn(EN_K3,-32);
    CHECK(trk[0].p[P_BMODE]==1&&trk[0].p[P_BAMOUNT]==100&&trk[0].p[P_BPERIOD]==1);
    tap(B_OCTUP);CHECK(pattern_hash(0)!=h);
    h=pattern_hash(0);song.rec=1;for(unsigned i=0;i<100;i++)bunker_cycle(&trk[0],0);CHECK(pattern_hash(0)==h);song.rec=0;
    tap(B_OCTDN);CHECK(trk[0].p[P_BMODE]==0);
    tap(B_PLAY);CHECK(song.playing);tap(B_PLAY);CHECK(!song.playing);
    tap(B_EDIT);CHECK(!ui.home);tap(B_HOME);CHECK(ui.home && bunker_page==0);
    for(unsigned scale=0;scale<32;scale++) {bunker_harmony(3,scale);for(unsigned k=0;k<4;k++)for(unsigned m=0;m<24;m++){bunker_pattern(k,m,1+m%16);check_fields();}}
    puts("Bunker smoke: panel -> real tracks, 24 worlds, 64 sounds, 24 motifs x 32 scales, project persistence, freeze and recording ownership passed");
}
#ifndef BUNKER_RUNTIME_LIBRARY
int main(int argc,char**argv) {
    boot();
    if(argc<2||!strcmp(argv[1],"smoke"))smoke();
    else if(!strcmp(argv[1],"world")) {
        unsigned w=argc>2?atoi(argv[2]):0, sec=argc>3?atoi(argv[3]):10;
        bunker_scene(w);run_ms(100);
        if(argc>4){wav=fopen(argv[4],"wb");CHECK(wav);wav_hdr(wav,sec*FS);}
        transport_req=1;run_ms(sec*1000);if(wav)fclose(wav);
    } else if(!strcmp(argv[1],"sound")) {
        unsigned k=atoi(argv[2]),p=atoi(argv[3]);CHECK(k<4&&p<16);
        bunker_scene(0);for(unsigned i=0;i<4;i++)trk[i].p[P_MUTE]=(i!=k);
        bunker_sound(k,p);run_ms(100);peak=0;energy=0;samples=0;audio_hash=2166136261u;
        transport_req=1;run_ms(5000);
    } else if(!strcmp(argv[1],"screens")) {
        bunker_scene(0);for(unsigned i=0;i<BP_COUNT;i++){char path[128];bunker_page=i;ui.msg_t=0;ui.force=1;run_ms(100);snprintf(path,sizeof path,"build/bunker-screen-%u.ppm",i);screen(path);}
    } else if(!strcmp(argv[1],"animate")) {
        bunker_scene(0);transport_req=1;run_ms(2000);bunker_page=BP_GROW;bunker_grow_sync();grow.all=1;
        bunker_grow_apply();ui.msg_t=0;
        for(unsigned i=0;i<32;i++){char p[80];run_ms(125);snprintf(p,sizeof p,"build/bunker-motion-%02u.ppm",i);screen(p);}
    } else if(!strcmp(argv[1],"stress")) {
        unsigned sec=argc>2?atoi(argv[2]):1800;uint32_t rng=7;
        for(unsigned i=0;i<sec;i++) {
            if(i%30==0){transport_req=2;run_ms(50);bunker_scene((i/30)%24);transport_req=1;}
            rng=rng*1664525u+1013904223u;unsigned k=(rng>>8)&3u;
            if(i%7==0)bunker_sound(k,(rng>>16)&15u);
            if(i%3==0){trk[k].p[P_BMODE]=(int16_t)(1+(rng%7));trk[k].p[P_BAMOUNT]=100;trk[k].p[P_BPERIOD]=1;bunker_mutate(&trk[k],1);}
            if(i%11==0){bunker_pattern(k,(rng>>20)%24,1+(rng%16));trk[k].p[P_SLEN]=(int16_t)(1+(rng%64));}
            if(i%13==0)bunker_harmony(rng%12,(rng>>12)%32);
            if(i%17==0){grow.root=rng%12;grow.scale=(rng>>12)%32;grow.style=(rng>>24)%4;grow.all=(rng>>28)&1;bunker_grow_apply();}
            if(i%17==1 && grow.valid)bunker_grow_undo();
            bunker_page=(uint8_t)(i%BP_COUNT);ui.force=1;
            song.g[G_BPM]=(int16_t)(30+rng%151);check_fields();run_ms(1000);
        }
    }
    printf("{\"checks\":%u,\"hash\":%u,\"peak\":%u,\"rms\":%.2f,\"maxVoices\":%u,\"seconds\":%.2f}\n",checks,audio_hash,peak,samples?sqrt(energy/samples):0,max_voices,samples/(FS*2.0));
    return 0;
}

#endif
