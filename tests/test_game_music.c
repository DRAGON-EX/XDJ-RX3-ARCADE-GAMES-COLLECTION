#define main rx3_program_main
#include "../native/arcade.c"
#undef main
#include <assert.h>
int main(void){
 int fd=open("work/test-game-song.pcm",1|64|512,0600);int16_t song[1000];for(int i=0;i<1000;i++)song[i]=1000;assert(write(fd,song,sizeof(song))==sizeof(song));close(fd);
 menu_music_fd=open("work/test-game-song.pcm",0);menu_music_frames=sizeof(song);menu_music_pos=0;music_count=0;master_target=1;game_music_gain=1;gain_seen[0]=gain_seen[1]=1;gain_percent[0]=0;gain_percent[1]=100;music_level=100;
 int16_t out[1470];for(int i=0;i<1470;i++)out[i]=2000;game_mix(out,735,0);for(int i=0;i<1470;i++)assert(out[i]==1000);assert(menu_music_pos==736);
 unsigned saved=menu_music_pos;music_paused=1;for(int i=0;i<1470;i++)out[i]=2000;game_mix(out,735,0);assert(menu_music_pos==saved);for(int i=0;i<1470;i++)assert(out[i]==0);
 gain_percent[0]=100;gain_percent[1]=0;game_music_gain=0;for(int i=0;i<1470;i++)out[i]=2000;game_mix(out,735,0);for(int i=0;i<1470;i++)assert(out[i]==2000);
 music_paused=0;rescue_begin(&rescue,0);game_mix(out,735,1);assert(menu_music_pos==saved);for(int i=0;i<1470;i++)assert(out[i]==0); // missing payload fails silently, no old jingle
 // Feed the actual supplied fanfare, while playlist remains frozen.
 rescue_pcm_fd=open("work/rescue-fanfare.pcm",0);assert(rescue_pcm_fd>=0);rescue_pcm_left=(unsigned)lseek(rescue_pcm_fd,0,2);lseek(rescue_pcm_fd,0,0);int signal=0;
 for(int j=0;j<120;j++){game_mix(out,735,1);assert(menu_music_pos==saved);for(int i=0;i<1470;i++)signal|=out[i];}assert(signal);
 gain_percent[0]=0;game_mix(out,735,1);for(int i=0;i<1470;i++)assert(out[i]==0);
 gain_percent[1]=100;game_music_gain=1;game_mix(out,735,0);assert(menu_music_pos>saved);for(int i=0;i<1470;i++)assert(out[i]==1000);
 close(menu_music_fd);cat_audio_close();printf("Independent FX/music gains, pause cursor, real fanfare, mute and exact song continuation passed.\n");
}
