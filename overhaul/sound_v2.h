#ifndef RIFT_SOUND_H
#define RIFT_SOUND_H
#include <stdint.h>
typedef struct {uint32_t pos,lead,bass,noise;int last_step;} RiftSound;
void rift_audio_generate(RiftSound *s,int16_t *out,int frames,int battle,int accent);
void rift_sound_effect(int kind);
void rift_sound_reset(RiftSound *s);
#endif
