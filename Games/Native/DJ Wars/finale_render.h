#ifndef RX3_DW_FINALE_RENDER_H
#define RX3_DW_FINALE_RENDER_H
/* The rescue stays on the real, destructible battlefield. No second stage. */
static void dw_finale_word(int center,int y,int scale,const char*t,unsigned color){
 int n=0;while(t[n])n++;int left=center-n*3*scale;dw_text(left,y,scale,t,color);
 for(int i=0;i<n;i++){
  int x=left+i*6*scale;
  if(t[i]=='!'){dw_rect(x+2*scale,y,scale,5*scale,color);dw_rect(x+2*scale,y+6*scale,scale,scale,color);}
  else if(t[i]==','){dw_rect(x+2*scale,y+5*scale,scale,scale,color);dw_rect(x+scale,y+6*scale,scale,scale,color);}
  else if(t[i]=='?'){static const unsigned char q[]={14,17,1,2,4,0,4};for(int row=0;row<7;row++)for(int col=0;col<5;col++)if(q[row]&(1<<(4-col)))dw_rect(x+col*scale,y+row*scale,scale,scale,color);}
 }
}
static void dw_finale_star(int x,int y,int size,unsigned c){dw_rect(x-size,y-1,size*2+1,3,c);dw_rect(x-1,y-size,3,size*2+1,c);}
/* Each shell is a deterministic little show: no particles are allocated or
   simulated. At most nine shells overlap, with short pixel trails per spark. */
static unsigned dw_firework_shade(unsigned c,int n,int d){return (((c>>16)*n/d)<<16)|((((c>>8)&255)*n/d)<<8)|((c&255)*n/d);}
static void dw_firework_spark(int x,int y,int tailx,int taily,unsigned c,int bright){
 dw_line(tailx,taily,x,y,3,dw_firework_shade(c,2,5));
 dw_line(tailx,taily,x,y,1,c);
 dw_rect(x-1,y-1,3,3,bright?0xfff1c9:c);
 if(bright){dw_rect(x-3,y,7,1,c);dw_rect(x,y-3,1,7,c);dw_rect(x,y,1,1,0xffffff);}
}
static void dw_firework_reflection(int x,int water,unsigned color,int age,unsigned seed){
 if(water<70)water=70;if(water>=760)return;
 for(int row=0;row<13;row++){
  int y=water+4+row*6;if(y>766)break;
  int wave=(int)(dw_sin(age*3+row*37+(int)seed)*10),w=(62-row*3)+(age%13);
  unsigned c=dw_firework_shade(color,row<4?2:1,row<4?5:4);
  dw_rect(x-w/2+wave,y,w,1+(row%3==0),c);
  if(row<6)dw_rect(x+wave-4,y,8,1,dw_firework_shade(color,3,5));
 }
}
static void dw_firework_shell(int age,int x,int y,int radius,int kind,unsigned color,unsigned seed,int water){
 if(age<0||age>159)return;
 const int climb=38,base=water>650&&water<770?water-10:690;
 if(age<climb){
  /* Rising rockets ease near their apex, leaving a broken incandescent trail. */
  int prevx=x,prevy=base;
  for(int trail=8;trail>=0;trail--){
   int a=age-trail*2;if(a<0)continue;
   int ease=a*(climb*2-a),yy=base-(base-y)*ease/(climb*climb);
   int xx=x+(int)(dw_sin(a*3+(int)seed)*5)+(climb-a)*((int)(seed%3)-1)/3;
   if(trail<8)dw_line(prevx,prevy,xx,yy,trail>4?1:2,trail>4?0x77543b:trail>1?0xe5a558:0xffeeb5);
   if(trail<3)dw_rect(xx-1,yy-2,3,4,0xffe8ad);
   prevx=xx;prevy=yy;
  }
  dw_rect(prevx-2,prevy-3,5,6,0xffce76);dw_rect(prevx-1,prevy-2,3,4,0xffffff);
  if(age>15)dw_firework_reflection(x,water,color,age,seed);
  return;
 }
 int a=age-climb,tail=a>7?a-7:0,fall=a*a/(kind==2?76:170),tailfall=tail*tail/(kind==2?76:170);
 int reach=radius*a*5/(4*(a+22)),oldreach=radius*tail*5/(4*(tail+22));
 int count=kind==2?18:kind==1?32:kind==3?40:28;
 for(int ring=0;ring<(kind==0?2:1);ring++)for(int j=0;j<count;j++){
  unsigned key=seed+j*37u+ring*101u;int angle=kind==2?136+j*112/(count-1):j*256/count+(int)(seed%17)+ring*5;
  int variation=kind==1?100:78+(int)(key%29),r=reach*variation/100,tr=oldreach*variation/100;
  if(ring){r=r*3/5;tr=tr*3/5;}
  int xx=x+(int)(dw_sin(angle+64)*r),yy=y+(int)(dw_sin(angle)*r)+fall;
  int tx=x+(int)(dw_sin(angle+64)*tr),ty=y+(int)(dw_sin(angle)*tr)+tailfall;
  if(kind==1){yy=y+(yy-y)*3/4;ty=y+(ty-y)*3/4;}
  if(a>75&&((key+a/3)%4==0))continue;
  unsigned c=a>78?0xc99357:ring?0xffd999:color;
  int bright=a<10||(a>36&&(key+(unsigned)a/3)%11==0);
  dw_firework_spark(xx,yy,tx,ty,c,bright);
  if(a>19&&a<100){int dot=a-14,dr=radius*dot*5/(4*(dot+22))*variation/100;if(ring)dr=dr*3/5;
   int dx=x+(int)(dw_sin(angle+64)*dr),dy=y+(int)(dw_sin(angle)*dr)+dot*dot/(kind==2?76:170);
   if(kind==1)dy=y+(dy-y)*3/4;
   dw_rect(dx,dy,2,2,(key+(unsigned)a)%3?dw_firework_shade(c,3,5):0xffcd79);
  }
 }
 /* Ring shells have a second golden starburst inside their colored crown. */
 if(kind==1&&a>=17&&a<80){int r=(a-17)*2;for(int j=0;j<12;j++){
  int angle=j*256/12+11;int xx=x+(int)(dw_sin(angle+64)*r),yy=y+(int)(dw_sin(angle)*r)+fall/2;
  dw_firework_spark(xx,yy,x+(int)(dw_sin(angle+64)*(r>9?r-9:0)),y+(int)(dw_sin(angle)*(r>9?r-9:0))+fall/2,0xffd58a,(j+a)%9==0);
 }}
 /* Short core flashes are shaped stars, never a full-screen white flash. */
 if(a<11){int r=14-a;dw_disk(x,y,r+4,dw_firework_shade(color,1,3));dw_finale_star(x,y,r,color);dw_finale_star(x,y,r/2+1,0xfff5d7);}
 if(a>47&&a<105)for(int j=0;j<8;j++){
  int angle=j*32+(int)seed,rr=radius*3/4,xx=x+(int)(dw_sin(angle+64)*rr),yy=y+(int)(dw_sin(angle)*rr)+(a-42)*(a-42)/65;
  if((a+j*3)%19<4)dw_finale_star(xx,yy,3,0xffedb4);else dw_rect(xx,yy,2,2,0xb48a56);
 }
 dw_firework_reflection(x,water,color,a,seed);
}
static void dw_finale_fireworks(unsigned ticks){
 struct shell{short start,x,y,radius;unsigned char kind,color;};
 static const unsigned palette[]={0x87f7e0,0xff92c6,0xb7a0ff,0xffd78a,0x94c9ff,0xffad7c};
 /* Ascending volleys, alternating colored crowns, then a golden canopy. */
 static const struct shell shells[]={
  {0,515,285,116,0,0},{14,1070,290,122,0,1},{42,790,225,141,1,2},
  {104,630,355,135,2,3},{123,965,335,135,2,3},
  {178,490,268,113,1,4},{198,1090,250,123,1,1},{218,795,342,148,0,0},
  {276,590,238,138,0,2},{293,1015,245,140,0,5},{316,795,355,151,1,3},
  {364,482,342,116,2,0},{380,1110,328,122,2,1},{404,790,233,157,0,4},
  {459,590,302,135,1,2},{478,1020,300,136,1,0},
  {520,480,278,119,3,3},{533,1105,276,126,3,3},{548,675,232,140,3,3},
  {560,930,235,148,3,3},{578,798,337,165,1,3},
  {604,535,360,128,2,3},{614,1060,358,133,2,3},{634,790,213,170,3,3},
  {698,505,285,133,0,0},{721,1080,280,138,0,1},{758,790,225,160,1,2},
  {812,600,340,142,2,4},{835,995,330,145,2,5},
  {892,480,260,135,1,1},{914,1110,248,140,1,0},{947,800,300,165,0,2},
  {998,520,290,148,3,3},{1015,1080,280,148,3,3},
  {1052,660,230,160,3,3},{1070,945,238,160,3,3},{1100,795,320,178,1,3},
  {1118,535,360,142,2,3},{1132,1060,358,146,2,3},{1146,790,213,178,3,3}
 };
 /* Two opening cycles keep the sky active, then the full closing sequence. */
 unsigned elapsed=ticks%DW_FIREWORKS_TICKS;unsigned time=elapsed<1500?elapsed%750:elapsed-1500;int water=dw_screen_y(DW_H-24);
 for(unsigned i=0;i<sizeof(shells)/sizeof(shells[0]);i++){
  const struct shell*s=&shells[i];dw_firework_shell((int)time-s->start,s->x,s->y,s->radius,s->kind,palette[s->color],i*29u+7u,water);
 }
 /* Gold fountains frame the final volley while leaving the center map open. */
 if(time>=535)for(int side=0;side<2;side++)for(int j=0;j<12;j++){
  int age=((int)time-535+j*11)%76,x=side?1211:390,base=water<760?water-4:710;
  int xx=x+(side?-1:1)*age*(j%3+2)/3,yy=base-age*(6+j%3)+age*age/13;
  dw_firework_spark(xx,yy,xx+(side?2:-2),yy+5,j%3?0xeaba70:0xffeec0,(age+j)%21<3);
 }
}
#include "../../../shared/dj_master_atlas.h"
static void dw_finale_master_size_facing(int x,int feet,int height,unsigned tick,int pose,int flip){
 unsigned frame=(unsigned)pose%4;int top=feet-height;
 if(pose==0)top-=(int)(dw_sin(tick*2)*2);
 for(int dy=0;dy<height;dy++)for(int dx=0;dx<height;){
  int sx=dx*128/height,sy=dy*128/height;unsigned c=dj_master_pixels[frame][sy*128+(flip?127-sx:sx)];int end=dx+1;
  while(end<height&&dj_master_pixels[frame][sy*128+(flip?127-end*128/height:end*128/height)]==c)end++;
  if(c)dw_rect(x-height/2+dx,top+dy,end-dx,1,dj_master_palette[frame][c]);dx=end;
 }
}
static void dw_finale_master_size(int x,int feet,int height,unsigned tick,int pose){dw_finale_master_size_facing(x,feet,height,tick,pose,0);}
static void dw_finale_master(int x,int feet,unsigned tick,int pose){dw_finale_master_size(x,feet,214,tick,pose);}
static unsigned dw_finale_party_clock(const struct dw_game*g){
 unsigned elapsed=g->ticks-g->finale_started,offset=g->finale_assisted?DW_CELEBRATE_TICK:0;
 return elapsed>offset?elapsed-offset:0;
}
static void dw_finale_defeated_helmet(int x,int y,int size,unsigned clock){
 /* The discarded boss helmet has rolled onto one side: dead LED eyes, a
    split shell and a broken ear. It is unmistakably the defeated mouse. */
 int r=size*62/100,ex=size*54/100,ey=size*39/100;
 dw_rect(x-r,y+r-3,r*2,9,0x11131e);dw_rect(x-r*3/4,y+r+6,r*3/2,4,0x171323);
 dw_disk(x-ex,y-ey,size*35/100,0x101119);dw_disk(x-ex,y-ey,size*29/100,0x37303e);
 dw_disk(x+ex,y-ey/4,size*31/100,0x101119);dw_disk(x+ex,y-ey/4,size*25/100,0x3d303c);
 dw_disk(x,y,r+4,0x0c1018);dw_disk(x,y,r,0x42343f);dw_disk(x+4,y+5,r-6,0x25232f);
 dw_rect(x-r/2,y-r+12,r/2,4,0x685163);dw_rect(x-r+9,y-r/3,4,r/3,0x584457);
 unsigned led=clock<100?0xbc4b65:0x6e4659;
 for(int side=-1;side<=1;side+=2){int xx=x+side*r/3,yy=y-r/8+side*5;
  dw_line(xx-8,yy-8,xx+8,yy+8,3,led);dw_line(xx-8,yy+8,xx+8,yy-8,3,led);
 }
 for(int j=0;j<7;j++)dw_rect(x-r/2+j*r/7,y+r/3+j/2,5,3,j%2?0x56404d:0x725363);
 dw_line(x+5,y-r+3,x-9,y-r/3,3,0x0b0d15);dw_line(x-9,y-r/3,x+7,y+1,3,0x0b0d15);dw_line(x+7,y+1,x-3,y+r-7,3,0x0b0d15);
 dw_line(x+9,y-r+3,x-4,y-r/3,1,0x9c7182);dw_line(x-3,y+3,x+14,y+15,1,0x9c7182);
 dw_tri(x+ex+9,y-ey/4-26,x+ex+26,y-ey/4-9,x+ex+10,y-ey/4-5,0x14121b);
 /* A snapped audio cable and dark red power LED finish the defeated prop. */
 dw_line(x+r-3,y+r/2,x+r+17,y+r/2+10,3,0x0b0e16);dw_line(x+r+17,y+r/2+10,x+r+23,y+r/2+4,3,0x67515e);
 dw_rect(x-r/2,y+r/2,3,3,clock<75&&clock%14<7?0xff4368:0x643044);
}
static void dw_finale_reunion(const struct dw_game*g,unsigned clock){
 int reveal=clock<180?0:clock<235?(int)(clock-180)*100/55:100;
 if(reveal){
  /* A slim foreground club ledge grounds the reunion above the water. */
  dw_rect(360,718,880,51,0x151823);dw_rect(360,718,880,2,0x705c69);
  dw_rect(360,721,880,2,0x393142);dw_rect(360,765,880,4,0x090e18);
  for(int x=375;x<1240;x+=82){dw_line(x,724,x-20,764,1,0x25232f);dw_rect(x+7,762,36,1,0x383044);}
 }
 int bx=800-reveal*335/100,by=480+reveal*173/100,bs=145-reveal*55/100;
 dw_finale_defeated_helmet(bx,by,bs,clock);
 if(!reveal){
  dw_finale_word(800,630,3,"THE MOUSE IS DOWN",0xffb4c9);
  dw_finale_word(800,666,2,"NO ENCORE. NO REFUNDS.",0xa9bac9);
  return;
 }
 dw_finale_word(458,741,1,dw_names[9],0xb994a7);dw_finale_word(458,756,1,"OFFLINE",0x836d86);
 static const int xs[]={570,690,956,1095},sizes[]={143,151,145,153};
 /* Foreground reunion portraits retain the map and its real units beneath;
    they never relocate, revive or otherwise mutate the combat simulation. */
 int settle=(100-reveal)*2,dragonY=718-settle/2;
 dw_rect(759,724,124,9,0x11141f);dw_rect(775,733,92,4,0x191725);
 dw_cafe_character(821,dragonY,0,0,214,g->ticks,3,0);
 dw_finale_word(821,749,1,"DJ-DRAGONX",0xffd296);
 for(int i=0;i<4;i++){
  int x=xs[i]+(i<2?-settle:settle),hop=0;
  unsigned phase=(clock+i*31)%138;if(phase<38)hop=(int)(dw_sin(phase*128/38)*12);
  int feet=718-hop;
  dw_rect(x-40,725,80,7,0x13131f);dw_rect(x-27,732,54,3,0x191726);
  dw_cafe_character(x,feet,1,i+1,sizes[i],g->ticks+i*23,3,i>=2);
  dw_finale_word(x,749,1,dw_names[i+1],i==0?0xffd7a5:i==1?0xffb9c7:i==2?0xe3c88f:0xb7e9d2);
 }
 if(g->finale_assisted&&reveal==100){dw_finale_master_size(1188,531,80,g->ticks,3);}
 if(clock>=265){
  unsigned who=((clock-265)/102)%4;static const char*lines[]={"NO MORE MOUSE SETS!","NOW THAT IS A RESCUE!","SOMEBODY ORDER TUNA?","FINALLY. A DECENT DJ."};
  const char*text=lines[who];int n=0;while(text[n])n++;int w=n*12+24,left=xs[who]-w/2;
  if(left<382)left=382;if(left+w>1224)left=1224-w;
  int top=488;dw_rect(left,top,w,47,0x302333);dw_rect(left+3,top+3,w-6,41,0xffe4be);
  dw_tri(xs[who]-7,top+45,xs[who]+8,top+45,xs[who],top+58,0xffe4be);
  dw_finale_word(left+w/2,top+16,2,text,0x493543);
 }
}
static void dw_finale_world(const struct dw_game*g){
 int t=g->finale_tick,div=dw_divisor(),celebrating=!g->finale_assisted||t>=DW_CELEBRATE_TICK;
 dw_clip_world=1;
 if(celebrating){
  /* A light, constant 25-percent dim keeps the original terrain recognizable
     and makes the foreground crew readable without an opaque stage/card. */
  for(int yy=145;yy<770;yy++)for(int xx=360;xx<1240;xx++){unsigned c=dw_pixels[yy*1280+xx];dw_pixels[yy*1280+xx]=((c&0xfefefe)>>1)+((c&0xfcfcfc)>>2);}
 }
 if(g->finale_assisted){
  int x=dw_screen_x(g->finale_x),y=dw_screen_y(g->finale_y),height=76/div;
  int pose=celebrating?3:t>=DW_BARRAGE_TICK?2:t>=120&&t<DW_REVIVE_TICK?1:0;
  int focus=dw_finale_focus_target(g),flip=focus>=0&&g->finale_x>g->unit[focus].x;
  dw_finale_master_size_facing(x,y,height,g->ticks,pose,flip);
  /* A glowing hover step makes the fast rescue flight readable over the terrain. */
  dw_line(x-19/div,y+1,x+19/div,y+1,2,0x7ae7dc);
  for(int k=0;k<3;k++)dw_rect(x+(k-1)*9/div,y+5+(int)(g->ticks+k*7)%11,2,2,0x9d94e4);
  if(dw_finale_beam_target(g)>=0){
   const struct dw_unit*u=&g->unit[g->finale_target];int ex=dw_screen_x(u->x),ey=dw_screen_y(u->y-18);
   int gun=x+(flip?-1:1)*height*2/5;
   dw_line(gun,y-height*9/20,ex,ey,5,0xbf6682);dw_line(gun,y-height*9/20,ex,ey,2,0xffe3a3);
   dw_crosshair(ex,ey,12,0x81ffdd);dw_finale_star(ex,ey,10,0xfff2d2);dw_text(ex-6,ey-38,1,"KO",0xffdf9b);
  }
 }
 int since=t-(g->finale_assisted?DW_REVIVE_TICK:180);
 if(since>=0&&since<120)for(int i=0;i<5;i++){
  const struct dw_unit*u=&g->unit[i];int x=dw_screen_x(u->x),y=dw_screen_y(u->y)-20/div;
  int radius=(18+since%40)/div;if(radius<8)radius=8;
  dw_blast_ring(x,y,radius,0x8ef0cd);dw_finale_star(x-12,y-27-since%20,4,0xffedaf);
  dw_finale_star(x+17,y-13-(since+11)%28,3,0x9decd5);
 }
 if(celebrating){unsigned clock=dw_finale_party_clock(g);dw_finale_fireworks(clock);dw_finale_reunion(g,clock);}
 dw_clip_world=0;
}
static void dw_finale_render(const struct dw_game*g){
 int t=g->finale_tick,assisted=g->finale_assisted,celebrating=!assisted||t>=DW_CELEBRATE_TICK;
 dw_finale_world(g);
 /* Reuse the existing score-panel footprint; the actual map remains visible. */
 dw_rect(0,0,338,800,0x080e19);dw_text(25,30,5,"DJ WARS",0xff7598);dw_text(26,82,2,"NIGHTCLUB SHOWDOWN",0x9ca9c4);dw_rect(25,112,290,2,0x473046);
 if(assisted&&t>=90){
  dw_finale_word(170,143,3,"DAVID GUETTA",0xffd7a1);
  dw_finale_master(170,403,g->ticks,celebrating?3:t>=DW_BARRAGE_TICK?2:t>=120&&t<DW_REVIVE_TICK?1:0);
  dw_finale_word(170,434,2,celebrating?"RESCUE COMPLETE":t>=DW_BARRAGE_TICK?"PRECISION MODE":"",0x98e7da);
 }else{
  dw_cafe_character(170,405,0,0,228,g->ticks,celebrating?3:0,0);
  dw_finale_word(170,435,2,"DJ-DRAGONX",0x98e7da);
 }
 dw_text(27,496,2,"THE DRAGON CREW",0xe7d7e7);
 for(int i=0;i<5;i++){
  int y=535+i*36;dw_text(28,y,1,dw_names[i],0xc9c3da);
  dw_rect(111,y-2,174,11,0x29233b);dw_rect(112,y-1,172*g->unit[i].hp/g->unit[i].max_hp,9,g->unit[i].hp?0x71d6b9:0x79516b);
  dw_number(294,y,1,g->unit[i].hp,0xf7d39e);
 }
 dw_finale_word(169,741,1,celebrating?"ALL PAWS BACK ON THE DANCE FLOOR":"THE MATCH WAITS FOR THE CREW",0x919eb7);
 /* The guest is not announced before he has actually appeared in the map. */
 if(assisted&&t<90)return;
 if(celebrating){
  dw_box(377,83,850,58,0x67a9a0);
  dw_finale_word(802,96,2,"THE MOUSE IS DOWN. THE CATS ARE FREE.",0xffdfa5);
  dw_finale_word(802,121,1,!assisted&&t<180?"THE MOUSE HAS LEFT THE BUILDING.":"DRAGON CREW WINS. LET THE AFTERPARTY BEGIN.",0x9de7db);
  return;
 }
 dw_box(377,83,850,125,0x7c668f);
 if(t<120){dw_finale_word(802,103,3,"WAIT. YOU LOST THE WHOLE CREW",0xffd6ae);dw_finale_word(802,159,2,"MOVE OVER. I HAVE GOT THIS.",0xd3c4db);}
 else if(t<DW_REVIVE_TICK){
  dw_finale_word(802,100,3,"YOU ABSOLUTE MUPPET!",0xffb988);
  if(t<270){dw_finale_word(802,148,2,"WERE YOU MIXING TRACKS...",0xf4d9d8);dw_finale_word(802,180,2,"OR JUST PRESSING BUTTONS?",0xf4d9d8);}
  else dw_finale_word(802,158,2,"IS SYNC THE ONLY BUTTON YOU KNOW?",0x9de7db);
 }else if(t<DW_BARRAGE_TICK){dw_finale_word(802,104,3,"CATS FIRST. ALWAYS.",0x9af1d5);dw_finale_word(802,160,2,"UP YOU GET. ALL OF YOU.",0xf5dec1);}
 else if(!celebrating){dw_finale_word(802,104,2,g->finale_result==2?"NO MICE LEFT. JUST A MIRACLE":"NOW THAT IS HOW YOU DROP THEM",0xffcf8d);dw_finale_word(802,160,2,g->finale_result==2?"NOBODY MISSES THE AFTERPARTY":"ONE SHOT. ONE MOUSE. EVERY TIME.",0xd3c4db);}

}
#endif
