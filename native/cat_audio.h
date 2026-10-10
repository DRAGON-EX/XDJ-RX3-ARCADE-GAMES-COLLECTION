#ifndef RX3_CAT_AUDIO_H
#define RX3_CAT_AUDIO_H
/* One quiet voice at a time, streamed in 1 KB blocks. Called only by the PCM owner. */
static unsigned cafe_audio_enabled,cafe_audio_greeting;
static struct game_pcm_stream cat_stream;
static int cat_fd=-1,cat_purr;static unsigned cat_position,cat_end,cat_wait,cat_rng=7319;
static int cat_was_cafe;
static void cat_audio_close(void){if(cat_fd>=0)close(cat_fd);cat_fd=-1;cat_end=cat_position=0;}
static void cat_audio_start(unsigned id){
 static const char*names[]={"cat-cute","cat-meow-1","cat-meow-2","cat-meow-3","cat-purr-soft","cat-purr-warm"};
 cat_audio_close();char path[100];snprintf(path,sizeof(path),"/tmp/rx3-launcher/payload/%s.pcm",names[id%6]);cat_fd=open(path,0);cat_stream=(struct game_pcm_stream){0};cat_purr=id%6>=4;
 if(cat_fd>=0){long n=lseek(cat_fd,0,2);if(n>0&&n<4000000){cat_end=(unsigned)n;lseek(cat_fd,0,0);}else cat_audio_close();}
}
static int cat_audio_sample(int cafe_mode){
 if(cafe_mode&&!cat_was_cafe){cat_audio_start(4);cat_wait=44100*12;}
 if(!cafe_mode&&cat_was_cafe)cat_audio_close();cat_was_cafe=cafe_mode;
 if(cafe_mode){if(__atomic_exchange_n(&cafe_audio_greeting,0,__ATOMIC_RELAXED)){cat_audio_start(cat_rng%4);cat_wait=44100*12;}
  if(!cat_wait){cat_rng=cat_rng*1664525u+1013904223u;cat_audio_start(cat_rng%6);cat_wait=44100*(12+cat_rng%10);}else cat_wait--;
 }
 if(cat_fd<0)return 0;int v=stream_sample(&cat_stream,cat_fd,&cat_position,cat_end)/(cat_purr?5:2);if(cat_position>=cat_end&&!cat_stream.repeat)cat_audio_close();return v;
}
#endif
