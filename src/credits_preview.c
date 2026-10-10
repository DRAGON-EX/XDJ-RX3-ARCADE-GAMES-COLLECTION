#include <stdint.h>
#include "../native/font.h"
#include "../native/theme.h"
static uint32_t framebuffer[1280*670];
static void cr_rect(int x,int y,int w,int h,unsigned c){
 c=theme_color(c);uint32_t rgba=0xff000000u|((c&255)<<16)|(c&0xff00)|((c>>16)&255);
 if(x<0){w+=x;x=0;}if(y<130){h+=y-130;y=130;}if(x+w>1280)w=1280-x;if(y+h>800)h=800-y;
 for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)framebuffer[(j-130)*1280+i]=rgba;
}
static void cr_text(int x,int y,int scale,const char*s,unsigned c){
 for(;*s;s++,x+=6*scale){int n=*s>='0'&&*s<='9'?*s-'0':*s>='A'&&*s<='Z'?*s-'A'+10:-1;
  if(n>=0)for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(font[n][yy]&(1<<(4-xx)))cr_rect(x+xx*scale,y+yy*scale,scale,scale,c);
  if(*s=='\''||*s==',')cr_rect(x+2*scale,y+(*s==','?6:0)*scale,scale,scale,c);
  if(*s=='/')for(int i=0;i<5;i++)cr_rect(x+(4-i)*scale,y+(i+1)*scale,scale,scale,c);
  if(*s=='?'){const unsigned char q[]={14,17,1,2,4,0,4};for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(q[yy]&(1<<(4-xx)))cr_rect(x+xx*scale,y+yy*scale,scale,scale,c);}
  if(*s=='&'){const unsigned char amp[]={12,18,20,8,21,18,13};for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(amp[yy]&(1<<(4-xx)))cr_rect(x+xx*scale,y+yy*scale,scale,scale,c);continue;}if(*s=='-')cr_rect(x,y+3*scale,5*scale,scale,c);if(*s=='.')cr_rect(x+2*scale,y+6*scale,scale,scale,c);
 }
}
#include "../shared/credits_scene.h"
#include "../shared/credits_confirm.h"
void credits_locked_frame(int neon){theme=neon;credits_locked_render();}
void credits_confirm_frame(int step,int neon){theme=neon;credits_confirm_render(step);}
void credits_frame(unsigned ms,int neon,int unlocked){theme=neon;credits_render(ms,unlocked);}
uint32_t*credits_buffer(void){return framebuffer;}

unsigned credits_duration(void){return CREDITS_DURATION_MS;}

unsigned credits_code_start(void){return CREDITS_GEAR_END_MS;}
unsigned credits_code_span(void){return CREDITS_CODE_END_MS-CREDITS_GEAR_END_MS;}

void rescue_locked_frame(unsigned mask,int neon){theme=neon;rescue_locked_render(mask);}
