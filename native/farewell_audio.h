#ifndef RX3_FAREWELL_AUDIO_H
#define RX3_FAREWELL_AUDIO_H
/* Private RAM payload survives USB removal. Stream bounded 44.1 kHz stereo
 * PCM through the RX3 output (advertised 48 kHz, measured 44.1 kHz clock).
 * Nonblocking writes and a deadline keep audio failure from blocking cleanup. */
extern long lseek(int,long,int);
extern int snd_pcm_drop(void*),snd_pcm_delay(void*,long*);
static void farewell_audio(const char*path,unsigned master,void(*update)(uint64_t)){
 if(master>1023)master=1023;
 int fd=open(path,0);if(fd<0)return;
 long size=lseek(fd,0,2);if(size<=0||size>2116800||size%4||lseek(fd,0,0)<0){close(fd);return;}
 void*pcm=0;int rc=snd_pcm_open(&pcm,"plughw:1,0",0,1);
 if(rc>=0)rc=snd_pcm_set_params(pcm,2,3,2,48000,1,160000);
 if(rc<0){if(pcm)snd_pcm_close(pcm);close(fd);return;}
 uint64_t deadline=now_ns()+13000000000ULL;int16_t block[1600];long remaining=size;int failed=0;
 while(remaining>0&&now_ns()<deadline){
  unsigned want=remaining>(long)sizeof(block)?sizeof(block):(unsigned)remaining,got=0;
  while(got<want){long n=read(fd,(unsigned char*)block+got,want-got);if(n<=0){failed=1;break;}got+=(unsigned)n;}
  if(failed)break;
  for(unsigned i=0;i<got/2;i++)block[i]=(int16_t)((int32_t)block[i]*(int32_t)master/1023);
  unsigned frames=got/4,pos=0;
  while(pos<frames&&now_ns()<deadline){
   if(update)update(now_ns());
   long n=snd_pcm_writei(pcm,block+pos*2,frames-pos);
   if(n>0)pos+=(unsigned)n;
   else if(n==0||n==-11){rx_timespec t={0,5000000};nanosleep(&t,0);}
   else if(snd_pcm_recover(pcm,(int)n,1)<0){failed=1;break;}
  }
  if(failed)break;remaining-=got;
 }
 /* Let the last queued samples finish, without an unbounded blocking drain. */
 while(!failed&&now_ns()<deadline){if(update)update(now_ns());long delay=0;if(snd_pcm_delay(pcm,&delay)<0||delay<=0)break;rx_timespec t={0,5000000};nanosleep(&t,0);}
 snd_pcm_drop(pcm);snd_pcm_close(pcm);close(fd);
}
#endif
