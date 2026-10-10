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
int test_pcm_drop(void*p){(void)p;return 0;}int test_pcm_prepare(void*p){(void)p;return 0;}int test_pcm_delay(void*p,long*d){(void)p;*d=0;return 0;}
static unsigned written;static int playlist_test;
static unsigned drains;int test_pcm_drain(void*p){(void)p;drains++;return 0;}
long test_pcm_writei(void*p,const void*b,unsigned long n){(void)p;if(playlist_test==7){const int16_t*s=b;if(written<800){assert(n==400&&s[0]==20&&s[799]==20);}else{assert(n==800&&s[0]==0&&s[1599]==0&&drains==1&&music_credits_complete);__atomic_store_n(&menu_sound_stop,1,__ATOMIC_RELAXED);}}if(playlist_test==5||playlist_test==6){const int16_t*s=b;int expected=playlist_test==6?20:(written?30:10);assert(n==400&&s[0]==expected&&s[799]==expected);if(written>=400)__atomic_store_n(&menu_sound_stop,1,__ATOMIC_RELAXED);}if(playlist_test==3){const int16_t*s=b;assert(n==400&&s[0]==10&&s[799]==10);__atomic_store_n(&menu_sound_stop,1,__ATOMIC_RELAXED);}if(playlist_test==4){const int16_t*s=b;assert(n==800&&s[0]==0&&s[1599]==0);__atomic_store_n(&menu_sound_stop,1,__ATOMIC_RELAXED);}if(playlist_test==1){const int16_t*s=b;assert(n==800&&s[0]==5&&s[799]==5&&s[800]==10&&s[1599]==10);if(__atomic_load_n(&written,__ATOMIC_RELAXED)>=1600)__atomic_store_n(&menu_sound_stop,1,__ATOMIC_RELAXED);}rx_timespec t={0,1000000};nanosleep(&t,0);__atomic_fetch_add(&written,(unsigned)n,__ATOMIC_RELAXED);return n;}
int test_pcm_recover(void*p,int e,int s){(void)p;(void)e;(void)s;return 0;}
int main(void){
 menu_sound_frames=48000;menu_sound=malloc(menu_sound_frames*4);for(unsigned i=0;i<menu_sound_frames*2;i++)menu_sound[i]=100;
 menu=1;master_target=0.5f;menu_sound_trigger();menu_sound_service((void*)1);assert(menu_sound_active);
 /* Deliberately do no UI servicing while the audio worker feeds PCM. */
 rx_timespec t={0,150000000};nanosleep(&t,0);assert(__atomic_load_n(&written,__ATOMIC_RELAXED)==48000);menu_sound_service((void*)1);assert(!menu_sound_active);
 menu_sound_trigger();menu_sound_service((void*)1);menu=0;menu_sound_service((void*)1);assert(!menu_sound_active);free(menu_sound);menu_sound=0;menu_sound_frames=menu_sound_pos=0;
 int16_t song[400];for(unsigned i=0;i<400;i++)song[i]=i<200?10:20;int fd=open("work/test-playlist.pcm",1|64|512,0600);assert(fd>=0&&write(fd,song,sizeof(song))==sizeof(song));close(fd);menu_music_fd=open("work/test-playlist.pcm",0);assert(menu_music_fd>=0);menu_music_frames=800;menu_music_pos=0;playlist_test=1;written=0;menu_sound_stop=0;menu_sound_gain=500;menu_sound_worker((void*)1);assert(written==2400);
 playlist_test=3;written=0;menu_sound_stop=0;music_seek_request=400;menu_music_pos=0;music_level=100;menu_sound_worker((void*)1);assert(written==400&&music_position==800);
 playlist_test=4;written=0;menu_sound_stop=0;music_paused=1;menu_music_pos=200;menu_sound_worker((void*)1);assert(written==800&&menu_music_pos==200);music_paused=0;close(menu_music_fd);menu_music_fd=-1;
 int16_t gated[600];for(unsigned i=0;i<600;i++)gated[i]=i<200?10:i<400?20:30;
 fd=open("work/test-gated-playlist.pcm",1|64|512,0600);assert(fd>=0&&write(fd,gated,sizeof(gated))==sizeof(gated));close(fd);menu_music_fd=open("work/test-gated-playlist.pcm",0);assert(menu_music_fd>=0);menu_music_frames=1200;music_count=3;music_tracks[0]=(struct music_track){0,400,"FIRST","",0};music_tracks[1]=(struct music_track){400,800,"SECRET","",2};music_tracks[2]=(struct music_track){800,1200,"LAST","",0};music_secret_unlocked=0;menu_sound_gain=1000;music_paused=0;
 for(int mode=5;mode<=6;mode++){playlist_test=mode;menu_music_pos=0;lseek(menu_music_fd,0,0);music_seek_request=~0u;music_credits_mode=mode==6;written=0;menu_sound_stop=0;menu_sound_worker((void*)1);assert(written==800);}
 playlist_test=7;menu_music_pos=0;lseek(menu_music_fd,0,0);music_seek_request=~0u;music_credits_mode=1;music_credits_epoch=1;music_credits_total=800;music_credits_position=music_credits_complete=0;written=drains=0;menu_sound_stop=0;menu_sound_worker((void*)1);assert(written==1600&&music_credits_complete&&music_credits_position==800&&drains==1);
 close(menu_music_fd);music_credits_mode=0;
 printf("Menu sound continues during UI stalls and joins before game audio\n");return 0;
}
