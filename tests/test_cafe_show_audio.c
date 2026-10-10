#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#define RX3_PLATFORM_H
#include "../native/cafe_show_audio.h"

static const char*path="work/cafe-show-audio-test.pcm";
static int16_t value(unsigned i){return (int16_t)((int)(i%301)*100-15000);}
static void fixture(unsigned count){FILE*f=fopen(path,"wb");assert(f);for(unsigned i=0;i<count;i++){int16_t s=value(i);assert(fwrite(&s,2,1,f)==1);}assert(!fclose(f));}
int main(void){
 assert(sizeof cafe_show_audio<1200);assert(!cafe_show_audio_begin("work/missing-cafe-song.pcm"));assert(!cafe_show_audio_active());
 fixture(0);assert(!cafe_show_audio_begin(path));FILE*f=fopen(path,"wb");fputc(1,f);fclose(f);assert(!cafe_show_audio_begin(path));
 fixture(22050);assert(cafe_show_audio_begin(path));assert(cafe_show_audio_active()&&!cafe_show_audio_ms());
 int16_t out[602];unsigned generated=0;
 /* Odd block lengths exercise repeated source samples across calls/buffer edges. */
 while(generated<44100){
  unsigned want=301;int n=cafe_show_audio_read(out,want,.5f);assert(n>0&&n<=301);
  for(int i=0;i<n;i++)assert(out[2*i]==(int16_t)(value((generated+i)/2)*.5f)&&out[2*i]==out[2*i+1]);
  assert(cafe_show_audio_ms()==(generated>441?(generated-441)*1000/44100:0));
  cafe_show_audio_commit((unsigned)n,441);generated+=(unsigned)n;
 }
 assert(cafe_show_audio_read(out,301,.5f)==0&&cafe_show_audio_active());assert(cafe_show_audio_ms()==990);
 cafe_show_audio_commit(0,2205);assert(cafe_show_audio_ms()==990); /* Delay wobble cannot reverse cues. */
 cafe_show_audio_drained();assert(!cafe_show_audio_active()&&cafe_show_audio_ms()==1000&&cafe_show_audio.fd<0);
 /* Restart resets the clock; read-ahead alone cannot advance it. */
 assert(cafe_show_audio_begin(path));assert(cafe_show_audio_read(out,301,1)==301);assert(!cafe_show_audio_ms());
 cafe_show_audio_drained();assert(cafe_show_audio_active());cafe_show_audio_commit(100,200);assert(!cafe_show_audio_ms());
 cafe_show_audio_commit(201,0);assert(cafe_show_audio_ms()==6);cafe_show_audio_stop();assert(!cafe_show_audio_active()&&!cafe_show_audio_ms()&&cafe_show_audio.fd<0);
 assert(cafe_show_audio_begin(path));assert(cafe_show_audio_read(out,301,0)==301);for(int i=0;i<602;i++)assert(!out[i]);cafe_show_audio_stop();
 /* Truncated payload is an error, never silently looping or advancing the clock. */
 assert(cafe_show_audio_begin(path));assert(!truncate(path,0));assert(cafe_show_audio_read(out,301,1)==-1);assert(!cafe_show_audio_active());
 assert(!unlink(path));puts("Cafe show: bounded 1 KB stream, 2x sample clock, gain, ALSA delay, drain, restart and I/O fallback passed.");
}
