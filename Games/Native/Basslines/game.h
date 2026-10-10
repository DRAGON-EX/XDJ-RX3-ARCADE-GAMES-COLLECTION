/* Basslines - original RX3 light-cycle game. No ROMs or third-party game assets. */
#ifndef BASSLINES_GAME_H
#define BASSLINES_GAME_H
#include <stdint.h>
#include "../../../shared/rescue_goals.h"
#ifndef BL_AUDIO_RATE
#define BL_AUDIO_RATE 48000u
#endif
#define BL_W 100
#define BL_H 76
#define BL_CELLS (BL_W*BL_H)
#define BL_UP 1
#define BL_DOWN 2
#define BL_LEFT 4
#define BL_RIGHT 8
#define BL_BOOST 16
enum {BL_COUNTDOWN,BL_PLAY,BL_RESULT,BL_MATCH};
static const int bl_dx[]={1,0,-1,0},bl_dy[]={0,1,0,-1};
static const char *bl_names[]={"YOU","PULSE","VOLT","ECHO"};
static const char *bl_arenas[]={"OPEN CIRCUIT","FOUR DECKS","CROSSFADE","BASS GATES","DIAMOND DRIVE","STAGGERED","THE DIVIDE","FINAL FREQUENCY"};
struct bl_rider {int x,y,dir,want,alive,clock,fuel,boost_lock,crash;};
struct bl_game {
 unsigned char grid[BL_CELLS],links[BL_CELLS];
 struct bl_rider rider[4];
 unsigned rng,ticks,old_buttons,score[4],points,best;
 int state,round,arena,countdown,result_age,winner,survival,boosting;
 unsigned audio_phase,hum_phase;int audio_left,audio_freq,audio_total;
};
static unsigned bl_rand(struct bl_game*g){g->rng^=g->rng<<13;g->rng^=g->rng>>17;g->rng^=g->rng<<5;return g->rng;}
static int bl_inside(int x,int y){return x>=0&&x<BL_W&&y>=0&&y<BL_H;}
static int bl_free(const struct bl_game*g,int x,int y){return bl_inside(x,y)&&!g->grid[y*BL_W+x];}
static void bl_sound(struct bl_game*g,int hz,int samples){g->audio_freq=hz;g->audio_left=g->audio_total=samples;}
static void bl_wall(struct bl_game*g,int x,int y,int w,int h){for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)if(bl_inside(xx,yy))g->grid[yy*BL_W+xx]=5;}
static void bl_round(struct bl_game*g){
 for(int i=0;i<BL_CELLS;i++)g->grid[i]=g->links[i]=0;
 /* Each player victory clears one distinct arena. Losses/draws retry it. */
 g->arena=g->score[0]<RESCUE_BASSLINES_ARENAS?(int)g->score[0]:7;g->state=BL_COUNTDOWN;g->countdown=180;g->winner=-1;g->result_age=0;g->survival=0;g->boosting=0;
 switch(g->arena){
 case 1:bl_wall(g,26,18,8,8);bl_wall(g,66,18,8,8);bl_wall(g,26,50,8,8);bl_wall(g,66,50,8,8);break;
 case 2:bl_wall(g,42,36,16,4);bl_wall(g,48,28,4,20);break;
 case 3:for(int x=22;x<=74;x+=26){bl_wall(g,x,16,3,18);bl_wall(g,x,45,3,15);}break;
 case 4:for(int i=0;i<14;i++){bl_wall(g,36+i,38-i,2,2);bl_wall(g,50+i,24+i,2,2);bl_wall(g,36+i,38+i,2,2);bl_wall(g,50+i,52-i,2,2);}bl_wall(g,46,22,8,7);bl_wall(g,46,48,8,7);break;
 case 5:bl_wall(g,20,20,28,3);bl_wall(g,53,34,27,3);bl_wall(g,20,53,28,3);break;
 case 6:bl_wall(g,48,8,4,19);bl_wall(g,48,49,4,19);bl_wall(g,25,36,15,4);bl_wall(g,60,36,15,4);break;
 case 7:for(int x=20;x<90;x+=20)for(int y=18;y<70;y+=20)if((x+y/2)%3)bl_wall(g,x,y,4,4);break;
 }
 static const int spawns[4][3]={{12,38,0},{87,38,2},{50,12,1},{50,63,3}};
 for(int i=0;i<4;i++){
  int x=spawns[i][0],y=spawns[i][1],d=spawns[i][2];
  g->rider[i]=(struct bl_rider){.x=x,.y=y,.dir=d,.want=d,.alive=1,.fuel=240};
  // Each starting lane has room to see the arena before the first required turn.
  for(int j=-3;j<=10;j++)for(int k=-2;k<=2;k++){int xx=x+bl_dx[d]*j+bl_dy[d]*k,yy=y+bl_dy[d]*j+bl_dx[d]*k;if(bl_inside(xx,yy))g->grid[yy*BL_W+xx]=0;}
 }
 for(int i=0;i<4;i++)g->grid[g->rider[i].y*BL_W+g->rider[i].x]=(unsigned char)(i+1);
}
static void bl_points(struct bl_game*g,unsigned n){g->points+=n;if(g->points>g->best)g->best=g->points;}
static void bl_reset(struct bl_game*g,unsigned seed){unsigned best=g->best;*g=(struct bl_game){.rng=seed?seed:2401,.round=1,.best=best};bl_round(g);}
static void bl_request(struct bl_game*g,int dir){struct bl_rider*r=&g->rider[0];if(dir>=0&&dir<4&&dir!=(r->dir+2)%4)r->want=dir;}
/* Bounded local space estimate. No allocations or unbounded path search. */
static int bl_space(const struct bl_game*g,int x,int y){
 if(!bl_free(g,x,y))return 0;
 unsigned char seen[BL_CELLS];for(int i=0;i<BL_CELLS;i++)seen[i]=0;
 unsigned short queue[192];int head=0,tail=1;queue[0]=(unsigned short)(y*BL_W+x);seen[queue[0]]=1;
 while(head<tail&&tail<192){int p=queue[head++];for(int d=0;d<4&&tail<192;d++){int xx=p%BL_W+bl_dx[d],yy=p/BL_W+bl_dy[d];if(bl_free(g,xx,yy)){int n=yy*BL_W+xx;if(!seen[n]){seen[n]=1;queue[tail++]=(unsigned short)n;}}}}
 return tail;
}
static int bl_ai(struct bl_game*g,int id){
 struct bl_rider*r=&g->rider[id];int best=r->dir,value=-100000;
 for(int turn=-1;turn<=1;turn++){int d=(r->dir+turn+4)%4,x=r->x+bl_dx[d],y=r->y+bl_dy[d];if(!bl_free(g,x,y))continue;
  int ahead=0;while(ahead<20&&bl_free(g,x+bl_dx[d]*ahead,y+bl_dy[d]*ahead))ahead++;
  int score=bl_space(g,x,y)*3+ahead*8+(turn==0?12:0)+(int)(bl_rand(g)%35);
  if(score>value){best=d;value=score;}
 }
 return best;
}
static void bl_resolve(struct bl_game*g,unsigned movers){
 int nx[4],ny[4],dead[4]={0};
 for(int i=0;i<4;i++){struct bl_rider*r=&g->rider[i];nx[i]=r->x;ny[i]=r->y;
  if(!r->alive||!(movers&(1u<<i)))continue;
  r->dir=r->want;nx[i]+=bl_dx[r->dir];ny[i]+=bl_dy[r->dir];dead[i]=!bl_free(g,nx[i],ny[i]);
 }
 // Compute every destination before changing occupancy: head-on ties are simultaneous.
 for(int i=0;i<4;i++)for(int j=i+1;j<4;j++)if(g->rider[i].alive&&g->rider[j].alive&&(movers&(1u<<i))&&(movers&(1u<<j))&&nx[i]==nx[j]&&ny[i]==ny[j])dead[i]=dead[j]=1;
 for(int i=0;i<4;i++){struct bl_rider*r=&g->rider[i];if(!r->alive)continue;
  if(dead[i]){r->alive=0;r->crash=45;bl_sound(g,100+i*45,11000);}
  else if(movers&(1u<<i)){
   // Store actual trail connections; neighboring parallel lanes must not join visually.
   g->links[r->y*BL_W+r->x]|=(unsigned char)(1u<<r->dir);
   r->x=nx[i];r->y=ny[i];g->grid[r->y*BL_W+r->x]=(unsigned char)(i+1);
   g->links[r->y*BL_W+r->x]|=(unsigned char)(1u<<((r->dir+2)%4));
  }
 }
 int alive=0,winner=-1;for(int i=0;i<4;i++)if(g->rider[i].alive){alive++;winner=i;}
 if(alive<=1){g->winner=winner;g->state=BL_RESULT;g->result_age=0;if(winner>=0){g->score[winner]++;if(winner==0)bl_points(g,250);bl_sound(g,winner==0?1200:340,18000);if(g->score[winner]>=RESCUE_BASSLINES_ARENAS){g->state=BL_MATCH;if(winner==0)bl_points(g,1000);}}}
}
static void bl_tick(struct bl_game*g,unsigned buttons){
 unsigned pressed=buttons&~g->old_buttons;g->old_buttons=buttons;g->ticks++;
 if(pressed&BL_UP)bl_request(g,3);else if(pressed&BL_DOWN)bl_request(g,1);else if(pressed&BL_LEFT)bl_request(g,2);else if(pressed&BL_RIGHT)bl_request(g,0);
 for(int i=0;i<4;i++)if(g->rider[i].crash)g->rider[i].crash--;
 if(g->state==BL_COUNTDOWN){if(g->countdown%60==0)bl_sound(g,g->countdown==60?880:440,3500);if(!--g->countdown)g->state=BL_PLAY;return;}
 if(g->state==BL_RESULT||g->state==BL_MATCH){g->result_age++;if(g->result_age>30&&(pressed&BL_BOOST)){if(g->state==BL_MATCH)bl_reset(g,g->rng);else{g->round++;bl_round(g);}}return;}
 g->survival++;struct bl_rider*r=&g->rider[0];if(r->alive&&g->survival%60==0)bl_points(g,10);if(!(buttons&BL_BOOST))r->boost_lock=0;
 g->boosting=r->alive&&(buttons&BL_BOOST)&&!r->boost_lock&&r->fuel>=2;
 if(g->boosting){r->fuel-=2;if(r->fuel<2)r->boost_lock=1;}else if(r->fuel<240)r->fuel++;
 unsigned movers=0;for(int i=0;i<4;i++)if(g->rider[i].alive){
  int period=i==0&&g->boosting?2:4;
  if(++g->rider[i].clock>=period){g->rider[i].clock=0;movers|=1u<<i;if(i)g->rider[i].want=bl_ai(g,i);}
 }
 if(movers)bl_resolve(g,movers);
 // Bounded rounds: surviving riders draw after 90 seconds.
 if(g->state==BL_PLAY&&g->survival>=90*60){g->state=BL_RESULT;g->winner=-1;g->result_age=0;}
}
static void bl_audio(struct bl_game*g,int16_t*out,unsigned frames,float*gain,float target){
 for(unsigned i=0;i<frames;i++){*gain+=(target-*gain)*.002f;int value=0;
  if(g->state==BL_PLAY&&g->rider[0].alive){g->hum_phase+=(g->boosting?125u:75u)*(unsigned)(4294967296ULL/BL_AUDIO_RATE);value=(int)(g->hum_phase>>24)-128;}
  if(g->audio_left){g->audio_phase+=(unsigned)g->audio_freq*(unsigned)(4294967296ULL/BL_AUDIO_RATE);value+=((g->audio_phase&0x80000000u)?1:-1)*2200*g->audio_left/g->audio_total;g->audio_left--;}
  out[2*i]=out[2*i+1]=(int16_t)(value**gain);
 }
}
#endif
