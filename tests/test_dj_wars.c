#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../Games/Native/DJ Wars/game.h"
#include "../native/font.h"
#include "../Games/Native/DJ Wars/render.h"
static struct dw_game g;
static unsigned pixels[1280*800];
static void reset(void){dw_reset(&g,7);}
static int solids(void){int n=0;for(int i=0;i<DW_W*DW_H;i++)n+=g.terrain[i]!=0;return n;}
static void sane(void){assert(g.active>=0&&g.active<10);assert(g.team==0||g.team==1);assert(g.state>=0&&g.state<=DW_FINALE);for(int i=0;i<10;i++){assert(isfinite(g.unit[i].x)&&isfinite(g.unit[i].y));assert(g.unit[i].hp>=0&&g.unit[i].hp<=g.unit[i].max_hp);}for(int i=0;i<DW_SHOTS;i++)if(g.shot[i].live){assert(isfinite(g.shot[i].x)&&isfinite(g.shot[i].y));assert(isfinite(g.shot[i].vx)&&isfinite(g.shot[i].vy));}}
static void mobility_fixture(void){
 reset();for(int i=0;i<DW_W*DW_H;i++)g.terrain[i]=0;
 for(int y=1200;y<DW_H-24;y++)for(int x=0;x<DW_W;x++)g.terrain[y*DW_W+x]=1;
 for(int y=600;y<720;y++)for(int x=800;x<1000;x++)g.terrain[y*DW_W+x]=1;
 for(int i=0;i<10;i++){g.unit[i].x=100+i*200;g.unit[i].y=1200;g.unit[i].vx=g.unit[i].vy=g.unit[i].fall=0;}
 g.active=4;g.unit[4].x=500;g.team=0;g.last[0]=4;
}
static int fall_damage(int height){mobility_fixture();g.unit[4].y-=height;for(int n=0;n<250;n++)dw_move_unit(&g,&g.unit[4]);return 100-g.unit[4].hp;}
static void test_mobility(void){
 reset();assert(g.unit[4].tools[0]==2&&g.unit[4].tools[1]==2&&g.unit[7].tools[0]==2&&g.unit[7].tools[1]==2);assert(g.unit[0].tools[0]==0);
 mobility_fixture();dw_choose_weapon(&g,5);dw_mark(&g,900,500);dw_tick(&g,DW_FIRE);assert(g.unit[4].x==900&&g.unit[4].y==600&&g.unit[4].tools[0]==1);assert(g.state==DW_SETTLE&&g.turn==1&&g.unit[4].fall==0);
 dw_choose_weapon(&g,6);assert(g.weapon==0); /* No more actions after teleport. */
 for(int n=0;n<60;n++)dw_tick(&g,0);assert(g.turn==2&&g.team==1&&g.remaining==DW_TURN_TICKS-1);
 /* Return to this fighter later: the remaining use survives the turn change. */
 g.active=4;g.team=0;g.state=DW_AIM;g.remaining=DW_TURN_TICKS;
 dw_choose_weapon(&g,5);dw_mark(&g,900,680);dw_tick(&g,DW_FIRE);assert(g.unit[4].tools[0]==1&&g.event==10&&g.state==DW_AIM); /* Inside terrain. */
 dw_tick(&g,0);dw_mark(&g,(int)g.unit[1].x,1000);dw_tick(&g,DW_FIRE);assert(g.unit[4].tools[0]==1&&g.state==DW_AIM); /* Occupied landing. */
 dw_tick(&g,0);dw_mark(&g,600,1000);dw_tick(&g,DW_FIRE);assert(!g.unit[4].tools[0]&&g.unit[4].hp==100&&g.state==DW_SETTLE);dw_choose_weapon(&g,5);assert(g.weapon==0);
 mobility_fixture();dw_choose_weapon(&g,6);dw_tick(&g,DW_FIRE);assert(g.jet_ticks==600&&g.unit[4].tools[1]==1);for(int n=0;n<50;n++)dw_tick(&g,DW_UP);assert(g.unit[4].y<1010&&g.unit[4].hp==100&&g.jet_ticks==550);dw_tick(&g,DW_FIRE);assert(!g.jet_ticks&&g.weapon==0);for(int n=0;n<180;n++)dw_tick(&g,0);assert(g.unit[4].hp<100); /* Dropping after stopping can hurt. */
 mobility_fixture();g.unit[4].x=900;g.unit[4].y=850;dw_choose_weapon(&g,6);dw_tick(&g,DW_FIRE);for(int n=0;n<100;n++)dw_tick(&g,DW_UP);assert(g.unit[4].y>=758&&g.unit[4].hp==100); /* Ceiling collision. */
 g.remaining=1;dw_tick(&g,0);assert(g.team==1&&!g.jet_ticks);
 mobility_fixture();dw_choose_weapon(&g,6);dw_tick(&g,DW_FIRE);for(int n=0;n<600;n++)dw_tick(&g,0);assert(!g.jet_ticks&&g.unit[4].tools[1]==1);dw_choose_weapon(&g,6);dw_tick(&g,DW_FIRE);assert(g.unit[4].tools[1]==0&&g.jet_ticks);dw_tick(&g,0);dw_tick(&g,DW_FIRE);dw_choose_weapon(&g,6);assert(g.weapon==0);
 assert(fall_damage(40)==0);int medium=fall_damage(200),high=fall_damage(350);assert(medium>0&&high>medium);assert(fall_damage(650)==100);
 mobility_fixture();g.active=7;g.team=1;g.unit[3].x=1100;g.ai_wait=1;dw_ai(&g);assert(g.unit[7].tools[0]==1&&g.unit[7].y<1050); /* Lower enemy can escape. */
}
static void test_large_blasts(void){
 const int previous[5]={38,46,29,38,57};
 for(int kind=0;kind<5;kind++){
  reset();for(int i=0;i<DW_W*DW_H;i++)g.terrain[i]=1;
  for(int i=0;i<10;i++){g.unit[i].x=2000+i*40;g.unit[i].y=1000;}
  int reach=previous[kind]*2+32;
  g.unit[1].x=600;g.unit[1].y=810;
  g.unit[2].x=600+previous[kind]+50;g.unit[2].y=810;
  g.unit[3].x=600+reach+2;g.unit[3].y=810;
  g.shot[0]=(struct dw_shot){600,800,0,0,1,kind,180,0};dw_detonate(&g,0);
  assert(g.unit[1].hp<g.unit[2].hp&&g.unit[2].hp<100&&g.unit[3].hp==100);
  assert(!dw_solid(&g,600+previous[kind]+10,800));assert(dw_solid(&g,600+previous[kind]*2+2,800));
  assert(g.blast[0].life==36&&g.blast[0].radius==previous[kind]*2);
  dw_render(&g,pixels,0,0);g.state=DW_OVER;for(int n=0;n<36;n++)dw_tick(&g,0);assert(!g.blast[0].life);
 }
}
/* Fixed-seed shooting trial: the forgiving AI must miss often but remain functional. */
static void test_easy_ai(void){int hits=0,damage=0,specials=0;
 for(int seed=1;seed<=200;seed++){
  dw_reset(&g,seed);for(int i=0;i<DW_W*DW_H;i++)g.terrain[i]=i/DW_W>=1200;
  for(int i=0;i<10;i++)g.unit[i].hp=0;
  g.unit[0]=(struct dw_unit){600,1200,0,0,140,140,0,0,1,1,0,{0,0}};
  g.unit[5]=(struct dw_unit){1100,1200,0,0,100,100,1,2,-1,1,0,{0,0}};
  g.active=5;g.team=1;g.turn=10;g.ai_wait=1;g.wind=seed%81-40;dw_ai(&g);
  if(g.weapon>=2){specials++;assert(g.target==-1);}
  for(int step=0;step<600&&g.state==DW_FLIGHT;step++)dw_tick(&g,0);
  int lost=140-g.unit[0].hp;damage+=lost;hits+=lost>0;
 }
 assert(hits>20&&hits<100);assert(damage<4000);assert(specials>0&&specials<35);
 for(int seed=1;seed<=64;seed++){dw_reset(&g,seed);g.team=1;g.active=5;g.turn=2;g.ai_wait=1;dw_ai(&g);assert(g.weapon<2&&g.state==DW_FLIGHT);}
 printf("Forgiving AI: %d/200 hits, %.2f average damage, %d strong specials.\n",hits,damage/200.0,specials);
}
static void test_attack_damage_cap(void){
 for(int boss=0;boss<2;boss++)for(int kind=0;kind<5;kind++){
  reset();g.active=boss?9:0;g.team=boss;g.weapon=kind;g.unit[1].x=600;g.unit[1].y=810;
  for(int attack=0;attack<2;attack++){
   g.state=DW_AIM;dw_fire(&g);
   for(int bomb=0;bomb<(kind==2?5:1);bomb++){g.shot[bomb]=(struct dw_shot){600,800,0,0,1,kind,100,0};dw_detonate(&g,bomb);}
   assert(g.attack_damage[1]<=50);assert(g.unit[1].hp==(attack?0:50));for(int n=0;n<60;n++)dw_tick(&g,0);
  }
 }
 /* A blast followed by a long landing cannot turn one hit into a 100-HP kill. */
 mobility_fixture();g.unit[4].x=600;g.unit[4].y=900;dw_explode(&g,600,890,76,100);assert(g.unit[4].hp==50);g.unit[4].vx=0;g.unit[4].vy=8;
 for(int n=0;n<150;n++)dw_move_unit(&g,&g.unit[4]);assert(g.unit[4].ground&&g.unit[4].hp==50&&!g.unit[4].blast_attack);
 /* A later, independent fall still hurts. */
 g.unit[4].y=900;g.unit[4].ground=0;g.unit[4].vy=0;for(int n=0;n<150;n++)dw_move_unit(&g,&g.unit[4]);assert(g.unit[4].hp<50);
}
static void test_hit_feedback(void){
 reset();dw_explode(&g,g.unit[1].x,g.unit[1].y-10,30,15);
 assert(g.feedback[1].damage==23&&g.feedback[1].life==102&&g.hit_pause==60);
 g.state=DW_FLIGHT;g.shot[0]=(struct dw_shot){700,100,2,0,1,0,180,0};
 int remaining=g.remaining,turn=g.turn;float y=g.unit[1].y;
 for(int n=0;n<60;n++)dw_tick(&g,DW_FIRE);
 assert(!g.hit_pause&&g.remaining==remaining&&g.turn==turn&&g.unit[1].y==y&&g.shot[0].x==700&&g.feedback[1].life==42);
 dw_tick(&g,0);assert(g.shot[0].x>700);
 reset();for(int i=0;i<5;i++)g.unit[i].hp=0;g.unit[1].hp=25;g.active=5;g.team=1;
 dw_explode(&g,g.unit[1].x,g.unit[1].y-10,30,50);dw_check_end(&g);
 assert(g.state!=DW_OVER&&g.grave[1].live&&g.feedback[1].damage==25);
 for(int n=0;n<60;n++)dw_tick(&g,0);assert(g.state!=DW_OVER);dw_tick(&g,0);assert(g.state==DW_FINALE&&g.winner==-1&&g.finale_assisted);
 mobility_fixture();g.state=DW_OVER;g.grave[1]=(struct dw_grave){600,900,0,1};
 for(int n=0;n<180;n++)dw_tick(&g,0);assert(g.grave[1].y>1198&&g.grave[1].y<=1200);float settled=g.grave[1].y;
 for(int n=0;n<30;n++)dw_tick(&g,0);assert(g.grave[1].y==settled);reset();assert(!g.grave[1].live&&!g.feedback[1].life&&!g.hit_pause);
}
int main(int argc,char**argv){
 test_mobility();test_large_blasts();test_easy_ai();test_attack_damage_cap();test_hit_feedback();
 reset();assert(dw_alive(&g,0)==5&&dw_alive(&g,1)==5);assert(g.remaining==5400);for(int i=0;i<10;i++){assert(g.unit[i].y<DW_H-24);assert(!dw_solid(&g,(int)g.unit[i].x,(int)g.unit[i].y-5));}assert(g.unit[9].max_hp>g.unit[5].max_hp);assert(dw_health(&g,0)==540&&dw_health(&g,1)==580);
 int before=solids();int crater_y=(int)g.unit[1].y+50;dw_explode(&g,200,crater_y,40,45);assert(solids()<before);assert(!dw_solid(&g,200,crater_y));assert(g.terrain_version==2);
 reset();g.remaining=1;dw_tick(&g,0);assert(g.team==1&&g.turn==2&&g.remaining==5400);for(int i=0;i<101;i++)dw_tick(&g,0);assert(g.state==DW_FLIGHT);
 reset();dw_tick(&g,DW_FIRE);for(int i=0;i<40;i++)dw_tick(&g,DW_FIRE);assert(g.state==DW_AIM&&g.charging);dw_tick(&g,0);assert(g.state==DW_FLIGHT&&g.shot[0].live);assert(g.power>50);
 reset();g.weapon=1;g.power=40;dw_fire(&g);assert(g.shot[0].kind==1&&g.ammo[0][1]==4);for(int i=0;i<185;i++)dw_tick(&g,0);assert(!g.shot[0].live);assert(g.terrain_version>1);
 reset();g.weapon=2;dw_mark(&g,610,430);dw_fire(&g);int live=0;for(int i=0;i<8;i++)live+=g.shot[i].live;assert(live==5&&g.ammo[0][2]==1);for(int i=0;i<240;i++)dw_tick(&g,0);assert(g.terrain_version>=5);
 reset();g.weapon=3;dw_fire(&g);g.shot[0]=(struct dw_shot){440,100,0,-1,1,3,200,30};g.target=8;dw_tick(&g,0);assert(g.shot[0].vx>0&&g.mark_x==g.unit[8].x);
 reset();g.weapon=4;dw_fire(&g);g.shot[0]=(struct dw_shot){440,100,3,0,1,4,200,30};dw_tick(&g,DW_UP);assert(g.shot[0].vy<0);dw_tick(&g,DW_FIRE);assert(!g.shot[0].live);
 reset();g.unit[1].y=DW_H;dw_tick(&g,0);assert(!g.unit[1].hp);
 reset();for(int i=5;i<10;i++)g.unit[i].hp=0;dw_tick(&g,0);assert(g.state==DW_FINALE&&g.winner==-1&&!g.finale_assisted);
 reset();for(int i=0;i<10;i++)g.unit[i].hp=0;dw_tick(&g,0);assert(g.state==DW_FINALE&&g.winner==-1&&g.finale_result==2);
 reset();dw_choose_weapon(&g,4);assert(g.weapon==4);g.ammo[0][3]=0;dw_choose_weapon(&g,3);assert(g.weapon==4);
 dw_camera(0,0,0);dw_render(&g,pixels,0,0);assert(dw_overview);dw_camera(3,2400,1400);assert(!dw_overview&&dw_camera_x<=DW_W-880&&dw_camera_y<=DW_H-700);dw_camera(1,0,0);
 /* Play randomized player turns and real enemy turns through terrain destruction. */
 for(int match=0;match<12;match++){dw_reset(&g,100+match);for(int step=0;step<10000&&g.state!=DW_OVER;step++){unsigned key=0;if(!g.team&&g.state==DW_AIM){if(step%70==0)dw_choose_weapon(&g,(step/70)%5);key=step%120<50?DW_FIRE:0;if(step%21==0)key|=DW_JUMP;if(step%20<8)key|=step%2?DW_UP:DW_DOWN;}else if(!g.team&&g.state==DW_FLIGHT&&g.weapon==4)key=(step/30)%2?DW_UP:DW_RIGHT;dw_tick(&g,key);sane();if(step%601==0)dw_render(&g,pixels,0,match%2);}}
 reset();dw_cached_version=0;dw_camera_x=(int)g.unit[0].x-440;dw_camera_y=(int)g.unit[0].y-410;dw_render(&g,pixels,0,0);
 if(argc>2){dw_overview=1;dw_render(&g,pixels,0,0);}
 if(argc>1){FILE*f=fopen(argv[1],"wb");assert(f);fprintf(f,"P6\n1280 800\n255\n");for(int i=0;i<1280*800;i++){unsigned char c[3]={pixels[i]>>16,pixels[i]>>8,pixels[i]};fwrite(c,1,3,f);}fclose(f);}
 printf("DJ Wars checks passed: hit pause/HP feedback/speaker graves, five weapons, larger blasts/falloff, teleport/jetpack, fall damage, AI, turns, terrain, win/draw, 12 randomized matches. Game state: %zu bytes; renderer cache + framebuffer: %zu bytes.\n",sizeof(g),sizeof(dw_back)+sizeof(pixels));return 0;
}
