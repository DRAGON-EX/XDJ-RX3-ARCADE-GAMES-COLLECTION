#ifndef RX3_DW_CAFE_SPRITES_H
#define RX3_DW_CAFE_SPRITES_H
/* Reuse the cafe's exact indexed artwork at battle and celebration sizes. */
#ifndef CAFE_CHIBI_COUNT
#include "../../../shared/cafe_chibi_atlas.h"
#endif
#ifndef RX3_CAFE_DRAGON_H
#include "../../../shared/cafe_dragon_atlas.h"
#endif
static void dw_cafe_character(int x,int feet,int kind,int variant,int height,unsigned ticks,int action,int flip){
 if(height<1)return;
 unsigned ms=ticks*1000u/60u,frame=0;
 int bob=0;
 if(kind==0){
  if(action==3)frame=2+(ms/650)%2;
  else if(action==1){static const unsigned char pose[]={0,4,0,5};frame=pose[(ms/180)%4];bob=(ms/180)%2;}
  else frame=ms%6000>5600?1:0;
  int left=x-height/2,top=feet-height-bob;
  /* Sample only displayed rows/columns; tiny combat sprites stay cheap. */
  for(int dy=0;dy<height;dy++)for(int dx=0;dx<height;){
   int sx=dx*128/height,sy=dy*128/height;if(flip)sx=127-sx;
   unsigned color=cafe_dragon_pixels[frame][sy*128+sx];int end=dx+1;
   while(end<height){int nx=end*128/height;if(flip)nx=127-nx;if(cafe_dragon_pixels[frame][sy*128+nx]!=color)break;end++;}
   if(color)dw_rect(left+dx,top+dy,end-dx,1,cafe_dragon_palette[frame][color]);dx=end;
  }
 }else{
  static const unsigned ids[]={10,10,3,1,4};unsigned id=ids[variant>=0&&variant<5?variant:0];
  if(action==1){static const unsigned char gait[]={4,6,5,6};frame=gait[(ms/165)%4];bob=ms/165%2;}
  else if(action==3)frame=2+(ms/550)%2;
  else if((ms+id*193)%6900>4550&&(ms+id*193)%6900<4720)frame=1;
  unsigned sprite=id*CAFE_CHIBI_FRAMES+frame;
  int w=CAFE_CHIBI_W*height/CAFE_CHIBI_H,left=x-w/2,top=feet-height-bob;
  for(unsigned i=cafe_chibi_offsets[sprite];i<cafe_chibi_offsets[sprite+1];i++){
   const unsigned char*r=cafe_chibi_spans[i];int sx=flip?CAFE_CHIBI_W-r[0]-r[2]:r[0];
   int x0=sx*height/CAFE_CHIBI_H,x1=(sx+r[2])*height/CAFE_CHIBI_H;
   int y0=r[1]*height/CAFE_CHIBI_H,y1=(r[1]+1)*height/CAFE_CHIBI_H;
   if(x1>x0&&y1>y0)dw_rect(left+x0,top+y0,x1-x0,y1-y0,cafe_chibi_palette[sprite][r[3]]);
  }
 }
}
#endif
