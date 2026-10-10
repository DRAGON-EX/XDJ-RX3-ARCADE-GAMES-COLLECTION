#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
typedef struct {long sec,nsec;} rx_timespec;
static uint64_t clock_ns;static unsigned opens,closes,drops,writes,frames;static int mode;static unsigned expected_sample=1234,updates;
static void update(uint64_t now){assert(now>0);updates++;}
static uint64_t now_ns(void){clock_ns+=1000000;return clock_ns;}
static int test_sleep(const rx_timespec*t,void*unused){(void)unused;clock_ns+=(uint64_t)t->sec*1000000000+t->nsec;return 0;}
#define nanosleep test_sleep
int snd_pcm_open(void**p,const char*name,int stream,int flags){assert(!strcmp(name,"plughw:1,0")&&stream==0&&flags==1);opens++;if(mode==2)return -16;*p=(void*)1;return 0;}
int snd_pcm_set_params(void*p,int format,int access,unsigned channels,unsigned rate,int resample,unsigned latency){assert(p&&format==2&&access==3&&channels==2&&rate==48000&&resample==1&&latency==160000);return 0;}
long snd_pcm_writei(void*p,const void*data,unsigned long n){assert(p);writes++;if(mode==1||writes<3)return -11;const int16_t*s=data;unsigned take=n>37?37:n;for(unsigned i=0;i<take*2;i++)assert(s[i]==expected_sample);frames+=take;return take;}
int snd_pcm_recover(void*p,int rc,int silent){(void)p;(void)silent;return rc;}
int snd_pcm_delay(void*p,long*n){(void)p;*n=0;return 0;}
int snd_pcm_close(void*p){assert(p);closes++;return 0;}
int snd_pcm_drop(void*p){assert(p);drops++;return 0;}
#include "../native/farewell_audio.h"
#include "../native/master_state.h"
int main(void){
 const char*path="work/test-farewell.pcm";int16_t samples[4000];for(unsigned i=0;i<4000;i++)samples[i]=1234;
 int fd=open(path,O_CREAT|O_TRUNC|O_WRONLY,0600);assert(fd>=0&&write(fd,samples,sizeof(samples))==sizeof(samples));close(fd);
 farewell_audio(path,1023,update);assert(frames==2000&&opens==1&&closes==1&&drops==1);
 expected_sample=617;writes=0;farewell_audio(path,512,update);assert(frames==4000&&updates>0&&closes==2);
 expected_sample=0;writes=0;farewell_audio(path,0,update);assert(frames==6000&&closes==3);
 mode=1;clock_ns=0;writes=0;farewell_audio(path,1023,update);assert(clock_ns>=13000000000ULL&&clock_ns<13100000000ULL&&closes==4&&writes<3000);
 mode=2;farewell_audio(path,1023,update);assert(opens==5&&closes==4);
 unlink(path);farewell_audio(path,1023,update);assert(opens==5);
 fd=open(path,O_CREAT|O_TRUNC|O_WRONLY,0600);assert(fd>=0);assert(ftruncate(fd,2116804)==0);close(fd);farewell_audio(path,1023,update);assert(opens==5);unlink(path);
 master_state_path="work/test-master-level";unlink(master_state_path);assert(master_state_load()==0);
 master_state_store(512);assert(master_state_load()==512);master_state_store(1024);assert(master_state_load()==512);
 unsigned char panel[104]={0};panel[3]=1;panel[74]=255;panel[75]=3;master_state_panel(panel,104);assert(master_state_load()==1023);
 panel[3]=0;panel[74]=0;panel[75]=0;master_state_panel(panel,104);assert(master_state_load()==1023);
 panel[3]=1;master_state_panel(panel,104);assert(master_state_load()==0);unlink(master_state_path);
 puts("Farewell PCM: partial writes, temporary/full stalls, deadline, missing/oversized file and unavailable output passed");return 0;
}
