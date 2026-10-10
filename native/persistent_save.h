#ifndef RX3_PERSISTENT_SAVE_H
#define RX3_PERSISTENT_SAVE_H
#include "platform.h"
/* Optional, installer-selected USB saves. No I/O at all in the default mode.
 * Two checksummed generations, atomic replacement, and a separate writer keep
 * stalls off the game/audio thread. Unplugging during FAT/exFAT writes still
 * cannot be made safe in software; the installer explicitly warns about it. */
extern int openat(int,const char*,int,...),renameat(int,const char*,int,const char*),unlinkat(int,const char*,int),fsync(int);
extern int pthread_create(unsigned long*,const void*,void*(*)(void*),void*),pthread_join(unsigned long,void**);
#ifdef __arm__
#define SAVE_NOFOLLOW 0100000
#else
#define SAVE_NOFOLLOW 0400000
#endif
#define SAVE_WORDS 10
#define SAVE_BYTES (16+SAVE_WORDS*4)
struct save_values { unsigned word[SAVE_WORDS]; };
static struct {int dir,started;unsigned pending[SAVE_WORDS],saved[SAVE_WORDS],epoch,stop,busy,failed,generation;unsigned long thread;} persistent={.dir=-1};
static void save_put(unsigned char*p,unsigned v){for(unsigned i=0;i<4;i++)p[i]=(unsigned char)(v>>(8*i));}
static unsigned save_get(const unsigned char*p){return p[0]|(unsigned)p[1]<<8|(unsigned)p[2]<<16|(unsigned)p[3]<<24;}
static unsigned save_hash(const unsigned char*p,unsigned n){unsigned h=2166136261u;for(unsigned i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
static void save_encode(unsigned char*b,const struct save_values*v,unsigned generation){save_put(b,0x53433352u);save_put(b+4,1);save_put(b+8,generation);for(unsigned i=0;i<SAVE_WORDS;i++)save_put(b+12+i*4,v->word[i]);save_put(b+SAVE_BYTES-4,save_hash(b,SAVE_BYTES-4));}
static int save_decode(const unsigned char*b,struct save_values*v,unsigned*g){
 if(save_get(b)!=0x53433352u||save_get(b+4)!=1||save_get(b+SAVE_BYTES-4)!=save_hash(b,SAVE_BYTES-4))return 0;
 for(unsigned i=0;i<SAVE_WORDS;i++)v->word[i]=save_get(b+12+i*4);
 if(v->word[5]>15||v->word[6]>1||v->word[7]>1||v->word[8]>15||(v->word[6]&&v->word[5]!=15)||(v->word[7]&&!v->word[6]))return 0;
 *g=save_get(b+8);return 1;
}
static int save_read_slot(int dir,const char*name,struct save_values*v,unsigned*g){
 int fd=openat(dir,name,SAVE_NOFOLLOW|04000);if(fd<0)return 0;
 unsigned char b[SAVE_BYTES+1];unsigned at=0;while(at<sizeof b){ssize_t n=read(fd,b+at,sizeof b-at);if(n<=0)break;at+=(unsigned)n;}close(fd);
 return at==SAVE_BYTES&&save_decode(b,v,g);
}
static int save_write_slot(int dir,const struct save_values*v,unsigned generation){
 unsigned char b[SAVE_BYTES];save_encode(b,v,generation);
 /* O_EXCL and O_NOFOLLOW ensure a hostile pre-existing temp/symlink is never followed. */
 const char*tmp=".rx3-save.tmp";unlinkat(dir,tmp,0);
 int fd=openat(dir,tmp,01|0100|0200|SAVE_NOFOLLOW,0600);if(fd<0)return 0;
 unsigned at=0;while(at<sizeof b){ssize_t n=write(fd,b+at,sizeof b-at);if(n<=0){close(fd);unlinkat(dir,tmp,0);return 0;}at+=(unsigned)n;}
 int ok=fsync(fd)==0;close(fd);
 struct save_values verify;unsigned g=0;
 ok=ok&&save_read_slot(dir,tmp,&verify,&g)&&g==generation&&!memcmp(&verify,v,sizeof verify);
 if(ok)ok=renameat(dir,tmp,dir,generation&1?"rx3-save-a.dat":"rx3-save-b.dat")==0;
 if(ok)ok=fsync(dir)==0;
 if(!ok)unlinkat(dir,tmp,0);
 return ok;
}
static void save_snapshot(struct save_values*v){
 for(;;){unsigned e=__atomic_load_n(&persistent.epoch,__ATOMIC_SEQ_CST);if(e&1)continue;
  for(unsigned i=0;i<SAVE_WORDS;i++)v->word[i]=__atomic_load_n(&persistent.pending[i],__ATOMIC_SEQ_CST);
  if(e==__atomic_load_n(&persistent.epoch,__ATOMIC_SEQ_CST))return;
 }
}
static void *save_worker(void*unused){
 (void)unused;struct save_values last;for(unsigned i=0;i<SAVE_WORDS;i++)last.word[i]=persistent.saved[i];unsigned delay=0;
 while(!__atomic_load_n(&persistent.stop,__ATOMIC_SEQ_CST)){
  rx_timespec pause={0,100000000};nanosleep(&pause,0);if(__atomic_load_n(&persistent.stop,__ATOMIC_SEQ_CST))break;if(delay)delay--;
  struct save_values next;save_snapshot(&next);
  if((!delay||memcmp(next.word+5,last.word+5,5*sizeof(unsigned)))&&memcmp(&next,&last,sizeof next)){
   __atomic_store_n(&persistent.busy,1,__ATOMIC_SEQ_CST);
   int ok=save_write_slot(persistent.dir,&next,persistent.generation+1);
   __atomic_store_n(&persistent.busy,0,__ATOMIC_SEQ_CST);
   if(!ok){__atomic_store_n(&persistent.failed,1,__ATOMIC_SEQ_CST);break;}
   persistent.generation++;last=next;delay=50;
  }
 }
 /* Removal/shutdown does not initiate another write to a disappearing stick. */
 return 0;
}
static int save_begin(const char*root,int enabled,struct save_values*out){
 if(!enabled||!root||!*root||persistent.started)return 0;
 persistent.stop=persistent.failed=persistent.busy=persistent.epoch=0;
 persistent.dir=open(root,SAVE_NOFOLLOW);if(persistent.dir<0)return 0;
 struct save_values a={0},b={0};unsigned ga=0,gb=0;
 int va=save_read_slot(persistent.dir,"rx3-save-a.dat",&a,&ga),vb=save_read_slot(persistent.dir,"rx3-save-b.dat",&b,&gb);
 *out=vb&&(!va||(int)(gb-ga)>0)?b:va?a:(struct save_values){0};persistent.generation=vb&&(!va||(int)(gb-ga)>0)?gb:va?ga:0;
 for(unsigned i=0;i<SAVE_WORDS;i++)persistent.pending[i]=persistent.saved[i]=out->word[i];
 if(pthread_create(&persistent.thread,0,save_worker,0)){close(persistent.dir);persistent.dir=-1;return 0;}
 persistent.started=1;return 1;
}
static void save_queue(const struct save_values*v){
 if(!persistent.started||__atomic_load_n(&persistent.failed,__ATOMIC_SEQ_CST))return;
 __atomic_add_fetch(&persistent.epoch,1,__ATOMIC_SEQ_CST);
 for(unsigned i=0;i<SAVE_WORDS;i++)__atomic_store_n(&persistent.pending[i],v->word[i],__ATOMIC_SEQ_CST);
 __atomic_add_fetch(&persistent.epoch,1,__ATOMIC_SEQ_CST);
}
static void save_end(void){if(!persistent.started)return;__atomic_store_n(&persistent.stop,1,__ATOMIC_SEQ_CST);pthread_join(persistent.thread,0);close(persistent.dir);persistent.dir=-1;persistent.started=0;}
#endif
