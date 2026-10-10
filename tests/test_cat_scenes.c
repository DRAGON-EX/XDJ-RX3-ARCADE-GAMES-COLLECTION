#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/cat_preview.c"
static uint32_t rear_cat[1280*800],front_cat[1280*800];
static struct cafe_cat_draw find_cafe_cat(const struct cafe_cat_draw*cats,unsigned count,unsigned id){
 for(unsigned i=0;i<count;i++)if(cats[i].id==id)return cats[i];
 assert(!"missing cafe cat");return cats[0];
}
static void capture_cafe_cat(struct cafe_cat_draw cat,uint32_t*pixels){
 memset(framebuffer,0,sizeof(framebuffer));cafe_chibi(cat.id,cat.x,cat.y,cat.s,cat.ms,cat.action);
 memcpy(pixels,framebuffer,sizeof(framebuffer));
}
static void check_cafe_overlap(struct cafe_cat_draw rear,struct cafe_cat_draw front){
 capture_cafe_cat(rear,rear_cat);capture_cafe_cat(front,front_cat);
 /* Deliberately submit the nearer cat first, as happened with the resident
  * stroller and rescued Yuki. Sprite pixels must survive every ground shadow. */
 struct cafe_cat_draw cats[]={front,rear};memset(framebuffer,0,sizeof(framebuffer));cafe_draw_cats(cats,2);
 unsigned overlap=0;for(unsigned i=0;i<1280*800;i++){
  if(front_cat[i])assert(framebuffer[i]==front_cat[i]);
  else if(rear_cat[i])assert(framebuffer[i]==rear_cat[i]);
  if(rear_cat[i]&&front_cat[i]&&rear_cat[i]!=front_cat[i])overlap++;
 }
 assert(overlap>100);
}
static void test_cafe_depth(void){
 struct cafe_state c;struct cafe_cat_draw cats[CAFE_CAT_CAPACITY];int px[4],py[4];
 cafe_open(&c,15,15,0,42);unsigned count=cafe_collect_cats(&c,0,cats,px,py);assert(count==11);
 struct cafe_cat_draw yuki=find_cafe_cat(cats,count,rescue_sprite[0]),stroller=find_cafe_cat(cats,count,7);
 assert(yuki.x==393&&yuki.y==682&&stroller.x==502&&stroller.y==732);
 /* Force a shared lane to verify both depth directions independently of the
  * current furniture layout or any path change that merely hides overlaps. */
 stroller.x=yuki.x;check_cafe_overlap(yuki,stroller);
 stroller.y=yuki.y-30;check_cafe_overlap(stroller,yuki);
 stroller.y=yuki.y+1;check_cafe_overlap(yuki,stroller);
 cafe_draw_cats(cats,count);
 static const unsigned expected[]={9,0,2,3,6,4,10,5,8,1,7};
 for(unsigned i=0;i<count;i++)assert(cats[i].id==expected[i]);
 /* Every rescued friend crosses behind the right-hand sleeping resident on
  * arrival. Compare real scene overlap pixels, including its floor shadow. */
 for(unsigned arrival=0;arrival<4;arrival++){
  cafe_open(&c,15^(1u<<arrival),15,0,42);cafe_update(&c,0);
  count=cafe_collect_cats(&c,450,cats,px,py);assert(count==11&&py[arrival]==670);
  struct cafe_cat_draw entering=find_cafe_cat(cats,count,rescue_sprite[arrival]);
  struct cafe_cat_draw resident=find_cafe_cat(cats,count,5);
  capture_cafe_cat(entering,rear_cat);capture_cafe_cat(resident,front_cat);cafe_room(&c,450);
  unsigned overlap=0;for(int y=620;y<670;y++)for(int x=1120;x<1230;x++){
   unsigned i=(unsigned)(y*1280+x);
   if(rear_cat[i]&&front_cat[i]&&rear_cat[i]!=front_cat[i]){assert(framebuffer[i]==front_cat[i]);overlap++;}
  }
  assert(overlap>100);
  /* Interpolated entrance feet, not the eventual seat, determine the order. */
  for(unsigned ms=0;ms<=2800;ms+=100){
   count=cafe_collect_cats(&c,ms,cats,px,py);cafe_draw_cats(cats,count);
   for(unsigned i=1;i<count;i++)assert(cats[i-1].y<=cats[i].y);
  }
 }
}
static void test_cafe_names_and_greetings(void){
 static const char*const expected_names[]={"SUNNY","COOKIE","LUNA","PEACH","MOCHI","CHAI","MISO","OREO","NALA","FUDGE","YUKI"};
 static const char*const speakers[]={"YUKI","PEACH","COOKIE","MOCHI","DJ DRAGON-X","SUNNY","LUNA","MISO","FUDGE","NALA","CHAI","OREO"};
 static const char*const greetings[]={
  "I MISSED YOU! NEVER LET THAT MOUSE NEAR ME AGAIN!",
  "I MISSED THIS PLACE! IT FEELS SO GOOD TO BE HOME.",
  "MY FAVORITE SOFA! MOVE OVER, I'M STAYING RIGHT HERE.",
  "I MISSED YOU ALL. CAN WE JUST STAY TOGETHER FOR A WHILE?"
 };
 for(unsigned i=0;i<CAT_NAME_COUNT;i++)assert(!strcmp(cat_name(i),expected_names[i]));
 for(unsigned i=0;i<12;i++)assert(!strcmp(cafe_speaker_name(i),speakers[i]));
 /* Check every centered name in the real quiet cafe, including the stroller. */
 struct cafe_state c;struct cafe_cat_draw cats[CAFE_CAT_CAPACITY];int px[4],py[4];
 cafe_open(&c,15,15,0,42);c.reunion=1;c.next=6000;
 unsigned count=cafe_collect_cats(&c,5000,cats,px,py);memset(framebuffer,0,sizeof(framebuffer));
 for(unsigned i=0;i<count;i++){
  const char*name=expected_names[cats[i].id];int width=(int)strlen(name)*6-1;
  cr_text(cats[i].x-width/2,cats[i].y+16,1,name,0xf4d69b);
 }
 memcpy(rear_cat,framebuffer,sizeof(framebuffer));cafe_room(&c,5000);
 for(unsigned i=0;i<1280*800;i++)if(rear_cat[i])assert(framebuffer[i]==rear_cat[i]);
 /* Each cat's actual arrival bubble must have its own name and complete line. */
 for(unsigned cat=0;cat<4;cat++){
  assert(!strcmp(cafe_arrival_speech(cat),greetings[cat]));
  for(unsigned other=0;other<cat;other++)assert(strcmp(cafe_arrival_speech(cat),cafe_arrival_speech(other)));
  unsigned rows=1,width=0;const char*word=greetings[cat];
  while(*word){unsigned length=0;while(word[length]&&word[length]!=' ')length++;assert(length<=29);
   if(width&&width+1+length>29){rows++;width=0;}width+=(width?1:0)+length;word+=length;if(*word)word++;
  }
  assert(rows<=3);
  cafe_open(&c,0,1u<<cat,0,42);cafe_update(&c,0);cafe_collect_cats(&c,3000,cats,px,py);
  memset(framebuffer,0,sizeof(framebuffer));cafe_bubble(px[cat],py[cat]-78,cat,greetings[cat],1);
  memcpy(rear_cat,framebuffer,sizeof(framebuffer));cafe_room(&c,3000);
  for(unsigned i=0;i<1280*800;i++)if(rear_cat[i])assert(framebuffer[i]==rear_cat[i]);
 }
}
static void test_cafe_vinyl_show(void){
 struct cafe_state c;cafe_open(&c,15,15,0,42);c.reunion=1;c.next=999999;
 assert(!c.show_active&&!c.show_ms);
 assert(cafe_vinyl_hit(1050,384)&&cafe_vinyl_hit(1008,344)&&cafe_vinyl_hit(1098,420));
 assert(!cafe_vinyl_hit(1007,384)&&!cafe_vinyl_hit(1050,421)&&!cafe_vinyl_hit(640,450));
 cafe_room(&c,15000);memcpy(rear_cat,framebuffer,sizeof(framebuffer));
 c.show_active=1;c.show_ms=45200;cafe_room(&c,15000);memcpy(front_cat,framebuffer,sizeof(framebuffer));
 assert(memcmp(rear_cat,front_cat,sizeof(framebuffer)));
 /* Lighting is driven by the song cursor, independent of a paused cafe timer. */
 c.show_ms=46000;cafe_room(&c,15000);assert(memcmp(front_cat,framebuffer,sizeof(framebuffer)));
 assert(c.seen==15&&c.mask==15&&c.arrival==-1);
 static const unsigned times[]={0,15148,30148,45148,60148,90148,105148,150148,165148,CAFE_SHOW_DURATION_MS-1};
 for(unsigned i=0;i<sizeof(times)/sizeof(times[0]);i++){c.show_ms=times[i];cafe_room(&c,15000);}
 /* Closing/stopping must restore the actual original room palette and scene. */
 c.show_active=0;cafe_room(&c,15000);assert(!memcmp(rear_cat,framebuffer,sizeof(framebuffer)));
 cats_open(0,0,0,42);cats_show(1,105200);assert(cafe.show_active&&cafe.show_ms==105200);
 cats_cafe(5000,0);assert(!cats_seen());cats_show(0,0);assert(!cafe.show_active);
}
int main(void){
 test_cafe_vinyl_show();
 test_cafe_depth();test_cafe_names_and_greetings();
 struct cafe_state c;cafe_open(&c,0,1,0,42);cafe_update(&c,0);assert(c.mode==1&&c.arrival==0&&!c.seen);cafe_update(&c,4999);assert(!c.seen);
 /* Closing during an arrival must not consume the next greeting. */
 cafe_open(&c,c.seen,1,0,42);cafe_update(&c,0);assert(c.arrival==0);cafe_update(&c,5000);assert(c.seen==1&&c.mode==0);
 unsigned seen=c.seen;cafe_open(&c,seen,15,0,42);for(unsigned t=0;t<15000;t+=5000){cafe_update(&c,t);assert(c.arrival==(int)(t/5000+1));}cafe_update(&c,15000);assert(c.seen==15&&c.mode==2);
 cafe_open(&c,15,15,1,42);cafe_update(&c,0);assert(c.mode==2);cafe_update(&c,6000);assert(c.mode==3);cafe_update(&c,12000);assert(c.mode==0);
 cafe_open(&c,1,1,0,913);unsigned hopes=0,phrases=0;for(unsigned t=0;t<200000;t+=7000){cafe_update(&c,t);assert(c.speaker==0||(c.speaker>=4&&c.speaker<12));assert(c.missing>0&&c.missing<4);if(strstr(cafe_speech(&c),"COMES BACK"))hopes++;phrases|=1u<<c.phrase;assert(strlen(cafe_speech(&c))*12<650);}assert(hopes&&phrases==255);
 cafe_open(&c,0,0,0,42);unsigned pleas_seen=0;
 for(unsigned t=0;t<200000;t+=7000){cafe_update(&c,t);assert(c.seen==0&&c.arrival==-1&&c.mode==0&&c.speaker>=4);pleas_seen|=1u<<c.phrase;assert(strlen(cafe_speech(&c))*12<650);}
 assert(pleas_seen==255);
 struct rescue_playback r;int16_t audio[1470];float gain=0;rescue_begin(&r,2);rescue_audio(&r,audio,735,44100,&gain,0);for(int i=0;i<1470;i++)assert(!audio[i]);rescue_audio(&r,audio,735,44100,&gain,1);int sound=0;for(int i=0;i<1470;i++)sound|=audio[i]!=0;assert(!sound); /* Host streams the supplied fanfare. */
 for(unsigned i=0;i<1199;i++)rescue_advance(&r);assert(r.active);rescue_advance(&r);assert(!r.active);
 /* Render real indexed artwork, arrivals, random dialogue and fireworks with bounds checking. */
 for(int neon=0;neon<2;neon++)for(unsigned cat=0;cat<4;cat++)for(unsigned ms=0;ms<5000;ms+=777)cats_rescue(cat,ms,neon);
 for(unsigned mask=1;mask<=15;mask++){cats_open(0,mask,mask==15,22);for(unsigned ms=0;ms<60000;ms+=1700)cats_cafe(ms,mask%2);assert(cats_seen()==mask);}
 printf("Cafe vinyl hit/clock/restore, names/individual greetings, feet-depth rendering/shadows, queued arrivals/reopen, all-home and victory dialogue, missing-cat chatter, rescue timing/mute and scene bounds passed.\n");
}
