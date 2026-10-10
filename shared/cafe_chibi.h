#ifndef RX3_CAFE_CHIBI_H
#define RX3_CAFE_CHIBI_H
#include "cafe_chibi_atlas.h"
static int chibi_bob(unsigned ms,unsigned period,int height){unsigned t=ms%period;return (int)((t<period/2?t:period-t)*2*height/period);}
/* Shared native/WASM sprite playback. All data is read-only packed colour runs:
 * no PNG decoding, textures, per-frame allocation or background polling. */
static void cafe_chibi(unsigned id,int x,int y,int s,unsigned ms,int action){
 if(id>=CAFE_CHIBI_COUNT||s<1)return;
 unsigned frame=0,phase=(ms+id*193)%6900;
 int walk=action==1||action==4,flip=action==4,bob=0;
 if(walk){static const unsigned char gait[]={4,6,5,6};frame=gait[(ms/165)%4];bob=(int)(ms/165%2);}
 else if(action==2){frame=7;bob=chibi_bob(ms,2700,1);}
 else if(action==3){frame=(ms/550)%3==0?0:2+(ms/550)%2;}
 else if(phase>5600&&phase<6450)frame=2+(phase/320)%2;
 else if(phase>4550&&phase<4720)frame=1;
 else bob=chibi_bob(ms+id*157,2400,1);
 unsigned sprite=id*CAFE_CHIBI_FRAMES+frame;
 int left=x-CAFE_CHIBI_W*s/4,top=y-CAFE_CHIBI_H*s/2-bob;
 for(unsigned i=cafe_chibi_offsets[sprite];i<cafe_chibi_offsets[sprite+1];i++){
  const unsigned char*r=cafe_chibi_spans[i];
  int sx=flip?CAFE_CHIBI_W-r[0]-r[2]:r[0];
  int x0=sx*s/2,x1=(sx+r[2])*s/2,y0=r[1]*s/2,y1=(r[1]+1)*s/2;
  cr_rect(left+x0,top+y0,x1-x0,y1-y0,cafe_chibi_palette[sprite][r[3]]);
 }
 if(action==2&&ms%4200<2400){int rise=(int)(ms%2400)/240;cr_text(x+15*s,y-26*s-rise,1,"Z",0xb9acc8);cr_text(x+20*s,y-30*s-rise,1,"Z",0x8fa1b5);}
}
#endif
