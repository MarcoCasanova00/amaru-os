/* SPDX-License-Identifier: GPL-3.0-only
 * Test adapter only. Real firmware USB/SysEx handler; simulated device memory.
 * Upstream harness Copyright (C) 2026 Leo Kuroshita, Hügelton Instruments.
 */
#define FELUCCA_VERSION "BE 1.0-RC2"
#define BUNKER_EDITOR_BRIDGE 1
#include "../firmware/tests/editor_test.c"
#undef main
#include "../firmware/firmware/src/bunker_ui.c"
int main(void) {
    char line[4096]; uint8_t b[1024]; reset();
    while(fgets(line,sizeof line,stdin)) {
        if(line[0]=='!') { unsigned k=0,i=0,d=0;char what[16]={0};sscanf(line+1,"%15s %u %u %u",what,&k,&i,&d);
            if(!strcmp(what,"sound"))bunker_sound(k,i);
            if(!strcmp(what,"motif"))bunker_pattern(k,i,d);
            puts("[]");fflush(stdout);continue;
        }
        char *p=line,*end;unsigned n=0;
        while(*p&&n<sizeof b){unsigned long v=strtoul(p,&end,10);if(p==end)break;if(v>255)return 2;b[n++]=(uint8_t)v;p=end;}
        if(n<6||b[0]!=240||b[n-1]!=247)return 3;
        request(b[4],b+5,n-6);
        if(transport_req==2){seq_stop();transport_req=0;}
        if(host_erases||host_writes)return 4;
        putchar('[');for(unsigned i=0;i<host_wire_n;i++)printf("%s%u",i?",":"",host_wire[i]);puts("]");fflush(stdout);
    }
    return 0;
}
