/* SPDX-License-Identifier: GPL-3.0-only
 * AMARU OS factory-preset display aliases.
 *
 * Bunker Evolved's engine tables carry their own preset names (MARIMBA,
 * BELL TREE,...) for compatibility with golden hashes and the host test
 * suites. AMARU OS does not modify those tables; instead it overrides the
 * displayed name in the device UI only.
 *
 * A factory preset with no override falls back to the engine's own name.
 */
#ifndef AMARU_SOUND_ALIAS_H
#define AMARU_SOUND_ALIAS_H
#include <stdint.h>

/* engine id = index in ENGINES[] (ANALOG=0, FM=1, DIGITAL=2, LOGIC=3,
 * PHYS=4, FM6=5, SYNTH=6, DRUM=7, GRAIN=8, FORMANT=9, ANALOG2=10,
 * LOFI=11, NOISE=12, ...); preset = 0..npresets-1 */
const char *amaru_preset_name(uint16_t engine, uint16_t preset,
                               const char *engine_name, const char *fallback);

#endif