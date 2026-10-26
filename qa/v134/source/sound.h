#ifndef RIFT_SOUND_H
#define RIFT_SOUND_H
#include <stdint.h>
/* Entire synthesizer state belongs to the player instance so restarting a
 * run resets pad, reverb and SFX without leftover sound from the prior run.
 * Static global PSP audio player is in BSS, not on the audio thread stack. */
typedef struct {
 uint32_t pos,lead,bass,noise;
 uint32_t pad_phase1,pad_phase2,sfx_phase;
 unsigned delay_pos,observed_serial;
 int last_step,fx_kind,fx_age;
 unsigned block_serial,block_age,block_phase;
 int16_t ambience_delay[4096];
} RiftSound;
void rift_audio_generate(RiftSound *s,int16_t *out,int frames,int battle,int accent);
void rift_sound_effect(int kind);
void rift_sound_reset(RiftSound *s);
#endif
