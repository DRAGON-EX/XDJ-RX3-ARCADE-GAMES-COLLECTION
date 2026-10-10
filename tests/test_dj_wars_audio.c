#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../Games/Native/DJ Wars/game.h"
#include "../Games/Native/DJ Wars/audio.h"
static struct dw_game g;
static int nonzero(const int16_t*out,unsigned n){int signal=0;for(unsigned i=0;i<n;i++){signal|=out[2*i];assert(out[2*i]==out[2*i+1]);assert(out[2*i]<=30000&&out[2*i]>=-30000);}return signal!=0;}
int main(void){
 struct dw_audio_state a,b;int16_t out[4096],reference[4096];float gain=1,other=1;
 g.rng=42;g.state=DW_AIM;g.winner=-1;
 for(int e=1;e<=15;e++){
  dw_audio_reset(&a);a.music_gain=0;g.event=e;g.event_serial++;
  dw_audio_mix(&a,&g,out,735,&gain,1,1,0);
  assert(nonzero(out,735)&&g.rng==42);
 }
 g.event=g.event_serial=0;dw_audio_reset(&a);dw_audio_reset(&b);
 /* With no effects, routing only music is bit-identical to the full mix. */
 a.fx_gain=0;dw_audio_mix(&a,&g,out,2048,&gain,1,0,1);
 dw_audio_mix(&b,&g,reference,2048,&other,1,1,1);
 assert(nonzero(out,2048)&&!memcmp(out,reference,sizeof out));
 /* Turning CH2 off silences music while retaining the effect bus. */
 dw_audio_reset(&a);a.music_gain=0;
 dw_audio_mix(&a,&g,out,2048,&gain,1,1,0);assert(!nonzero(out,2048));
 g.event=2;g.event_serial=1;dw_audio_mix(&a,&g,out,2048,&gain,1,1,0);assert(nonzero(out,2048));
 /* Turning CH1 off prevents a loud explosion from reaching the output. */
 dw_audio_reset(&a);a.fx_gain=a.music_gain=0;
 dw_audio_mix(&a,&g,out,2048,&gain,1,0,0);assert(!nonzero(out,2048));
 gain=0;dw_audio_mix(&a,&g,out,2048,&gain,0,1,1);assert(!nonzero(out,2048));gain=1;
 /* A new match resets the original score reproducibly; combat RNG is untouched. */
 g.event=g.event_serial=0;dw_audio_reset(&a);dw_audio_reset(&b);
 int musical_blocks=0;for(int block=0;block<900;block++){
  dw_audio_mix(&a,&g,out,735,&gain,1,1,1);dw_audio_mix(&b,&g,reference,735,&other,1,1,1);
  assert(!memcmp(out,reference,1470*sizeof(int16_t)));musical_blocks+=nonzero(out,735);
 }
 assert(musical_blocks>450&&g.rng==42&&sizeof a<1024);
 /* The finale is suspenseful until the guest has won, then celebrates. */
 g.state=DW_FINALE;g.finale_assisted=1;g.finale_tick=100;a.step_left=0;
 dw_audio_mix(&a,&g,out,735,&gain,1,1,1);assert(a.victory==0);
 g.finale_tick=DW_CELEBRATE_TICK;a.step_left=0;
 dw_audio_mix(&a,&g,out,735,&gain,1,1,1);assert(a.victory==1);
 /* Movement, jump and grenade-fuse cues do not rely on a new weapon event. */
 g.state=DW_AIM;g.finale_tick=0;dw_audio_reset(&a);a.music_gain=0;
 g.old_buttons=DW_JUMP;g.unit[0].vy=-4;
 dw_audio_mix(&a,&g,out,735,&gain,1,1,0);assert(nonzero(out,735));
 dw_audio_reset(&a);a.music_gain=0;g.old_buttons=0;g.shot[0].live=1;g.shot[0].kind=1;g.shot[0].life=30;
 dw_audio_mix(&a,&g,out,735,&gain,1,1,0);assert(nonzero(out,735));
 dw_audio_reset(&a);dw_audio_set_rate(&a,48000);g.old_buttons=0;g.shot[0].live=0;
 dw_audio_mix(&a,&g,out,800,&gain,1,1,1);assert(a.rate==48000&&a.step_left==48000u*60u/224u-800u);
 printf("DJ Wars audio: original score, 15 cues, motion/fuse cues, independent buses, master mute, finale music, bounds and independent RNG passed (%zu bytes).\n",sizeof a);
}
