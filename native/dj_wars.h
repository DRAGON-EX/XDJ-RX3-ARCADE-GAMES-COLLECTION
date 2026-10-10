#ifndef RX3_DJ_WARS_NATIVE_H
#define RX3_DJ_WARS_NATIVE_H
#include "../Games/Native/DJ Wars/game.h"
#include "../Games/Native/DJ Wars/audio.h"
#define DW_NATIVE
#include "../Games/Native/DJ Wars/render.h"
#undef DW_NATIVE
static struct dw_game *dj_wars;
static struct dw_audio_state dj_wars_audio;
/* The finale reads only its packed track range, using an independent descriptor.
 * A 1 KB buffer keeps the paused menu playlist's cursor completely untouched. */
static const char*dj_wars_fireworks_path="/tmp/rx3-launcher/payload/menu-music.pcm";
static struct {int fd,checked;int16_t data[512],sample;unsigned at,count,repeat,position,start,end;} dj_wars_fireworks={.fd=-1};
static void dj_wars_fireworks_close(void){
 if(dj_wars_fireworks.fd>=0)close(dj_wars_fireworks.fd);
 dj_wars_fireworks.fd=-1;dj_wars_fireworks.at=dj_wars_fireworks.count=dj_wars_fireworks.repeat=0;
}
static void dj_wars_fireworks_reset(void){dj_wars_fireworks_close();dj_wars_fireworks.checked=0;dj_wars_fireworks.position=dj_wars_fireworks.start=dj_wars_fireworks.end=0;}
static int dj_wars_fireworks_ready(void){return dj_wars&&dj_wars->state==DW_FINALE&&(!dj_wars->finale_assisted||dj_wars->finale_tick>=DW_CELEBRATE_TICK);}
static void dj_wars_fireworks_start(void){
 if(dj_wars_fireworks.checked||!dj_wars_fireworks_ready())return;
 dj_wars_fireworks.checked=1;
 for(unsigned i=0;i<music_count;i++)if(music_tracks[i].locked==3&&music_tracks[i].end>music_tracks[i].start){
  int fd=open(dj_wars_fireworks_path,0);if(fd<0)return;
  long size=lseek(fd,0,2);if(size<0||(unsigned long)size<music_tracks[i].end||lseek(fd,(long)music_tracks[i].start,0)<0){close(fd);return;}
  dj_wars_fireworks.fd=fd;dj_wars_fireworks.position=dj_wars_fireworks.start=music_tracks[i].start;dj_wars_fireworks.end=music_tracks[i].end;
  dj_wars_audio.music_gain=0;return;
 }
}
static int dj_wars_fireworks_sample(void){
 if(dj_wars_fireworks.fd<0)return 0;
 if(dj_wars_fireworks.repeat){dj_wars_fireworks.repeat=0;return dj_wars_fireworks.sample;}
 if(dj_wars_fireworks.position>=dj_wars_fireworks.end){
  if(lseek(dj_wars_fireworks.fd,(long)dj_wars_fireworks.start,0)<0){dj_wars_fireworks_close();return 0;}
  dj_wars_fireworks.position=dj_wars_fireworks.start;dj_wars_fireworks.at=dj_wars_fireworks.count=0;
 }
 if(dj_wars_fireworks.at>=dj_wars_fireworks.count){
  unsigned bytes=dj_wars_fireworks.end-dj_wars_fireworks.position;if(bytes>sizeof dj_wars_fireworks.data)bytes=sizeof dj_wars_fireworks.data;
  long n=read(dj_wars_fireworks.fd,dj_wars_fireworks.data,bytes);if(n<=0||n%2){dj_wars_fireworks_close();return 0;}
  dj_wars_fireworks.at=0;dj_wars_fireworks.count=(unsigned)n/2;
 }
 dj_wars_fireworks.sample=dj_wars_fireworks.data[dj_wars_fireworks.at++];dj_wars_fireworks.position+=2;dj_wars_fireworks.repeat=1;
 return dj_wars_fireworks.sample;
}
static void dj_wars_audio_block(int16_t*out,unsigned frames,float*gain,float master,float fx,float music){
 dj_wars_fireworks_start();int fireworks=dj_wars_fireworks.fd>=0;
 dw_audio_mix(&dj_wars_audio,dj_wars,out,frames,gain,master,fx,fireworks?0:music);
 if(fireworks)for(unsigned i=0;i<frames;i++){
  int song=(int)(dj_wars_fireworks_sample()**gain*music);
  for(unsigned c=0;c<2;c++)out[2*i+c]=(int16_t)dw_audio_limit(out[2*i+c]+song);
 }
}

static unsigned dj_wars_keys,dj_wars_pulse;
static uint64_t dj_wars_next_step;static unsigned dj_wars_clock_resets;
static void dj_wars_result(void){if(session_finale_ready()&&dj_wars&&dj_wars->state==DW_OVER&&dj_wars->winner==0)session_dj_wars_won=1;}
static void dj_wars_release(void){dj_wars_result();dj_wars_fireworks_reset();free(dj_wars);dj_wars=0;free(dw_back);dw_back=0;dj_wars_keys=dj_wars_pulse=0;dj_wars_next_step=0;dw_audio_reset(&dj_wars_audio);}
/* Switch directly from the last fireworks frame into credits. Releasing the
 * match makes the transition once-only and prevents old game audio returning. */
static int dj_wars_begin_credits(uint64_t now){
 if(!dj_wars||dj_wars->state!=DW_OVER||dj_wars->winner!=0||!session_finale_ready())return 0;
 dj_wars_result();dj_wars_release();menu=1;paused=0;direction=0;help_page=5;help_offset=0;
 credits_preview_ms=-1;credits_end();credits_begin(now);return 1;
}
static int dj_wars_reset(void){
 if(width!=1280||height!=800)return 0;
 if(!dj_wars)dj_wars=malloc(sizeof(*dj_wars));
 if(!dw_back)dw_back=malloc(1280*800*sizeof(*dw_back));
 if(!dj_wars||!dw_back){dj_wars_release();return 0;}
 dj_wars_fireworks_reset();
 /* RX3 measured output clock is 44.1 kHz despite nominal 48 kHz ALSA: 735 frames/tick. */
 dw_reset(dj_wars,0x5833u+tick);dw_audio_reset(&dj_wars_audio);dw_audio_set_rate(&dj_wars_audio,44100);dw_cached_version=0;dw_minimap_version=0;dw_cached_theme=-1;
 dw_camera_x=(int)dj_wars->unit[0].x-440;dw_camera_y=(int)dj_wars->unit[0].y-410;
 dw_overview=dw_manual=0;dw_camera_active=dw_camera_state=-1;dw_camera_limit();
 dj_wars_keys=dj_wars_pulse=0;dj_wars_next_step=0;return 1;
}
static unsigned dj_wars_buttons(unsigned held,unsigned fire){
 return dj_wars_keys|dj_wars_pulse|((held&1)?DW_UP:0)|((held&2)?DW_DOWN:0)|((held&4)?DW_LEFT:0)|((held&8)?DW_RIGHT:0)|(fire?DW_FIRE:0);
}
/* A render may take longer than one 60 Hz step; advance real game time
 * with bounded catch-up rather than slowing movement and the turn clock. */
static unsigned dj_wars_advance(uint64_t now,int headless){
 if(!dj_wars)return 0;
 if(headless){dw_tick(dj_wars,dj_wars_buttons(nes_held,nes_a));dj_wars_pulse=0;dj_wars_result();return 1;}
 if(!dj_wars_next_step)dj_wars_next_step=now;
 unsigned count=0;
 while(now>=dj_wars_next_step&&count<6){
  dw_tick(dj_wars,dj_wars_buttons(nes_held,nes_a));dj_wars_pulse=0;
  dj_wars_next_step+=16666667ULL;count++;
 }
 if(now>=dj_wars_next_step){dj_wars_next_step=now+16666667ULL;dj_wars_clock_resets++;}
 dj_wars_result();return count;
}
#endif
