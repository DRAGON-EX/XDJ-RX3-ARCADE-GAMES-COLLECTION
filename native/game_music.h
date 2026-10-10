#ifndef RX3_GAME_MUSIC_H
#define RX3_GAME_MUSIC_H
/* The menu worker and game loop hand off one playlist descriptor; never two PCM writers. */
struct game_pcm_stream { int16_t data[512],sample; unsigned at,count,repeat; };
static struct game_pcm_stream game_song,game_fanfare;
static int rescue_pcm_fd=-1;static unsigned rescue_pcm_left;static int rescue_pcm_started;
static float game_music_gain;
static int stream_sample(struct game_pcm_stream*s,int fd,unsigned*position,unsigned end){
 if(!s->repeat){
  if(*position>=end)return 0;
  if(s->at>=s->count){unsigned bytes=end-*position;if(bytes>sizeof(s->data))bytes=sizeof(s->data);long n=read(fd,s->data,bytes);s->at=0;s->count=n>0?(unsigned)n/2:0;if(!s->count)return 0;}
  s->sample=s->data[s->at++];*position+=2;s->repeat=1;
 }else s->repeat=0;
 return s->sample;
}
#include "cat_audio.h"
static void game_music_handoff(void){game_song=(struct game_pcm_stream){0};if(menu_music_fd>=0)lseek(menu_music_fd,(long)menu_music_pos,0);}
static void game_mix(int16_t*samples,unsigned frames,int rescuing){
 float fx=(gain_seen[0]?gain_percent[0]:100)/100.f;
 float target=master_target*(gain_seen[1]?gain_percent[1]:100)/100.f*__atomic_load_n(&music_level,__ATOMIC_RELAXED)/100.f;
 if(rescuing&&(!rescue_pcm_started||rescue.sample==0)){rescue_pcm_started=1;rescue_pcm_fd=open("/tmp/rx3-launcher/payload/rescue-fanfare.pcm",0);game_fanfare=(struct game_pcm_stream){0};rescue_pcm_left=0;if(rescue_pcm_fd>=0){long size=lseek(rescue_pcm_fd,0,2);if(size>0&&size<2000000){rescue_pcm_left=(unsigned)size;lseek(rescue_pcm_fd,0,0);}else{close(rescue_pcm_fd);rescue_pcm_fd=-1;}}}
 if(!rescuing&&rescue_pcm_started){if(rescue_pcm_fd>=0)close(rescue_pcm_fd);rescue_pcm_fd=-1;rescue_pcm_started=0;}
 static unsigned fanfare_position;
 if(rescuing&&rescue.sample==0)fanfare_position=0;
 if(rescuing&&rescue.sample<771750&&rescue.sample+frames>=771750)cat_audio_start(rescue.cat);
 for(unsigned i=0;i<frames;i++){
  int music=(int)(cat_audio_sample(0)*master_target*fx);game_music_gain+=(target-game_music_gain)*.002f;
  if(rescuing){if(rescue_pcm_fd>=0)music+=(int)(stream_sample(&game_fanfare,rescue_pcm_fd,&fanfare_position,rescue_pcm_left)*master_target*fx);}
  else if(menu_music_fd>=0&&!__atomic_load_n(&music_paused,__ATOMIC_RELAXED)){
   unsigned end=menu_music_frames,next=game_song.repeat?menu_music_pos:music_audio_next(menu_music_pos,menu_music_frames,&end);
   if(next!=menu_music_pos){menu_music_pos=next;game_music_handoff();}
   music+=(int)(stream_sample(&game_song,menu_music_fd,&menu_music_pos,end)*game_music_gain);
  }
  for(unsigned c=0;c<2;c++){int v=(rescuing?0:(int)(samples[i*2+c]*fx))+music;if(v>32767)v=32767;if(v<-32768)v=-32768;samples[i*2+c]=(int16_t)v;}
 }
 if(rescuing)rescue.sample+=frames;
 else __atomic_store_n(&music_position,menu_music_pos,__ATOMIC_RELAXED);
}
#endif
