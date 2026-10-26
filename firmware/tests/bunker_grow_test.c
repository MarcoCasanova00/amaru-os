/* SPDX-License-Identifier: GPL-3.0-only */
#define BUNKER_RUNTIME_LIBRARY 1
#include "bunker_runtime.c"
static void scale_ok(unsigned k,unsigned root,unsigned scale)
{
    for(unsigned i=0;i<NSTEP;i++)for(unsigned j=0;j<trk[k].step[i].n;j++) {
        int n=trk[k].step[i].note[j];CHECK(n>=24 && n<=108);
        CHECK(SCALE_MASK[scale] & (1u<<((n-(int)root+120)%12)));
    }
}
static void variations(void)
{
    /* Every key/scale/style, both targeted and ensemble application. */
    for(unsigned root=0;root<12;root++)for(unsigned scale=0;scale<32;scale++)for(unsigned style=0;style<4;style++) {
        bunker_scene((root+scale)%24);
        project_t before,after;project_capture(&before);
        grow.root=root;grow.scale=scale;grow.style=style;grow.all=1;
        bunker_grow_apply();CHECK(grow.valid);
        for(unsigned k=0;k<4;k++) {
            if(!(grow.mask&(1u<<k)))continue;
            scale_ok(k,root,scale);CHECK(trk[k].p[P_BMODE]==0);
            CHECK(trk[k].p[P_SLEN]==before.t[k].p[P_SLEN]);
            CHECK(trk[k].p[P_LEVEL]==before.t[k].p[P_LEVEL]);
            CHECK(trk[k].eng_req==before.t[k].engine);
            for(unsigned i=0;i<NSTEP;i++){step_t a=trk[k].step[i],b=grow.before[k].step[i];memset(a.note,0,4);memset(b.note,0,4);CHECK(!memcmp(&a,&b,sizeof a));}
        }
        CHECK(P_COUNT==114);project_capture(&after);
        project_store_t disk;project_t restored;
        CHECK(proj_pack(&disk,&after));CHECK(proj_import(&restored,&disk,sizeof disk));CHECK(!memcmp(after.t,restored.t,sizeof after.t));
        bunker_grow_undo();CHECK(!grow.valid);project_capture(&after);
        CHECK(!memcmp(before.t,after.t,sizeof before.t));
    }
    /* Actual panel: enter GROW, choose a different key/scale, generate and undo. */
    bunker_scene(0);track_select(1);go_home();bunker_page=BP_WORLD;
    uint32_t orig[4];for(unsigned k=0;k<4;k++)orig[k]=grow_signature(&trk[k]);
    tap(B_HOME);CHECK(bunker_page==BP_GROW);
    turn(EN_K1,3);turn(EN_K2,4);turn(EN_K3,1);
    CHECK(grow.root==5 && grow.scale==7 && grow.style>=1);
    grow.all=0;tap(B_OCTUP);CHECK(grow.valid && grow.mask==2);CHECK(grow_signature(&trk[1])!=orig[1]);
    for(unsigned k=0;k<4;k++)if(k!=1)CHECK(grow_signature(&trk[k])==orig[k]);
    tap(B_OCTDN);for(unsigned k=0;k<4;k++)CHECK(grow_signature(&trk[k])==orig[k]);
    grow.all=1;tap(B_OCTUP);CHECK(grow.valid);
    /* A newer edit makes the old undo unavailable; never overwrite it. */
    trk[1].step[0].note[0]++;
    uint32_t edited=grow_signature(&trk[1]);bunker_grow_undo();CHECK(!grow.valid && grow_signature(&trk[1])==edited);
    /* Recording and chains are the sole owners of their notes. */
    song.rec=1;bunker_grow_apply();CHECK(!grow.valid && grow_signature(&trk[1])==edited);song.rec=0;
    chain.running=1;bunker_grow_apply();CHECK(!grow.valid && grow_signature(&trk[1])==edited);chain.running=0;
    /* Drum and slice addresses are preserved even for ALL. */
    set_engine_of(&trk[2],ENGI_DRUM);set_engine_of(&trk[3],13);
    orig[2]=grow_signature(&trk[2]);orig[3]=grow_signature(&trk[3]);bunker_grow_apply();
    CHECK(grow.mask==3);CHECK(grow_signature(&trk[2])==orig[2] && grow_signature(&trk[3])==orig[3]);
    /* Empty sequence behavior and note range extremes. */
    bunker_scene(0);track_select(1);grow.all=0;grow.root=11;grow.scale=31;
    memset(trk[1].step,0,sizeof trk[1].step);
    grow.style=1;bunker_grow_apply();for(unsigned i=0;i<NSTEP;i++)CHECK(trk[1].step[i].n==0);
    grow.style=3;bunker_grow_apply();CHECK(trk[1].step[0].n==1);scale_ok(1,11,31);
    trk[1].step[0].note[0]=0;trk[1].step[4].note[0]=127;
    grow.style=0;bunker_grow_apply();scale_ok(1,11,31);
    /* Live audition stays fixed through complete cycles, and undo is available. */
    bunker_scene(0);grow.all=1;grow.style=2;bunker_grow_apply();
    for(unsigned k=0;k<4;k++)orig[k]=grow_signature(&trk[k]);
    transport_req=1;run_ms(30000);
    for(unsigned k=0;k<4;k++)CHECK(grow_signature(&trk[k])==orig[k]);
    bunker_grow_undo();CHECK(!grow.valid);transport_req=2;run_ms(50);
    puts("PASS GROW: all 12 keys x 32 scales x 4 styles, sound/rhythm preservation, project roundtrip, exact undo, panel actions, single/all scope, guards, pitched-only, empty track, edge notes, live lock");
}
int main(void){boot();variations();printf("GROW checks: %u; peak: %u; max voices: %u\n",checks,peak,max_voices);return 0;}
