/* SPDX-License-Identifier: GPL-3.0-only
 * Record actual UI + DSP in lockstep for the product film. Host-only tool.
 */
#define BUNKER_RUNTIME_LIBRARY 1
#include "bunker_runtime.c"
int main(int argc,char **argv) {
    if(argc!=4)return 2;
    int chapter=atoi(argv[1]),frames=atoi(argv[2]);
    if(chapter<0||chapter>7||frames<1||frames>1000)return 2;
    boot();bunker_scene(0);go_home();
    for(unsigned k=0;k<4;k++)trk[k].p[P_BMODE]=0;
    bunker_page=chapter==2?BP_GROW:chapter==3?BP_SOUND:chapter==4?BP_EVOLVE:BP_WORLD;
    track_select(chapter==2||chapter==3||chapter==4?1:0);
    bunker_grow_sync();ui.force=1;run_ms(200);
    if(chapter==4){trk[1].p[P_BMODE]=1;trk[1].p[P_BAMOUNT]=100;trk[1].p[P_BPERIOD]=1;}
    char path[512];snprintf(path,sizeof path,"%s/audio.wav",argv[3]);wav=fopen(path,"wb");CHECK(wav);wav_hdr(wav,frames*4410u);
    transport_req=1;
    for(int i=0;i<frames;i++) {
        /* Physical control events, not a mock parameter-to-sound mapping. */
        if(chapter==2) {
            if(i==65)web_enc(EN_K1,2);
            if(i==100)web_enc(EN_K2,2);
            if(i==130)web_enc(EN_K3,1);
            if(i==170)web_enc(EN_K4,1);
            if(i==205)web_buttons(1u<<B_OCTUP);
            if(i==206)web_buttons(0);
            if(i==265)web_buttons(1u<<B_OCTDN);
            if(i==266)web_buttons(0);
        }
        if(chapter==3) {
            if(i==60)web_enc(EN_PRESET,2);
            if(i==115)web_enc(EN_K2,12);
            if(i==155)web_enc(EN_K3,-16);
            if(i==195)web_enc(EN_K4,12);
        }
        if(chapter==4) {
            if(i==115)web_enc(EN_K1,5);
            if(i==210)web_buttons(1u<<B_OCTDN);
            if(i==211)web_buttons(0);
        }
        run_ms(100);
        snprintf(path,sizeof path,"%s/%04d.ppm",argv[3],i);screen(path);
    }
    fclose(wav);printf("chapter %d frames %d checks %u peak %u voices %u\n",chapter,frames,checks,peak,max_voices);
    return 0;
}
