#ifndef RX3_NES_CONTROLS_H
#define RX3_NES_CONTROLS_H
#include <stdint.h>
#include <stddef.h>
/* Independent protocol implementation: SUB crossfader ADC at 22/23,
 * Mixer headphone CUE is decoded separately from the EUP packet. NES joypad bit numbers follow libretro. */
struct nes_steering {unsigned baseline,raw;int seen,active,valid;unsigned cue,select;};
static void nes_sub_decode(struct nes_steering*s,const unsigned char*b,size_t n){
 s->valid=0;s->cue=s->select=0;
 if((n!=100&&n!=104)||b[0]||b[1]||b[2]||b[3]!=1)return;
 unsigned raw=b[22]|((unsigned)b[23]<<8);if(raw>1023)return;
 s->valid=1;s->raw=raw;
 if(!s->seen){s->seen=1;s->baseline=raw;}
 if((raw>s->baseline?raw-s->baseline:s->baseline-raw)>24)s->active=1;
}
static unsigned nes_buttons(unsigned directions,unsigned a,unsigned start,const struct nes_steering*s){
 unsigned b=(directions&1?1u<<4:0)|(directions&2?1u<<5:0)|(directions&4?1u<<6:0)|(directions&8?1u<<7:0)|(a?1u<<8:0)|(start?1u<<3:0);
 if(s->valid){if(s->cue)b|=8;/* right CUE: Start */if(s->select)b|=4;/* left CUE: Select */
 if(!(directions&12)&&s->active){if(s->raw<448)b|=1u<<6;else if(s->raw>576)b|=1u<<7;}}
 if((b&192)==192)b&=~192u;
 return b;
}
#endif
