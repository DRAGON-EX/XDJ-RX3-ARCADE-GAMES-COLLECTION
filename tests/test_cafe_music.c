#define main rx3_program_main
#include "../native/arcade.c"
#undef main
#include <assert.h>
int main(void){
 music_count=4;music_tracks[0]=(struct music_track){0,400,"PLAYLIST","",0};music_tracks[1]=(struct music_track){400,800,"PIXEL PLAINS","JUNIPERSONA",0};music_tracks[2]=(struct music_track){800,1200,"CREDITS","",2};music_tracks[3]=(struct music_track){1200,1600,"FIREWORKS","",3};
 session_rescued_cats=15;session_dj_wars_won=1;menu=1;unsigned end=0;
 for(unsigned paused=0;paused<=1;paused++){
  help_page=0;music_position=music_seek_request=120;music_paused=paused;open_cafe();
  assert(help_page==11&&music_cafe_mode&&music_seek_request==400&&!music_paused);
  assert(music_cafe_saved_position==120&&music_cafe_saved_pause==paused);
  assert(music_audio_next(400,1600,&end)==400&&end==800);
  assert(music_audio_next(799,1600,&end)==799&&end==800);
  assert(music_audio_next(800,1600,&end)==400&&end==800); /* only Pixel Plains loops */
  music_seek_request=600;music_position=600;open_cafe();assert(music_seek_request==400&&music_cafe_saved_position==120);
  music_cafe_end();assert(!music_cafe_mode&&music_seek_request==120&&music_paused==paused);
 }
 /* Credits-to-café saves the pre-credits playlist state, never the secret song. */
 for(unsigned paused=0;paused<=1;paused++){
  menu=1;help_page=5;music_position=42;music_paused=paused;credits_begin(1000);
  assert(credits_saved_position==42&&credits_saved_pause==paused);
  music_position=999;credits_completed=0;assert(!credits_to_cafe());
  credits_completed=1;assert(credits_to_cafe());
  assert(!credits_active&&!music_credits_mode&&music_cafe_mode&&help_page==11&&music_seek_request==400&&!music_paused);
  assert(music_cafe_saved_position==42&&music_cafe_saved_pause==paused);
  /* Even an in-flight restored-position command is clamped to the café range. */
  assert(music_audio_next(42,1600,&end)==400&&end==800);
  music_cafe_end();assert(music_seek_request==42&&music_paused==paused&&!music_cafe_mode);
 }
 /* Startup/preview opens before the catalog loads; service initializes once available. */
 menu=1;help_page=11;music_position=18;music_paused=1;menu_sound_service(0);
 assert(music_cafe_mode&&music_seek_request==400&&!music_paused);help_page=0;menu_sound_service(0);
 assert(!music_cafe_mode&&music_seek_request==18&&music_paused);
 music_count=1;music_seek_request=22;assert(!music_cafe_begin(22,0)&&!music_cafe_mode&&music_seek_request==22);
 printf("Cafe music: every-entry restart, single-track loop, saved pause/position, credits handoff routing and startup fallback passed.\n");
}
