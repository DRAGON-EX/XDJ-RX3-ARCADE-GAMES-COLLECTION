#define main rx3_program_main
#include "../native/arcade.c"
#undef main
#include <assert.h>
extern int setenv(const char*,const char*,int),unlink(const char*);
int main(void){
 /* Only an actual winning DJ Wars result opens the credits. */
 menu=1;session_dj_wars_won=0;credits_request();assert(help_page==8);
 help_page=5;credits_update(1,0);assert(help_page==8&&!credits_active);
 music_unlock_secret();assert(!music_secret_unlocked);
 dj_wars=malloc(sizeof(*dj_wars));dj_wars->state=DW_OVER;dj_wars->winner=1;dj_wars_result();assert(!session_dj_wars_won);
 dj_wars->winner=2;dj_wars_result();assert(!session_dj_wars_won);
 dj_wars->winner=0;dj_wars_result();assert(!session_dj_wars_won);for(int i=0;i<6;i++)session_save_cat(i);assert(session_rescued_cats==15);dj_wars_result();assert(session_dj_wars_won);
 dj_wars_release();credits_request();assert(help_page==6);
 music_count=3;music_tracks[0]=(struct music_track){0,100,"FIRST","ARTIST",0};music_tracks[1]=(struct music_track){100,200,"SECRET","ARTIST",2};music_tracks[2]=(struct music_track){200,300,"LAST","ARTIST",0};music_secret_unlocked=0;
 unsigned end;assert(music_audio_next(98,300,&end)==98&&end==100);assert(music_audio_next(100,300,&end)==200&&end==300);assert(music_audio_next(140,300,&end)==200);assert(music_audio_next(300,300,&end)==0);
 music_seek_request=42;music_choose(1);assert(music_seek_request==42);music_adjacent(0,1);assert(music_selected==2);
 help_page=5;menu=1;music_position=24;music_paused=1;credits_update(1000000000ULL,0);assert(credits_active&&music_credits_mode&&music_seek_request==100&&!music_paused);
 assert(music_audio_next(0,300,&end)==100&&end==200);assert(music_audio_next(200,300,&end)==100); /* credits loop only the secret */
 credits_update(1000000000000ULL,0);assert(credits_elapsed==250&&!music_secret_unlocked); /* a clock jump isn't a full viewing */
 help_page=0;credits_update(1000000000001ULL,0);assert(!credits_active&&!music_credits_mode&&music_seek_request==24&&music_paused&&!music_secret_unlocked);
 help_page=5;uint64_t now=2000000000000ULL;credits_update(now,0);for(unsigned i=0;i<(CREDITS_DURATION_MS-1)/250;i++){now+=250000000;credits_update(now,0);}assert(!credits_completed&&!music_secret_unlocked);now+=250000000;credits_update(now,0);assert(credits_completed&&music_secret_unlocked);
 help_page=0;credits_update(now,0);music_secret_unlocked=0;help_page=5;credits_update(now,1);now+=1000000000000ULL;credits_update(now,1);assert(credits_elapsed==0&&!credits_completed&&!music_secret_unlocked);
 music_credits_position=music_credits_total/2;credits_update(++now,1);assert(credits_elapsed==CREDITS_DURATION_MS/2&&!credits_completed);
 music_credits_position=music_credits_total;credits_update(++now,1);assert(!credits_completed);music_credits_complete=1;credits_update(++now,1);assert(credits_completed&&music_secret_unlocked);
 assert(music_secret_unlocked);help_page=0;credits_update(now,0);music_choose(1);assert(music_seek_request==100);assert(music_audio_next(100,300,&end)==100&&end==200);
 /* Old packages without the secret PCM never play an empty locked track. */
 music_tracks[1].end=100;music_seek_request=42;music_choose(1);assert(music_seek_request==42);
 session_dj_wars_won=music_secret_unlocked=0;credits_request();assert(help_page==8);music_unlock_secret();assert(!music_secret_unlocked);
 printf("Credits victory gate, abort, clock stall, secret-only loop, PCM boundaries and volatile unlock passed\n");return 0;
}
