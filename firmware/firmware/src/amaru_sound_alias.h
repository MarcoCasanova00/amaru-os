#ifndef AMARU_SOUND_ALIAS_H
#define AMARU_SOUND_ALIAS_H
#include <stdint.h>

uint16_t amaru_engine_index(const engine_t *e);
const char *amaru_preset_name(uint16_t engine, uint16_t preset,
                               const char *engine_name, const char *fallback);

#endif