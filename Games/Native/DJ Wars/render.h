#ifndef RX3_DJ_WARS_RENDER_H
#define RX3_DJ_WARS_RENDER_H
/* Original pixel art, cached club/terrain, fixed storage. Host supplies font[36][7]. */
#ifdef DW_NATIVE
static unsigned *dw_back;
#else
static unsigned dw_back[1280*800];
#endif
static unsigned *dw_pixels;
static unsigned dw_minimap[220*133],dw_minimap_version;
static unsigned dw_cached_version;static int dw_cached_theme=-1,dw_theme,dw_clip_world;
static int dw_camera_x,dw_camera_y,dw_overview,dw_manual,dw_camera_active=-1,dw_camera_state=-1;
static int dw_cache_x=-9999,dw_cache_y=-9999,dw_cache_overview=-1;
/* Scrolling repaints only newly exposed strips in the existing cache. */
static int dw_dirty_x=0,dw_dirty_y=0,dw_dirty_w=1280,dw_dirty_h=800;
static int dw_divisor(void){return dw_overview?3:1;}
static int dw_yoffset(void){return dw_overview?(700-DW_H/3)/2:0;}
static int dw_screen_x(float x){return 360+((int)x-dw_camera_x)/dw_divisor();}
static int dw_screen_y(float y){return 70+dw_yoffset()+((int)y-dw_camera_y)/dw_divisor();}
static void dw_camera_limit(void){dw_camera_x=(int)dw_clamp(dw_camera_x,0,DW_W-880);dw_camera_y=(int)dw_clamp(dw_camera_y,0,DW_H-700);}
static void dw_camera_update(struct dw_game*g){if(g->state==DW_FINALE||(g->state==DW_OVER&&g->finale_result>=0)){
 int t=g->finale_tick;dw_manual=0;dw_camera_active=g->active;dw_camera_state=g->state;
 dw_overview=!g->finale_assisted||(t>=DW_REVIVE_TICK&&t<DW_REVIVE_TICK+66)||t>=DW_CELEBRATE_TICK;
 if(dw_overview){dw_camera_x=dw_camera_y=0;return;}
 float x=g->finale_x,y=g->finale_y;int target=dw_finale_focus_target(g);
 if(target>=0){x=(x+g->unit[target].x)*.5f;y=(y+g->unit[target].y)*.5f;}
 int tx=(int)dw_clamp(x-440,0,DW_W-880),ty=(int)dw_clamp(y-365,0,DW_H-700);
 dw_camera_x+=(int)((tx-dw_camera_x)*.35f);dw_camera_y+=(int)((ty-dw_camera_y)*.35f);dw_camera_limit();return;
 }if(g->active!=dw_camera_active||(g->state==DW_FLIGHT&&dw_camera_state!=DW_FLIGHT))dw_manual=0;dw_camera_active=g->active;dw_camera_state=g->state;if(dw_overview){dw_camera_x=dw_camera_y=0;return;}if(!dw_manual){float x=g->unit[g->active].x,y=g->unit[g->active].y;for(int i=0;i<DW_SHOTS;i++)if(g->shot[i].live){x=g->shot[i].x;y=g->shot[i].y;break;}if(g->hit_pause){x=g->feedback[g->hit_focus].x;y=g->feedback[g->hit_focus].y;}int tx=(int)dw_clamp(x-440,0,DW_W-880),ty=(int)dw_clamp(y-410,0,DW_H-700);dw_camera_x+=(int)((tx-dw_camera_x)*.3f);dw_camera_y+=(int)((ty-dw_camera_y)*.3f);}dw_camera_limit();}
static void dw_camera(int action,int x,int y){if(action==0){dw_overview=!dw_overview;dw_manual=0;}else if(action==1){dw_manual=0;dw_overview=0;}else if(action==2&&!dw_overview){dw_manual=1;dw_camera_x+=x;dw_camera_y+=y;dw_camera_limit();}else if(action==3){dw_overview=0;dw_manual=1;dw_camera_x=x-440;dw_camera_y=y-350;dw_camera_limit();}}

static unsigned dw_color(unsigned c){if(dw_theme){if(c==0xff405c)c=0x64ffe1;if(c==0xff7394)c=0xa390ff;}return c;}
static void dw_rect(int x,int y,int w,int h,unsigned c){c=dw_color(c);if(dw_clip_world){if(x<360){w-=360-x;x=360;}if(y<70){h-=70-y;y=70;}if(x+w>1240)w=1240-x;if(y+h>770)h=770-y;}if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x+w>1280)w=1280-x;if(y+h>800)h=800-y;if(x<dw_dirty_x){w-=dw_dirty_x-x;x=dw_dirty_x;}if(y<dw_dirty_y){h-=dw_dirty_y-y;y=dw_dirty_y;}if(x+w>dw_dirty_x+dw_dirty_w)w=dw_dirty_x+dw_dirty_w-x;if(y+h>dw_dirty_y+dw_dirty_h)h=dw_dirty_y+dw_dirty_h-y;for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)dw_pixels[yy*1280+xx]=c;}
static void dw_text(int x,int y,int s,const char*t,unsigned c){for(;*t;t++,x+=6*s){int n=*t>='0'&&*t<='9'?*t-'0':*t>='A'&&*t<='Z'?*t-'A'+10:-1;if(n>=0){for(int j=0;j<7;j++)for(int i=0;i<5;i++)if(font[n][j]&(1<<(4-i)))dw_rect(x+i*s,y+j*s,s,s,c);}else if(*t=='-')dw_rect(x,y+3*s,5*s,s,c);else if(*t==':'){dw_rect(x+s,y+s,s,s,c);dw_rect(x+s,y+5*s,s,s,c);}else if(*t=='.')dw_rect(x+s,y+6*s,s,s,c);else if(*t=='/')for(int i=0;i<5;i++)dw_rect(x+(4-i)*s,y+(i+1)*s,s,s,c);else if(*t=='+'){dw_rect(x,y+3*s,5*s,s,c);dw_rect(x+2*s,y+s,s,5*s,c);}}}
static void dw_number(int x,int y,int s,int n,unsigned c){char b[12];int k=0;if(n<0){dw_text(x,y,s,"-",c);x+=s*6;n=-n;}do{b[k++]=(char)('0'+n%10);n/=10;}while(n&&k<10);char out[12];for(int i=0;i<k;i++)out[i]=b[k-i-1];out[k]=0;dw_text(x,y,s,out,c);}
static void dw_disk(int x,int y,int r,unsigned c){for(int yy=-r;yy<=r;yy++){int dx=0;while((dx+1)*(dx+1)+yy*yy<=r*r)dx++;dw_rect(x-dx,y+yy,dx*2+1,1,c);}}
static void dw_line(int x,int y,int a,int b,int width,unsigned c){int dx=a>x?a-x:x-a,dy=b>y?b-y:y-b,sx=x<a?1:-1,sy=y<b?1:-1,e=dx-dy;for(;;){dw_rect(x-width/2,y-width/2,width,width,c);if(x==a&&y==b)break;int p=2*e;if(p>-dy){e-=dy;x+=sx;}if(p<dx){e+=dx;y+=sy;}}}
static void dw_crosshair(int x,int y,int radius,unsigned color){
 for(int a=0;a<256;a+=8){int b=a+8;dw_line(x+(int)(dw_sin(a+64)*radius),y+(int)(dw_sin(a)*radius),x+(int)(dw_sin(b+64)*radius),y+(int)(dw_sin(b)*radius),2,color);}
 dw_line(x-radius-5,y,x-radius+4,y,2,color);dw_line(x+radius-4,y,x+radius+5,y,2,color);
 dw_line(x,y-radius-5,x,y-radius+4,2,color);dw_line(x,y+radius-4,x,y+radius+5,2,color);dw_rect(x-1,y-1,3,3,color);
}
static void dw_blast_ring(int x,int y,int radius,unsigned color){for(int a=0;a<256;a+=8)dw_line(x+(int)(dw_sin(a+64)*radius),y+(int)(dw_sin(a)*radius),x+(int)(dw_sin(a+72)*radius),y+(int)(dw_sin(a+8)*radius),2,color);}
static void dw_draw_blasts(const struct dw_game*g){for(int i=0;i<DW_BLASTS;i++){const struct dw_blast*b=&g->blast[i];if(!b->life)continue;
 int age=36-b->life,div=dw_divisor(),x=dw_screen_x(b->x),y=dw_screen_y(b->y),extent=b->reach/div;
 if(x+extent<360||x-extent>=1240||y+extent<70||y-extent>=770)continue;
 int shock=b->reach*(age<12?age+3:15)/15/div;
 dw_blast_ring(x,y,shock,age<12?0xffd89a:age<23?0xf48c53:0x814250);
 if(age<25){int r=b->radius*(age<8?age+2:age<14?10:(26-age)*10/12)/10/div;
  if(r<1)r=1;dw_disk(x,y,r,age<14?0xeb603e:0xb04d45);dw_disk(x,y,r*3/4,age<14?0xffad54:0xe77745);dw_disk(x,y,r*2/5,age<8?0xfff1bc:0xffd17e);
 }
}}
static void dw_tri(int x,int y,int a,int b,int c,int d,unsigned col){int low=y<b?y:b;if(d<low)low=d;int high=y>b?y:b;if(d>high)high=d;if(low<dw_dirty_y)low=dw_dirty_y;if(high>=dw_dirty_y+dw_dirty_h)high=dw_dirty_y+dw_dirty_h-1;if(dw_clip_world){if(low<70)low=70;if(high>=770)high=769;}int xs[3]={x,a,c},ys[3]={y,b,d};for(int yy=low;yy<=high;yy++){int hit[3],n=0;for(int i=0;i<3;i++){int j=(i+1)%3;if((ys[i]<=yy&&ys[j]>yy)||(ys[j]<=yy&&ys[i]>yy))hit[n++]=xs[i]+(yy-ys[i])*(xs[j]-xs[i])/(ys[j]-ys[i]);}if(n==2){int l=hit[0]<hit[1]?hit[0]:hit[1],r=hit[0]>hit[1]?hit[0]:hit[1];dw_rect(l,yy,r-l+1,1,col);}}}
static void dw_box(int x,int y,int w,int h,unsigned edge){dw_rect(x,y,w,h,edge);dw_rect(x+2,y+2,w-4,h-4,0x131020);}
static void dw_speaker(int x,int y){dw_rect(x-32,y-4,64,128,0x070a13);dw_rect(x-28,y,56,120,0x21203a);for(int k=0;k<2;k++){dw_disk(x,y+29+k*58,25,0x403654);dw_disk(x,y+29+k*58,21,0x080c1a);dw_disk(x,y+29+k*58,14,0x292a46);dw_disk(x,y+29+k*58,6,0x695970);dw_rect(x-3,y+24+k*58,3,3,0xa089ad);}dw_rect(x-28,y+119,56,3,0xff405c);}
static void dw_world_rect(int x,int y,int w,int h,unsigned c){int d=dw_divisor();dw_rect(dw_screen_x(x),dw_screen_y(y),(w+d-1)/d,(h+d-1)/d,c);}
static void dw_world_line(int x,int y,int a,int b,int w,unsigned c){dw_line(dw_screen_x(x),dw_screen_y(y),dw_screen_x(a),dw_screen_y(b),w>dw_divisor()?w/dw_divisor():1,c);}
static void dw_world_disk(int x,int y,int r,unsigned c){dw_disk(dw_screen_x(x),dw_screen_y(y),r/dw_divisor(),c);}
static void dw_world_tri(int x,int y,int a,int b,int c,int d,unsigned col){dw_tri(dw_screen_x(x),dw_screen_y(y),dw_screen_x(a),dw_screen_y(b),dw_screen_x(c),dw_screen_y(d),col);}
static void dw_world_text(int x,int y,int s,const char*t,unsigned c){dw_text(dw_screen_x(x),dw_screen_y(y),s>=dw_divisor()?s/dw_divisor():1,t,c);}
static void dw_world_speaker(int x,int y){dw_world_rect(x-43,y-6,86,180,0x090b16);dw_world_rect(x-39,y,78,169,0x2b273f);for(int k=0;k<2;k++){dw_world_disk(x,y+42+k*80,35,0x4b3d5e);dw_world_disk(x,y+42+k*80,29,0x0a0f1b);dw_world_disk(x,y+42+k*80,21,0x30364d);dw_world_disk(x,y+42+k*80,8,0x827088);}dw_world_rect(x-40,y+165,80,4,0xf35685);}
static void dw_world_booth(int x,int y){dw_world_rect(x-150,y,300,63,0x28243b);dw_world_rect(x-148,y+3,296,5,0x6aded0);dw_world_rect(x-120,y+17,86,32,0x111724);dw_world_rect(x+34,y+17,86,32,0x111724);dw_world_disk(x-77,y+33,14,0x944c73);dw_world_disk(x-77,y+33,5,0xf6be74);dw_world_disk(x+77,y+33,14,0x36788a);dw_world_disk(x+77,y+33,5,0x78edd4);for(int i=0;i<8;i++)dw_world_rect(x-26+i*7,y+21,3,24,0x847193);}
static void dw_club(struct dw_game*g){dw_clip_world=0;dw_rect(0,0,1280,800,0x080b14);dw_rect(343,52,915,732,0x35243d);dw_rect(347,56,907,724,0x131020);dw_clip_world=1;
 int div=dw_divisor(),off=dw_yoffset();
 for(int y=0;y<700;y++){int wy=dw_camera_y+(y-off)*div;dw_rect(360,70+y,880,1,wy<300?0x101426:wy<750?0x1a1329:wy<1150?0x20152a:0x101426);}
 for(int x=0;x<DW_W;x+=110){dw_world_rect(x,100,2,1460,0x292438);for(int y=100;y<1600;y+=110)dw_world_rect(x,y,110,2,0x241e33);}
 for(int k=0;k<5;k++){int x=170+k*535,y=190+(k%2)*180;dw_world_tri(x,y,x-220,y+980,x+320,y+980,k%2?0x29182d:0x172a35);}
 for(int band=0;band<3;band++){int y=135+band*510;for(int x=35;x<DW_W;x+=105){dw_world_rect(x,y,96,7,0x474056);dw_world_line(x,y-12,x+94,y+24,2,0x40304c);dw_world_line(x+94,y-12,x,y+24,2,0x40304c);if(x%3){dw_world_rect(x+33,y+17,29,18,0x635276);dw_world_rect(x+38,y+33,19,4,band%2?0x6bc4d7:0xe569ae);}}}
 const int balls[][3]={{1350,200,47},{655,400,33},{2360,520,32}};
 for(int k=0;k<3;k++){int x=balls[k][0],y=balls[k][1],r=balls[k][2];dw_world_line(x,y-160,x,y-r,2,0x7c8ba0);dw_world_disk(x,y,r,0x344460);for(int yy=-r+5;yy<r;yy+=9)for(int xx=-r+5;xx<r;xx+=9)if(xx*xx+yy*yy<r*r)dw_world_rect(x+xx,y+yy,6,6,(xx+yy)%3?0xc6d7e0:0x6988a1);}
 const int sign[][2]={{430,325},{1350,385},{2210,805}};const char*signs[]={"THE CAT LOUNGE","THE AFTERHOURS CLUB","MOUSE PIT"};
 for(int k=0;k<3;k++){int x=sign[k][0],y=sign[k][1];dw_world_rect(x-252,y-5,504,96,0x7c3152);dw_world_rect(x-249,y-2,498,90,0x1b1027);dw_world_text(x-(k==1?216:k==0?156:108),y+17,4,signs[k],k==0?0x7ce0d1:0xff83ad);dw_world_text(x-156,y+65,2,"NO SYNC - JUST SURVIVORS",0x9bacbf);}
 dw_world_speaker(105,615);dw_world_speaker(198,615);dw_world_speaker(780,1060);dw_world_speaker(865,1060);dw_world_speaker(1460,590);dw_world_speaker(1545,590);dw_world_speaker(2090,930);dw_world_speaker(2180,930);dw_world_speaker(2570,1040);
 dw_world_booth(485,900);dw_world_booth(1330,645);dw_world_booth(2210,1200);
 for(int section=0;section<5;section++){int base=110+section*570,yy=850+(section%2)*280;for(int i=0;i<10;i++){int h=35+(i*43)%130;dw_world_rect(base+i*12,yy-h,7,h,i%2?0x6e315b:0x307885);}}
 /* Pixel mask gives both craters and terrain layers the same edge treatment. */
 int sy0=dw_dirty_y>70?dw_dirty_y-70:0,sy1=dw_dirty_y+dw_dirty_h<770?dw_dirty_y+dw_dirty_h-70:700;
 int sx0=dw_dirty_x>360?dw_dirty_x-360:0,sx1=dw_dirty_x+dw_dirty_w<1240?dw_dirty_x+dw_dirty_w-360:880;
 for(int sy=sy0;sy<sy1;sy++){int y=dw_camera_y+(sy-off)*div;if(y<0||y>=DW_H)continue;for(int sx=sx0;sx<sx1;sx++){int x=dw_camera_x+sx*div;if(x>=DW_W)continue;if(g->terrain[y*DW_W+x]){unsigned palette[5]={0x392640,0x46314a,0x302138,0x513349,0x3f2a47};unsigned hash=(unsigned)(x/37)*719u+(unsigned)((y+(x/37%2)*15)/33)*173u;unsigned c=palette[(hash^(hash>>4))%5];if((y+(x/37%2)*15)%33<2||x%37<2)c=0x261b31;if(!dw_solid(g,x,y-div))c=0x93e4cb;else if(!dw_solid(g,x-div,y)||!dw_solid(g,x+div,y))c=0xbd7198;else if(!dw_solid(g,x,y+div))c=0x674265;else if(!dw_solid(g,x,y-3*div))c=0x427e82;dw_pixels[(sy+70)*1280+sx+360]=c;}}}
 dw_clip_world=0;dw_cache_x=dw_camera_x;dw_cache_y=dw_camera_y;dw_cache_overview=dw_overview;dw_cached_version=g->terrain_version;dw_cached_theme=dw_theme;
}
static void dw_scroll_cache(int dx,int dy){
 int x0=dx<0?-dx:0,x1=dx>0?880-dx:880;
 int y0=dy<0?-dy:0,y1=dy>0?700-dy:700;
 /* Direction keeps overlapping copies safe without an extra allocation. */
 for(int row=0;row<y1-y0;row++){
  int y=dy<0?y1-1-row:y0+row;
  unsigned *dst=dw_back+(70+y)*1280+360,*src=dw_back+(70+y+dy)*1280+360+dx;
  if(dx<0){for(int x=x1-1;x>=x0;x--)dst[x]=src[x];}
  else{for(int x=x0;x<x1;x++)dst[x]=src[x];}
 }
}
static void dw_background(struct dw_game*g){
 int dx=dw_camera_x-dw_cache_x,dy=dw_camera_y-dw_cache_y;
 int full=dw_cached_version!=g->terrain_version||dw_cached_theme!=dw_theme||dw_cache_overview!=dw_overview||dx<=-880||dx>=880||dy<=-700||dy>=700;
 if(full)dw_club(g);
 else if(dx||dy){
  dw_scroll_cache(dx,dy);
  if(dx){dw_dirty_x=360+(dx>0?880-dx:0);dw_dirty_y=70;dw_dirty_w=dx>0?dx:-dx;dw_dirty_h=700;dw_club(g);}
  if(dy){dw_dirty_x=360+(dx<0?-dx:0);dw_dirty_y=70+(dy>0?700-dy:0);dw_dirty_w=880-(dx>0?dx:-dx);dw_dirty_h=dy>0?dy:-dy;dw_club(g);}
 }
 dw_dirty_x=dw_dirty_y=0;dw_dirty_w=1280;dw_dirty_h=800;
}
#include "cafe_sprites.h"
static int dw_sx,dw_sy,dw_scale,dw_sprite_div=1;
static void dw_sprite_rect(int x,int y,int w,int h,unsigned c){dw_rect(dw_sx+x*dw_scale/dw_sprite_div,dw_sy+y*dw_scale/dw_sprite_div,(w*dw_scale+dw_sprite_div-1)/dw_sprite_div,(h*dw_scale+dw_sprite_div-1)/dw_sprite_div,c);}
static void dw_sprite_disk(int x,int y,int r,unsigned c){dw_disk(dw_sx+x*dw_scale/dw_sprite_div,dw_sy+y*dw_scale/dw_sprite_div,(r*dw_scale+dw_sprite_div-1)/dw_sprite_div,c);}
static void dw_character(int x,int y,int kind,int variant,int scale,unsigned tick,int divisor){if(kind<2){dw_cafe_character(x,y,kind,variant,(kind?40:44)*scale/divisor,tick,0,0);return;}dw_sprite_div=divisor;dw_sx=x-16*scale/divisor;dw_sy=y-38*scale/divisor;dw_scale=scale;
 #define SR dw_sprite_rect
 #define SD dw_sprite_disk
 if(kind==0){
  SR(1,19,8,13,0x33202d);SR(24,19,8,13,0x33202d);SR(2,21,6,8,0x97354d);SR(26,21,5,8,0x97354d);SR(6,10,20,18,0x493a42);SR(9,5,4,9,0xeb9675);SR(22,5,4,9,0xeb9675);SR(10,3,2,5,0xffd584);SR(23,3,2,5,0xffd584);SR(4,15,3,4,0xeb784e);SR(5,22,5,4,0xff813e);SR(10,17,5,3,0xffc75f);SR(21,17,5,3,0xffc75f);SR(12,17,2,3,0x17131d);SR(23,17,2,3,0x17131d);SR(13,22,16,7,0x73534a);SR(23,23,2,2,0x291c27);SR(15,28,11,2,0xffa269);SR(11,29,14,8,0x483444);SR(15,30,5,7,0xfb8452);SR(8,36,8,2,0xa47867);SR(22,36,8,2,0xa47867);SR(2,34,9,2,0xe6714a);
 }else if(kind==1){unsigned fur=variant%3==0?0xc38560:variant%3==1?0x9b9fb7:0xdbc8ac,shade=variant%3==0?0x855955:0x79768d;SR(7,6,6,10,fur);SR(22,6,6,10,fur);SR(9,8,2,5,0xf2a0ae);SR(24,8,2,5,0xf2a0ae);SR(6,14,23,14,fur);SR(10,12,15,3,fur);SR(13,12,3,5,shade);SR(18,12,3,5,shade);SR(10,20,4,4,0x201a2d);SR(22,20,4,4,0x201a2d);SR(10,20,2,1,0xfdf5e3);SR(22,20,2,1,0xfdf5e3);SR(17,25,3,2,0xef8d9c);SR(9,29,18,8,0x242239);SR(10,30,16,2,0x57cecd);SR(17,32,3,3,0xecab61);SR(7,36,8,2,fur);SR(23,36,6,2,fur);SR(27,31,5,3,fur);SR(30,28,3,5,fur);SR(3,24,5,1,0xbeb0ab);SR(27,24,5,1,0xbeb0ab);
 }else{int boss=kind==3;unsigned shell=boss?0x802d56:0x483364,shine=boss?0xd25786:0x8868af;SD(boss?3:5,boss?3:8,boss?10:7,0x20182e);SD(boss?29:28,boss?3:8,boss?10:7,0x20182e);SD(boss?3:5,boss?3:8,boss?8:5,shell);SD(boss?29:28,boss?3:8,boss?8:5,shell);SD(16,19,boss?17:14,shell);SR(7,10,6,3,shine);SR(9,18,5,5,0xf4e9ff);SR(22,18,5,5,0xf4e9ff);SR(11,19,2,3,boss?0xff405c:0x8054be);SR(24,19,2,3,boss?0xff405c:0x8054be);SR(10,27,16,4,0xf4e9ff);for(int i=0;i<5;i++)SR(11+i*3,29,1,2,0x483364);SR(10,33,16,5,0x181b2d);SR(5,35,9,3,0x694f7b);SR(22,35,10,3,0x694f7b);if(boss){SR(10,0,16,4,0xfbbb69);SR(10,-4,3,5,0xfbbb69);SR(17,-6,3,7,0xfbbb69);SR(23,-4,3,5,0xfbbb69);}}
 #undef SR
 #undef SD
 (void)tick;
}
static void dw_weapon_icon(int x,int y,int kind){if(kind==0){dw_rect(x,y+9,30,8,0x86a29c);dw_rect(x+5,y+17,6,8,0x55475c);dw_rect(x+27,y+7,4,12,0xffcd79);}else if(kind==1){dw_disk(x+14,y+15,10,0x69967f);dw_rect(x+10,y+1,9,6,0xb8bea6);dw_rect(x+12,y+10,3,14,0x314551);}else if(kind==2){dw_line(x+2,y+14,x+30,y+14,4,0xa7bad5);dw_tri(x+15,y+2,x+7,y+27,x+22,y+27,0xc1cddd);}else if(kind==3){dw_line(x+4,y+25,x+25,y+4,7,0xdbd1bd);dw_rect(x+1,y+24,8,5,0xff804b);dw_rect(x+21,y+2,7,7,0xff405c);}else if(kind==5){dw_line(x+7,y+4,x+7,y+27,3,0x83e6e0);dw_line(x+24,y+4,x+24,y+27,3,0x83e6e0);dw_line(x+7,y+4,x+24,y+4,3,0xbf91ef);dw_line(x+7,y+27,x+24,y+27,3,0xbf91ef);}else if(kind==6){dw_rect(x+5,y+6,9,20,0x97a1ba);dw_rect(x+20,y+6,9,20,0x97a1ba);dw_rect(x+13,y+11,8,7,0x554d72);dw_tri(x+5,y+26,x+9,y+33,x+14,y+26,0xff935e);dw_tri(x+20,y+26,x+24,y+33,x+29,y+26,0xff935e);}else{dw_tri(x+12,y+12,x,y,x+3,y+23,0xdc7654);dw_tri(x+20,y+12,x+33,y,x+29,y+23,0xdc7654);dw_rect(x+12,y+8,9,18,0xb55c66);dw_rect(x+20,y+6,8,7,0xffb970);}}
static void dw_grave_art(const struct dw_grave*t,int team){int x=(int)t->x,y=(int)t->y;unsigned trim=team?0xd867a9:0x60c9b4;
 dw_world_rect(x-18,y-3,36,4,0x444457);dw_world_rect(x-15,y-45,30,43,trim);dw_world_rect(x-13,y-43,26,39,0x151725);
 dw_world_disk(x,y-17,11,0x505269);dw_world_disk(x,y-17,8,0x080c15);dw_world_disk(x,y-17,4,0x8c91a9);
 dw_world_disk(x,y-35,5,0x777a91);dw_world_disk(x,y-35,3,0x111221);dw_world_rect(x-10,y-6,20,2,trim);
 dw_world_text(x-9,y-55,1,"RIP",0xc0becb);
}
static void dw_health_badge(const struct dw_unit*u,int i,int x,int y,int active){
 int head=dw_overview?18:u->kind==3?50:42;unsigned col=u->team?0xe387c6:0x7becd6;
 int chars=0;while(dw_names[i][chars])chars++;
 dw_rect(x-chars*3-2,y-head-32,chars*6+3,10,0x10121c);dw_text(x-chars*3,y-head-31,1,dw_names[i],col);
 dw_rect(x-23,y-head-20,46,7,0x0a1120);dw_rect(x-22,y-head-19,44*u->hp/u->max_hp,5,u->hp<=25?0xff5963:col);
 dw_rect(x-21,y-head-12,42,10,0x10121c);dw_number(x-18,y-head-11,1,u->hp,0xffedcf);dw_text(x+5,y-head-11,1,"HP",0x9faac0);
 if(active){int yy=y-head-43;dw_tri(x-5,yy,x+5,yy,x,yy+6,0xffcf7b);}
}
static void dw_hit_numbers(const struct dw_game*g){for(int i=0;i<DW_UNITS;i++){const struct dw_feedback*f=&g->feedback[i];if(!f->life)continue;
 int x=dw_screen_x(f->x),y=dw_screen_y(f->y)-(dw_overview?72:105)-(102-f->life)/7;
 dw_rect(x-31,y-4,62,23,0x10121c);dw_text(x-24,y,2,"-",0xff927a);dw_number(x-11,y,2,f->damage,0xffd89a);
}}
static void dw_minimap_draw(const struct dw_game*g){
 if(dw_minimap_version!=g->terrain_version){
  for(int y=0;y<133;y++)for(int x=0;x<220;x++)dw_minimap[y*220+x]=dw_solid(g,x*12,y*12)?0x56718b:0x131020;
  dw_minimap_version=g->terrain_version;
 }
 dw_box(1003,79,230,146,0x46364f);
 for(int y=0;y<133;y++){
#ifdef DW_NATIVE
  memcpy(dw_pixels+(86+y)*1280+1008,dw_minimap+y*220,220*sizeof(unsigned));
#else
  for(int x=0;x<220;x++)dw_pixels[(86+y)*1280+1008+x]=dw_minimap[y*220+x];
#endif
 }
}
#include "finale_render.h"
static void dw_render(struct dw_game*g,unsigned*buffer,int paused,int neon){dw_theme=neon;dw_clip_world=0;dw_camera_update(g);dw_pixels=dw_back;dw_background(g);dw_pixels=buffer;
#ifdef DW_NATIVE
 memcpy(buffer,dw_back,1280*800*sizeof(unsigned));
#else
 for(int i=0;i<1280*800;i++)buffer[i]=dw_back[i];
#endif

 dw_clip_world=1;int pulse=g->ticks/8%16;for(int i=0;i<10;i++){int x=377+i*83,y=184+(i*31)%167;dw_rect(x,y,2+(i%2),2+(i%2),pulse==i?0xc2f1e4:0x34586f);}dw_text(377,25,2,"DRAGON AND CATS / THE MOUSE CREW",0x9fb4cc);
 int water=dw_screen_y(DW_H-24);if(water<770)for(int x=360;x<1240;x+=16){int y=water+(int)(dw_sin(x+g->ticks)*3);dw_rect(x,y,16,770-y,0x25385d);dw_rect(x,y,15,2,0x5b95b0);}
 for(int i=0;i<DW_PARTICLES;i++){struct dw_particle*p=&g->particle[i];if(p->life&&p->x>=0&&p->x<DW_W&&p->y>=0&&p->y<DW_H)dw_rect(dw_screen_x(p->x),dw_screen_y(p->y),p->life>20?3:2,p->life>20?3:2,p->kind==0?0x81ead2:p->kind==1?0xffab65:p->kind==2?0x6eb7ef:0x68677f);}
 for(int i=0;i<DW_UNITS;i++)if(g->grave[i].live)dw_grave_art(&g->grave[i],g->unit[i].team);
 for(int i=0;i<DW_UNITS;i++){struct dw_unit*u=&g->unit[i];if(!u->hp)continue;int x=dw_screen_x(u->x),y=dw_screen_y(u->y);if(x<342||x>1255||y<68||y>850)continue;dw_disk(x,y-1,12/dw_divisor(),0x161725);if(u->kind<2)dw_cafe_character(x,y,u->kind,i,(u->kind?40:44)/dw_divisor(),g->ticks,(g->state==DW_FINALE||g->state==DW_OVER)?3:dw_abs(u->vx)>.2f?1:0,u->face<0);else dw_character(x,y,u->kind,i,1,g->ticks,dw_divisor());dw_health_badge(u,i,x,y,i==g->active&&g->state<DW_OVER);}

 if(g->state==DW_AIM&&g->weapon<5&&!g->jet_ticks){struct dw_unit*u=&g->unit[g->active];float dx=u->face*dw_sin(g->aim+64),dy=-dw_sin(g->aim);int x=dw_screen_x(u->x),y=dw_screen_y(u->y)-17/dw_divisor();dw_line(x,y,x+(int)(dx*27/dw_divisor()),y+(int)(dy*27/dw_divisor()),6,0x19222a);dw_line(x,y,x+(int)(dx*27/dw_divisor()),y+(int)(dy*27/dw_divisor()),3,0xc6b9a9);dw_crosshair(x+(int)(dx*76/dw_divisor()),y+(int)(dy*76/dw_divisor()),dw_overview?6:13,0xff3548);}
 if((g->weapon==2||g->weapon==3||g->weapon==5)&&g->state<DW_OVER){int x=dw_screen_x(g->mark_x),y=dw_screen_y(g->mark_y);dw_line(x-13,y,x-5,y,2,0xffda81);dw_line(x+5,y,x+13,y,2,0xffda81);dw_line(x,y-13,x,y-5,2,0xffda81);dw_line(x,y+5,x,y+13,2,0xffda81);}
 for(int i=0;i<DW_SHOTS;i++)if(g->shot[i].live){struct dw_shot*s=&g->shot[i];int x=dw_screen_x(s->x),y=dw_screen_y(s->y);if(x<362||x>1235||y<74||y>766)continue;if(s->kind==4){dw_cafe_character(x,y+16,0,0,34,g->ticks,3,s->vx<0);}else if(s->kind==1){dw_disk(x,y,6,0x8ed28b);dw_rect(x-2,y-8,4,3,0xffb780);}else{dw_line(x,y,x-(int)s->vx,y-(int)s->vy,5,0xff955f);dw_disk(x,y,4,0xe9d5b5);}}
 dw_draw_blasts(g);dw_hit_numbers(g);
 dw_clip_world=0;dw_minimap_draw(g);for(int i=0;i<10;i++)if(g->unit[i].hp)dw_rect(1007+(int)g->unit[i].x/12,85+(int)g->unit[i].y/12,3,3,i<5?0x71efd5:0xf786bb);if(!dw_overview){int mx=1008+dw_camera_x/12,my=86+dw_camera_y/12;dw_line(mx,my,mx+73,my,1,0xffd391);dw_line(mx,my,mx,my+58,1,0xffd391);dw_line(mx+73,my,mx+73,my+58,1,0xffd391);dw_line(mx,my+58,mx+73,my+58,1,0xffd391);}if(g->state<DW_OVER)dw_text(375,742,1,dw_overview?"TACTICAL OVERVIEW - M TO ZOOM IN":"DRAG TO EXPLORE - M MAP - C FOLLOW",0xb1cbd7);
 dw_text(377,25,2,"DRAGON AND CATS / THE MOUSE CREW",0x9fb4cc);
 if(g->state==DW_FINALE||(g->state==DW_OVER&&g->finale_result>=0)){dw_finale_render(g);return;}
 dw_text(25,30,5,"DJ WARS",0xff405c);dw_text(26,81,2,"NIGHTCLUB SHOWDOWN",0x9ca9c4);dw_rect(25,112,290,2,0x473046);
 struct dw_unit*u=&g->unit[g->active];dw_character(69,213,u->kind,g->active,2,g->ticks,1);dw_text(135,129,2,g->team?"ENEMY TURN":"YOUR TURN",g->team?0xe387c6:0x7becd6);dw_number(135,165,5,(g->remaining+59)/60,g->remaining<900?0xff405c:0xffd59a);dw_text(211,192,1,"SECONDS",0x899ab5);dw_text(27,228,2,dw_names[g->active],0xe6dcec);dw_text(217,230,1,"TURN",0x7b8da9);dw_number(265,228,2,g->turn,0xe6dcec);
 for(int t=0;t<2;t++){int y=267+t*35;dw_text(25,y,1,t?"MOUSE CREW":"DRAGON CREW",0x929ab2);dw_rect(123,y,154,9,0x231d31);dw_rect(123,y,154*dw_health(g,t)/(t?580:540),9,t?0xd867a9:0x60c9b4);dw_number(291,y,1,dw_alive(g,t),0xffffff);}
 for(int w=0;w<7;w++){int y=336+w*32,ammo=dw_ammo(g,w);dw_box(24,y,292,30,w==g->weapon?0xff405c:0x322738);dw_weapon_icon(31,y-2,w);dw_text(73,y+11,w==3||w==4?1:2,dw_weapons[w],ammo?0xdfd5e6:0x686174);if(w)dw_number(294,y+11,1,ammo,0xf6c781);}
 dw_text(25,570,1,g->jet_ticks?"JETPACK FUEL":"SHOT POWER",0x99a6bd);dw_rect(25,583,290,8,0x29223b);dw_rect(25,583,g->jet_ticks?g->jet_ticks*290/600:g->power*290/100,8,g->jet_ticks?0xffb970:g->charging?0xffb970:0x548d98);dw_text(25,596,2,"WIND",0x8ca0b8);dw_number(215,596,2,g->wind<0?-g->wind:g->wind,0xc9dded);int tip=g->wind>=0?195:127;dw_line(128,603,194,603,2,0x84d7d0);dw_line(tip,603,tip+(g->wind>=0?-7:7),597,2,0x84d7d0);dw_line(tip,603,tip+(g->wind>=0?-7:7),609,2,0x84d7d0);
 dw_box(24,633,292,49,g->team?0x473047:0xff405c);dw_text(47,650,2,g->hit_pause?"HIT - WATCH THE DAMAGE":g->jet_ticks?"ARROWS FLY - FIRE STOP":g->team?"ENEMY IS THINKING":g->weapon==5?"PICK SPOT - FIRE":g->weapon==6?"FIRE TO USE JETPACK":g->state==DW_FLIGHT&&g->weapon==4?"STEER / DETONATE":"HOLD FIRE - RELEASE",0xf4e4df);
 dw_text(26,701,1,"ARROWS MOVE / AIM - ENTER JUMP",0x919ab3);dw_text(26,717,1,"1-7 EQUIPMENT - T TARGET - SPACE",0x919ab3);dw_box(25,742,132,33,0x473047);dw_box(177,742,138,33,0x473047);dw_text(46,752,2,paused?"RESUME":"PAUSE",0xe5ced8);dw_text(211,752,2,"MENU",0xe5ced8);
 if(g->event_age&&g->state!=DW_OVER){const char*msg=g->event==1?(g->team?"THE MOUSE CREW TAKES THE STAGE":"DRAGON CREW - YOUR STAGE"):g->event==2?"BASS DROP":g->event==3?"INCOMING AIRSTRIKE":g->event==4?"SUPER DRAGON - ARROWS TO FLY":g->event==5?"LAST CALL":g->event==7?"TELEPORT - TURN OVER":g->event==8?"JETPACK - ARROWS TO FLY":g->event==9?"FALL DAMAGE":g->event==10?"CHOOSE A CLEAR LANDING SPOT":"FIRE IN THE CLUB";dw_box(540,653,530,41,0x4d3c58);dw_text(556,667,2,msg,0xe6d7db);if((g->event==2||g->event==9)&&g->damage){dw_text(1090,667,2,"-",0xffb680);dw_number(1108,667,2,g->damage,0xffb680);}}
 if((paused&&!dw_overview)||g->state==DW_OVER){dw_box(480,287,636,186,0xff405c);dw_text(510,316,4,paused?"DJ WARS":g->winner==0?"DRAGON CREW WINS":g->winner==1?"MOUSE CREW WINS":"DOUBLE KNOCKOUT",0xff7394);dw_text(510,371,2,paused?"5 VS 5 - ONE CLUB - NO SECOND CHANCES":"THE DANCE FLOOR HAS A WINNER",0xaac7cd);dw_text(510,408,2,paused?"PRESS P OR CLICK RESUME TO PLAY":"PRESS R TO PLAY AGAIN",0xffd59a);}
}
#endif
