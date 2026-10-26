/* SPDX-License-Identifier: GPL-3.0-only
 * Bunker RC2: one-button, scale-safe variation. UI/main-loop work only.
 * Fixed storage; no heap; no new serialized parameters or format migration.
 * Snapshot under IRQ lock, compose outside it, short atomic commit.
 */
static const char *const B_GROW_STYLE[] = {"TUNE", "SOFT", "FLOW", "NEW"};
static const char *const B_SCALE_LONG[] = {
    "CHROMATIC", "MAJOR", "MINOR", "DORIAN", "MIXOLYDIAN", "MAJOR PENTA", "MINOR PENTA", "HARMONIC MINOR",
    "PHRYGIAN", "LYDIAN", "LOCRIAN", "MELODIC MINOR", "BLUES", "WHOLE TONE", "DIM HALF WHOLE", "DIM WHOLE HALF",
    "HIRAJOSHI", "INSEN", "IWATO", "KUMOI", "EGYPTIAN", "RYUKYU", "HUNGARIAN", "PHRYG DOMINANT",
    "LYDIAN DOMINANT", "DORIAN b2", "NEAPOLITAN", "AUGMENTED", "PROMETHEUS", "TRITONE", "HARMONIC MAJOR", "ENIGMATIC"
};
typedef struct {
    step_t step[NSTEP];
    int16_t root, scale, quant, mode;
    uint8_t active;
} bunker_phrase_t;
static struct {
    bunker_phrase_t before[NTRK];
    step_t after[NTRK][NSTEP];
    uint32_t signature[NTRK];
    uint32_t rng;
    uint16_t serial;
    uint8_t root, scale, style, all, mask, valid;
    uint8_t previous_page, previous_track;
} grow = {.rng=0x47524f57u, .style=1, .previous_page=255, .previous_track=255};

static uint32_t grow_rand(void)
{
    uint32_t x=grow.rng; x^=x<<13; x^=x>>17; x^=x<<5;
    return grow.rng=x ? x : 1u;
}
static uint32_t grow_signature(const track_t *t)
{
    uint32_t h=2166136261u;
    const uint8_t *p=(const uint8_t *)t->step;
    for(uint32_t i=0;i<sizeof t->step;i++)h=(h^p[i])*16777619u;
    static const uint8_t ids[]={P_ROOT,P_SCALE,P_QUANT,P_BMODE,P_SLEN};
    for(uint32_t i=0;i<NELEM(ids);i++)h=(h^(uint16_t)t->p[ids[i]])*16777619u;
    return (h^t->seq_active)*16777619u;
}
static void bunker_grow_sync(void)
{
    if(bunker_page==BP_GROW && (grow.previous_page!=BP_GROW || grow.previous_track!=song.sel)) {
        grow.root=(uint8_t)clamp(TSEL->p[P_ROOT],0,11);
        grow.scale=(uint8_t)clamp(TSEL->p[P_SCALE],0,31);
    }
    grow.previous_page=bunker_page; grow.previous_track=(uint8_t)song.sel;
}
static int grow_snap(int note, uint32_t root, uint32_t scale)
{
    uint32_t mask=SCALE_MASK[scale];
    int n=clamp(note,24,108);
    /* Prefer the lower scale note; at the lower bound search upwards. */
    for(int j=0;j<12;j++) {
        if(n-j>=24 && (mask & (1u<<((n-j-(int)root+120)%12))))return n-j;
    }
    for(int j=1;j<12;j++)if(mask & (1u<<((n+j-(int)root+120)%12)))return n+j;
    return n;
}
static int grow_walk(int note,int steps)
{
    uint32_t mask=SCALE_MASK[grow.scale];
    int n=note,dir=steps<0 ? -1 : 1;
    for(int i=0;i<(steps<0 ? -steps : steps);i++) {
        int next=n;
        for(int j=0;j<12;j++) {
            next+=dir;
            if(next<24 || next>108)return n;
            if(mask & (1u<<((next-(int)grow.root+120)%12)))break;
        }
        n=next;
    }
    return n;
}
static void bunker_grow_apply(void)
{
    if(song.rec || chain_busy()) {ui_message("STOP REC / SONG FIRST");return;}
    uint32_t mask=grow.all ? 15u : 1u<<song.sel;
    /* Percussion and slices use their notes as addresses, not scale pitches. */
    for(uint32_t k=0;k<NTRK;k++)if(trk[k].eng_req==ENGI_DRUM || trk[k].eng_req==13u)mask&=~(1u<<k);
    if(!mask){ui_message("CHOOSE A PITCHED SOUND");return;}
    grow.mask=(uint8_t)mask; grow.valid=0;
    fm1_irq_off();
    for(uint32_t k=0;k<NTRK;k++)if(mask & (1u<<k)) {
        track_t *t=&trk[k];bunker_phrase_t *b=&grow.before[k];
        memcpy(b->step,t->step,sizeof b->step);memcpy(grow.after[k],t->step,sizeof t->step);
        b->root=t->p[P_ROOT];b->scale=t->p[P_SCALE];b->quant=t->p[P_QUANT];b->mode=t->p[P_BMODE];b->active=t->seq_active;
    }
    fm1_irq_on();
    for(uint32_t k=0;k<NTRK;k++)if(mask & (1u<<k)) {
        uint32_t len=(uint32_t)clamp(trk[k].p[P_SLEN],1,NSTEP),hits=0,changed=0;
        step_t *p=grow.after[k];
        int delta=(int)grow.root-grow.before[k].root;
        for(uint32_t i=0;i<len;i++)hits+=p[i].n && p[i].time==ST_NOTE;
        /* NEW also creates a sparse seed on an empty track; the other styles
         * preserve intentional silence. Existing rhythms/velocities stay intact. */
        if(!hits && grow.style==3) {
            uint32_t spacing=k==2 ? 8u : k==0 ? 8u : 4u;
            for(uint32_t i=0;i<len;i+=spacing) {
                memset(&p[i],0,sizeof p[i]);p[i].time=ST_NOTE;p[i].n=k==0 ? 2 : 1;p[i].vel=76;
                p[i].note[0]=(uint8_t)(k==2 ? 36 : k==0 ? 48 : k==1 ? 60 : 72);
                if(p[i].n==2)p[i].note[1]=p[i].note[0]+7;
            }
        }
        int phrase[8];
        for(uint32_t i=0;i<8;i++)phrase[i]=(int)(grow_rand()%9u)-4;
        uint32_t hit=0;
        for(uint32_t i=0;i<NSTEP;i++) {
            step_t *s=&p[i];
            for(uint32_t j=0;j<s->n && j<4u;j++)s->note[j]=(uint8_t)grow_snap((int)s->note[j]+delta,grow.root,grow.scale);
            if(i>=len || !s->n || s->time!=ST_NOTE || !grow.style)continue;
            int shift=0;
            if(grow.style==3)shift=phrase[hit++&7u];
            else if(grow_rand()%100u < (grow.style==1 ? 30u : 65u)) {
                shift=(int)(1u+grow_rand()%(grow.style==1 ? 1u : 3u));
                if(grow_rand()&1u)shift=-shift;
            }
            for(uint32_t j=0;j<s->n && j<4u;j++) {
                int old=s->note[j];s->note[j]=(uint8_t)grow_walk(old,shift);changed+=s->note[j]!=old;
            }
        }
        /* Every requested pitched variation moves at least one audible note. */
        if(grow.style && !changed)for(uint32_t i=0;i<len;i++)if(p[i].n && p[i].time==ST_NOTE) {
            int n=p[i].note[0],next=grow_walk(n,1);
            p[i].note[0]=(uint8_t)(next==n ? grow_walk(n,-1) : next);break;
        }
    }
    fm1_irq_off();
    for(uint32_t k=0;k<NTRK;k++)if(mask & (1u<<k)) {
        track_t *t=&trk[k];memcpy(t->step,grow.after[k],sizeof t->step);
        t->p[P_ROOT]=grow.root;t->p[P_SCALE]=grow.scale;t->p[P_QUANT]=QN_SEQ;t->p[P_BMODE]=0;
        if(grow.style==3)t->seq_active=1;
        bunker_loops[k]=0;
        grow.signature[k]=grow_signature(t);
    }
    fm1_irq_on();
    grow.valid=1;grow.serial++;undo.keep=0;ui.force=1;
    ui_message(grow.style ? "NEW PHRASE - LOCKED" : "SCALE FIT - LOCKED");
}
static void bunker_grow_undo(void)
{
    if(song.rec || chain_busy()){ui_message("STOP REC / SONG FIRST");return;}
    if(!grow.valid){ui_message("NO GROW TO UNDO");return;}
    fm1_irq_off();
    for(uint32_t k=0;k<NTRK;k++)if((grow.mask&(1u<<k)) && grow_signature(&trk[k])!=grow.signature[k]) {
        fm1_irq_on();grow.valid=0;ui_message("PHRASE EDITED - NO UNDO");return;
    }
    for(uint32_t k=0;k<NTRK;k++)if(grow.mask&(1u<<k)) {
        track_t *t=&trk[k];const bunker_phrase_t *b=&grow.before[k];
        memcpy(t->step,b->step,sizeof t->step);t->p[P_ROOT]=b->root;t->p[P_SCALE]=b->scale;
        t->p[P_QUANT]=b->quant;t->p[P_BMODE]=b->mode;t->seq_active=b->active;bunker_loops[k]=0;
    }
    fm1_irq_on();grow.valid=0;undo.keep=0;ui.force=1;
    grow.root=(uint8_t)TSEL->p[P_ROOT];grow.scale=(uint8_t)TSEL->p[P_SCALE];
    ui_message("PREVIOUS PHRASE BACK");
}
