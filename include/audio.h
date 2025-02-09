#ifndef AUDIO_H
#define AUDIO_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <maxmod9.h>

void initSound();
void PlaySound(uint8_t effect);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H