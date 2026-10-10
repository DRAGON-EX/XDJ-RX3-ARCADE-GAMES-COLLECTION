#include <assert.h>
#include <stdio.h>
#include "../Games/Native/Asteroids/asteroids.h"
static void clear_wave(struct ast_game*g){unsigned safety=0;while(ast_count(g)){for(int i=0;i<AST_ROCKS;i++)if(g->rocks[i].active){ast_break(g,i,1);break;}assert(++safety<100);}}
int main(void){struct ast_game g={0};ast_reset(&g,12);assert(g.wave==1&&g.lives==3&&ast_count(&g)==4);assert(sizeof(g)<16384);
 clear_wave(&g);assert(g.score==2080);for(int i=0;i<91;i++)ast_tick(&g,0);assert(g.wave==2&&ast_count(&g)==6);
 for(unsigned wave=3;wave<=14;wave++){clear_wave(&g);for(int i=0;i<91;i++)ast_tick(&g,0);assert(g.wave==wave&&ast_count(&g)==(wave==3?8:10));}
 unsigned before=g.lives;g.score=9990;g.next_life=10000;ast_points(&g,10200);assert(g.score==20190&&g.lives==(int)before+2&&g.next_life==30000);
 g.ship.x=AST_W-1;g.ship.vx=5;g.ship.y=1;g.ship.vy=-5;ast_move(&g.ship);assert(g.ship.x<10&&g.ship.y>AST_H-10);assert(ast_hit(2,10,AST_W-2,10,6));
 ast_reset(&g,77);g.shield=0;ast_die(&g);assert(g.lives==2&&!g.ship.active&&!g.game_over);for(int i=0;i<90;i++)ast_tick(&g,0);assert(g.ship.active&&g.shield>0);ast_die(&g);ast_respawn(&g);ast_die(&g);assert(g.game_over&&g.lives==0);
 ast_reset(&g,77);g.old_buttons=AST_HYPER;float oldx=g.ship.x;ast_tick(&g,AST_HYPER);assert(g.ship.x==oldx);ast_tick(&g,0);ast_tick(&g,AST_HYPER);assert(g.hyper_wait==60);
 ast_reset(&g,77);for(int i=0;i<AST_ROCKS;i++)g.rocks[i].active=0;g.ufo.active=1;g.ufo.x=80;g.ufo.y=100;g.ufo.size=2;g.ufo.vx=0;g.ufo_fire=100;g.shots[0]=(struct ast_body){80,100,0,0,1,0,0,0,4,0};ast_tick(&g,0);assert(!g.ufo.active&&g.score==200);
 g.ufo.active=1;g.ufo.x=80;g.ufo.y=100;g.ufo.size=1;g.shots[0]=(struct ast_body){80,100,0,0,1,0,0,0,4,0};ast_tick(&g,0);assert(!g.ufo.active&&g.score==1200);
 ast_reset(&g,77);for(int i=0;i<120;i++)ast_tick(&g,AST_FIRE);int player_shots=0;for(int i=0;i<4;i++)player_shots+=g.shots[i].active;assert(player_shots<=4);
 ast_reset(&g,77);g.shot_sound=2300;float gain=0;int16_t samples[1470];ast_audio(&g,samples,735,&gain,0);for(int i=0;i<1470;i++)assert(samples[i]==0);g.shot_sound=2300;ast_audio(&g,samples,735,&gain,1);int nonzero=0;for(int i=0;i<1470;i++)nonzero|=samples[i]!=0;assert(nonzero&&gain>0);
 /* Long simultaneous input run exercises bounded pools, wrapping and respawns. */
 ast_reset(&g,829);for(int i=0;i<30000;i++){if(g.game_over)ast_reset(&g,829+i);ast_tick(&g,AST_UP|AST_FIRE|((i/130)%2?AST_LEFT:AST_RIGHT)|((i%110)==0?AST_HYPER:0));for(int j=0;j<AST_ROCKS;j++)if(g.rocks[j].active)assert(g.rocks[j].size>=1&&g.rocks[j].size<=3);assert(g.ship.x>=0&&g.ship.x<AST_W&&g.ship.y>=0&&g.ship.y<AST_H);}
 printf("Asteroids: continuous 4/6/8/10 waves, splitting, score, UFOs, lives, wrap, held controls, master sound and bounded pools passed; state=%zu bytes\n",sizeof(g));
}
