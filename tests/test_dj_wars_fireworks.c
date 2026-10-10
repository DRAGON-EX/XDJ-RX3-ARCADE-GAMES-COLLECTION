#define main rx3_program_main
#include "../native/arcade.c"
#undef main
#include <assert.h>
#include <string.h>
int main(void){
 const int16_t packed[]={9000,9000,1000,2000,3000,4000,9000,9000};
 int fd=open("work/test-fireworks-playlist.pcm",1|64|512,0600);assert(fd>=0&&write(fd,packed,sizeof packed)==sizeof packed);close(fd);
 dj_wars_fireworks_path="work/test-fireworks-playlist.pcm";
 music_count=3;music_tracks[0]=(struct music_track){0,4,"FIRST","",0};music_tracks[1]=(struct music_track){4,12,"PIXEL QUEST","",3};music_tracks[2]=(struct music_track){12,16,"LAST","",0};
 session_dj_wars_won=music_secret_unlocked=0;music_credits_mode=0;
 unsigned end=0;assert(music_track_locked(1));assert(music_audio_next(4,16,&end)==12&&end==16);
 music_seek_request=42;music_choose(1);assert(music_seek_request==42);
 music_credits_mode=1;assert(!music_audio_allowed(1));music_credits_mode=0;
 menu_music_fd=open(dj_wars_fireworks_path,0);assert(menu_music_fd>=0);lseek(menu_music_fd,2,0);menu_music_pos=music_position=2;
 assert(dj_wars_reset());session_rescued_cats=15;
 int16_t out[32];float master=1;dj_wars_audio.fx_gain=0;
 dj_wars->state=DW_FINALE;dj_wars->finale_assisted=1;dj_wars->finale_tick=DW_CELEBRATE_TICK-1;
 dj_wars_audio_block(out,16,&master,1,0,1);assert(dj_wars_fireworks.fd==-1&&!dj_wars_fireworks.checked);
 dj_wars->finale_tick=DW_CELEBRATE_TICK;
 dj_wars_audio_block(out,16,&master,1,0,1);assert(dj_wars_fireworks.fd>=0);
 for(int i=0;i<16;i++)assert(out[2*i]==packed[2+(i/2)%4]&&out[2*i+1]==out[2*i]);
 assert(!session_dj_wars_won&&music_track_locked(1));assert(menu_music_pos==2&&music_position==2&&lseek(menu_music_fd,0,1)==2);
 /* Music fader affects the PCM independently; master also mutes it. */
 dj_wars_audio_block(out,16,&master,1,0,.5f);for(int i=0;i<16;i++)assert(out[2*i]==packed[2+(i/2)%4]/2);
 dj_wars_audio_block(out,16,&master,1,0,0);for(int i=0;i<32;i++)assert(!out[i]);
 master=0;dj_wars_audio_block(out,16,&master,0,0,1);for(int i=0;i<32;i++)assert(!out[i]);master=1;
 /* Regular SFX remain audible with the fireworks music fader down. */
 dj_wars_audio.fx_gain=1;dj_wars->event=2;dj_wars->event_serial++;int signal=0;
 for(int block=0;block<20;block++){dj_wars_audio_block(out,16,&master,1,1,0);for(int i=0;i<32;i++)signal|=out[i];}assert(signal);
 dj_wars_release();assert(dj_wars_fireworks.fd==-1&&!session_dj_wars_won);
 /* Normal victory starts immediately; the song unlocks only at the finale end. */
 assert(dj_wars_reset());dj_wars->state=DW_FINALE;dj_wars->finale_assisted=0;dj_wars->finale_tick=0;dj_wars_audio.fx_gain=0;
 dj_wars_audio_block(out,16,&master,1,0,1);assert(dj_wars_fireworks.fd>=0&&music_track_locked(1));
 dj_wars->state=DW_OVER;dj_wars->winner=0;dj_wars_release();assert(dj_wars_fireworks.fd==-1&&session_dj_wars_won&&!music_track_locked(1)&&!music_secret_unlocked);
 music_choose(1);assert(music_seek_request==4);
 /* Missing/truncated assets fail quietly and keep the procedural score. */
 assert(dj_wars_reset());dj_wars_fireworks_path="work/no-such-fireworks-file.pcm";dj_wars->state=DW_FINALE;
 dj_wars_audio_block(out,16,&master,1,0,1);assert(dj_wars_fireworks.fd==-1&&dj_wars_fireworks.checked);dj_wars_release();
 close(menu_music_fd);menu_music_fd=-1;
 printf("DJ Wars fireworks: exact track bounds/loop, separate cursor, correct cue timing, gains, effects, cleanup and finale-only RAM unlock passed.\n");
}
