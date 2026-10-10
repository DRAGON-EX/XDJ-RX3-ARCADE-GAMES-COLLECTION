#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../native/cafe_show_leds.h"

/* CRC-16/X25 fixture, expressed as a byte stream in bus word order. */
static unsigned crc_bus(const unsigned char*p,unsigned n){
 unsigned short crc=0xffff;
 for(unsigned i=0;i<n;i++){
  unsigned char ch=p[(i&~3u)+(3-(i&3u))];
  for(unsigned k=0;k<8;k++){
   unsigned bit=(crc^ch)&1u;crc>>=1;ch>>=1;
   if(bit)crc^=0x8408;
  }
 }
 return crc^0xffffu;
}
static void validate(const unsigned char*e,const unsigned char*s,const unsigned char*base){
 static const unsigned char allowed[20]={0,0xe4,8,0x19,0,0xe4,8,0x19,0,0,0x38,0,0,0,0,0,0,0,0,255};
 assert((e[22]|(unsigned)e[23]<<8)==crc_bus(e,20));
 assert((s[102]|(unsigned)s[103]<<8)==crc_bus(s,100));
 for(unsigned i=0;i<20;i++)assert(!((e[i]^base[i])&~allowed[i]));
 assert(e[20]==0&&e[21]==0);
 for(unsigned i=24;i<100;i++)assert(e[i]==0);
 for(unsigned i=0;i<24;i++){
  assert(s[i*4]==1);assert(s[i*4+1]<=60);assert(s[i*4+2]<=28);assert(s[i*4+3]<=26);
 }
 assert(s[96]==0&&s[97]==0&&(s[98]&15)==0&&(s[99]&15)==0&&s[100]==0&&s[101]==0);
}
int main(void){
 unsigned char e[100],s[104],base[100],baseline_sub[104],previous[104];
 tutorial_packet(base,0,1,1);tutorial_cue_packet(baseline_sub,1);
 unsigned rgb_changed[24]={0},e_on[20]={0},e_off[20]={0},s_on[2]={0},s_off[2]={0};
 memset(previous,0,sizeof previous);
 for(unsigned ms=0;ms<CAFE_SHOW_DURATION_MS;ms+=50){
  cafe_show_led_packets(e,s,cafe_show_frame(ms));validate(e,s,base);
  for(unsigned i=0;i<24;i++)if(memcmp(previous+i*4+1,s+i*4+1,3))rgb_changed[i]++;
  for(unsigned i=0;i<20;i++){e_on[i]|=e[i];e_off[i]|=(unsigned char)~e[i];}
  for(unsigned i=0;i<2;i++){s_on[i]|=s[98+i];s_off[i]|=(unsigned char)~s[98+i];}
  memcpy(previous,s,sizeof previous);
 }
 for(unsigned i=0;i<24;i++)assert(rgb_changed[i]>100);
 assert((e_on[19]&e_off[19])==255);
 for(unsigned d=0;d<2;d++){
  assert((e_on[4*d+1]&e_off[4*d+1]&0xe4)==0xe4);
  assert((e_on[4*d+2]&e_off[4*d+2]&8)==8);
  assert((e_on[4*d+3]&e_off[4*d+3]&0x19)==0x19);
  assert((s_on[d]&s_off[d]&0xf0)==0xf0);
 }
 assert((e_on[10]&e_off[10]&0x38)==0x38);
 struct cafe_show_led_state state={0};
 assert(cafe_show_led_update(&state,0,0,0,e,s)==0);
 assert(cafe_show_led_update(&state,1,0,0,e,s)==3);
 for(unsigned i=1;i<50;i++)assert(cafe_show_led_update(&state,1,i,(uint64_t)i*1000000,e,s)==0);
 assert(cafe_show_led_update(&state,1,50,50000000,e,s));
 /* Frozen audio means no further packet changes, even as the loop runs. */
 assert(cafe_show_led_update(&state,1,50,100000000,e,s)==0);
 /* Exit bypasses the rate limiter; restoration is byte-for-byte baseline. */
 assert(cafe_show_led_update(&state,0,50,100000001,e,s)==3);
 assert(!memcmp(e,base,100)&&!memcmp(s,baseline_sub,104));
 assert(cafe_show_led_update(&state,0,0,100000002,e,s)==0);
 assert(cafe_show_led_update(&state,1,1000,100000003,e,s)==3);
 assert(cafe_show_led_update(&state,1,CAFE_SHOW_DURATION_MS,100000004,e,s)==3);
 assert(!memcmp(e,base,100)&&!memcmp(s,baseline_sub,104));
 assert(!cafe_show_led_update(&state,1,CAFE_SHOW_DURATION_MS+1,100000005,e,s));
 /* Six authored moods have genuinely different pad colours/patterns. */
 unsigned char section[6][104];
 for(unsigned i=0;i<6;i++){
  struct cafe_show_frame f={.ms=12000,.beat=9,.phase=80,.energy=200,.bass=220,.high=190,.hit=230,.section=i};
  cafe_show_led_packets(e,section[i],f);validate(e,section[i],base);
  for(unsigned j=0;j<i;j++)assert(memcmp(section[j],section[i],96));
 }
 puts("Cafe show LEDs: full-song CRC/padding/control-mask checks, all 59 mapped zones animate, six moods, <=20Hz diff output and exact one-shot restoration passed");
 return 0;
}
