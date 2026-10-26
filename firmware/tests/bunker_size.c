#define _Static_assert(...)
#define main unused_main
#include "hostsim.c"
#undef main
#include <stddef.h>
int main(void){printf("size=%zu pad=%zu armp=%zu end=%zu CPU2=%d\n",sizeof(track_t),offsetof(track_t,rsv_sz),offsetof(track_t,armp),offsetof(track_t,m_rnd),FELUCCA_CPU2);}
