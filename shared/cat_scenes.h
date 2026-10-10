#ifndef RX3_CAT_SCENES_H
#define RX3_CAT_SCENES_H
/* Hosts provide cr_rect/cr_text. The same scene is used on ARM and in WASM. */
#include "credits_scene.h"
#include "cafe_chibi.h"
#include "cafe_dragon.h"
#include "rescue_playback.h"
#include "cafe_state.h"
static const char*rescue_names[]={"YUKI","PEACH","COOKIE","MOCHI"};
static const unsigned rescue_sprite[]={10,3,1,4};
static const char*rescue_titles[]={"YAY! YOU RESCUED YUKI!","YAY! YOU RESCUED PEACH!","YAY! YOU RESCUED COOKIE!","YAY! YOU RESCUED MOCHI!"};
static const char*rescue_lines[]={"TEN LEVELS SMASHED. ONE DJ CAT FREED.","THREE WAVES ROCKED. PEACH IS BACK!","EIGHT ARENAS WON. COOKIE OWNS THE GRID!","WAVE FOUR! MOCHI IS CLEAR FOR LANDING."};
static const char*rescue_tails[]={"THE MUSIC PLAYER HAS A NEW RESIDENT.","SPACE IS LOUD. PURRING IS LOUDER.","NO MORE BAD DROPS. JUST GOOD COMPANY.","ALIENS OUT. PAWS UP. DROP THE BASS!"};
static int cat_bob(unsigned ms,unsigned period,int height){unsigned t=ms%period;return (int)((t<period/2?t:period-t)*2*height/period);}
static void rescue_scene(unsigned cat,unsigned ms){
 if(cat>=4)return;
 cr_rect(0,0,1280,800,0x080f1b);cr_rect(36,32,1208,736,0x10243c);
 cr_rect(36,32,1208,4,0x64ffe1);cr_rect(36,764,1208,4,0x64ffe1);
 cr_center(70,2,"CAT RESCUE SAGA - A LITTLE VICTORY DANCE",0x8fa1b5);
 cr_center(124,5,rescue_titles[cat],0xffcd55);
 /* A different victory backdrop for each arcade mission. */
 for(int i=0;i<24;i++){
  int x=92+(i*167)%1080,y=210+(i*59)%370;
  if(cat==0){int fall=(int)(ms%1500);y+=fall*fall/18000;cr_rect(x,y,28,12,i%2?0xffcd55:0x64ffe1);}
  else if(cat==1){x=(x+(int)(ms/12))%1120+45;cr_star(x,y,3+i%3,i%2?0xee8dd0:0x64e8ee);}
  else if(cat==2){x=(x+(int)(ms/5))%1100+60;cr_rect(x,y,48,3,i%2?0x64ffe1:0xee8dd0);cr_rect(x+46,y-4,8,10,0xffffff);}
  else{y=210+(y+(int)(ms/9))%330;cr_rect(x,y,4,19,0x64ffe1);cr_star(x+28,y,3,0xffcd55);}
 }
 cr_rect(155,599,970,6,0x64ffe1);cr_rect(170,606,940,20,0x14283d);
 int entrance=ms<800?(int)(800-ms)/3:0;
 cafe_dragon_at(390-entrance,597,5,ms,1);
 int bob=cat_bob(ms+100,600,18);cafe_chibi(rescue_sprite[cat],850+entrance,599-bob,6,ms,3);
 for(int i=0;i<4;i++)cr_heart(580+i*30,330-(int)((ms/10+i*47)%130),2,i%2?0xee8dd0:0xff405c);
 cr_center(659,3,rescue_lines[cat],0xffffff);cr_center(710,2,rescue_tails[cat],0x64ffe1);
 cr_rect(100,748,(int)((ms<20000?ms:20000)*1080/20000),3,0xffcd55);
}
static void cafe_table(int x,int y,unsigned ms){
 cr_rect(x,y,170,10,0xa76842);cr_rect(x+76,y+10,16,85,0x422533);cr_rect(x+40,y+90,92,7,0x71535a);
 cr_rect(x+110,y-29,25,28,0xffefd4);cr_rect(x+135,y-23,8,17,0xffefd4);
 for(int i=0;i<3;i++)cr_rect(x+114+i*8+cat_bob(ms+i*200,1400,4),y-42-(int)((ms/70+i*9)%24),2,9,0x8fa1b5);
}
static void cafe_fireworks(unsigned ms){
 static const signed char dx[]={100,87,50,0,-50,-87,-100,-87,-50,0,50,87};
 static const signed char dy[]={0,50,87,100,87,50,0,-50,-87,-100,-87,-50};
 for(unsigned b=0;b<5;b++){unsigned t=(ms+b*437)%2700,cycle=(ms+b*437)/2700;
  int cx=100+(b*241+cycle*97)%1080,cy=350+(b*71)%220;unsigned color=b%3==0?0xffcd55:b%3==1?0xee8dd0:0x64e8ee;
  if(t<600){cr_rect(cx,cy+150-(int)t/4,3,11,color);continue;}if(t>2050)continue;
  int age=(int)t-600,r=age*95/1450,fall=age*age/35000;
  for(int k=0;k<12;k++){int x=cx+dx[k]*r/100,y=cy+dy[k]*r/100+fall;cr_rect(x,y,4,4,color);if(age<850)cr_rect(x-dx[k]/14,y-dy[k]/14,2,2,0xffefd4);}
 }
}
#include "cafe_room.h"
static void cat_cafe_scene(struct cafe_state*c,unsigned ms){cafe_room(c,ms);}
#endif
