/*
 * Minimal IMA ADPCM decoder prototype for AMARU OS research.
 *
 * Purpose: show that a 3-drum kit (snare/hihat/clap) stored as IMA ADPCM
 * in a flash sector (loaf-breaker uses 0xDE000, 8 KB) can be decoded to
 * 16-bit PCM on the fly without re-enabling the retired SAMPLE engine.
 *
 * NOT part of the verified build. Do not compile into firmware.
 * Standard IMA ADPCM (4-bit nibbles, step table, predictor per block).
 */
#include <stdint.h>
#include <stddef.h>

/* IMA ADPCM step table (standard) */
static const int16_t step_table[89] = {
    7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,
    50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,
    279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,
    1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,
    4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,
    13899,15289,16818,18500,20350,22385,24623,27086,29794,32767
};

/* Index change table (standard) */
static const int8_t index_table[16] = {
    -1,-1,-1,-1, 2,4,6,8, -1,-1,-1,-1, 2,4,6,8
};

typedef struct {
    int32_t predictor;   /* current PCM value */
    int32_t step_index;  /* into step_table */
} adpcm_state_t;

/* Decode one nibble */
static int16_t adpcm_nibble(adpcm_state_t* st, uint8_t nibble) {
    int step = step_table[st->step_index];
    int diff = step >> 3;
    if (nibble & 1) diff += step >> 2;
    if (nibble & 2) diff += step >> 1;
    if (nibble & 4) diff += step;
    if (nibble & 8) diff = -diff;

    st->predictor += diff;
    if (st->predictor >  32767) st->predictor =  32767;
    if (st->predictor < -32768) st->predictor = -32768;

    st->step_index += index_table[nibble];
    if (st->step_index < 0)  st->step_index = 0;
    if (st->step_index > 88) st->step_index = 88;

    return (int16_t)st->predictor;
}

/*
 * Decode a raw IMA ADPCM byte stream into 16-bit PCM.
 * Each byte = two 4-bit nibbles (high nibble first).
 * Returns number of 16-bit samples written.
 */
size_t adpcm_decode(const uint8_t* in, size_t in_len, int16_t* out, size_t out_cap) {
    adpcm_state_t st = { 0, 0 };
    size_t written = 0;
    for (size_t i = 0; i < in_len && written < out_cap; ++i) {
        out[written++] = adpcm_nibble(&st, (in[i] >> 4) & 0x0F);
        if (written < out_cap)
            out[written++] = adpcm_nibble(&st, in[i] & 0x0F);
    }
    return written;
}

/*
 * Example: decode 3 drums from a 3-byte-per-sample layout.
 * (Real layout depends on loaf-breaker's kit_frames.py — adjust here.)
 */
void adpcm_decode_kit(const uint8_t* raw, size_t len, int16_t out[3][8]) {
    for (int drum = 0; drum < 3; ++drum) {
        adpcm_state_t st = { 0, 0 };
        for (int i = 0; i < 8; ++i) {
            size_t idx = drum * 8 + i;
            if (idx >= len) { out[drum][i] = 0; continue; }
            out[drum][i] = adpcm_nibble(&st, raw[idx] & 0x0F);
        }
    }
}
