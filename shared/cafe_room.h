#ifndef RX3_CAFE_ROOM_H
#define RX3_CAFE_ROOM_H
/* Warm cafe room with animated chibi cats in the user-selected style. */
static void cafe_oval(int x,int y,int rx,int ry,unsigned color){
 for(int yy=-ry;yy<=ry;yy++){int span=rx;while(span>0&&span*span*ry*ry+yy*yy*rx*rx>rx*rx*ry*ry)span--;cr_rect(x-span,y+yy,span*2+1,1,color);}
}
static void cafe_line(int x,int y,int xx,int yy,int thick,unsigned color){
 int dx=xx-x,dy=yy-y,n=dx<0?-dx:dx;if((dy<0?-dy:dy)>n)n=dy<0?-dy:dy;
 for(int i=0;i<=n;i++)cr_rect(x+(n?dx*i/n:0),y+(n?dy*i/n:0),thick,thick,color);
}
#include "cafe_chibi.h"
#include "cafe_dragon.h"
#include "cat_names.h"
/* Seven residents and at most four rescued friends share one floor depth. */
#define CAFE_CAT_CAPACITY 11
static const unsigned cafe_resident_sprite[]={0,2,6,9,8,5,7};
struct cafe_cat_draw {unsigned id;int x,y,s;unsigned ms;int action;};
static void cafe_draw_cats(struct cafe_cat_draw*cats,unsigned count){
 /* Stable insertion sort uses each cat's feet, including its arrival motion.
  * Keep ground shadows below every body so they cannot paint across a cat. */
 for(unsigned i=1;i<count;i++){
  struct cafe_cat_draw cat=cats[i];unsigned j=i;
  while(j&&cats[j-1].y>cat.y){cats[j]=cats[j-1];j--;}cats[j]=cat;
 }
 for(unsigned i=0;i<count;i++){
  const struct cafe_cat_draw*cat=&cats[i];
  cafe_oval(cat->x,cat->y+3,14*cat->s,3*cat->s,0x342431);
  if(cat->id==rescue_sprite[1]){cr_disc(cat->x+50,cat->y+4,12,0x261e31);cr_disc(cat->x+50,cat->y+4,4,0x64e8ee);}
 }
 for(unsigned i=0;i<count;i++){
  const struct cafe_cat_draw*cat=&cats[i];
  cafe_chibi(cat->id,cat->x,cat->y,cat->s,cat->ms,cat->action);
 }
}
static void cafe_draw_cat_names(const struct cafe_cat_draw*cats,unsigned count){
 for(unsigned i=0;i<count;i++){
  const char*name=cat_name(cats[i].id);int length=0;while(name[length])length++;
  int x=cats[i].x-(length*6-1)/2,y=cats[i].y+16;
  cr_text(x+1,y+1,1,name,0x261e31);cr_text(x,y,1,name,0xf4d69b);
 }
}
static const char*cafe_speaker_name(unsigned speaker){
 if(speaker<4)return cat_name(rescue_sprite[speaker]);
 if(speaker==4)return "DJ DRAGON-X";
 return speaker<12?cat_name(cafe_resident_sprite[speaker-5]):"CAFE CREW";
}
static const char*cafe_arrival_speech(unsigned cat){
 static const char*const lines[]={
  "I MISSED YOU! NEVER LET THAT MOUSE NEAR ME AGAIN!",
  "I MISSED THIS PLACE! IT FEELS SO GOOD TO BE HOME.",
  "MY FAVORITE SOFA! MOVE OVER, I'M STAYING RIGHT HERE.",
  "I MISSED YOU ALL. CAN WE JUST STAY TOGETHER FOR A WHILE?"
 };
 return cat<4?lines[cat]:"IT'S GOOD TO BE HOME!";
}
#include "cafe_room_art.h"
#include "cafe_show.h"
static const unsigned*cafe_backdrop_colors=cafe_room_palette;
/* The small indexed background is read directly; no full-resolution texture.
 * Horizontal runs keep the shared rectangle renderer inexpensive on the RX3. */
static void cafe_backdrop(int from_y,int to_y,int from_x,int to_x){
 for(int y=from_y;y<to_y;y++)for(int x=from_x;x<to_x;){
  unsigned p=cafe_room_pixels[y*CAFE_ROOM_W+x];int end=x+1;
  while(end<to_x&&cafe_room_pixels[y*CAFE_ROOM_W+end]==p)end++;
  cr_rect(44+x*2,242+y*2,(end-x)*2,2,cafe_backdrop_colors[p]);x=end;
 }
}
static unsigned cafe_mix_color(unsigned a,unsigned b,unsigned amount){
 unsigned r=(((a>>16)&255)*(255-amount)+((b>>16)&255)*amount)/255;
 unsigned g=(((a>>8)&255)*(255-amount)+((b>>8)&255)*amount)/255;
 unsigned blue=((a&255)*(255-amount)+(b&255)*amount)/255;
 return r<<16|g<<8|blue;
}
static unsigned cafe_show_color(const struct cafe_show_frame*f,int secondary){
 static const unsigned colors[6][2]={{0x63bbeb,0xcc89de},{0x4af0d1,0xb766ef},{0xeb7cc0,0xf9ba66},{0x57eff8,0xf86fcb},{0x798de8,0xc59ee8},{0xffd17b,0x74ede2}};
 return colors[f->section<6?f->section:0][secondary!=0];
}
static void cafe_show_palette(const struct cafe_show_frame*f){
 /* Change the room's lighting without painting over its wood, shelves or sofas.
  * The tiny palette is the only mutable colour data; sprites stay readable. */
 static unsigned palette[256];unsigned tint=cafe_show_color(f,0),strength=18+f->energy/16;
 for(unsigned i=0;i<256;i++)palette[i]=cafe_mix_color(cafe_mix_color(cafe_room_palette[i],0x111127,80),tint,strength);
 cafe_backdrop_colors=palette;
}
static int cafe_show_triangle(unsigned phase,int amount){phase&=255;return (int)(phase<128?phase:255-phase)*amount/127;}
static void cafe_show_beam(int source_x,int source_y,int end_x,int end_y,unsigned color,int width){
 cafe_line(source_x,source_y,end_x,end_y,1,cafe_mix_color(color,0x111127,125));
 cafe_line(source_x+2,source_y,end_x+width,end_y,1,cafe_mix_color(color,0x111127,190));
 /* A broken pixel haze preserves the illustrated room between its light rays. */
 int dy=end_y-source_y;if(dy<=0)return;
 for(int y=source_y+24;y<end_y;y+=12){int d=y-source_y,x=source_x+(end_x-source_x)*d/dy,w=width*d/dy;
  for(int j=0;j<w;j+=18)cr_rect(x+j,y,2,2,cafe_mix_color(color,0x111127,150));}
}
static void cafe_show_room_lights(const struct cafe_show_frame*f){
 unsigned a=cafe_show_color(f,0),b=cafe_show_color(f,1);
 int sweep=cafe_show_triangle((f->beat*19+f->phase/5)&255,330);
 /* The hanging lamps become moving spots; builds open outward into the drop. */
 int rays=f->section==3||f->section==5?5:f->section==2?4:2;
 for(int i=0;i<rays;i++){
  cafe_show_beam(408,290,80+sweep+i*70,742,i%2?a:b,36);
  cafe_show_beam(816,290,1195-sweep-i*65,742,i%2?b:a,-32);
 }
 cr_rect(391,278,33,5,a);cr_rect(799,278,33,5,b);
 cr_rect(400,283,15,6,0xeafcff);cr_rect(808,283,15,6,0xffeaff);
 /* Small mirror ball, with a slow rotation even in the song's quiet bridge. */
 cafe_line(903,244,903,266,2,0xa39fbb);cr_disc(903,282,19,0x161827);
 for(int y=-14;y<=14;y+=5)for(int x=-14;x<=14;x+=5)if(x*x+y*y<260){
  unsigned shimmer=((unsigned)(x+14)*11+(unsigned)(y+14)*7+f->ms/75)%64;
  cr_rect(903+x,282+y,4,4,shimmer<15?0xeafcff:shimmer<32?a:0x576179);
 }
 if(f->section!=0&&f->section!=4)for(int i=0;i<16;i++){
  int x=64+(i*157+(int)f->ms/50)%1150,y=316+(i*61)%398;
  cr_rect(x,y,2+(i%2),2,cafe_mix_color(i%2?a:b,0x111127,65));
 }
}
static void cafe_show_console(const struct cafe_show_frame*f){
 unsigned a=cafe_show_color(f,0),b=cafe_show_color(f,1);int beat=255-(int)f->phase;
 /* Deck rings and illuminated trim frame the party without covering the cats. */
 cr_rect(478,420,337,2,cafe_mix_color(a,0x111127,60));
 cr_rect(479,517,337,3,b);cr_rect(479,420,2,100,a);cr_rect(814,420,2,100,b);
 const int platter_x[]={529,751};
 for(int deck=0;deck<2;deck++){
  int x=platter_x[deck];unsigned col=deck?b:a;
  cr_disc(x,453,18+f->bass/90,col);cr_disc(x,453,15,0x171327);cr_disc(x,453,4,0xe8daf6);
  int move=cafe_show_triangle((f->phase+(deck?128:0))&255,12)-6;
  cr_rect(x+move,439,3,4,0xffffff);
 }
 for(int i=0;i<9;i++){
  int h=4+(int)((i%3==0?f->bass:i%3==1?f->energy:f->high)*(18+(i*11)%17)/255);
  cr_rect(594+i*10,438,4,35,0x1c182a);cr_rect(594+i*10,473-h,4,h,i<5?a:b);
  cr_rect(594+i*10,473-h,4,2,0xf8eeff);
 }
 for(int i=0;i<8;i++){unsigned col=i%2?a:b;int on=(f->beat+i)%8<2||f->hit>180;
  cr_rect(510+i*12,478,8,4,on?col:cafe_mix_color(col,0x111127,180));
  cr_rect(713+i*12,478,8,4,on?col:cafe_mix_color(col,0x111127,180));}
 /* Reflections follow the floor's perspective; no full-screen flash. */
 for(int i=0;i<8;i++){
  int y=566+i*24,x=180-i*13;
  unsigned col=cafe_mix_color(i%2?a:b,0x111127,205-(beat*(int)f->bass/255)/10);
  cafe_line(x,y,1100+i*13,y,1,col);
 }
 for(int i=0;i<7;i++)cafe_line(450+i*54,532,60+i*193,745,1,cafe_mix_color(i%2?a:b,0x111127,220));
 for(int i=0;i<8;i++){int x=76+i*147,y=744;unsigned col=(f->beat/2+i)%4==0?a:cafe_mix_color(b,0x111127,140);cr_rect(x,y,95,3,col);}
}
static void cafe_vinyl(unsigned ms,int active,const struct cafe_show_frame*f){
 unsigned accent=active?cafe_show_color(f,1):0xe9b982;
 /* This is the existing shelf record, now with its own label and needle. */
 cr_disc(1050,384,30,0x221626);cr_disc(1050,384,27,accent);cr_disc(1050,384,24,0x100d17);
 for(int i=0;i<3;i++){int radius=11+i*4;for(int k=0;k<12;k++){
  static const signed char dx[]={100,87,50,0,-50,-87,-100,-87,-50,0,50,87};
  static const signed char dy[]={0,50,87,100,87,50,0,-50,-87,-100,-87,-50};
  int spin=active?(int)(ms/70%12):0,j=(k+spin)%12;
  cr_rect(1050+dx[j]*radius/100,384+dy[j]*radius/100,2,2,k<3?0x736475:0x34283c);
 }}
 cr_disc(1050,384,8,accent);cr_disc(1050,384,2,0xf8eeef);
 if(active){cafe_line(1078,359,1076,383,2,0xe3d9d4);cafe_line(1076,383,1062,391,2,0xe3d9d4);}
 cr_rect(1090,374,116,23,0x17121e);cr_text(1101,383,1,active?"STOP THE SET":"PLAY THE VINYL",accent);
 if(!active&&ms%3600<2000)cr_rect(1094,383,3,5,accent);
}
static void cafe_atmosphere(unsigned ms){
 /* Falling rain stays within the glass. Steam drifts up from both actual cups. */
 for(int i=0;i<16;i++){
  int x=142+(i*67)%215,y=257+(int)((ms/60+i*43)%122);
  cafe_line(x,y,x-3,y+8,1,0x47617f);
 }
 const int cups_x[]={402,1088},cups_y[]={436,458};
 for(int cup=0;cup<2;cup++)for(int i=0;i<3;i++){
  int rise=(int)((ms/100+i*11)%32);
  int x=cups_x[cup]+i*3+cat_bob(ms+i*370,1700,4);
  cr_rect(x,cups_y[cup]-rise,2,5,rise>22?0x8c7270:0xb6a18b);
 }
 /* Turntable indicators and tiny warm mixer meters sit in the painted booth. */
 for(int i=0;i<9;i++){
  int h=4+(int)((ms/(105+i*13)+i*7)%22);
  cr_rect(594+i*10,445,3,28,0x281e27);
  cr_rect(594+i*10,473-h,3,h,0xca814d);
  cr_rect(594+i*10,473-h,3,2,0xffd290);
 }
 int beat=(ms/160)%4;
 cr_rect(512+beat*12,478,9,3,0xff9b9f);
 cr_rect(725+(3-beat)*12,478,9,3,0xf4b5df);
}
static void cafe_bubble(int x,int y,unsigned speaker,const char*message,int emphasis){
 /* Wrap by words into a compact speech balloon above the speaking character. */
 char lines[3][31];int row=0,col=0,last_space=-1;for(int r=0;r<3;r++)lines[r][0]=0;
 for(int i=0;message[i]&&row<3;i++){
  if(col==29){if(last_space>=0){int leftover=col-last_space-1;lines[row][last_space]=0;row++;if(row>=3)break;for(int j=0;j<leftover;j++)lines[row][j]=lines[row-1][last_space+1+j];col=leftover;}else{lines[row][col]=0;row++;col=0;}last_space=-1;}
  if(row>=3)break;if(!col&&message[i]==' ')continue;lines[row][col]=message[i];if(message[i]==' ')last_space=col;col++;lines[row][col]=0;
 }
 int w=378,h=39+(row+1)*19,left=x-w/2,top=y-h-33;if(left<49)left=49;if(left+w>1231)left=1231-w;if(top<241)top=241;if(speaker==4){left=453;top=554;}
 cr_rect(left+4,top+4,w,h,0x261e31);cr_rect(left,top,w,h,0xf5e2c6);cr_rect(left,top,w,3,emphasis?0xffcd55:0xc3989b);
 cr_text(left+14,top+11,1,cafe_speaker_name(speaker),0x7d4556);
 for(int i=0;i<=row&&i<3;i++)cr_text(left+14,top+31+i*19,2,lines[i],0x372638);
 int tip=x;if(tip<left+20)tip=left+20;if(tip>left+w-30)tip=left+w-30;
 for(int i=0;i<13;i++)cr_rect(tip+i/2,top+h+i,14-i,1,0xf5e2c6);
}
static unsigned cafe_collect_cats(const struct cafe_state*c,unsigned ms,struct cafe_cat_draw cats[CAFE_CAT_CAPACITY],int px[4],int py[4]){
 cats[0]=(struct cafe_cat_draw){cafe_resident_sprite[0],145,398,2,ms,3};
 cats[1]=(struct cafe_cat_draw){cafe_resident_sprite[1],220,399,2,ms+517,0};
 cats[2]=(struct cafe_cat_draw){cafe_resident_sprite[2],211,512,3,ms+1011,2};
 cats[3]=(struct cafe_cat_draw){cafe_resident_sprite[3],1085,338,2,ms+441,0};
 cats[4]=(struct cafe_cat_draw){cafe_resident_sprite[4],120,689,3,ms+324,3};
 cats[5]=(struct cafe_cat_draw){cafe_resident_sprite[5],1183,687,3,ms+611,2};
 cats[6]=(struct cafe_cat_draw){cafe_resident_sprite[6],502+cat_bob(ms,17000,150),732,2,ms+300,ms%17000<8500?4:1};
 unsigned count=7,mask=c->seen|(c->arrival>=0?(1u<<c->arrival):0);
 static const int home_x[]={393,323,950,966},home_y[]={682,474,704,514};
 for(unsigned cat=0;cat<4;cat++){
  px[cat]=home_x[cat];py[cat]=home_y[cat];
  if(!(mask&(1u<<cat)))continue;
  int action=cat==3?2:cat==1?3:0;
  if(cat==2){px[cat]+=cat_bob(ms,11000,100)-50;action=ms%11000<5500?4:1;}
  if(c->arrival==(int)cat){unsigned age=ms-c->arrival_at;if(age<2700){
   int dest_x=px[cat],dest_y=py[cat];
   if(age<450){px[cat]=1248-(int)age*68/450;py[cat]=670;}
   else if(age<2100){px[cat]=1180+(dest_x-1180)*(int)(age-450)/1650;py[cat]=670;}
   else{int u=(int)age-2100;px[cat]=dest_x;py[cat]=670+(dest_y-670)*u/600-u*(600-u)/4000;}
   action=1;}else action=0;}
  if(c->mode==2||c->mode==3)action=0;
  cats[count++]=(struct cafe_cat_draw){rescue_sprite[cat],px[cat],py[cat],3,ms+cat*231,action};
 }
 return count;
}
static void cafe_room(struct cafe_state*c,unsigned ms){
 cafe_update(c,ms);
 struct cafe_show_frame show=cafe_show_frame(c->show_ms);
 cafe_backdrop_colors=cafe_room_palette;if(c->show_active)cafe_show_palette(&show);
 cr_rect(0,130,1280,670,0x080f1b);cr_rect(30,145,1220,620,0x251b2d);cr_rect(30,145,1220,3,0x64ffe1);
 cr_text(63,168,4,"PURR HOUSE CAFE",0x64ffe1);cr_rect(403,160,4,4,0x64ffe1);cr_rect(407,156,4,4,0x64ffe1);
 cr_text(65,210,2,c->show_active?"GOOD - LIVE WITH DJ-DRAGONX":"COFFEE, CATS & GOOD COMPANY",c->show_active?cafe_show_color(&show,1):0xd5b5b3);
 if(c->show_active){cr_rect(878,172,9,9,cafe_show_color(&show,0));cr_text(902,171,2,"LIVE SET",0xf5e2c6);cr_rect(878,199,147,3,0x49354c);cr_rect(878,199,(int)((c->show_ms<CAFE_SHOW_DURATION_MS?c->show_ms:CAFE_SHOW_DURATION_MS)*147/CAFE_SHOW_DURATION_MS),3,cafe_show_color(&show,0));}
 cr_rect(1080,163,120,44,0x763044);cr_text(1100,177,2,"CLOSE",0xffffff);
 /* The room uses the selected illustrated style: timber, velvet, rainy city,
  * hanging lamps and vinyl shelves. Characters remain separately animated. */
 cafe_backdrop(0,CAFE_ROOM_H,0,CAFE_ROOM_W);
 if(c->show_active)cafe_show_room_lights(&show);
 if(c->show_active)cafe_dragon_at(642,424-cafe_show_triangle(show.phase,3+(int)show.bass/80),3,show.ms,show.section==2||show.section==3||show.section==5);
 else cafe_dragon(ms,c->mode==1||c->mode==2);
 /* Restore the foreground console over his lower body, placing him behind it. */
 cafe_backdrop(86,112,213,386);
 cafe_atmosphere(ms);
 if(c->show_active)cafe_show_console(&show);
 cafe_vinyl(c->show_active?show.ms:ms,c->show_active,&show);
 /* Residents and arriving friends all pass in front of or behind each other. */
 struct cafe_cat_draw cats[CAFE_CAT_CAPACITY];int px[4],py[4];
 unsigned count=cafe_collect_cats(c,ms,cats,px,py);
 if(c->show_active)for(unsigned i=0;i<count;i++){
  struct cafe_cat_draw*cat=&cats[i];unsigned phase=(show.phase+i*21)&255;
  /* The same named cafe sprites wake up, wave and bounce on the actual beat. */
  cat->ms=show.ms+i*137;cat->y-=cafe_show_triangle(phase,3+(int)show.bass/22);
  cat->x+=cafe_show_triangle((show.phase+i*31)&255,4)-2;
  cat->action=show.section==4?0:3;
 }
 cafe_draw_cats(cats,count);
 cafe_draw_cat_names(cats,count);
 int stroll=cat_bob(ms,17000,150);
 unsigned mask=c->seen|(c->arrival>=0?(1u<<c->arrival):0);
 /* Reunion hearts stay readable above the completed character pass. */
 for(unsigned cat=0;cat<4;cat++)if(mask&(1u<<cat)){
  if(c->mode==1||c->mode==2)cr_heart(px[cat]+25,py[cat]-102-cat_bob(ms+cat*193,1300,13),2,0xee8dd0);
 }
 /* A few confetti trails and fireworks celebrate victory without flashing the room. */
 if(c->won&&!c->show_active){cafe_fireworks(ms);for(int i=0;i<22;i++){int x=60+(i*157)%1150,y=249+(int)((ms/22+i*97)%467);cr_rect(x,y,3,6,i%2?0xffcd55:0xee8dd0);}}
 unsigned speaker=c->speaker;const char*speech=cafe_speech(c);int sx=speaker<4?px[speaker]:650,sy=speaker<4?py[speaker]-78:384;
 if(speaker>=5){static const int guest_x[]={145,220,211,1085,120,1183,502},guest_y[]={398,399,512,338,689,687,732};unsigned g=(speaker-5)%7;sx=guest_x[g]+(g==6?stroll:0);sy=guest_y[g]-(g==0||g==1||g==3||g==6?52:78);}
 if(c->mode==1&&ms-c->arrival_at>2850){speaker=(unsigned)c->arrival;sx=px[speaker];sy=py[speaker]-78;speech=cafe_arrival_speech(speaker);}
 if((!c->show_active||c->mode==1)&&(c->mode||ms%6200<4600))cafe_bubble(sx,sy,speaker,speech,c->mode!=0);
 cr_text(62,772,1,c->show_active?"PAWS UP. LIGHTS DOWN. THIS ONE IS FOR THE CATS.":c->won?"THE MOUSE LOST. THE CATS WON. LET THE AFTERPARTY BEGIN.":"A LITTLE SAFE PLACE BETWEEN THE BIG ADVENTURES.",c->show_active?cafe_show_color(&show,1):0x8fa1b5);
}
#endif
