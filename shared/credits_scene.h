#ifndef RX3_CREDITS_SCENE_H
#define RX3_CREDITS_SCENE_H
/* Portable pixel scene. Host supplies cr_rect and cr_text; no heap or image files. */
#include "credits_data.h"
static int cr_len(const char*s){int n=0;while(s[n])n++;return n;}
static void cr_center(int y,int scale,const char*s,unsigned c){cr_text((1280-cr_len(s)*6*scale)/2,y,scale,s,c);}
static int cr_wave(unsigned t,int range){int p=(int)(t%1000);return (p<500?p:1000-p)*range/500;}
static void cr_heart(int x,int y,int s,unsigned c){
 const char*rows[]={"0110110","1111111","1111111","0111110","0011100","0001000"};
 for(int j=0;j<6;j++)for(int i=0;i<7;i++)if(rows[j][i]=='1')cr_rect(x+i*s,y+j*s,s,s,c);
}
static void cr_star(int x,int y,int s,unsigned c){cr_rect(x,y-s,s,3*s,c);cr_rect(x-s,y,3*s,s,c);}
static void cr_disc(int x,int y,int r,unsigned c){for(int yy=-r;yy<=r;yy++){int xx=0;while((xx+1)*(xx+1)+yy*yy<=r*r)xx++;cr_rect(x-xx,y+yy,2*xx+1,1,c);}}
#include "cafe_chibi.h"
#include "cafe_dragon.h"
/* Integer polygon helper retained for the suspended credits robot. */
static void cr_dragon_shape(int x,int y,int s,const signed char*points,int count,unsigned color){
 int low=40,high=0;for(int i=0;i<count;i++){int py=points[2*i+1];if(py<low)low=py;if(py>high)high=py;}
 for(int row=low;row<high;row++){
  int hits[16],n=0;
  for(int i=0,j=count-1;i<count;j=i++){
   int ax=points[2*j],ay=points[2*j+1],bx=points[2*i],by=points[2*i+1];
   if((ay<=row&&by>row)||(by<=row&&ay>row))hits[n++]=ax+(row-ay)*(bx-ax)/(by-ay);
  }
  for(int i=1;i<n;i++){int v=hits[i],j=i;while(j&&hits[j-1]>v){hits[j]=hits[j-1];j--;}hits[j]=v;}
  for(int i=0;i+1<n;i+=2)cr_rect(x+hits[i]*s,y+row*s,(hits[i+1]-hits[i]+1)*s,s,color);
 }
}
#include "credits_robot.h"
static void cr_deck(int x,int y,int w,unsigned t){
 cr_rect(x,y,w,82,0x100e1a);cr_rect(x+5,y+6,w-10,67,0x243b50);cr_rect(x+5,y+6,w-10,2,0x64ffe1);
 cr_disc(x+46,y+38,25,0x090709);cr_disc(x+w-46,y+38,25,0x090709);cr_disc(x+46,y+38,17,0x64e8ee);cr_disc(x+w-46,y+38,17,0xee8dd0);cr_disc(x+46,y+38,12,0x100e1a);cr_disc(x+w-46,y+38,12,0x100e1a);
 int phase=(t/150)%4;cr_rect(x+44+(phase==0?12:phase==2?-12:0),y+36+(phase==1?12:phase==3?-12:0),4,4,0xffffff);
 for(int k=0;k<5;k++){cr_rect(x+w/2-29+k*13,y+20,3,36,0x100e1a);cr_rect(x+w/2-32+k*13,y+26+(k*7)%22,9,4,0xffcd55);}
 for(int k=0;k<4;k++){cr_rect(x+15+k*17,y+65,12,5,0x64ffe1);cr_rect(x+w-80+k*17,y+65,12,5,0xee8dd0);}
}
static void cr_speaker(int x,int y,unsigned t){cr_rect(x,y,120,165,0x100e1a);cr_rect(x+5,y+5,110,155,0x243b50);cr_disc(x+60,y+45,28,0x100e1a);cr_disc(x+60,y+45,12,0x8fa1b5);cr_disc(x+60,y+113,41,0x100e1a);cr_disc(x+60,y+113,22+cr_wave(t,3),0x64ffe1);cr_disc(x+60,y+113,14,0x100e1a);}
static void cr_chest(unsigned t,int open){
 int x=554,y=475;cr_rect(x,y,172,100,0x9e563c);cr_rect(x+8,y+8,156,84,0x422533);cr_rect(x+16,y+12,140,75,0xa76842);
 cr_rect(x,y+43,172,8,0xffcd55);cr_rect(x+75,y+35,22,24,0xffcd55);
 if(open){cr_rect(x,y-32,172,28,0xffcd55);for(int i=0;i<8;i++)cr_star(552+i*24,440-(int)((t/12+i*31)%100),3,i%2?0xffcd55:0x64ffe1);cr_heart(619,425,6,0xff405c);}
}
static void credits_render(unsigned ms,int unlocked){
 if(ms>CREDITS_DURATION_MS+10000)ms=CREDITS_DURATION_MS+10000+(ms-CREDITS_DURATION_MS-10000)%10000;
 const unsigned dj_count=sizeof(credits_djs)/sizeof(*credits_djs),dj_span=CREDITS_DJ_END_MS-CREDITS_SCROLL_END_MS;
 unsigned dj_index=ms>=CREDITS_SCROLL_END_MS&&ms<CREDITS_DJ_END_MS?(ms-CREDITS_SCROLL_END_MS)*dj_count/dj_span:0;
 int code_scene=ms>=CREDITS_GEAR_END_MS&&ms<CREDITS_CODE_END_MS;
 int tribute=ms>=CREDITS_SCROLL_END_MS&&ms<CREDITS_DJ_END_MS&&dj_index==1;unsigned beat=ms;
 cr_rect(0,130,1280,670,0x080f1b);
 cr_rect(20,150,1240,3,0x64ffe1);cr_text(48,178,3,"THE AFTERPARTY - CREDITS",0x64ffe1);
 cr_rect(1090,168,140,46,0x763044);cr_text(1110,183,2,"CLOSE",0xffffff);
 for(int i=0;i<45;i++){int x=22+(i*197)%1230,y=235+(i*89)%410;unsigned c=((ms/650+i)%5)?0x243b50:0x8fa1b5;if(tribute)c=((ms/350+i)%3)?0x8fa1b5:0xffffff;cr_rect(x,y,2,2,c);}
 /* Calm center stage, speaker stacks and an illuminated dance floor. */
 cr_rect(0,676,1280,96,0x10243c);for(int x=0;x<1280;x+=80)cr_rect(x,677,1,94,0x243b50);for(int y=684;y<770;y+=28)cr_rect(0,y,1280,1,0x243b50);
 if(!tribute)for(int i=0;i<12;i++){int h=12+(int)((ms/45+i*19)%52);cr_rect(24+i*17,670-h,11,h,0x64ffe1);cr_rect(1045+i*17,670-h,11,h,0xee8dd0);}
 cr_speaker(42,483,beat);cr_speaker(1118,483,beat);
 cafe_chibi(1,224,540,2,beat,ms>94000?3:0);if(!code_scene)cafe_chibi(4,1168,496,2,beat,2);
 cafe_dragon_at(114,405,3,beat,0);cr_deck(20,345,184,beat);
 if(code_scene)cr_robot(1020,235,4,beat);else cafe_chibi(3,1147,389,3,beat,3);
 /* A mischievous kitten follows a moving spot along the bottom edge. */
 int chase=(int)((ms/7)%1700)-200;int catx=chase<1230?chase:2460-chase;
 cafe_chibi(10,catx+32,764,2,beat,chase<1230?4:1);cr_disc(catx+92,754,3,0xff405c);
 if(ms<6000){
  cr_center(282,3,"ONE LAST TRACK",0x8fa1b5);cr_center(336,5,"BEFORE YOU GO",0xffffff);
  cr_center(404,2,ms<2500?"POWERING UP THE CAT CREW":"PAWS ON PLAY - HEARTS ON BEAT",0x64ffe1);
  cafe_dragon_at(548,610,3,beat,0);cafe_chibi(10,720,601,3,beat,3);cr_deck(450,585,380,beat);
 }else if(ms<CREDITS_SCROLL_END_MS){
  unsigned n=sizeof(credits_lines)/sizeof(*credits_lines);int distance=440;
  for(unsigned i=0;i<n;i++){int advance=7*credits_lines[i].scale+22;distance+=advance>40?advance:40;}
  int base=665-(int)((unsigned long long)(ms-6000)*distance/(CREDITS_SCROLL_END_MS-6000));
  int y=base;
  for(unsigned i=0;i<n;i++){const struct credits_line*l=&credits_lines[i];if(y>=242&&y+7*l->scale<662)cr_center(y,l->scale,l->value,l->accent?0x64ffe1:0xffffff);int advance=7*l->scale+22;y+=advance>40?advance:40;}
 }else if(ms<CREDITS_DJ_END_MS){
  unsigned index=dj_index,local=ms-CREDITS_SCROLL_END_MS-index*dj_span/dj_count;
  cr_center(269,2,tribute?"FOREVER IN OUR HEARTS":"THANKS FOR THE INSPIRATION FOR BECOMING A DJ",0x8fa1b5);
  int scale=cr_len(credits_djs[index])>18?4:5;
  int slide=local<400?(400-(int)local)/8:0;
  cr_rect(265+slide,330,750,94,0x14283d);cr_center(362+slide,scale,credits_djs[index],0xffffff);
  if(tribute){cr_heart(606,470,10,0xff405c);cr_center(580,2,"YOUR MELODIES STAY WITH US",0xffffff);}
  else{cafe_chibi(3,536,609,3,beat,3);cafe_chibi(4,740,609,3,beat+300,3);for(int j=0;j<18;j++)cr_rect(300+(j*71)%680,250+(int)((local/9+j*33)%380),4,8,j%2?0xffcd55:0xee8dd0);}
 }else if(ms<CREDITS_GEAR_END_MS){
  unsigned local=ms-CREDITS_DJ_END_MS;
  cr_rect(280,242,720,420,0x14283d);cr_rect(280,242,720,3,0x64ffe1);
  cr_center(264,3,"THANK YOU,",0x64ffe1);cr_center(303,3,"PIONEER DJ / ALPHATHETA.",0xffffff);
  for(unsigned i=0;i<sizeof(credits_gear_thanks)/sizeof(*credits_gear_thanks);i++)cr_center(355+(int)i*31,3,credits_gear_thanks[i],0xffffff);
  if(local>=11000){cr_center(591,6,"PRICELESS.",0xffcd55);cr_heart(317,596,5,0xff405c);cr_heart(928,596,5,0xff405c);}
 }else if(ms<CREDITS_CODE_END_MS){
  cr_rect(280,242,720,420,0x14283d);cr_rect(280,242,720,3,0x64ffe1);
  for(unsigned i=0;i<8;i++)cr_center(266+(int)i*32,3,credits_code_thanks[i],i<3?0x64ffe1:0xffffff);
  cr_center(556,4,credits_code_thanks[8],0xffcd55);
  cr_center(614,3,credits_code_thanks[9],0xffffff);
 }else if(ms<CREDITS_GAG_END_MS){
  unsigned local=ms-CREDITS_CODE_END_MS;int pull=local<1200?(1200-(int)local)/5:0;
  cr_rect(250+pull,265,780,214,0x14283d);cr_center(304,5,"EXCEPT DEADMAU5.",0xffffff);cr_center(369,3,"THE CATS HAVE VOTED.",0x64ffe1);
  if(local>=1800)cr_center(430,3,"NO MICE IN THIS CLUB.",0xffcd55);
  cafe_chibi(1,610+pull,659,4,beat,3);cr_text(800,565,2,"MEOW.",0xee8dd0);
 }else if(ms<CREDITS_DURATION_MS){
  cr_center(267,5,"THANKS FOR PLAYING",0xffffff);cr_center(325,4,"DJ DRAGON-X",0x64ffe1);cr_center(388,2,"BUILT FOR FUN - SHARED WITH LOVE",0x8fa1b5);
  cr_chest(ms,0);cafe_chibi(10,369,613,3,beat,3);cafe_chibi(3,900,613,3,beat,3);
  cr_center(625,2,"A LITTLE SOMETHING FOR STAYING...",0xffcd55);
 }else{
  cr_center(274,4,unlocked?"SECRET SONG UNLOCKED":"THANK YOU FOR WATCHING",0xffcd55);
  cr_center(340,3,unlocked?"THAT GAME ARCADE - MOODMODE":"THE CAT CREW LOVES YOU",0xffffff);
  cr_chest(ms,1);cafe_chibi(10,369,613,3,beat,3);cafe_chibi(3,900,613,3,beat,3);
  cr_rect(442,615,396,48,0x64ffe1);cr_text(478,631,3,"OPEN MUSIC PLAYER",0x080f1b);
 }
 cr_rect(40,781,1200,4,0x243b50);cr_rect(40,781,(int)((unsigned long long)(ms<CREDITS_DURATION_MS?ms:CREDITS_DURATION_MS)*1200/CREDITS_DURATION_MS),4,0x64ffe1);
}
#endif
