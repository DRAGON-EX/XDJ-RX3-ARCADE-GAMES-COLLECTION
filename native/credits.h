#ifndef RX3_CREDITS_H
#define RX3_CREDITS_H
#define cr_rect rect
#define cr_text text
#include "../shared/credits_scene.h"
#include "../shared/credits_confirm.h"
#undef cr_rect
#undef cr_text
static int credits_confirm_step,credits_confirm_return;
static void credits_request(void){credits_confirm_return=help_page;credits_confirm_step=0;help_offset=0;help_page=session_dj_wars_won?6:8;}
static int credits_active,credits_preview_ms=-1,credits_completed,credits_has_sound;
static unsigned credits_elapsed,credits_saved_position,credits_saved_pause;
static uint64_t credits_last,credits_watch_ns;
static void credits_begin(uint64_t now){
 credits_active=1;credits_completed=0;credits_elapsed=0;credits_watch_ns=0;credits_last=now;
 credits_saved_position=__atomic_load_n(&music_position,__ATOMIC_RELAXED);credits_saved_pause=__atomic_load_n(&music_paused,__ATOMIC_RELAXED);
 credits_has_sound=0;
 __atomic_store_n(&music_credits_position,0,__ATOMIC_RELAXED);__atomic_store_n(&music_credits_total,0,__ATOMIC_RELAXED);__atomic_store_n(&music_credits_complete,0,__ATOMIC_RELAXED);
 for(unsigned i=0;i<music_count;i++)if(music_tracks[i].locked==2&&music_tracks[i].end>music_tracks[i].start){music_seek(music_tracks[i].start);__atomic_store_n(&music_credits_total,2*(music_tracks[i].end-music_tracks[i].start),__ATOMIC_RELAXED);__atomic_store_n(&music_paused,0,__ATOMIC_RELAXED);credits_has_sound=1;break;}
 __atomic_add_fetch(&music_credits_epoch,1,__ATOMIC_RELEASE);__atomic_store_n(&music_credits_mode,1,__ATOMIC_RELEASE);
}
static void credits_end(void){
 if(!credits_active)return;credits_active=0;
 music_seek(credits_saved_position);__atomic_store_n(&music_credits_mode,0,__ATOMIC_RELEASE);__atomic_store_n(&music_paused,credits_saved_pause,__ATOMIC_RELAXED);
}
static void credits_update(uint64_t now,int audio_available){
 if((help_page==5||help_page==6)&&!session_dj_wars_won)help_page=8;
 if(help_page!=5||!menu){credits_end();return;}
 if(!credits_active)credits_begin(now);
 uint64_t step=now>=credits_last?now-credits_last:0;credits_last=now;
 /* Long stalls never count as watched animation. */
 if(step>250000000ULL)step=250000000ULL;
  if(credits_has_sound&&audio_available&&!credits_completed){
  unsigned pos=__atomic_load_n(&music_credits_position,__ATOMIC_ACQUIRE),total=__atomic_load_n(&music_credits_total,__ATOMIC_RELAXED);
  credits_elapsed=total?(unsigned)((uint64_t)pos*CREDITS_DURATION_MS/total):0;
  if(__atomic_load_n(&music_credits_complete,__ATOMIC_ACQUIRE)){credits_elapsed=CREDITS_DURATION_MS;credits_completed=1;music_unlock_secret();credits_watch_ns=(uint64_t)CREDITS_DURATION_MS*1000000;}
 }else{
  credits_watch_ns+=step;if(credits_watch_ns>=(uint64_t)(CREDITS_DURATION_MS+20000)*1000000)credits_watch_ns-=10000000000ULL;credits_elapsed=(unsigned)(credits_watch_ns/1000000);
  if(credits_elapsed>=CREDITS_DURATION_MS&&!credits_completed){credits_completed=1;music_unlock_secret();}
 }
}
static void draw_credits(void){unsigned t=credits_preview_ms>=0?(unsigned)credits_preview_ms:credits_elapsed;credits_render(t,__atomic_load_n(&music_secret_unlocked,__ATOMIC_ACQUIRE));}
#endif
