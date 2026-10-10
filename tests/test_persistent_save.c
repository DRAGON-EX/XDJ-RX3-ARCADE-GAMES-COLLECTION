#include "../native/persistent_save.h"
#include <assert.h>
extern char*mkdtemp(char*);
extern int rmdir(const char*);
static void wait_saved(unsigned generation){for(int i=0;i<100;i++){struct save_values v;unsigned g=0;if(save_read_slot(persistent.dir,generation&1?"rx3-save-a.dat":"rx3-save-b.dat",&v,&g)&&g==generation)return;rx_timespec t={0,20000000};nanosleep(&t,0);}assert(!"writer timeout");}
int main(void){
 char root[]="/tmp/rx3-save-test-XXXXXX";assert(mkdtemp(root));struct save_values v={0},out;unsigned g;
 assert(!save_begin(root,0,&out));assert(!persistent.started);
 assert(save_begin(root,1,&out));assert(!out.word[0]);
 v.word[0]=12340;v.word[1]=500;v.word[5]=1;v.word[9]=7;save_queue(&v);wait_saved(1);save_end();
 assert(save_begin(root,1,&out));assert(!memcmp(&out,&v,sizeof v));
 v.word[5]=15;v.word[6]=1;v.word[7]=1;v.word[8]=15;v.word[9]=0xffff;save_queue(&v);wait_saved(2);save_end();
 assert(save_begin(root,1,&out));assert(!memcmp(&out,&v,sizeof v));save_end();
 int dir=open(root,0);assert(dir>=0);
 /* A torn newest generation must fall back to the older complete state. */
 int fd=openat(dir,"rx3-save-b.dat",01|01000);assert(fd>=0);assert(write(fd,"bad",3)==3);close(fd);
 assert(save_begin(root,1,&out));assert(out.word[0]==12340&&out.word[5]==1&&!out.word[6]);save_end();
 unsigned char b[SAVE_BYTES];save_encode(b,&v,4);b[15]^=1;assert(!save_decode(b,&out,&g));
 v.word[5]=1;save_encode(b,&v,4);assert(!save_decode(b,&out,&g));
 /* Default/off never consumes existing saves. */
 assert(!save_begin(root,0,&out));
 unlinkat(dir,"rx3-save-a.dat",0);unlinkat(dir,"rx3-save-b.dat",0);close(dir);assert(!rmdir(root));
 printf("Persistent saves: opt-in, live writes, reload and corrupt-generation fallback passed\n");return 0;
}
