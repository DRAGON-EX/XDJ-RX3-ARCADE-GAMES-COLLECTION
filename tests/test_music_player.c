#define main rx3_program_main
#include "../native/arcade.c"
#undef main
#include <assert.h>
#include <string.h>
int main(void){
 const char*fixture="0\t88200\tFIRST TRACK\tFIRST ARTIST\n88200\t176400\tSECOND TRACK\tSECOND ARTIST\n176400\t81000000\tTOO LARGE\tBAD\n";
 int fd=open("work/test-music-index.tsv",1|64|512,0600);assert(fd>=0);assert(write(fd,fixture,strlen(fixture))==(long)strlen(fixture));close(fd);music_index_path="work/test-music-index.tsv";music_catalog_load();assert(music_count==2&&!strcmp(music_tracks[1].title,"SECOND TRACK"));
 char b[32];music_time(b,sizeof(b),66150,0);assert(!strcmp(b,"00:01.500"));music_time(b,sizeof(b),441,1);assert(!strcmp(b,"-00:00.010"));
 music_position=0;music_choose(1);assert(music_seek_request==88200&&!music_paused);music_position=88200;assert(music_current()==1);music_choose(2);assert(music_seek_request==0);music_browse(-1,1);assert(music_selected==1&&music_seek_request==88200);
 music_position=0;music_touch(430,454);assert(music_seek_request>43000&&music_seek_request<45000&&!(music_seek_request&1));music_touch(600,510);assert(music_remaining);music_touch(600,510);assert(!music_remaining);
 music_paused=0;music_touch(300,590);assert(music_paused);music_touch(300,590);assert(!music_paused);music_touch(225,680);assert(music_level==0);music_touch(650,680);assert(music_level==100);
 music_touch(900,380);assert(music_seek_request==88200);music_position=0;music_touch(535,590);assert(music_seek_request==88200);

 const char*expanded="0\t88200\tONE\tARTIST\n88200\t176400\tTWO\tARTIST\n176400\t9000000\tTHREE\tARTIST\n9000000\t12000000\tFOUR\tARTIST\n12000000\t14000000\tFIVE\tARTIST\n14000000\t81000000\tTOO LARGE\tBAD\n";
 fd=open("work/test-music-index.tsv",1|64|512,0600);assert(fd>=0);assert(write(fd,expanded,strlen(expanded))==(long)strlen(expanded));close(fd);music_catalog_load();assert(music_count==5);
 music_selected=0;music_browse(4,1);assert(music_browser_page==1&&music_seek_request==12000000);music_position=12000000;assert(music_current()==4);music_choose(5);assert(music_selected==0&&music_seek_request==0);
 music_touch(1100,680);assert(music_browser_page==1);music_touch(900,300);assert(music_selected==4&&music_seek_request==12000000);
 const char*locked="0\t88200\tPLAYABLE\tARTIST\n88200\t88200\tLOCKED ONE\tARTIST\t1\n88200\t88200\tLOCKED TWO\tARTIST\t2\n";
 fd=open("work/test-music-index.tsv",1|64|512,0600);assert(fd>=0);assert(write(fd,locked,strlen(locked))==(long)strlen(locked));close(fd);music_catalog_load();assert(music_count==3&&music_tracks[1].locked&&music_tracks[2].locked);
 music_seek_request=42;music_paused=1;music_choose(2);assert(music_seek_request==42&&music_paused&&music_tracks[2].locked==2);music_choose(1);assert(music_seek_request==42&&music_paused);music_adjacent(0,1);assert(music_seek_request==0);music_adjacent(0,-1);assert(music_seek_request==0);
 /* The finale reward is independently gated from the credits secret. */
 const char*secrets="0\t100\tPUBLIC\tARTIST\t0\n100\t200\tCREDITS SECRET\tARTIST\t2\n200\t300\tPIXEL QUEST\tALEX MORGAN\t3\n";
 fd=open("work/test-music-index.tsv",1|64|512,0600);assert(fd>=0&&write(fd,secrets,strlen(secrets))==(long)strlen(secrets));close(fd);music_catalog_load();assert(music_count==3&&music_tracks[2].locked==3);
 session_dj_wars_won=music_secret_unlocked=0;assert(music_track_locked(1)&&music_track_locked(2));music_seek_request=42;music_choose(2);assert(music_seek_request==42);
 session_dj_wars_won=1;assert(!music_track_locked(2)&&music_track_locked(1));music_choose(2);assert(music_seek_request==200);session_dj_wars_won=0;
 width=1280;height=800;pixels=malloc(width*height*4);assert(pixels);
 music_animation_path="work/no-cover-cat.rgb";session_rescued_cats=0;assert(!music_animation_draw()&&music_animation_fd==-1&&!music_animation_pixels&&!music_animation_failed);session_save_cat(1);assert(music_animation_draw());assert(music_animation_fd>=0&&music_animation_pixels&&music_animation_frame==0);assert(lseek(music_animation_fd,0,1)==97200);
 music_animation_start=ns()-60000000ULL;assert(music_animation_draw()&&music_animation_frame==1);assert(lseek(music_animation_fd,0,1)==194400);
 music_animation_release();assert(music_animation_fd==-1&&!music_animation_pixels);
 music_animation_path="work/test-music-index.tsv";assert(!music_animation_draw()&&music_animation_failed&&!music_animation_pixels&&music_animation_fd==-1);music_animation_release();free(pixels);pixels=0;
 printf("Music metadata bounds, track navigation, seek, millisecond display, pause and volume passed\n");return 0;
}
