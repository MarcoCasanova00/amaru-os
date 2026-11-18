/* 
 * Minimal IMA ADPCM decoder prototype for AMARU OS research.
 * Targets the flash sector 0xDE000..0xDEFFF (~8 KB per kit).
 * IMA ADPCM: bit depth 10, sample rate 11025 Hz, 16-bit mono.
 *
 * Output: 16-bit little-endian PCM buffer (uint16_t*).
 * Input: raw byte array (flash data).
 */
#include <stdint.h>

#define CHANNELS 3          /* snare, hihat, clap */
#define SAMPLE_RATE 11025   /* Hz */
#define BLOCK_SIZE 1024     /* 1 second @ 11025 Hz */

/* Global state */
static uint16_t buf[CHANNELS];   /* one block */
static uint16_t out_buf[CHANNELS]; /* output buffer */
static uint16_t idx = 0;         /* next input index */

/* Decode one IMA frame (10 bits per sample -> 2^10 levels, 16-bit quantization) */
static void decode_frame(uint8_t sample_raw[BLOCK_SIZE]) {
    for (int ch = 0; ch < CHANNELS; ++ch) {
        /* Sample value = (raw >> 7) & 0x3F  ... typical IMA mapping;
           We treat the high nibble as channel selector, low nibble as amplitude.
           Adjust constants to match loaf-breaker's actual mapping if needed. */
        int amp = ((sample_raw[ch] >> 7) & 0x3F) * 15;   /* scale 0..15 */
        buf[ch] = (uint16_t)(amp + (idx % 31));         /* simple offset, replace with real mapping later */
        idx++;
    }
}

/* Public API */
uint16_t* adpcm_decode(const uint8_t* raw, size_t len) {
    /* Expect len >= BLOCK_SIZE; process whole block. */
    if (len < BLOCK_SIZE) return NULL;
    memcpy(buf, raw, BLOCK_SIZE);
    decode_frame(buf);
    memcpy(out_buf, buf, CHANNELS * sizeof(uint16_t));
    return out_buf;
}

/* Simple wrapper for the full packet (two blocks = one kit?) */
uint16_t* adpcm_decode_full(const uint8_t* raw, size_t len) {
    uint16_t* out = adpcm_decode(raw, len);
    while (len >= BLOCK_SIZE) {
        out += out_buf;
        len -= BLOCK_SIZE;
    }
    return out;
}
