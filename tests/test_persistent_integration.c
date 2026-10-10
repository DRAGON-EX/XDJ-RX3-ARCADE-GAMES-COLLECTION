#define main rx3_program_main
#include "../native/arcade.c"
#undef main
#include <assert.h>
extern char *mkdtemp(char*);
extern int rmdir(const char*),setenv(const char*,const char*,int);
int main(void){
 char root[]="/tmp/rx3-progress-test-XXXXXX";assert(mkdtemp(root));int dir=open(root,0);assert(dir>=0);
 struct save_values v={{12340,2345,3456,4567,5678,15,1,1,15,3}};assert(save_write_slot(dir,&v,1));
 music_count=2;music_tracks[0].locked=music_tracks[1].locked=1;
 setenv("RX3_USB_ROOT",root,1);setenv("RX3_PERSISTENT_SAVE","0",1);persistent_restore();assert(!persistent.started&&!session_rescued_cats);
 setenv("RX3_PERSISTENT_SAVE","1",1);persistent_restore();
 assert(arcade_value(6)==12340&&equalizarr.best==2345&&asteroid.high==3456&&basslines.best==4567&&invaders.best==5678);
 assert(session_rescued_cats==15&&session_dj_wars_won&&music_secret_unlocked&&cafe_seen==15);
 assert(!music_tracks[0].locked&&!music_tracks[1].locked);
 arcade_reset();eq_reset(&equalizarr,1,0);ast_reset(&asteroid,1);bl_reset(&basslines,1);si_reset(&invaders,1);
 assert(arcade_value(6)==12340&&equalizarr.best==2345&&asteroid.high==3456&&basslines.best==4567&&invaders.best==5678);
 equalizarr.best=9876;persistent_update();struct save_values queued;save_snapshot(&queued);assert(queued.word[1]==9876);
 debug_unlocked=1;equalizarr.best=9999;persistent_update();save_snapshot(&queued);assert(queued.word[1]==9876);save_end();
 unlinkat(dir,"rx3-save-a.dat",0);unlinkat(dir,"rx3-save-b.dat",0);close(dir);assert(!rmdir(root));
 printf("All five highscores, cats, finale, secrets restored; resets preserve scores and debug changes stay unsaved\n");return 0;
}
