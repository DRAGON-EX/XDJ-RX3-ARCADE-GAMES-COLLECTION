#ifndef RX3_RESCUE_PLAYBACK_H
#define RX3_RESCUE_PLAYBACK_H
#define RESCUE_SCENE_TICKS 1200u
struct rescue_playback { unsigned cat,ticks,sample; int active; };
static void rescue_begin(struct rescue_playback*r,unsigned cat){*r=(struct rescue_playback){.cat=cat,.active=1};}
static void rescue_advance(struct rescue_playback*r){if(r->active&&++r->ticks>=RESCUE_SCENE_TICKS)r->active=0;}
/* Fanfare is streamed by the host, keeping decoded audio out of each game WASM. */
static void rescue_audio(struct rescue_playback*r,int16_t*out,unsigned frames,unsigned rate,float*gain,float target){
 (void)rate;(void)gain;(void)target;r->sample+=frames;
 for(unsigned i=0;i<frames*2;i++)out[i]=0;
}
#endif
