#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../Games/Native/Space Invaders/game.h"
static struct si_game g;
static void quiet(void){g.enemy_fire=g.ufo_wait=100000;g.march=0;}
int main(void){
 si_reset(&g,42);assert(g.remaining==55&&g.lives==3&&g.wave==1);si_tick(&g,SI_FIRE,-1);assert(g.shot[0].active&&g.reload==14);
 si_tick(&g,SI_LEFT,0);assert(g.player==22);si_tick(&g,SI_RIGHT,9999);assert(g.player==818);
 // Bottom-up hit, single-hit enemies across waves, and row scores.
 si_reset(&g,42);quiet();g.shot[0]=(struct si_shot){g.formation_x+17,g.formation_y+4*39+31,-9,1};si_tick(&g,0,-1);assert(!g.alien[44]&&g.remaining==54&&g.score==10);
 si_wave(&g,3);quiet();assert(g.alien[0]==1);g.shot[0]=(struct si_shot){g.formation_x+17,g.formation_y+31,-9,1};si_tick(&g,0,-1);assert(g.alien[0]==0&&g.remaining==54);
 // One player shot at a time even while FIRE is held; only one bonus life.
 si_reset(&g,42);quiet();for(int n=0;n<28;n++)si_tick(&g,SI_FIRE,-1);assert(g.shot[0].y<400);si_award(&g,1500);si_award(&g,2500);assert(g.lives==4&&!g.extra_at);
 // Edge reversal descends, only surviving columns determine the edge.
 si_reset(&g,42);g.formation_x=210;int y=g.formation_y;si_march(&g);assert(g.dir==-1&&g.formation_y==y+18);
 memset(g.alien,0,sizeof g.alien);g.alien[0]=1;g.formation_x=210;g.dir=1;y=g.formation_y;si_march(&g);assert(g.formation_y==y&&g.formation_x==218);
 // Bunkers erode locally, holes let shots through; no world wrap.
 si_reset(&g,42);int before=0,after=0;for(int i=0;i<SI_BUNKER_CELLS;i++)before+=g.bunker[0][i];assert(si_shield(&g,130,512));for(int i=0;i<SI_BUNKER_CELLS;i++)after+=g.bunker[0][i];assert(after<before&&after>0);assert(!si_shield(&g,-1,512)&&!si_shield(&g,150,548));
 // UFO bonus / score / extra life; no repeated life award for the same threshold.
 si_reset(&g,42);quiet();g.ufo_active=1;g.ufo_x=420;g.ufo_dir=1;g.shot[0]=(struct si_shot){420,45,-9,1};si_tick(&g,0,-1);assert(!g.ufo_active&&g.score>=50&&g.ufo_notice);
 si_award(&g,1500);assert(g.lives==4&&g.best>=g.score);si_award(&g,10);assert(g.lives==4);unsigned high=g.best;si_reset(&g,55);assert(g.best==high&&!g.score);
 // Damage grace, life pause, defeat and fresh run.
 si_hit_player(&g);assert(g.lives==2&&g.life_pause==75&&g.invulnerable);si_hit_player(&g);assert(g.lives==2);g.invulnerable=0;g.lives=1;si_hit_player(&g);assert(g.over);g.old_buttons=0;si_tick(&g,SI_FIRE,-1);assert(!g.over&&g.lives==3);
 // Wave transition keeps score/lives and repairs bunkers.
 g.score=300;g.lives=2;memset(g.alien,0,sizeof g.alien);memset(g.bunker,0,sizeof g.bunker);g.remaining=0;quiet();si_tick(&g,0,-1);assert(g.transition==120);for(int i=0;i<120;i++)si_tick(&g,0,-1);assert(g.wave==2&&g.remaining==55&&g.score==300&&g.lives==2&&g.bunker[0][30]);
 g.formation_y=590;si_march(&g);assert(g.over&&!g.lives);
 // Only the bottom live alien in a column fires.
 si_reset(&g,42);si_enemy_shoot(&g);assert(g.enemy[0].active&&g.enemy[0].y==g.formation_y+4*39+26);
 for(int wave=1;wave<=12;wave++){si_reset(&g,(unsigned)wave);si_wave(&g,wave);for(int t=0;t<18000&&!g.over;t++){si_tick(&g,SI_FIRE,(t/4)%790+25);assert(g.remaining>=0&&g.remaining<=55);for(int i=0;i<SI_PLAYER_SHOTS;i++)if(g.shot[i].active)assert(g.shot[i].y>=0&&g.shot[i].y<=SI_PLAYER_Y);}}
 int16_t audio[1600];float gain=1;si_sound(&g,700,800);si_audio(&g,audio,800,&gain,1);int signal=0;for(int i=0;i<1600;i++)signal|=audio[i];assert(signal);gain=0;si_sound(&g,700,800);si_audio(&g,audio,800,&gain,0);for(int i=0;i<1600;i++)assert(!audio[i]);
 printf("Space Invaders: waves, single shots, UFOs, shields, score/highscore, lives, invasion, bounded pools and audio passed. State %zu bytes.\n",sizeof g);
}
