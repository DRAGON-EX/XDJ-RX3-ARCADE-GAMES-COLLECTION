#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../Games/Native/Basslines/game.h"
static void empty(struct bl_game*g){bl_reset(g,42);memset(g->grid,0,sizeof g->grid);for(int i=0;i<4;i++)g->rider[i].alive=0;g->state=BL_PLAY;}
static void rider(struct bl_game*g,int i,int x,int y,int d){g->rider[i]=(struct bl_rider){.x=x,.y=y,.dir=d,.want=d,.alive=1,.fuel=240};g->grid[y*BL_W+x]=i+1;}
int main(void){
 struct bl_game g={0};bl_reset(&g,42);for(int i=0;i<180;i++)bl_tick(&g,0);assert(g.state==BL_PLAY&&g.rider[0].x==12);
 bl_request(&g,2);assert(g.rider[0].want==0);bl_tick(&g,BL_UP);assert(g.rider[0].want==3);for(int i=0;i<3;i++)bl_tick(&g,BL_UP);assert(g.rider[0].y==37&&g.rider[0].dir==3);
 bl_request(&g,1);assert(g.rider[0].want==3);
 // Simultaneous riders entering the same free cell both crash, without a point.
 empty(&g);rider(&g,0,20,20,0);rider(&g,1,22,20,2);bl_resolve(&g,3);assert(!g.rider[0].alive&&!g.rider[1].alive&&g.winner==-1&&g.state==BL_RESULT&&!g.score[0]&&!g.score[1]);
 empty(&g);rider(&g,0,20,20,0);rider(&g,1,21,20,2);bl_resolve(&g,3);assert(!g.rider[0].alive&&!g.rider[1].alive);
 // Wall and trail collisions, including a stationary rider while boosting.
 empty(&g);rider(&g,0,0,20,2);rider(&g,1,70,20,0);bl_resolve(&g,1);assert(!g.rider[0].alive&&g.winner==1&&g.score[1]==1);
 for(int value=1;value<=5;value++){empty(&g);rider(&g,0,20,20,0);rider(&g,1,70,20,0);g.grid[20*BL_W+21]=value;bl_resolve(&g,1);assert(!g.rider[0].alive&&g.winner==1);}
 empty(&g);rider(&g,0,20,20,0);rider(&g,1,21,20,1);bl_resolve(&g,1);assert(!g.rider[0].alive&&g.rider[1].alive);
 // Visually adjacent lanes must not acquire false connecting segments.
 empty(&g);rider(&g,0,20,20,0);rider(&g,1,70,20,0);g.grid[21*BL_W+21]=1;bl_resolve(&g,1);
 assert(g.links[20*BL_W+20]==1&&g.links[20*BL_W+21]==4&&!g.links[21*BL_W+21]);
 // Match victory and explicit next-round / restart input, no held-button cascade.
 empty(&g);rider(&g,0,10,10,0);rider(&g,1,99,20,0);g.score[0]=7;bl_resolve(&g,2);assert(g.state==BL_MATCH&&g.score[0]==8&&g.points==1250&&g.best>=1250);
 for(int i=0;i<45;i++)bl_tick(&g,BL_BOOST);assert(g.state==BL_MATCH);bl_tick(&g,0);bl_tick(&g,BL_BOOST);assert(g.state==BL_COUNTDOWN&&g.round==1&&g.score[0]==0&&g.points==0&&g.best>=1250);
 g.state=BL_RESULT;g.result_age=31;g.old_buttons=0;g.score[0]=2;bl_tick(&g,BL_BOOST);assert(g.round==2&&g.score[0]==2&&g.state==BL_COUNTDOWN);
 // Boost gives twice the grid speed and cannot oscillate at zero fuel.
 bl_reset(&g,42);g.state=BL_PLAY;for(int i=0;i<4;i++)bl_tick(&g,BL_BOOST);assert(g.rider[0].x==14&&g.rider[0].fuel==232);
 g.rider[0].fuel=2;bl_tick(&g,BL_BOOST);assert(g.rider[0].fuel==0&&g.rider[0].boost_lock);bl_tick(&g,BL_BOOST);assert(!g.boosting&&g.rider[0].fuel==1);bl_tick(&g,0);assert(!g.rider[0].boost_lock);
 // Each arena is different, all start lanes clear and all rounds terminate.
 unsigned hashes[8];for(int a=0;a<8;a++){bl_reset(&g,42);g.score[0]=a;g.round=a+1;bl_round(&g);unsigned hash=2166136261u;for(int j=0;j<BL_CELLS;j++)hash=(hash^g.grid[j])*16777619u;hashes[a]=hash;for(int b=0;b<a;b++)assert(hashes[b]!=hash);
  for(int i=0;i<4;i++){struct bl_rider*r=&g.rider[i];assert(r->alive&&g.grid[r->y*BL_W+r->x]==i+1);for(int j=1;j<=10;j++)assert(bl_free(&g,r->x+bl_dx[r->dir]*j,r->y+bl_dy[r->dir]*j));}
 }
 unsigned total_ticks=0;for(unsigned seed=1;seed<=40;seed++){bl_reset(&g,seed);g.score[0]=seed%8;g.round=(int)(seed%8)+1;bl_round(&g);
  for(int n=0;n<6000&&g.state<BL_RESULT;n++){
   if(g.rider[0].alive)bl_request(&g,bl_ai(&g,0));bl_tick(&g,(n%300<60)?BL_BOOST:0);total_ticks++;
   for(int i=0;i<4;i++){struct bl_rider*r=&g.rider[i];assert(bl_inside(r->x,r->y));assert(r->fuel>=0&&r->fuel<=240);if(r->alive)assert(g.grid[r->y*BL_W+r->x]==i+1);}
  }assert(g.state==BL_RESULT||g.state==BL_MATCH);unsigned score=g.score[0]+g.score[1]+g.score[2]+g.score[3]-(seed%8);assert(score==(unsigned)(g.winner>=0));
 }
 int16_t audio[1600];float gain=1;bl_sound(&g,880,800);bl_audio(&g,audio,800,&gain,1);int signal=0;for(int i=0;i<1600;i++)signal|=audio[i];assert(signal&&g.audio_left==0);gain=0;bl_sound(&g,880,800);bl_audio(&g,audio,800,&gain,0);for(int i=0;i<1600;i++)assert(audio[i]==0);
 printf("Basslines: collisions, ties, turns, boost, 8 arenas, match progression, audio and %u stress ticks passed. State: %zu bytes.\n",total_ticks,sizeof g);
}
