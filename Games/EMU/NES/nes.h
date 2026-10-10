#ifndef RX3_NES_H
#define RX3_NES_H
#include <stdint.h>
#include <stddef.h>
int nes_open(const char *rom,const char *core);
void nes_close(void),nes_tick(unsigned buttons);
extern uint32_t nes_video[512*480];
extern unsigned nes_width,nes_height,nes_audio_frames;
extern int16_t nes_audio[4096];
extern uint64_t nes_period;
extern int nes_video_enabled;
#endif
