#ifndef RX3_CAFE_SHOW_AUDIO_H
#define RX3_CAFE_SHOW_AUDIO_H
#include "platform.h"
extern long lseek(int,long,int);

/* Audio-worker-owned stream, with atomic read-only UI status. Input is the
 * established 22050 Hz S16 mono format; each sample feeds two 44100 Hz frames.
 * Keep the show clock tied to written minus queued frames, never render ticks. */
static struct {
 int fd;
 int16_t data[512],sample;
 unsigned at,count,repeat,generated,written,total;
 unsigned active,heard_ms;
} cafe_show_audio={.fd=-1};

static unsigned cafe_show_audio_active(void){return __atomic_load_n(&cafe_show_audio.active,__ATOMIC_ACQUIRE);}
static unsigned cafe_show_audio_ms(void){return __atomic_load_n(&cafe_show_audio.heard_ms,__ATOMIC_ACQUIRE);}
static void cafe_show_audio_stop(void){
 __atomic_store_n(&cafe_show_audio.active,0,__ATOMIC_RELEASE);
 if(cafe_show_audio.fd>=0)close(cafe_show_audio.fd);
 cafe_show_audio.fd=-1;cafe_show_audio.at=cafe_show_audio.count=cafe_show_audio.repeat=0;
 cafe_show_audio.generated=cafe_show_audio.written=cafe_show_audio.total=0;
 __atomic_store_n(&cafe_show_audio.heard_ms,0,__ATOMIC_RELEASE);
}
static int cafe_show_audio_begin(const char*path){
 cafe_show_audio_stop();if(!path)return 0;
 int fd=open(path,0);if(fd<0)return 0;
 long bytes=lseek(fd,0,2);
 /* Bounded input also avoids narrowing arbitrary file sizes on ARM. */
 if(bytes<=0||bytes>32*1024*1024||bytes%2||lseek(fd,0,0)<0){close(fd);return 0;}
 cafe_show_audio.fd=fd;cafe_show_audio.total=(unsigned)bytes;
 __atomic_store_n(&cafe_show_audio.active,1,__ATOMIC_RELEASE);return 1;
}
/* Returns stereo output frames, 0 for EOF (caller must drain), -1 on I/O error.
 * The caller applies master * CH2 * music level exactly once via gain. */
static int cafe_show_audio_read(int16_t*out,unsigned frames,float gain){
 if(cafe_show_audio.fd<0||!out)return -1;
 if(gain<0)gain=0;if(gain>1)gain=1;
 unsigned remain=cafe_show_audio.total-cafe_show_audio.generated;if(frames>remain)frames=remain;
 for(unsigned i=0;i<frames;i++){
  if(cafe_show_audio.repeat)cafe_show_audio.repeat=0;
  else {
   if(cafe_show_audio.at>=cafe_show_audio.count){
    unsigned bytes=cafe_show_audio.total-cafe_show_audio.generated;
    if(bytes>sizeof cafe_show_audio.data)bytes=sizeof cafe_show_audio.data;
    unsigned got=0;while(got<bytes){long n=read(cafe_show_audio.fd,(unsigned char*)cafe_show_audio.data+got,bytes-got);if(n<0&&*__errno_location()==4)continue;if(n<=0){cafe_show_audio_stop();return -1;}got+=(unsigned)n;}
    cafe_show_audio.at=0;cafe_show_audio.count=got/2;
   }
   cafe_show_audio.sample=cafe_show_audio.data[cafe_show_audio.at++];cafe_show_audio.repeat=1;
  }
  int16_t sample=(int16_t)(cafe_show_audio.sample*gain);out[2*i]=out[2*i+1]=sample;cafe_show_audio.generated++;
 }
 return (int)frames;
}
static void cafe_show_audio_commit(unsigned frames,long queued){
 if(!cafe_show_audio_active())return;
 unsigned remain=cafe_show_audio.generated-cafe_show_audio.written;if(frames>remain)frames=remain;
 cafe_show_audio.written+=frames;
 unsigned heard=cafe_show_audio.written;
 if(queued>0)heard=(unsigned long)queued<heard?heard-(unsigned)queued:0;
 unsigned ms=(unsigned)((uint64_t)heard*1000/44100);
 /* ALSA delay estimates can wobble slightly; cues never run backwards. */
 if(ms>cafe_show_audio_ms())__atomic_store_n(&cafe_show_audio.heard_ms,ms,__ATOMIC_RELEASE);
}
static void cafe_show_audio_drained(void){
 /* Only a successful drain after every output frame counts as natural end. */
 if(cafe_show_audio.written<cafe_show_audio.total)return;
 cafe_show_audio_commit(0,0);
 if(cafe_show_audio.fd>=0)close(cafe_show_audio.fd);
 cafe_show_audio.fd=-1;__atomic_store_n(&cafe_show_audio.active,0,__ATOMIC_RELEASE);
}
#endif
