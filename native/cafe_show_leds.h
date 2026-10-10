#ifndef RX3_CAFE_SHOW_LEDS_H
#define RX3_CAFE_SHOW_LEDS_H
#include <stdint.h>
#include <stddef.h>
extern void *memcpy(void*,const void*,size_t);
extern int memcmp(const void*,const void*,size_t);
#include "panel_output.h"
#include "../shared/cafe_show.h"

/* Song-position-driven choreography. The only output is the existing LED
 * transfer format, never a panel key event, device mode or GPIO command.
 * Verified local protocol sources and byte masks: docs/cafe-light-show.md. */
#define CAFE_SHOW_LED_INTERVAL_NS UINT64_C(50000000)
#define CAFE_SHOW_LED_EUP 1u
#define CAFE_SHOW_LED_SUB 2u

static unsigned cafe_led_min(unsigned a,unsigned b){return a<b?a:b;}
static unsigned cafe_led_max(unsigned a,unsigned b){return a>b?a:b;}
static void cafe_led_bit(unsigned char*p,unsigned mask,int on){
 *p=(unsigned char)((*p&~mask)|(on?mask:0));
}
static void cafe_led_rgb(unsigned char*p,unsigned rgb,unsigned level){
 /* SUB's full-colour layout is enable, blue, green, red. Channel limits stay
  * within values produced by the firmware colour converter (R/G <=30, B<=90).
  * Lower show caps leave headroom and avoid filling every pad with white. */
 level=cafe_led_min(level,255);
 p[0]=1;
 p[1]=(unsigned char)(((rgb&255u)*level*60u+32512u)/65025u);
 p[2]=(unsigned char)((((rgb>>8)&255u)*level*28u+32512u)/65025u);
 p[3]=(unsigned char)((((rgb>>16)&255u)*level*26u+32512u)/65025u);
}
static void cafe_show_led_packets(unsigned char eup[100],unsigned char sub[104],struct cafe_show_frame f){
 static const unsigned palettes[6][3]={
  {0xff862b,0x813eff,0xffd277}, /* intro: warm record-room glow */
  {0x19bfff,0xf34bcc,0x70ffca}, /* groove: deck-to-deck chase */
  {0xff9c27,0xff305d,0xffed9b}, /* build: climbing amber bars */
  {0x13dbff,0xf72bb9,0xc9fdff}, /* drop: cyan/magenta responses */
  {0x366bcc,0x985eff,0x69d5b3}, /* break: slow velvet colours */
  {0xffc54b,0xff6ba6,0xffffb2}  /* finale: gold expanding fans */
 };
 unsigned sec=cafe_led_min(f.section,5),phase=f.phase&255u,beat=f.beat;
 unsigned energy=cafe_led_min(f.energy,255),bass=cafe_led_min(f.bass,255);
 unsigned high=cafe_led_min(f.high,255),hit=cafe_led_min(f.hit,255);
 unsigned pump=255-phase,half=phase>=128,step=beat*2u+half;
 unsigned presence=20+energy/5,impact=cafe_led_max(bass,hit);
 int accent=phase<76&&impact>60;
 tutorial_packet(eup,0,1,1);tutorial_cue_packet(sub,1);
 for(unsigned deck=0;deck<2;deck++){
  unsigned mirror=deck?7:0;
  for(unsigned pad=0;pad<8;pad++){
   unsigned pos=pad^mirror,head=step&7u,distance=(pos+8-head)&7u;
   unsigned level=presence/2,rgb=palettes[sec][deck];
   if(sec==0){
    unsigned glow=(beat+pad+deck*3u)&7u;
    level=18+energy/4+(glow<3?(pump*(3-glow))/6:0);
   }else if(sec==1){
    level=presence+(distance==0?125:distance==1?65:distance==2?25:0);
    if(distance==0)rgb=palettes[sec][2];
   }else if(sec==2){
    unsigned fill=1+(energy*7)/255;
    level=pos<fill?65+energy/3+pump/4:12;
    if(pos==head){level=225;rgb=palettes[sec][2];}
   }else if(sec==3){
    int active=((beat+deck)&1u)==0;
    level=presence+(active?(pump*2)/3:0)+(distance<2?45:0);
    if(accent&&(beat&3u)==0){level=205+impact/5;rgb=palettes[sec][2];}
   }else if(sec==4){
    unsigned wave=((beat*256u+phase)/3u+pos*32u+deck*96u)&511u;
    if(wave>255)wave=511-wave;
    level=12+wave/4+energy/6;
   }else{
    unsigned spread=(phase*5)/256;
    unsigned from_center=pad<4?3-pad:pad-4;
    level=presence+(from_center<=spread?125+pump/4:15);
    if((beat&3u)==3&&phase<110){level=220;rgb=palettes[sec][2];}
   }
   cafe_led_rgb(sub+deck*48u+16+pad*4,rgb,cafe_led_min(level,255));
  }
  for(unsigned mode=0;mode<4;mode++){
   int chosen=mode==((beat/2u+deck)&3u);
   unsigned level=chosen?95+pump/3+high/5:16+energy/10;
   if(sec==4)level=chosen?65+pump/6:9;
   cafe_led_rgb(sub+deck*48u+mode*4,palettes[sec][chosen?2:deck],level);
  }
  /* EUP deck byte offsets are mirrored at +4; all unnamed bits stay at
   * the launcher's normal baseline, especially byte 3's upper bit. */
  unsigned base=deck*4u;
  int side=((beat+deck)&1u)==0;
  cafe_led_bit(eup+base+1,0x80,side&&phase<160); /* SLIP */
  cafe_led_bit(eup+base+1,0x40,side&&accent);    /* SYNC */
  cafe_led_bit(eup+base+1,0x20,side);           /* MASTER */
  cafe_led_bit(eup+base+1,0x04,phase<80);       /* QUANTIZE */
  cafe_led_bit(eup+base+2,0x08,((beat/2u+deck)&1u)==0); /* VINYL */
  cafe_led_bit(eup+base+3,0x10,(step&3u)==deck);        /* RELOOP */
  cafe_led_bit(eup+base+3,0x08,sec>=2&&sec!=4&&accent); /* MASTER TEMPO */
  cafe_led_bit(eup+base+3,0x01,(step&3u)==(deck+2));    /* REVERSE */
  /* SUB common order: right at 98, left at 99, then IN/OUT/CUE/PLAY. */
  unsigned at=deck?98:99;
  cafe_led_bit(sub+at,0x80,side&&!half);
  cafe_led_bit(sub+at,0x40,side&&half);
  cafe_led_bit(sub+at,0x20,!side&&phase<128);
  cafe_led_bit(sub+at,0x10,side&&phase<160);
 }
 unsigned fx=0;
 for(unsigned i=0;i<6;i++){
  unsigned chase=(step+i)%6;
  int on=(sec==4)?chase<2:sec==2?i<=energy*5/255:chase<3;
  if((sec==3||sec==5)&&accent&&(beat&3u)==0)on=1;
  if(on)fx|=1u<<i;
 }
 eup[19]=(unsigned char)(fx|(accent?0x40:0)|(phase<96?0x80:0));
 cafe_led_bit(eup+10,0x20,(beat&1u)==0&&phase<170); /* CH1 headphone CUE */
 cafe_led_bit(eup+10,0x10,(beat&1u)!=0&&phase<170); /* CH2 headphone CUE */
 cafe_led_bit(eup+10,0x08,(beat&3u)==0&&accent);    /* MASTER headphone CUE */
 unsigned crc=led_crc_words(eup,5);eup[22]=(unsigned char)crc;eup[23]=(unsigned char)(crc>>8);
 crc=led_crc_words(sub,25);sub[102]=(unsigned char)crc;sub[103]=(unsigned char)(crc>>8);
}

struct cafe_show_led_state{
 uint64_t last_ns;
 int active;
 unsigned char eup[100],sub[104];
};
/* Returns EUP/SUB bits for the caller to send using its existing led_send().
 * now_ns is monotonic wall time, ms is the audible song position. A stopped,
 * completed or exited show restores both normal packets exactly once and
 * immediately, even if the most recent animated write was less than 50ms ago.
 * Call this every main-loop pass, independently of panel read availability. */
static unsigned cafe_show_led_update(struct cafe_show_led_state*s,int active,unsigned ms,uint64_t now_ns,unsigned char eup[100],unsigned char sub[104]){
 if(!active||ms>=CAFE_SHOW_DURATION_MS){
  if(!s->active)return 0;
  s->active=0;tutorial_packet(eup,0,1,1);tutorial_cue_packet(sub,1);
  memcpy(s->eup,eup,100);memcpy(s->sub,sub,104);
  return CAFE_SHOW_LED_EUP|CAFE_SHOW_LED_SUB;
 }
 int first=!s->active;
 if(!first&&now_ns-s->last_ns<CAFE_SHOW_LED_INTERVAL_NS)return 0;
 s->active=1;s->last_ns=now_ns;
 cafe_show_led_packets(eup,sub,cafe_show_frame(ms));
 unsigned changed=0;
 if(first||memcmp(s->eup,eup,100)){changed|=CAFE_SHOW_LED_EUP;memcpy(s->eup,eup,100);}
 if(first||memcmp(s->sub,sub,104)){changed|=CAFE_SHOW_LED_SUB;memcpy(s->sub,sub,104);}
 return changed;
}
#endif
