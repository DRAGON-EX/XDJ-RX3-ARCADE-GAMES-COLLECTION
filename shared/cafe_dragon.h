#ifndef RX3_CAFE_DRAGON_H
#define RX3_CAFE_DRAGON_H
#include "cafe_dragon_atlas.h"
/* Fixed sprite frames keep the reference design consistent during DJ gestures. */
static void cafe_dragon_at(int cx,int feet,int scale,unsigned ms,int greeting){
 unsigned phase=ms%12000,frame=0;
 if(greeting)frame=(ms/1100)%3==0?0:2+(ms/1100)%2;
 else if(phase<6000){static const unsigned char dj[]={0,6,0,7,0,4,0,5};frame=dj[phase/750];}
 else if(phase>7900&&phase<8070)frame=1;
 else if(phase>9500&&phase<10700)frame=2;
 else if(phase>=10700&&phase<11600)frame=3;
 for(int row=0;row<128;row++)for(int col=0;col<128;){
  unsigned color=cafe_dragon_pixels[frame][row*128+col];int end=col+1;
  while(end<128&&cafe_dragon_pixels[frame][row*128+end]==color)end++;
  if(color){int x0=col*scale/2,x1=end*scale/2,y0=row*scale/2,y1=(row+1)*scale/2;
   cr_rect(cx-32*scale+x0,feet-64*scale+y0,x1-x0,y1-y0,cafe_dragon_palette[frame][color]);}
  col=end;
 }
}
static void cafe_dragon(unsigned ms,int greeting){cafe_dragon_at(642,424,3,ms,greeting);}
#endif
