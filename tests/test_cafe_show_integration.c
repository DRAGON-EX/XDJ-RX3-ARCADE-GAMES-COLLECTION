#define main rx3_program_main
#define snd_pcm_writei test_pcm_writei
#define snd_pcm_recover test_pcm_recover
#define snd_pcm_drop test_pcm_drop
#define snd_pcm_prepare test_pcm_prepare
#define snd_pcm_delay test_pcm_delay
#define snd_pcm_drain test_pcm_drain
#include "../native/arcade.c"
#undef main
#include <assert.h>
extern int unlink(const char*),truncate(const char*,long);

enum {NATURAL,TOGGLE_STOP,WORKER_STOP,MISSING,PARTIAL_WRITE,FATAL_WRITE,DRAIN_ERROR,TRUNCATED};
static int scenario;
static unsigned writes,show_frames,cafe_frames,drops,prepares,drains,recoveries;
static const char*show_path="work/test-cafe-show-integration.pcm";
static const char*playlist_path="work/test-cafe-show-playlist.pcm";
static const char*voice_path="work/test-cafe-show-voice.pcm";
static void pcm_file(const char*path,int16_t sample,unsigned samples){
 int fd=open(path,1|64|512,0600);assert(fd>=0);int16_t data[256];for(unsigned i=0;i<256;i++)data[i]=sample;
 while(samples){unsigned n=samples>256?256:samples;assert(write(fd,data,n*2)==n*2);samples-=n;}assert(!close(fd));
}
int test_pcm_drop(void*p){(void)p;drops++;return 0;}
int test_pcm_prepare(void*p){(void)p;prepares++;return 0;}
int test_pcm_delay(void*p,long*frames){(void)p;*frames=cafe_show_audio_active()?441:0;return 0;}
int test_pcm_recover(void*p,int error,int silent){(void)p;(void)error;(void)silent;recoveries++;return scenario==FATAL_WRITE?-1:0;}
int test_pcm_drain(void*p){
 (void)p;drains++;assert(cafe_show_audio_active());assert(show_frames==44100);
 assert(menu_music_pos==600&&music_position==600&&cat_position==0);
 assert(cafe_show_audio_ms()==990);
 return scenario==DRAIN_ERROR?-1:0;
}
long test_pcm_writei(void*p,const void*data,unsigned long frames){
 (void)p;writes++;assert(writes<2000);const int16_t*samples=data;
 if(cafe_show_audio_active()){
  assert(menu_music_pos==600&&music_position==600&&lseek(menu_music_fd,0,1)==600);
  assert(cat_position==0&&cat_wait==100000); /* Neither voices nor playlist advance. */
  for(unsigned long i=0;i<frames*2;i++)assert(samples[i]==2000); /* .8 master * .5 CH2 * .5 level, exactly once. */
  unsigned expected_ms=show_frames>441?(show_frames-441)*1000/44100:0;
  /* Commit occurs after the whole block, not after each short ALSA write. */
  if(scenario!=PARTIAL_WRITE)assert(cafe_show_audio_ms()==expected_ms);
  if(scenario==FATAL_WRITE)return -32;
  if(scenario==PARTIAL_WRITE&&writes==1)return -32;
  unsigned accepted=(unsigned)frames;if(scenario==PARTIAL_WRITE&&accepted>173)accepted=173;
  show_frames+=accepted;
  if(scenario==TOGGLE_STOP)__atomic_store_n(&cafe_show_requested,0,__ATOMIC_RELEASE);
  if(scenario==WORKER_STOP)__atomic_store_n(&menu_sound_stop,1,__ATOMIC_RELAXED);
  if(scenario==TRUNCATED&&show_frames>=800)assert(!truncate(show_path,0));
  return accepted;
 }
 assert(!cafe_show_requested&&music_cafe_mode&&music_cafe_track==1);
 assert(menu_music_pos==600&&music_position==600);
 for(unsigned long i=0;i<frames*2;i++)assert(samples[i]==1400); /* Pixel Plains plus CH1 cat voice resumes. */
 cafe_frames+=(unsigned)frames;__atomic_store_n(&menu_sound_stop,1,__ATOMIC_RELAXED);return (long)frames;
}
static void reset_case(int mode){
 scenario=mode;writes=show_frames=cafe_frames=drops=prepares=drains=recoveries=0;
 cafe_show_audio_stop();cat_audio_close();if(menu_music_fd>=0)close(menu_music_fd);
 pcm_file(show_path,10000,22050);pcm_file(playlist_path,4000,1000);pcm_file(voice_path,6000,1024);
 menu_music_fd=open(playlist_path,0);assert(menu_music_fd>=0);assert(lseek(menu_music_fd,600,0)==600);
 menu_music_frames=2000;menu_music_pos=music_position=600;music_seek_request=~0u;music_paused=0;music_level=50;
 music_count=2;music_tracks[0]=(struct music_track){0,400,"PLAYLIST","",0};music_tracks[1]=(struct music_track){400,2000,"PIXEL PLAINS","",0};
 music_cafe_mode=1;music_cafe_track=1;music_cafe_saved_position=222;music_cafe_saved_pause=1;
 menu_sound=0;menu_sound_frames=menu_sound_pos=menu_sound_stop=menu_sound_done=menu_sound_kick=0;
 menu_sound_gain=800;menu_music_channel=50;menu_fx_channel=25;music_credits_mode=0;
 cafe_audio_enabled=1;cafe_audio_greeting=0;cat_was_cafe=1;cat_wait=100000;cat_purr=0;cat_stream=(struct game_pcm_stream){0};
 cat_fd=open(voice_path,0);assert(cat_fd>=0);cat_position=0;cat_end=2048;
 cafe_show_path=mode==MISSING?"work/no-such-cafe-show-song.pcm":show_path;cafe_show_requested=1;
}
int main(void){
 for(int mode=NATURAL;mode<=TRUNCATED;mode++){
  reset_case(mode);printf("Cafe show integration case %d\n",mode);fflush(0);menu_sound_worker((void*)1);
  assert(menu_sound_done&&!cafe_show_requested&&!cafe_show_audio_active()&&cafe_show_audio.fd<0);
  assert(music_cafe_mode&&music_cafe_saved_position==222&&music_cafe_saved_pause==1);
  if(mode==NATURAL||mode==PARTIAL_WRITE){assert(show_frames==44100&&cafe_frames==800&&drains==1);assert(drops==1&&prepares==2);}
  if(mode==TOGGLE_STOP){assert(show_frames==800&&cafe_frames==800&&!drains&&drops==2);}
  if(mode==WORKER_STOP){assert(show_frames==800&&!cafe_frames&&!drains&&menu_music_pos==600&&cat_position==0&&drops==2);}
  if(mode==MISSING){assert(!show_frames&&cafe_frames==800&&!drains&&!drops);}
  if(mode==PARTIAL_WRITE)assert(recoveries==1);
  if(mode==FATAL_WRITE)assert(!show_frames&&!cafe_frames&&recoveries==1);
  if(mode==DRAIN_ERROR)assert(show_frames==44100&&!cafe_frames&&drains==1&&menu_music_pos==600);
  if(mode==TRUNCATED)assert(show_frames==800&&cafe_frames==800&&!drains&&drops==2);
 }
 cat_audio_close();close(menu_music_fd);menu_music_fd=-1;
 assert(!unlink(show_path)&&!unlink(playlist_path)&&!unlink(voice_path));
 printf("Cafe show integration: whole-worker override, audio clock, single gains, frozen playlist/voices, stop/end/failure restoration and ALSA short-write recovery passed.\n");
}
