#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../Games/Native/Equalizarr/game.h"
static struct eq_game g;
static void isolated(void){eq_reset(&g,1729,0);memset(g.brick,0,sizeof(g.brick));memset(g.kind,0,sizeof(g.kind));g.brick[EQ_CELLS-1]=1;g.remaining=1;g.ball[0]=(struct eq_ball){.x=400,.y=430,.vx=2,.vy=-5,.active=1};}
int main(void){
 // A practice level number is not a rescue; only ten actual clears qualify.
 eq_reset(&g,12,20);assert(!g.cleared_levels);
 for(unsigned cleared=1;cleared<=10;cleared++){
  for(int i=0;i<EQ_CELLS;i++)eq_hit(&g,i,1);
  eq_tick(&g,0,-1);assert(g.cleared_levels==cleared);
  for(int i=0;i<120;i++)eq_tick(&g,0,-1);
  assert(g.cleared_levels==cleared);
 }
 eq_reset(&g,12,0);assert(!g.cleared_levels);
 unsigned long long hashes[50];
 for(int l=0;l<50;l++){
  eq_reset(&g,1729,l);assert(g.remaining>=20&&g.remaining<=EQ_CELLS);unsigned long long hash=1469598103934665603ULL;
  for(int i=0;i<EQ_CELLS;i++){hash^=g.brick[i]+g.kind[i]*4;hash*=1099511628211ULL;}
  hashes[l]=hash;for(int j=0;j<l;j++)assert(hash!=hashes[j]);
  for(int i=0;i<EQ_CELLS;i++)eq_hit(&g,i,1);
  assert(g.remaining==0);eq_tick(&g,0,-1);assert(g.transition==120);
  for(int i=0;i<120;i++)eq_tick(&g,0,-1);
  assert(l==49?g.won:g.level==l+1&&g.remaining>0);
 }
 eq_reset(&g,1,0);float y=g.ball[0].y;eq_tick(&g,0,100);assert(g.ball[0].stuck&&g.paddle==100);eq_tick(&g,EQ_FIRE,-1);assert(!g.ball[0].stuck&&g.ball[0].y<y);
 isolated();g.ball[0]=(struct eq_ball){.x=400,.y=EQ_PY-16,.vy=8,.active=1};g.paddle=400;eq_tick(&g,0,-1);assert(g.ball[0].vy<0);
 isolated();g.brick[0]=2;g.remaining++;g.ball[0]=(struct eq_ball){.x=50,.y=EQ_BY+EQ_BH+8,.vy=-8,.active=1};eq_tick(&g,0,-1);assert(g.brick[0]==1&&g.ball[0].vy>0);
 // Side collision and high-speed multi-hit block.
 isolated();g.brick[0]=1;g.remaining++;g.ball[0]=(struct eq_ball){.x=EQ_BX-9,.y=EQ_BY+12,.vx=8,.active=1};eq_tick(&g,0,-1);assert(!g.brick[0]&&g.ball[0].vx<0);
 isolated();eq_apply(&g,EQ_SHIELD);g.ball[0]=(struct eq_ball){.x=40,.y=EQ_H-21,.vy=8,.active=1};eq_tick(&g,0,-1);assert(g.power[EQ_SHIELD]==2&&g.ball[0].vy<0&&g.lives==4);
 isolated();g.ball[0].y=EQ_H+20;g.ball[0].vy=8;eq_tick(&g,0,-1);assert(g.lives==3&&g.ball[0].stuck);
 isolated();eq_apply(&g,EQ_MULTI);assert(eq_count(&g)==3);g.ball[0].y=EQ_H+20;g.ball[0].vy=8;eq_tick(&g,0,-1);assert(eq_count(&g)==2&&g.lives==4);
 isolated();eq_apply(&g,EQ_WIDE);assert(eq_half(&g)==94);g.power[EQ_WIDE]=1;eq_tick(&g,0,-1);assert(eq_half(&g)==62);
 isolated();eq_apply(&g,EQ_STICKY);g.ball[0]=(struct eq_ball){.x=400,.y=EQ_PY-16,.vy=8,.active=1};g.paddle=400;eq_tick(&g,0,-1);assert(g.ball[0].stuck);eq_tick(&g,EQ_FIRE,-1);assert(!g.ball[0].stuck);
 isolated();eq_apply(&g,EQ_SLOW);float before=g.ball[0].y;eq_tick(&g,0,-1);assert(eq_abs(g.ball[0].y-before)<4);
 isolated();eq_apply(&g,EQ_LIFE);assert(g.lives==5);g.lives=9;eq_apply(&g,EQ_LIFE);assert(g.lives==9);
 isolated();g.brick[0]=3;g.remaining++;eq_apply(&g,EQ_FIREBALL);g.ball[0]=(struct eq_ball){.x=50,.y=EQ_BY+EQ_BH+8,.vy=-8,.active=1};eq_tick(&g,0,-1);assert(!g.brick[0]&&g.ball[0].vy<0);
 isolated();eq_apply(&g,EQ_LASER);g.paddle=EQ_BX+EQ_BW/2+eq_half(&g)-8;g.brick[0]=1;g.remaining++;
 for(int i=0;i<55;i++)eq_tick(&g,EQ_FIRE,-1);
 assert(!g.brick[0]);
 isolated();g.brick[0]=g.brick[1]=g.brick[EQ_COLS]=1;g.remaining+=3;g.kind[0]=1;eq_hit(&g,0,0);assert(!g.brick[0]&&!g.brick[1]&&!g.brick[EQ_COLS]&&g.remaining==1);
 isolated();g.drop[0]=(struct eq_drop){g.paddle,EQ_PY-13,1,EQ_LIFE};eq_tick(&g,0,-1);assert(g.lives==5&&!g.drop[0].active);
 isolated();g.lives=1;g.ball[0].y=EQ_H+20;g.ball[0].vy=8;eq_tick(&g,0,-1);assert(g.over);eq_tick(&g,EQ_FIRE,-1);assert(!g.over&&g.lives==4&&g.level==0);
 // Steel remains after normal/laser/explosion/charged-ball damage and is excluded from targets.
 isolated();g.brick[0]=255;g.kind[0]=2;eq_hit(&g,0,1);assert(g.brick[0]==255&&g.remaining==1);
 eq_apply(&g,EQ_FIREBALL);g.ball[0]=(struct eq_ball){.x=50,.y=EQ_BY+EQ_BH+8,.vy=-8,.active=1};
 eq_tick(&g,0,-1);assert(g.brick[0]==255&&g.ball[0].vy>0);
 // User requested the original frequent drop cadence: one bonus per five broken bricks.
 isolated();g.broken=4;g.brick[0]=1;g.remaining++;eq_hit(&g,0,1);int drops=0;for(int i=0;i<EQ_DROPS;i++)drops+=g.drop[i].active;assert(drops==1);
 assert(eq_speed(&g)>6);eq_reset(&g,1729,49);assert(eq_speed(&g)>9);
 int16_t audio[1600];float gain=0;eq_sound(&g,440,4800);eq_audio(&g,audio,800,&gain,1);int nonzero=0;for(int i=0;i<1600;i++)nonzero|=audio[i]!=0;assert(nonzero);
 // Long deterministic playthroughs, fixed pools, finite coordinates; also exercise all 50 layouts.
 for(int l=0;l<50;l++){eq_reset(&g,1729,l);for(int t=0;t<12000&&!g.won&&!g.over&&g.level==l;t++){int target=(int)g.ball[0].x;g.lives=9;eq_tick(&g,t%50<25?EQ_FIRE:0,target);for(int b=0;b<EQ_BALLS;b++)if(g.ball[b].active){assert(g.ball[b].x>=0&&g.ball[b].x<=EQ_W);assert(g.ball[b].y>=0&&g.ball[b].y<=EQ_H+20);}}}
 printf("Equalizarr: 50 distinct layouts, progression/finale, collisions, all 8 powers, pool stress and audio passed. State %zu bytes.\n",sizeof(g));
}
