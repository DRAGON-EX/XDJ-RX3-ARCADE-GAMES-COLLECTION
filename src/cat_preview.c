#include <stdint.h>
#include "../native/font.h"
#include "../native/theme.h"
static uint32_t framebuffer[1280*800];
static void cr_rect(int x,int y,int w,int h,unsigned c){
 c=theme_color(c);uint32_t rgba=0xff000000u|((c&255)<<16)|(c&0xff00)|((c>>16)&255);
 if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x+w>1280)w=1280-x;if(y+h>800)h=800-y;
 for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)framebuffer[j*1280+i]=rgba;
}
static void cr_text(int x,int y,int scale,const char*s,unsigned c){
 for(;*s;s++,x+=6*scale){int n=*s>='0'&&*s<='9'?*s-'0':*s>='A'&&*s<='Z'?*s-'A'+10:-1;
  if(n>=0)for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(font[n][yy]&(1<<(4-xx)))cr_rect(x+xx*scale,y+yy*scale,scale,scale,c);
  if(*s=='\''||*s==',')cr_rect(x+2*scale,y+(*s==','?6:0)*scale,scale,scale,c);
  if(*s=='/')for(int i=0;i<5;i++)cr_rect(x+(4-i)*scale,y+(i+1)*scale,scale,scale,c);
  if(*s=='!'){cr_rect(x+2*scale,y,scale,5*scale,c);cr_rect(x+2*scale,y+6*scale,scale,scale,c);}
  if(*s=='?'){const unsigned char q[]={14,17,1,2,4,0,4};for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(q[yy]&(1<<(4-xx)))cr_rect(x+xx*scale,y+yy*scale,scale,scale,c);}
  if(*s=='&'){const unsigned char amp[]={12,18,20,8,21,18,13};for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(amp[yy]&(1<<(4-xx)))cr_rect(x+xx*scale,y+yy*scale,scale,scale,c);continue;}if(*s=='-')cr_rect(x,y+3*scale,5*scale,scale,c);if(*s=='.')cr_rect(x+2*scale,y+6*scale,scale,scale,c);
 }
}
#include "../shared/cat_scenes.h"
#include "../native/cafe_show_leds.h"
static unsigned char cafe_led_preview[204];
unsigned char*cats_leds(unsigned ms,int active){
 if(active)cafe_show_led_packets(cafe_led_preview,cafe_led_preview+100,cafe_show_frame(ms));
 else {tutorial_packet(cafe_led_preview,0,1,1);tutorial_cue_packet(cafe_led_preview+100,1);}
 return cafe_led_preview;
}
static struct cafe_state cafe;
void cats_open(unsigned seen,unsigned mask,int won,unsigned seed){cafe_open(&cafe,seen,mask,won,seed);}
void cats_cafe(unsigned ms,int neon){theme=neon;cat_cafe_scene(&cafe,ms);}
void cats_show(int active,unsigned ms){cafe.show_active=active!=0;cafe.show_ms=ms;}
int cats_vinyl_hit(int x,int y){return cafe_vinyl_hit(x,y);}
unsigned cats_seen(void){return cafe.seen;}
void cats_rescue(unsigned cat,unsigned ms,int neon){theme=neon;rescue_scene(cat,ms);}
uint32_t*cats_buffer(void){return framebuffer;}
