#ifndef AMARU_SOUND_ALIAS_C_INCLUDED
#define AMARU_SOUND_ALIAS_C_INCLUDED
#include <stdint.h>
#include "amaru_sound_alias.h"

typedef struct { uint16_t engine, preset; const char *name; } A;

static const A AME[] = {
    {9,0,"SKY BREATH"}, {9,1,"GRAIN VOID"}, {9,2,"HYPOGEUM"},
    {4,0,"SH-2 DRIVE"}, {4,1,"SH-2 GLOW"}, {4,2,"SILBURY"},
    {12,0,"SWARM LEAD"}, {12,1,"REESE PAD"}, {5,4,"DNB PAD"},
    {2,3,"ACID WAIL"}, {11,0,"MENTAL NOISE"},
    {7,0,"MYC PUNCH"}, {7,2,"MYC BLOWN"},
    {8,1,"ORACLE CHOIR"}, {8,2,"AMARU CHOIR"},
    {0,8,"ANALOG 9"}, {1,4,"PHASE LEAD"},
};
static char buf[128][13];

const char *amaru_preset_name(uint16_t engine, uint16_t preset,
                               const char *engine_name, const char *fallback)
{
    for (unsigned i = 0; i < sizeof AME/sizeof AME[0]; i++)
        if (AME[i].engine == engine && AME[i].preset == preset)
            return AME[i].name;
    int key = engine * 32 + preset;
    if (key >= 128) key %= 128;
    if (!buf[key][0]) {
        buf[key][0] = 'P';
        buf[key][1] = (char)('0' + ((preset / 10) % 10));
        buf[key][2] = (char)('0' + (preset % 10));
        buf[key][3] = 0;
    }
    return buf[key];
}
#endif
