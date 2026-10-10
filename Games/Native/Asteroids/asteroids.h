#ifndef RX3_ASTEROIDS_H
#define RX3_ASTEROIDS_H
/* RX3 adaptation of the raylib classic Asteroids concept.
 * Original sample: Ian Eito, Albert Martos and Ramon Santamaria, (c) 2015.
 * Altered implementation: independent fixed-pool simulation/backend, continuous
 * arcade waves and UFOs. See LICENSE.raylib-games and upstream/raylib-asteroids.c.
 * 60 Hz simulation; host renders at 30 Hz. No heap or platform dependencies. */
#include <stdint.h>
#define AST_W 880
#define AST_H 700
#define AST_ROCKS 64
#define AST_SHOTS 12
#define AST_PARTICLES 96
#define AST_UP 1
#define AST_HYPER 2
#define AST_LEFT 4
#define AST_RIGHT 8
#define AST_FIRE 16
struct ast_body {float x,y,vx,vy;int active,size,angle,spin,life;unsigned shape;};
struct ast_game {
 struct ast_body ship,rocks[AST_ROCKS],shots[AST_SHOTS],particles[AST_PARTICLES],ufo;
 uint32_t rng;unsigned ticks,score,high,wave,next_life;int lives,game_over,respawn,shield,fire_wait,hyper_wait,wave_wait,ufo_wait,ufo_fire,old_buttons;
 int thrust,shot_sound,boom_sound,warp_sound,beat_sound;unsigned sound_phase,noise;
};
static const int16_t ast_sine[256]={0,804,1608,2410,3212,4011,4808,5602,6393,7179,7962,8739,9512,10278,11039,11793,12539,13279,14010,14732,15446,16151,16846,17530,18204,18868,19519,20159,20787,21403,22005,22594,23170,23731,24279,24811,25329,25832,26319,26790,27245,27683,28105,28510,28898,29268,29621,29956,30273,30571,30852,31113,31356,31580,31785,31971,32137,32285,32412,32521,32609,32678,32728,32757,32767,32757,32728,32678,32609,32521,32412,32285,32137,31971,31785,31580,31356,31113,30852,30571,30273,29956,29621,29268,28898,28510,28105,27683,27245,26790,26319,25832,25329,24811,24279,23731,23170,22594,22005,21403,20787,20159,19519,18868,18204,17530,16846,16151,15446,14732,14010,13279,12539,11793,11039,10278,9512,8739,7962,7179,6393,5602,4808,4011,3212,2410,1608,804,0,-804,-1608,-2410,-3212,-4011,-4808,-5602,-6393,-7179,-7962,-8739,-9512,-10278,-11039,-11793,-12539,-13279,-14010,-14732,-15446,-16151,-16846,-17530,-18204,-18868,-19519,-20159,-20787,-21403,-22005,-22594,-23170,-23731,-24279,-24811,-25329,-25832,-26319,-26790,-27245,-27683,-28105,-28510,-28898,-29268,-29621,-29956,-30273,-30571,-30852,-31113,-31356,-31580,-31785,-31971,-32137,-32285,-32412,-32521,-32609,-32678,-32728,-32757,-32767,-32757,-32728,-32678,-32609,-32521,-32412,-32285,-32137,-31971,-31785,-31580,-31356,-31113,-30852,-30571,-30273,-29956,-29621,-29268,-28898,-28510,-28105,-27683,-27245,-26790,-26319,-25832,-25329,-24811,-24279,-23731,-23170,-22594,-22005,-21403,-20787,-20159,-19519,-18868,-18204,-17530,-16846,-16151,-15446,-14732,-14010,-13279,-12539,-11793,-11039,-10278,-9512,-8739,-7962,-7179,-6393,-5602,-4808,-4011,-3212,-2410,-1608,-804};
static float ast_sin(int a){return ast_sine[a&255]/32767.0f;}
static uint32_t ast_rand(struct ast_game*g){uint32_t x=g->rng;x^=x<<13;x^=x>>17;x^=x<<5;return g->rng=x?x:1;}
static int ast_random(struct ast_game*g,int n){return (int)(ast_rand(g)%(unsigned)n);}
static float ast_abs(float v){return v<0?-v:v;}
static float ast_wrap(float v,float size){while(v<0)v+=size;while(v>=size)v-=size;return v;}
static void ast_move(struct ast_body*b){b->x=ast_wrap(b->x+b->vx,AST_W);b->y=ast_wrap(b->y+b->vy,AST_H);}
static int ast_hit(float x,float y,float a,float b,float r){float dx=ast_abs(x-a),dy=ast_abs(y-b);if(dx>AST_W/2)dx=AST_W-dx;if(dy>AST_H/2)dy=AST_H-dy;return dx*dx+dy*dy<r*r;}
static float ast_radius(int size){return size==3?38:size==2?22:12;}
static unsigned ast_score(int size){return size==3?20:size==2?50:100;}
static int ast_count(struct ast_game*g){int n=0;for(int i=0;i<AST_ROCKS;i++)n+=g->rocks[i].active!=0;return n;}
static void ast_points(struct ast_game*g,unsigned p){g->score+=p;if(g->score>g->high)g->high=g->score;while(g->score>=g->next_life){g->lives++;g->next_life+=10000;}}
static void ast_sparks(struct ast_game*g,float x,float y,int count,int kind){for(int k=0;k<count;k++)for(int i=0;i<AST_PARTICLES;i++)if(!g->particles[i].active){int a=ast_random(g,256);float v=.8f+ast_random(g,40)/10.f;g->particles[i]=(struct ast_body){x,y,ast_sin(a)*v,-ast_sin(a+64)*v,1,kind,0,0,18+ast_random(g,24),0};break;}}
static void ast_rock(struct ast_game*g,float x,float y,int size,float vx,float vy){for(int i=0;i<AST_ROCKS;i++)if(!g->rocks[i].active){g->rocks[i]=(struct ast_body){x,y,vx,vy,1,size,ast_random(g,256),ast_random(g,3)-1,0,ast_rand(g)};return;}}
static void ast_break(struct ast_game*g,int index,int player){struct ast_body r=g->rocks[index];g->rocks[index].active=0;if(player)ast_points(g,ast_score(r.size));ast_sparks(g,r.x,r.y,10,0);g->boom_sound=6000;
 if(r.size>1)for(int k=0;k<2;k++){int a=ast_random(g,256);float v=1.1f+(3-r.size)*.55f;ast_rock(g,r.x,r.y,r.size-1,r.vx*.55f+ast_sin(a)*v,r.vy*.55f-ast_sin(a+64)*v);}}
static void ast_wave(struct ast_game*g){g->wave++;int count=g->wave==1?4:g->wave==2?6:g->wave==3?8:10;float speed=1.1f+(g->wave<12?g->wave:12)*.07f;
 for(int i=0;i<count;i++){int a=ast_random(g,256),side=ast_random(g,4);float x=side<2?(side?AST_W-45:45):ast_random(g,AST_W),y=side>=2?(side==2?45:AST_H-45):ast_random(g,AST_H);ast_rock(g,x,y,3,ast_sin(a)*speed,-ast_sin(a+64)*speed);}
 g->wave_wait=0;g->ufo_wait=480+ast_random(g,360);}
static void ast_respawn(struct ast_game*g){g->ship=(struct ast_body){AST_W/2,AST_H/2,0,0,1,0,0,0,0,0};g->shield=150;g->respawn=0;}
static void ast_reset(struct ast_game*g,unsigned seed){unsigned high=g->high;*g=(struct ast_game){0};g->rng=seed?seed:1;g->noise=seed?seed:1;g->high=high;g->lives=3;g->next_life=10000;ast_respawn(g);ast_wave(g);}
static void ast_die(struct ast_game*g){if(!g->ship.active)return;ast_sparks(g,g->ship.x,g->ship.y,28,1);g->ship.active=0;g->lives--;g->respawn=90;g->boom_sound=12000;if(g->lives<=0)g->game_over=1;}
static void ast_shoot(struct ast_game*g,struct ast_body*b,int enemy,int angle){int begin=enemy?4:0,end=enemy?AST_SHOTS:4;for(int i=begin;i<end;i++)if(!g->shots[i].active){float dx=ast_sin(angle),dy=-ast_sin(angle+64),v=enemy?5.5f:10;g->shots[i]=(struct ast_body){ast_wrap(b->x+dx*18,AST_W),ast_wrap(b->y+dy*18,AST_H),dx*v+(enemy?0:b->vx*.35f),dy*v+(enemy?0:b->vy*.35f),1,enemy,0,0,enemy?100:58,0};if(!enemy)g->shot_sound=2300;break;}}
static void ast_ufo(struct ast_game*g){int small=g->score>=10000||ast_random(g,3)==0,side=ast_random(g,2);g->ufo=(struct ast_body){side?AST_W-1:1,(float)(70+ast_random(g,AST_H-140)),side?-2.1f:2.1f,0,1,small?1:2,0,0,0,0};g->ufo_fire=75;}
static void ast_tick(struct ast_game*g,unsigned buttons){g->ticks++;g->thrust=0;if(g->fire_wait)g->fire_wait--;if(g->hyper_wait)g->hyper_wait--;if(g->shield)g->shield--;
 if(g->game_over){g->old_buttons=buttons;return;}
 if(!g->ship.active&&g->respawn&&!--g->respawn)ast_respawn(g);
 if(g->ship.active){struct ast_body*s=&g->ship;if(buttons&AST_LEFT)s->angle=(s->angle-3)&255;if(buttons&AST_RIGHT)s->angle=(s->angle+3)&255;
 if(buttons&AST_UP){s->vx+=ast_sin(s->angle)*.115f;s->vy-=ast_sin(s->angle+64)*.115f;g->thrust=1;}
 s->vx*=.994f;s->vy*=.994f;float v=s->vx*s->vx+s->vy*s->vy;if(v>64){s->vx*=.97f;s->vy*=.97f;}ast_move(s);
 if((buttons&AST_FIRE)&&!g->fire_wait){ast_shoot(g,s,0,s->angle);g->fire_wait=10;}
 if((buttons&AST_HYPER)&&!(g->old_buttons&AST_HYPER)&&!g->hyper_wait){ast_sparks(g,s->x,s->y,18,2);s->x=ast_random(g,AST_W);s->y=ast_random(g,AST_H);s->vx=s->vy=0;g->shield=0;g->hyper_wait=60;g->warp_sound=4000;if(ast_random(g,6)==0)ast_die(g);}
 }
 for(int i=0;i<AST_PARTICLES;i++)if(g->particles[i].active){ast_move(&g->particles[i]);if(!--g->particles[i].life)g->particles[i].active=0;}
 for(int i=0;i<AST_ROCKS;i++)if(g->rocks[i].active){ast_move(&g->rocks[i]);g->rocks[i].angle=(g->rocks[i].angle+g->rocks[i].spin)&255;}
 if(g->ufo.active){g->ufo.x+=g->ufo.vx;g->ufo.y=ast_wrap(g->ufo.y+(g->ticks%180<90?.35f:-.35f),AST_H);if(g->ufo.x<0||g->ufo.x>=AST_W){g->ufo.active=0;g->ufo_wait=360+ast_random(g,360);}else if(!--g->ufo_fire){int a=ast_random(g,256);if(g->ufo.size==1&&g->ship.active){float best=-1e20f,dx=g->ship.x-g->ufo.x,dy=g->ship.y-g->ufo.y;for(int k=0;k<256;k++){float dot=dx*ast_sin(k)-dy*ast_sin(k+64);if(dot>best){best=dot;a=k;}}}ast_shoot(g,&g->ufo,1,a);g->ufo_fire=g->ufo.size==1?65:95;}}
 else if(g->ufo_wait>0&&!--g->ufo_wait)ast_ufo(g);
 for(int i=0;i<AST_SHOTS;i++)if(g->shots[i].active){struct ast_body*b=&g->shots[i];ast_move(b);if(!--b->life){b->active=0;continue;}
 for(int j=0;j<AST_ROCKS;j++)if(g->rocks[j].active&&ast_hit(b->x,b->y,g->rocks[j].x,g->rocks[j].y,ast_radius(g->rocks[j].size)+2)){ast_break(g,j,!b->size);b->active=0;break;}
 if(b->active&&!b->size&&g->ufo.active&&ast_hit(b->x,b->y,g->ufo.x,g->ufo.y,g->ufo.size==1?14:25)){ast_points(g,g->ufo.size==1?1000:200);ast_sparks(g,g->ufo.x,g->ufo.y,18,2);g->ufo.active=0;g->ufo_wait=420;g->boom_sound=7000;b->active=0;}
 if(b->active&&b->size&&g->ship.active&&!g->shield&&ast_hit(b->x,b->y,g->ship.x,g->ship.y,11)){b->active=0;ast_die(g);}}
 if(g->ship.active&&!g->shield){for(int i=0;i<AST_ROCKS;i++)if(g->rocks[i].active&&ast_hit(g->ship.x,g->ship.y,g->rocks[i].x,g->rocks[i].y,ast_radius(g->rocks[i].size)+9)){ast_die(g);break;}if(g->ufo.active&&ast_hit(g->ship.x,g->ship.y,g->ufo.x,g->ufo.y,g->ufo.size==1?24:34))ast_die(g);}
 if(!ast_count(g)&&!g->ufo.active){if(!g->wave_wait)g->wave_wait=90;else if(!--g->wave_wait)ast_wave(g);}
 g->old_buttons=buttons;
}
/* Procedural sound effects; no original ROM/audio assets. Real output clock 44.1kHz. */
static void ast_audio(struct ast_game*g,int16_t*out,unsigned frames,float*gain,float target){int count=ast_count(g);unsigned interval=10+(unsigned)count*3;if(interval>90)interval=90;
 if(!g->game_over&&g->ticks%interval==0)g->beat_sound=1300;
 for(unsigned i=0;i<frames;i++){g->sound_phase++;g->noise^=g->noise<<13;g->noise^=g->noise>>17;g->noise^=g->noise<<5;float v=0,noise=((int)(g->noise&65535)-32768)/32768.f;
 if(g->thrust)v+=noise*.065f;if(g->shot_sound){v+=((g->sound_phase%(14+g->shot_sound/180))<7?1:-1)*.12f*(g->shot_sound/2300.f);g->shot_sound--;}
 if(g->boom_sound){v+=noise*.22f*(g->boom_sound/12000.f);g->boom_sound--;}
 if(g->warp_sound){v+=ast_sin((int)(g->sound_phase*3))* .13f*(g->warp_sound/4000.f);g->warp_sound--;}
 if(g->beat_sound){v+=(g->sound_phase%((g->ticks/interval)%2?330:410)<150?.10f:-.10f)*(g->beat_sound/1300.f);g->beat_sound--;}
 if(g->ufo.active)v+=ast_sin((int)(g->sound_phase/40+ast_sin((int)(g->sound_phase/300)) *20))*.045f;
 *gain+=(target-*gain)*.002f;float z=v*32767*(*gain);if(z>32767)z=32767;if(z<-32768)z=-32768;out[2*i]=out[2*i+1]=(int16_t)z;
 }
}
#endif
