#ifndef RX3_DJ_WARS_GAME_H
#define RX3_DJ_WARS_GAME_H
/* Original, allocation-free artillery simulation. 60 Hz; display target 30 Hz. */
#include <stdint.h>
#define DW_W 2640
#define DW_H 1600
#define DW_UNITS 10
#define DW_SHOTS 8
#define DW_PARTICLES 128
#define DW_BLASTS 8
#define DW_LEFT 1
#define DW_RIGHT 2
#define DW_UP 4
#define DW_DOWN 8
#define DW_FIRE 16
#define DW_JUMP 32
#define DW_WEAPON 64
#define DW_TARGET 128
#define DW_TURN_TICKS 5400
#define DW_AIM 0
#define DW_FLIGHT 1
#define DW_SETTLE 2
#define DW_OVER 3
#define DW_FINALE 4
#define DW_REVIVE_TICK 420
#define DW_BARRAGE_TICK 540
#define DW_BARRAGE_INTERVAL 54
#define DW_CELEBRATE_TICK 810
#define DW_FIREWORKS_TICKS 2700
#define DW_ATTACK_WEAPONS 4
#define DW_TELEPORT 4
#define DW_JETPACK 5
#define DW_WEAPON_COUNT 6
#define DW_VICTORY_TICKS DW_FIREWORKS_TICKS
#define DW_ASSIST_TICKS (DW_CELEBRATE_TICK+DW_FIREWORKS_TICKS)
struct dw_unit {float x,y,vx,vy;int hp,max_hp,team,kind,face,ground;float fall;int tools[2];unsigned blast_attack;int fall_limit;};
struct dw_shot {float x,y,vx,vy;int live,kind,life,age;};
struct dw_blast {float x,y;int radius,reach,life;};
struct dw_feedback {float x,y;int damage,life;};
struct dw_grave {float x,y,vy;int live;};
struct dw_particle {float x,y,vx,vy;int life,kind;};
struct dw_game {
 uint8_t terrain[DW_W*DW_H];
 struct dw_unit unit[DW_UNITS];struct dw_shot shot[DW_SHOTS];struct dw_particle particle[DW_PARTICLES];struct dw_blast blast[DW_BLASTS];
 uint32_t rng,ticks,terrain_version,event_serial;unsigned old_buttons;
 int team,active,last[2],turn,remaining,state,settle,winner,weapon,aim,power,charging,wind,target,ai_wait;
 int ammo[2][DW_ATTACK_WEAPONS],event,event_age,damage,flash,jet_ticks;
 unsigned attack_id;int attack_damage[DW_UNITS];
 struct dw_feedback feedback[DW_UNITS];struct dw_grave grave[DW_UNITS];int hit_pause,hit_focus;
 int finale_tick,finale_assisted,finale_result,finale_target,finale_shots;
 unsigned finale_started;
 float finale_x,finale_y,finale_home_x,finale_home_y;
 float mark_x,mark_y;
};
static const char*dw_names[10]={"DJ-DRAGONX","YUKI","PEACH","COOKIE","MOCHI","SYNC BUTTON LOVER","MC TRAINWRECKER","DJ REDLINE","CAPTAIN BAD DROP","DEADMAU5"};
static const char*dw_weapons[DW_WEAPON_COUNT]={"BAZOOKA","GRENADE","AIRSTRIKE","HOMING ROCKET","TELEPORT","JETPACK"};
static const float dw_quarter[17]={0,.098017f,.195090f,.290285f,.382683f,.471397f,.555570f,.634393f,.707107f,.773010f,.831470f,.881921f,.923880f,.956940f,.980785f,.995185f,1};
static float dw_sin(int a){a&=255;int neg=a>=128;if(neg)a-=128;if(a>64)a=128-a;int i=a/4;float v=i==16?1:dw_quarter[i]+(dw_quarter[i+1]-dw_quarter[i])*(a%4)*.25f;return neg?-v:v;}
static float dw_abs(float v){return v<0?-v:v;}
static float dw_clamp(float v,float a,float b){return v<a?a:v>b?b:v;}
static uint32_t dw_rand(struct dw_game*g){uint32_t x=g->rng;x^=x<<13;x^=x>>17;x^=x<<5;return g->rng=x?x:1;}
static int dw_solid(const struct dw_game*g,int x,int y){return x>=0&&x<DW_W&&y>=0&&y<DW_H&&g->terrain[y*DW_W+x];}
static int dw_alive(const struct dw_game*g,int team){int n=0;for(int i=0;i<10;i++)if(g->unit[i].team==team&&g->unit[i].hp>0)n++;return n;}
static int dw_health(const struct dw_game*g,int team){int n=0;for(int i=0;i<10;i++)if(g->unit[i].team==team)n+=g->unit[i].hp;return n;}
static int dw_nearest(const struct dw_game*g,int team,float x){int best=-1;float distance=1e9f;for(int i=0;i<10;i++)if(g->unit[i].team==team&&g->unit[i].hp>0){float d=dw_abs(g->unit[i].x-x);if(d<distance){distance=d;best=i;}}return best;}
static void dw_event(struct dw_game*g,int e){g->event=e;g->event_age=140;g->event_serial++;}
static void dw_sparks(struct dw_game*g,float x,float y,int count,int kind){for(int n=0;n<count;n++)for(int i=0;i<DW_PARTICLES;i++)if(!g->particle[i].life){int a=dw_rand(g)%256;float v=1+(dw_rand(g)%40)*.1f;g->particle[i]=(struct dw_particle){x,y,dw_sin(a)*v,-dw_sin(a+64)*v,25+(int)(dw_rand(g)%30),kind};break;}}
/* Presentation uses wall-clock ticks; combat and the turn clock pause on real damage. */
static void dw_damage_feedback(struct dw_game*g,int i,int damage){if(damage<=0)return;
 struct dw_unit*u=&g->unit[i];struct dw_feedback*f=&g->feedback[i];
 f->damage=f->life?f->damage+damage:damage;f->life=102;f->x=u->x;f->y=u->y;
 if(!g->hit_pause||f->damage>=g->feedback[g->hit_focus].damage)g->hit_focus=i;
 g->hit_pause=60;
 if(!u->hp&&!g->grave[i].live)g->grave[i]=(struct dw_grave){u->x,dw_clamp(u->y,40,DW_H-24),0,1};
}
static void dw_update_graves(struct dw_game*g){for(int i=0;i<DW_UNITS;i++){struct dw_grave*t=&g->grave[i];if(!t->live)continue;
 t->vy+=.18f;if(t->vy>8)t->vy=8;int steps=(int)t->vy+1;
 for(int n=0;n<steps;n++){float y=t->y+t->vy/steps;if(y>=DW_H-24){t->y=DW_H-24;t->vy=0;break;}if(dw_solid(g,(int)t->x,(int)y)){t->vy=0;break;}t->y=y;}
}}
/* A match only becomes won after its on-stage reunion. The guest rescue is an
   explicitly assisted result; held controls cannot shoot or skip the sequence. */
static void dw_finale_begin(struct dw_game*g,int result){
 g->state=DW_FINALE;g->winner=-1;g->finale_tick=0;g->finale_result=result;g->finale_started=g->ticks;
 g->finale_assisted=result!=0;g->finale_target=-1;g->finale_shots=0;
 g->finale_home_x=dw_clamp(g->unit[0].x-90,40,DW_W-40);
 g->finale_home_y=dw_clamp((g->grave[0].live?g->grave[0].y:g->unit[0].y)-45,100,DW_H-100);
 g->finale_x=dw_clamp(g->finale_home_x-210,40,DW_W-40);g->finale_y=g->finale_home_y-80;
 g->charging=g->jet_ticks=0;g->hit_pause=0;
 for(int i=0;i<DW_SHOTS;i++)g->shot[i].live=0;
 for(int i=0;i<DW_UNITS;i++){g->unit[i].vx=g->unit[i].vy=0;g->feedback[i].life=0;}
 dw_event(g,g->finale_assisted?11:14);
}
static void dw_revive_team(struct dw_game*g){
 for(int i=0;i<5;i++){
  struct dw_unit*u=&g->unit[i];
  if(!u->hp){
   /* Keep each cat beside its own grave. Only water/removed ground needs
      relocation, to the nearest intact floor at that same world X. */
   int found=0,x=(int)dw_clamp(u->x,24,DW_W-25);
   int desired=(int)dw_clamp(g->grave[i].live?g->grave[i].y:u->y,40,DW_H-25);
   for(int attempt=0;attempt<23&&!found;attempt++){
    int xx=attempt?24+(attempt-1)*117:x,best_y=-1,best_distance=DW_H;
    for(int y=40;y<DW_H-24;y++)if(dw_solid(g,xx,y)&&!dw_solid(g,xx,y-1)&&!dw_solid(g,xx,y-24)){
     int distance=(int)dw_abs((float)(y-desired));if(distance<best_distance){best_distance=distance;best_y=y;}
    }
    if(best_y>=0){u->x=(float)xx;u->y=(float)best_y;found=1;}
   }
   if(!found){u->x=(float)(260+i*130);u->y=900;} /* No terrain remains: a brief rescue hover. */
  }
  u->hp=u->max_hp;u->vx=u->vy=u->fall=0;u->blast_attack=0;u->ground=1;
  g->grave[i]=(struct dw_grave){0};g->feedback[i]=(struct dw_feedback){0};
  dw_sparks(g,u->x,u->y-20,12,0);
 }
 dw_event(g,12);
}
static int dw_finale_focus_target(const struct dw_game*g){
 int t=g->finale_tick;if(!g->finale_assisted||t<DW_BARRAGE_TICK-72||t>=DW_CELEBRATE_TICK)return -1;
 int slot=t<DW_BARRAGE_TICK?0:(t-DW_BARRAGE_TICK+DW_BARRAGE_INTERVAL-18)/DW_BARRAGE_INTERVAL;
 return 5+(slot>4?4:slot);
}
static int dw_finale_beam_target(const struct dw_game*g){
 int t=g->finale_tick;if(t<DW_BARRAGE_TICK||t>=DW_CELEBRATE_TICK)return -1;
 int slot=(t-DW_BARRAGE_TICK)/DW_BARRAGE_INTERVAL;
 return g->finale_target==5+slot&&(t-DW_BARRAGE_TICK)%DW_BARRAGE_INTERVAL<18?g->finale_target:-1;
}
static void dw_finale_move_master(struct dw_game*g){
 int target=dw_finale_focus_target(g);float x=g->finale_home_x,y=g->finale_home_y;
 if(target>=0){
  const struct dw_unit*u=&g->unit[target];
  static const int offsets[]={-130,-95,-60,60,95,130,0};int found=0;
  /* Choose free air in the target's own cavern. Pushing upward through solid
     terrain would incorrectly move the guest onto a different floor. */
  x=u->x;y=dw_clamp(u->y-22,100,DW_H-100);
  for(int up=0;up<7&&!found;up++)for(int j=0;j<7&&!found;j++){
   int xx=(int)dw_clamp(u->x+offsets[j],40,DW_W-40),yy=(int)dw_clamp(u->y-14-up*14,100,DW_H-100),clear=1;
   for(int dy=5;dy<=75;dy+=14)for(int dx=-19;dx<=19;dx+=19)if(dw_solid(g,xx+dx,yy-dy))clear=0;
   if(clear){x=(float)xx;y=(float)yy;found=1;}
  }
 }
 /* A fast magical glide connects the clear hovering positions without moving
    any fighters or cutting new holes in the actual battlefield. */
 g->finale_x+=(x-g->finale_x)*.13f;g->finale_y+=(y-g->finale_y)*.13f;
}
static void dw_finale_tick(struct dw_game*g){
 int t=++g->finale_tick;
 if(g->finale_assisted)dw_finale_move_master(g);
 if(g->finale_assisted){
  if(t==120)dw_event(g,15);
  if(t==DW_REVIVE_TICK)dw_revive_team(g);
  /* Five scripted, precise shots. These are a guest-DJ cinematic, not combat
     damage: regular weapons retain their two-hit cap and forgiving AI. */
  if(t>=DW_BARRAGE_TICK&&t<DW_CELEBRATE_TICK&&(t-DW_BARRAGE_TICK)%DW_BARRAGE_INTERVAL==0){
   int i=5+(t-DW_BARRAGE_TICK)/DW_BARRAGE_INTERVAL;
   if(i<DW_UNITS&&g->unit[i].hp){
    struct dw_unit*u=&g->unit[i];u->hp=0;u->vx=u->vy=0;
    g->grave[i]=(struct dw_grave){u->x,dw_clamp(u->y,40,DW_H-24),0,1};
    g->finale_target=i;g->finale_shots++;dw_sparks(g,u->x,u->y-15,20,1);dw_event(g,13);
   }
  }
  if(t==DW_CELEBRATE_TICK)dw_event(g,14);
 }else if(t==180)dw_revive_team(g);
 if(t>=(g->finale_assisted?DW_ASSIST_TICKS:DW_VICTORY_TICKS)){
  g->state=DW_OVER;g->winner=0;g->team=0;g->active=0;g->finale_target=-1;g->event_age=0;
 }
}
static void dw_check_end(struct dw_game*g){
 if(g->hit_pause||g->state==DW_FINALE||g->state==DW_OVER)return;
 int a=dw_alive(g,0),b=dw_alive(g,1);if(!a||!b)dw_finale_begin(g,a?0:b?1:2);
}
static void dw_select_target(struct dw_game*g){g->target=dw_nearest(g,1-g->team,g->unit[g->active].x);if(g->target>=0){g->mark_x=g->unit[g->target].x;g->mark_y=g->unit[g->target].y-12;}}
static void dw_next_turn(struct dw_game*g){if(g->hit_pause)return;dw_check_end(g);if(g->state==DW_OVER||g->state==DW_FINALE)return;g->team=1-g->team;g->turn++;int base=g->team*5;for(int n=1;n<=5;n++){int i=base+(g->last[g->team]+n)%5;if(g->unit[i].hp>0){g->active=i;g->last[g->team]=i-base;break;}}g->remaining=DW_TURN_TICKS;g->state=DW_AIM;g->settle=0;g->aim=32;g->power=20;g->charging=0;g->jet_ticks=0;g->weapon=0;g->ai_wait=100;g->wind=(int)(dw_rand(g)%81)-40;g->unit[g->active].face=g->team?-1:1;dw_select_target(g);dw_event(g,1);}
static void dw_reset(struct dw_game*g,unsigned seed){*g=(struct dw_game){0};g->rng=seed?seed:1;g->terrain_version=1;g->attack_id=1;g->active=0;g->last[0]=0;g->last[1]=4;g->turn=1;g->remaining=DW_TURN_TICKS;g->aim=32;g->power=20;g->winner=-1;g->finale_target=-1;g->finale_result=-1;g->wind=12;
 /* Two irregular terrain layers with an open passage between them. */
 const int upper[][3]={
 {60,950,950},{150,720,980},{200,430,900},{285,540,890},{340,250,960},
 {395,645,940},{535,700,1010},{610,470,945},{710,830,980},{795,750,1030},
 {860,380,905},{940,350,965},{1000,615,930},{1110,580,985},{1195,375,895},
 {1260,460,860},{1320,900,900},{1420,880,880},{1510,735,980},{1600,820,1000},
 {1680,500,925},{1720,280,990},{1800,640,920},{1950,720,1010},
 {2070,470,950},{2180,385,975},{2250,230,885},{2320,565,930},
 {2450,690,990},{2580,820,970},{2610,990,990}};
 for(unsigned k=0;k<sizeof(upper)/sizeof(upper[0])-1;k++){
  if(k==16)continue; /* Gap between the two upper islands. */
  int ax=upper[k][0],bx=upper[k+1][0];
  for(int x=ax;x<bx;x++){
   int top=upper[k][1]+(upper[k+1][1]-upper[k][1])*(x-ax)/(bx-ax)+(int)(dw_sin(x*2)*5);
   int bottom=upper[k][2]+(upper[k+1][2]-upper[k][2])*(x-ax)/(bx-ax)+(int)(dw_sin(x*3)*8);
   for(int y=top;y<bottom;y++)g->terrain[y*DW_W+x]=1;
  }
 }
 const int lower[][2]={{45,1575},{110,1390},{235,1270},{370,1340},{465,1210},
 {565,1190},{675,1300},{805,1420},{925,1260},{1070,1175},{1190,1270},
 {1315,1435},{1400,1370},{1530,1210},{1640,1255},{1740,1380},{1850,1240},
 {1930,1180},{2030,1210},{2160,1320},{2250,1190},{2370,1280},{2490,1400},{2610,1575}};
 for(unsigned k=0;k<sizeof(lower)/sizeof(lower[0])-1;k++){
  int ax=lower[k][0],bx=lower[k+1][0];
  for(int x=ax;x<bx;x++){
   int top=lower[k][1]+(lower[k+1][1]-lower[k][1])*(x-ax)/(bx-ax)+(int)(dw_sin(x*2)*5);
   for(int y=top;y<DW_H-24;y++)g->terrain[y*DW_W+x]=1;
  }
 }
 int positions[10]={1195,200,460,710,565,1510,1720,1930,2180,2480};
 for(int i=0;i<10;i++){int x=positions[i],y=(i==4||i==7)?1100:0;while(y<DW_H&&dw_solid(g,x,y))y++;while(y<DW_H&&!dw_solid(g,x,y))y++;int hp=i==0?140:i==9?180:100;g->unit[i]=(struct dw_unit){(float)x,(float)y,0,0,hp,hp,i/5,i==0?0:i<5?1:i==9?3:2,i<5?1:-1,1,0,{0,0}};if(i==4||i==7)g->unit[i].tools[0]=g->unit[i].tools[1]=2;}
 for(int t=0;t<2;t++){g->ammo[t][0]=99;g->ammo[t][1]=5;g->ammo[t][2]=2;g->ammo[t][3]=3;}dw_select_target(g);dw_event(g,1);
}
static int dw_body_clear(const struct dw_game*g,float x,float y){return !dw_solid(g,(int)x-6,(int)y-3)&&!dw_solid(g,(int)x+6,(int)y-3)&&!dw_solid(g,(int)x,(int)y-13);}
static int dw_pose_clear(const struct dw_game*g,int x,int y){for(int yy=y-38;yy<y;yy++)for(int xx=x-16;xx<=x+16;xx++)if(dw_solid(g,xx,yy))return 0;return 1;}
static int dw_ammo(const struct dw_game*g,int w){if(w<0||w>=DW_WEAPON_COUNT)return 0;return w<DW_ATTACK_WEAPONS?g->ammo[g->team][w]:g->unit[g->active].tools[w-DW_TELEPORT];}
static void dw_move_unit(struct dw_game*g,struct dw_unit*u){if(u->hp<=0)return;
 int flying=g->jet_ticks>0&&u==&g->unit[g->active];
 float nx=dw_clamp(u->x+u->vx,9,DW_W-9);if(flying?dw_pose_clear(g,(int)nx,(int)u->y):dw_body_clear(g,nx,u->y))u->x=nx;else if(u->ground&&dw_body_clear(g,nx,u->y-4)){u->x=nx;u->y-=4;}else u->vx=0;
 if(!flying)u->vy+=.18f;if(u->vy>8)u->vy=8;
 if(flying||u->vy<0)u->fall=0;
 int n=(int)dw_abs(u->vy)+1;float step=u->vy/n;u->ground=0;
 for(int k=0;k<n;k++){float y=u->y+step;
  if(step>=0&&(dw_solid(g,(int)u->x-4,(int)y)||dw_solid(g,(int)u->x+4,(int)y))){
   if(!flying&&u->fall>85){int damage=(int)((u->fall-85)*.22f);if(u->blast_attack&&damage>u->fall_limit)damage=u->fall_limit;if(damage>u->hp)damage=u->hp;if(u->blast_attack==g->attack_id)g->attack_damage[u-g->unit]+=damage;u->hp-=damage;g->damage=damage;dw_sparks(g,u->x,u->y,10,1);dw_event(g,9);dw_damage_feedback(g,(int)(u-g->unit),damage);}
   u->fall=0;u->blast_attack=0;u->vy=0;u->ground=1;break;
  }
  if(step<0&&(y<38||(flying?!dw_pose_clear(g,(int)u->x,(int)y):dw_solid(g,(int)u->x,(int)y-38)))){u->vy=0;break;}
  u->y=y;if(step>0&&!flying)u->fall+=step;
 }
 u->vx*=flying?.9f:u->ground?.7f:.98f;if(u->y>DW_H-17){int lost=u->hp;u->hp=0;dw_damage_feedback(g,(int)(u-g->unit),lost);dw_sparks(g,u->x,DW_H-22,20,2);dw_event(g,5);}
}
/* Destinations must fit the entire sprite, have ground below and avoid other fighters. */
static int dw_teleport_spot(const struct dw_game*g,int x,int requested_y,int*out_y){
 if(x<20||x>=DW_W-20||requested_y<0||requested_y>=DW_H-24)return 0;
 int y=DW_H-24;for(int xx=x-16;xx<=x+16;xx++){int floor=requested_y;while(floor<DW_H-24&&!dw_solid(g,xx,floor))floor++;if(floor<y)y=floor;}
 if(y>=DW_H-24||y<40)return 0;
 if(!dw_pose_clear(g,x,y))return 0;
 for(int i=0;i<10;i++)if(i!=g->active&&g->unit[i].hp&&dw_abs(g->unit[i].x-x)<35&&dw_abs(g->unit[i].y-y)<45)return 0;
 *out_y=y;return 1;
}
static int dw_teleport(struct dw_game*g){struct dw_unit*u=&g->unit[g->active];int y;
 if(!u->tools[0]||!dw_teleport_spot(g,(int)g->mark_x,(int)g->mark_y,&y)){dw_event(g,10);return 0;}
 dw_sparks(g,u->x,u->y-15,20,0);u->x=g->mark_x;u->y=(float)y;u->vx=u->vy=u->fall=0;u->blast_attack=0;u->ground=1;u->tools[0]--;g->weapon=0;g->charging=0;
 dw_sparks(g,u->x,u->y-15,20,0);g->state=DW_SETTLE;g->settle=60;dw_event(g,7);return 1;
}
static void dw_jetpack(struct dw_game*g){struct dw_unit*u=&g->unit[g->active];if(!u->tools[1])return;u->tools[1]--;u->fall=0;u->blast_attack=0;u->vy=-2;g->jet_ticks=600;g->charging=0;dw_event(g,8);}
static void dw_explode(struct dw_game*g,float x,float y,int radius,int force){int left=(int)dw_clamp(x-radius,0,DW_W-1),right=(int)dw_clamp(x+radius,0,DW_W-1),top=(int)dw_clamp(y-radius,0,DW_H-1),bottom=(int)dw_clamp(y+radius,0,DW_H-1);int reach=radius+32;for(int yy=top;yy<=bottom;yy++)for(int xx=left;xx<=right;xx++){float dx=xx-x,dy=yy-y;if(dx*dx+dy*dy<radius*radius)g->terrain[yy*DW_W+xx]=0;}g->terrain_version++;g->flash=12;g->damage=0;
 for(int i=0;i<10;i++){struct dw_unit*u=&g->unit[i];if(!u->hp)continue;float dx=u->x-x,dy=u->y-10-y,dist2=dx*dx+dy*dy,r=(float)reach;if(dist2<r*r){int damage=8+(int)(force*(1-dist2/(r*r)));int budget=50-g->attack_damage[i];if(damage>budget)damage=budget;if(damage<0)damage=0;if(damage>u->hp)damage=u->hp;g->attack_damage[i]+=damage;u->blast_attack=g->attack_id;u->fall_limit=50-g->attack_damage[i];u->hp-=damage;g->damage+=damage;dw_damage_feedback(g,i,damage);u->vx+=(dx>=0?1:-1)*(2+damage*.035f);u->vy=-2-damage*.035f;u->ground=0;if(!u->hp)dw_sparks(g,u->x,u->y-12,15,u->team?1:0);}}
 int slot=0;for(int i=0;i<DW_BLASTS;i++){if(!g->blast[i].life){slot=i;break;}if(g->blast[i].life<g->blast[slot].life)slot=i;}
 g->blast[slot]=(struct dw_blast){x,y,radius,reach,36};
 dw_sparks(g,x,y,45,1);dw_event(g,2);
}
static void dw_detonate(struct dw_game*g,int i){struct dw_shot*s=&g->shot[i];int radius=s->kind==1?92:s->kind==2?58:76,damage=s->kind==1?52:s->kind==2?30:42;if(g->unit[g->active].kind==3)damage=damage*6/5;s->live=0;dw_explode(g,s->x,s->y,radius,damage);}
static void dw_choose_weapon(struct dw_game*g,int w){if(g->state==DW_AIM&&!g->team&&!g->hit_pause&&!g->jet_ticks&&w>=0&&w<DW_WEAPON_COUNT&&dw_ammo(g,w)){g->weapon=w;g->charging=0;g->power=20;}}
static void dw_mark(struct dw_game*g,int x,int y){if(g->state==DW_AIM&&!g->team&&!g->hit_pause){g->mark_x=dw_clamp(x,0,DW_W-1);g->mark_y=dw_clamp(y,0,DW_H-25);g->target=-1;}}
static void dw_fire(struct dw_game*g){if(g->hit_pause||g->state!=DW_AIM||!dw_ammo(g,g->weapon)||g->unit[g->active].hp<=0)return;struct dw_unit*u=&g->unit[g->active];int w=g->weapon;if(w==DW_TELEPORT){dw_teleport(g);return;}if(w==DW_JETPACK){dw_jetpack(g);return;}if(!++g->attack_id)g->attack_id=1;for(int j=0;j<DW_UNITS;j++)g->attack_damage[j]=0;if(w)g->ammo[g->team][w]--;g->state=DW_FLIGHT;g->charging=0;g->settle=0;
 if(w==2){for(int i=0;i<5;i++)g->shot[i]=(struct dw_shot){dw_clamp(g->mark_x+(i-2)*27,4,DW_W-5),-30-i*28,0,3,1,2,360,0};dw_event(g,3);}
 else{float speed=3+g->power*.125f,dx=u->face*dw_sin(g->aim+64),dy=-dw_sin(g->aim);g->shot[0]=(struct dw_shot){u->x+dx*18,u->y-17+dy*18,dx*speed,dy*speed,1,w,w==1?180:480,0};dw_event(g,6);}
}
static void dw_cycle_target(struct dw_game*g){for(int k=1;k<=10;k++){int i=(g->target+k+10)%10;if(g->unit[i].team!=g->team&&g->unit[i].hp>0){g->target=i;g->mark_x=g->unit[i].x;g->mark_y=g->unit[i].y-10;break;}}}
static void dw_ai(struct dw_game*g){if(--g->ai_wait>0)return;dw_select_target(g);if(g->target<0)return;struct dw_unit*u=&g->unit[g->active],*t=&g->unit[g->target];u->face=t->x<u->x?-1:1;g->weapon=0;
 if(u->y>1050&&u->tools[0]){int dest;
  for(int off=100;off<=350;off+=25){int x=(int)t->x+u->face*off;if(dw_teleport_spot(g,x,0,&dest)&&dest<1050){g->mark_x=(float)x;g->mark_y=0;dw_teleport(g);g->ai_wait=45;return;}}
 }
 if(u->y>1050&&!u->tools[0]&&u->tools[1]){g->weapon=DW_JETPACK;dw_jetpack(g);return;}
 /* Forgiving AI: coarse estimates, noticeable aim errors and rare specials. */
 unsigned pick=dw_rand(g)%100;
 if(pick<20&&g->ammo[1][1])g->weapon=1;
 else if(g->turn>=8){
  if(pick>=90&&pick<94&&g->ammo[1][2])g->weapon=2;
  else if(pick>=94&&pick<97&&g->ammo[1][3])g->weapon=3;
 }
 int error=(int)(dw_rand(g)%181)+60;if(dw_rand(g)&1)error=-error;
 float tx=dw_clamp(t->x+error,20,DW_W-20),ty=dw_clamp(t->y+(int)(dw_rand(g)%121)-60,40,DW_H-25);
 float estimated_wind=g->wind*.4f+(int)(dw_rand(g)%51)-25;
 float best=1e20f;int ba=32,bp=70;
 for(int a=16;a<=52;a+=12)for(int power=35;power<=95;power+=20){float speed=3+power*.125f,vx=u->face*dw_sin(a+64)*speed,vy=-dw_sin(a)*speed,x=u->x+u->face*dw_sin(a+64)*18,y=u->y-17-dw_sin(a)*18;for(int n=0;n<240;n++){vx+=estimated_wind*.0007f;vy+=.14f;x+=vx;y+=vy;if(n>3&&(dw_solid(g,(int)x,(int)y)||y>=DW_H-20||x<0||x>=DW_W))break;}float dx=x-tx,dy=y-ty,score=dx*dx+dy*dy;if(score<best){best=score;ba=a;bp=power;}}
 g->aim=(int)dw_clamp(ba+(int)(dw_rand(g)%13)-6,8,60);
 g->power=(int)dw_clamp(bp+(int)(dw_rand(g)%31)-15,20,100);
 /* Guided enemy weapons aim at an imperfect fixed point, not a live fighter lock. */
 if(g->weapon>=2){g->mark_x=tx;g->mark_y=ty-12;g->target=-1;}
 dw_fire(g);
}
static void dw_tick(struct dw_game*g,unsigned buttons){g->ticks++;unsigned edge=buttons&~g->old_buttons;g->old_buttons=buttons;if(g->event_age)g->event_age--;if(g->flash)g->flash--;for(int i=0;i<DW_BLASTS;i++)if(g->blast[i].life)g->blast[i].life--;for(int i=0;i<DW_UNITS;i++)if(g->feedback[i].life)g->feedback[i].life--;
 for(int i=0;i<DW_PARTICLES;i++){struct dw_particle*p=&g->particle[i];if(p->life){p->life--;p->x+=p->vx;p->y+=p->vy;p->vy+=.065f;}}
 if(g->state==DW_FINALE){dw_finale_tick(g);return;}
 if(g->hit_pause){g->hit_pause--;return;}
 dw_update_graves(g);if(g->state==DW_OVER)return;
 dw_check_end(g);if(g->state==DW_FINALE)return;
 if(g->jet_ticks){struct dw_unit*u=&g->unit[g->active];unsigned steer=buttons;
  if(g->team){steer=DW_UP;if(dw_solid(g,(int)u->x,(int)u->y-45))steer|=u->face>0?DW_RIGHT:DW_LEFT;else if(u->y<600)steer|=u->face>0?DW_RIGHT:DW_LEFT;if(u->y<300)g->jet_ticks=1;}
  u->vx=((steer&DW_RIGHT)?3.2f:0)-((steer&DW_LEFT)?3.2f:0);u->vy=((steer&DW_DOWN)?3.4f:0)-((steer&DW_UP)?4.2f:0);if(u->vx)u->face=u->vx>0?1:-1;
  if(g->ticks%4==0)dw_sparks(g,u->x,u->y,2,1);
  if(!--g->jet_ticks||(!g->team&&(edge&DW_FIRE))||!u->hp){g->jet_ticks=0;g->weapon=0;g->old_buttons=buttons;edge&=~DW_FIRE;u->vy=0;u->fall=0;}
 }
 for(int i=0;i<10;i++)dw_move_unit(g,&g->unit[i]);
 if(g->hit_pause)return;
 if(g->state==DW_AIM){struct dw_unit*u=&g->unit[g->active];if(!u->hp){dw_next_turn(g);return;}if(--g->remaining<=0){dw_next_turn(g);return;}
  if(g->jet_ticks)return;
  if(g->team)dw_ai(g);else{
   if(!g->charging){if(buttons&DW_LEFT){u->vx=-1.4f;u->face=-1;}if(buttons&DW_RIGHT){u->vx=1.4f;u->face=1;}if((edge&DW_JUMP)&&u->ground){u->vy=-4.8f;u->ground=0;u->vx=u->face*1.8f;}}
   if(buttons&DW_UP){if(g->aim<60)g->aim++;}if(buttons&DW_DOWN){if(g->aim>4)g->aim--;}
   if(edge&DW_WEAPON)for(int i=1;i<=DW_WEAPON_COUNT;i++){int w=(g->weapon+i)%DW_WEAPON_COUNT;if(dw_ammo(g,w)){dw_choose_weapon(g,w);break;}}
   if(edge&DW_TARGET)dw_cycle_target(g);
   if(edge&DW_FIRE){if(g->weapon==2||g->weapon>=DW_TELEPORT){g->power=70;dw_fire(g);}else{g->charging=1;g->power=20;}}
   if(g->charging){if(buttons&DW_FIRE){if(g->power<100)g->power++;}else dw_fire(g);}
  }
 }
 if(g->state==DW_FLIGHT){int active=0;for(int i=0;i<DW_SHOTS;i++){struct dw_shot*s=&g->shot[i];if(!s->live)continue;active++;s->age++;s->life--;
  if(s->kind==3&&s->age>18){if(g->target>=0&&g->unit[g->target].hp>0){g->mark_x=g->unit[g->target].x;g->mark_y=g->unit[g->target].y-12;}float dx=g->mark_x-s->x,dy=g->mark_y-s->y,n=dw_abs(dx)+dw_abs(dy)+.001f;s->vx=s->vx*.975f+dx/n*.3f;s->vy=s->vy*.975f+dy/n*.3f;}
  else{s->vx+=g->wind*.0007f;s->vy+=.14f;}
  int steps=(int)(dw_abs(s->vx)+dw_abs(s->vy))+1;if(steps>64)steps=64;int touched=0;
  for(int k=0;k<steps&&s->live;k++){float nx=s->x+s->vx/steps,ny=s->y+s->vy/steps;if(dw_solid(g,(int)nx,(int)ny)){if(s->kind==1){if(dw_solid(g,(int)nx,(int)s->y))s->vx*=-.6f;else s->vy*=-.58f;s->vx*=.86f;touched=1;break;}else{dw_detonate(g,i);break;}}s->x=nx;s->y=ny;for(int j=0;j<10&&s->live;j++)if(g->unit[j].hp>0&&(j!=g->active||s->age>16)&&s->kind!=1){float dx=s->x-g->unit[j].x,dy=s->y-(g->unit[j].y-13);if(dx*dx+dy*dy<140)dw_detonate(g,i);}}
  if(!s->live)continue;if(!s->life){dw_detonate(g,i);continue;}if(s->x<0||s->x>=DW_W||s->y>DW_H-20||s->y<-250){s->live=0;continue;}if(g->ticks%3==0&&!touched)dw_sparks(g,s->x,s->y,1,3);
 }
 if(!active){g->state=DW_SETTLE;g->settle=110;}}
 if(!g->hit_pause&&g->state==DW_SETTLE&&!--g->settle)dw_next_turn(g);
 dw_check_end(g);
}
#endif
