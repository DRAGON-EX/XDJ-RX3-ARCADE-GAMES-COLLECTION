/*
 * Equalizarr: altered/extended RX3 implementation based on the Arkanoid sample
 * by Marc Palau and Ramon Santamaria, copyright (c) 2015 Ramon Santamaria.
 * See LICENSE.raylib-games and upstream/arkanoid.c.
 * Original 50-level campaign, power-ups, bounded pools, collisions and audio.
 */
#ifndef EQUALIZARR_GAME_H
#define EQUALIZARR_GAME_H
#include <stdint.h>
#ifndef EQ_AUDIO_RATE
#define EQ_AUDIO_RATE 48000u
#endif
#include "levels.h"
#define EQ_LEFT 4
#define EQ_RIGHT 8
#define EQ_FIRE 16
#define EQ_BALLS 6
#define EQ_DROPS 24
#define EQ_SHOTS 24
#define EQ_PARTICLES 64
#define EQ_W 840
#define EQ_H 680
#define EQ_PY 621
enum {EQ_SHIELD,EQ_LASER,EQ_MULTI,EQ_WIDE,EQ_STICKY,EQ_SLOW,EQ_FIREBALL,EQ_LIFE,EQ_POWERS};
static const char *eq_power_names[]={"SHIELD","LASER","MULTIBALL","WIDE PADDLE","STICKY","SLOW MOTION","FIREBALL","EXTRA LIFE"};
static const char *eq_power_letters[]={"S","L","M","W","C","T","F","+"};
struct eq_ball {float x,y,vx,vy,trail_x[4],trail_y[4];int active,stuck;};
struct eq_drop {float x,y;int active,kind;};
struct eq_shot {float x,y;int active;};
struct eq_particle {float x,y,vx,vy;int life;unsigned color;};
struct eq_game {
 unsigned rng,ticks,score,best,old_buttons,broken,combo,cleared_levels;
 int level,lives,remaining,over,won,transition,notice,notice_kind,fire_delay;
 float paddle;
 unsigned char brick[EQ_CELLS],kind[EQ_CELLS];
 int power[EQ_POWERS];
 struct eq_ball ball[EQ_BALLS];
 struct eq_drop drop[EQ_DROPS];
 struct eq_shot shot[EQ_SHOTS];
 struct eq_particle particle[EQ_PARTICLES];
 unsigned audio_phase;int audio_left,audio_freq,audio_total;
};
static float eq_abs(float a){return a<0?-a:a;}
static float eq_clamp(float a,float lo,float hi){return a<lo?lo:a>hi?hi:a;}
static unsigned eq_rand(struct eq_game*g){g->rng^=g->rng<<13;g->rng^=g->rng>>17;g->rng^=g->rng<<5;return g->rng;}
static void eq_sound(struct eq_game*g,int freq,int length){g->audio_freq=freq;g->audio_total=g->audio_left=length;}
static int eq_half(const struct eq_game*g){return g->power[EQ_WIDE]?94:62;}
static float eq_speed(const struct eq_game*g){return 6.2f+g->level*.065f;}
static int eq_count(const struct eq_game*g){int n=0;for(int i=0;i<EQ_BALLS;i++)n+=g->ball[i].active;return n;}
static void eq_serve(struct eq_game*g){g->ball[0]=(struct eq_ball){.x=g->paddle,.y=EQ_PY-17,.active=1,.stuck=1};}
static void eq_load(struct eq_game*g,int level){
 g->level=level;g->remaining=0;g->transition=0;g->combo=0;g->paddle=EQ_W/2;
 for(int i=0;i<EQ_CELLS;i++){int c=eq_levels[level][i/EQ_COLS][i%EQ_COLS]-'0';g->kind[i]=c==5?2:c==4?1:0;g->brick[i]=c==5?255:c>=1&&c<=4?(c==4?1:c):0;g->remaining+=g->brick[i]!=0&&g->brick[i]!=255;}
 for(int i=0;i<EQ_BALLS;i++)g->ball[i]=(struct eq_ball){0};
 for(int i=0;i<EQ_DROPS;i++)g->drop[i].active=0;
 for(int i=0;i<EQ_SHOTS;i++)g->shot[i].active=0;
 for(int i=0;i<EQ_PARTICLES;i++)g->particle[i].life=0;
 for(int i=0;i<EQ_POWERS;i++)g->power[i]=0;
 g->fire_delay=0;eq_serve(g);
}
static void eq_reset(struct eq_game*g,unsigned seed,int level){unsigned best=g->best;*g=(struct eq_game){.rng=seed?seed:1729,.lives=4,.best=best};eq_load(g,level>=0&&level<50?level:0);}
static void eq_spark(struct eq_game*g,float x,float y,unsigned color){for(int j=0;j<8;j++)for(int i=0;i<EQ_PARTICLES;i++)if(!g->particle[i].life){g->particle[i]=(struct eq_particle){x,y,(int)(eq_rand(g)%81)/20.f-2,(int)(eq_rand(g)%61)/20.f-2.5f,22,color};break;}}
static void eq_drop_power(struct eq_game*g,float x,float y){for(int i=0;i<EQ_DROPS;i++)if(!g->drop[i].active){g->drop[i]=(struct eq_drop){x,y,1,(int)(eq_rand(g)%EQ_POWERS)};return;}}
static void eq_hit(struct eq_game*g,int index,int force){
 if(index<0||index>=EQ_CELLS||!g->brick[index])return;
 if(g->brick[index]==255){eq_sound(g,160,1000);return;}
 if(!force&&--g->brick[index]){eq_sound(g,260,1600);return;}
 g->brick[index]=0;g->remaining--;g->broken++;g->score+=100+g->level*5+(g->combo<10?g->combo*10:100);g->combo++;
 float x=EQ_BX+(index%EQ_COLS)*EQ_DX+EQ_BW/2,y=EQ_BY+(index/EQ_COLS)*EQ_DY+EQ_BH/2;
 eq_spark(g,x,y,g->kind[index]?0xffac64:0x64ffe1);eq_sound(g,500+g->combo%8*90,2400);
 if(g->broken%5==0)eq_drop_power(g,x,y);
 if(g->kind[index]){g->kind[index]=0;int row=index/EQ_COLS,col=index%EQ_COLS;for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++)if(row+dy>=0&&row+dy<EQ_ROWS&&col+dx>=0&&col+dx<EQ_COLS)eq_hit(g,(row+dy)*EQ_COLS+col+dx,1);}
 if(g->score>g->best)g->best=g->score;
}
static void eq_apply(struct eq_game*g,int kind){
 g->notice_kind=kind;g->notice=150;eq_sound(g,1100,7200);
 if(kind==EQ_LIFE){if(g->lives<9)g->lives++;return;}
 if(kind==EQ_MULTI){int from=-1;for(int i=0;i<EQ_BALLS;i++)if(g->ball[i].active&&!g->ball[i].stuck){from=i;break;}if(from<0)from=0;struct eq_ball b=g->ball[from];int added=0;for(int i=0;i<EQ_BALLS&&added<2;i++)if(!g->ball[i].active){g->ball[i]=b;g->ball[i].active=1;g->ball[i].stuck=0;g->ball[i].vx=added?-3.5f:3.5f;g->ball[i].vy=-eq_speed(g);added++;}return;}
 g->power[kind]=kind==EQ_SHIELD?3:kind==EQ_FIREBALL?600:kind==EQ_SLOW?900:1200;
}
static void eq_fire(struct eq_game*g){for(int side=-1;side<=1;side+=2)for(int i=0;i<EQ_SHOTS;i++)if(!g->shot[i].active){g->shot[i]=(struct eq_shot){g->paddle+side*(eq_half(g)-8),EQ_PY-10,1};break;}g->fire_delay=12;eq_sound(g,1600,1800);}
static void eq_tick(struct eq_game*g,unsigned buttons,int pointer){
 if(g->over||g->won){if((buttons&EQ_FIRE)&&!(g->old_buttons&EQ_FIRE))eq_reset(g,g->rng,0);g->old_buttons=buttons;return;}
 g->ticks++;if(g->notice)g->notice--;
 if(g->transition){if(!--g->transition){if(g->level==49)g->won=1;else eq_load(g,g->level+1);}g->old_buttons=buttons;return;}
 for(int i=0;i<EQ_POWERS;i++)if(i!=EQ_SHIELD&&g->power[i])g->power[i]--;
 if(g->fire_delay)g->fire_delay--;
 int half=eq_half(g);if(pointer>=0)g->paddle=pointer;
 if(buttons&EQ_LEFT)g->paddle-=9;if(buttons&EQ_RIGHT)g->paddle+=9;
 g->paddle=eq_clamp(g->paddle,half+10,EQ_W-half-10);
 int edge=(buttons&EQ_FIRE)&&!(g->old_buttons&EQ_FIRE);g->old_buttons=buttons;
 if(buttons&EQ_FIRE&&g->power[EQ_LASER]&&!g->fire_delay)eq_fire(g);
 for(int i=0;i<EQ_BALLS;i++){
  struct eq_ball*b=&g->ball[i];if(!b->active)continue;
  for(int j=3;j>0;j--){b->trail_x[j]=b->trail_x[j-1];b->trail_y[j]=b->trail_y[j-1];}b->trail_x[0]=b->x;b->trail_y[0]=b->y;
  if(b->stuck){b->x=g->paddle;b->y=EQ_PY-17;if(edge){b->stuck=0;b->vx=1.6f;b->vy=-eq_speed(g);}else continue;}
  float factor=g->power[EQ_SLOW]?.68f:1.f;
  // Substeps cap travel below the ball radius; local cell lookup avoids scanning the full board.
  for(int sub=0;sub<4&&b->active&&!b->stuck;sub++){
   float ox=b->x,oy=b->y;b->x+=b->vx*factor*.25f;b->y+=b->vy*factor*.25f;
   if(b->x<10){b->x=10;b->vx=eq_abs(b->vx);}if(b->x>EQ_W-10){b->x=EQ_W-10;b->vx=-eq_abs(b->vx);}
   if(b->y<10){b->y=10;b->vy=eq_abs(b->vy);eq_sound(g,220,1000);}
   if(b->vy>0&&oy<=EQ_PY-15&&b->y>=EQ_PY-15&&eq_abs(b->x-g->paddle)<=half+7){
    b->y=EQ_PY-15;b->vx=eq_clamp((b->x-g->paddle)/half,-1,1)*eq_speed(g)*.85f;
    if(eq_abs(b->vx)<.6f)b->vx=b->vx<0?-.6f:.6f;
    b->vy=-eq_speed(g)*(1.0f-eq_abs(b->vx)/eq_speed(g)*.28f);g->combo=0;
    if(g->power[EQ_STICKY])b->stuck=1;eq_sound(g,390,1800);
   }
   if(b->y>EQ_H-20&&b->vy>0&&g->power[EQ_SHIELD]){g->power[EQ_SHIELD]--;b->y=EQ_H-20;b->vy=-eq_abs(b->vy);eq_sound(g,180,4000);}
   if(b->y>EQ_H+10){b->active=0;break;}
   int col=(int)(b->x-EQ_BX)/EQ_DX,row=(int)(b->y-EQ_BY)/EQ_DY,hit=0;
   for(int yy=row-1;yy<=row+1&&!hit;yy++)for(int xx=col-1;xx<=col+1&&!hit;xx++){
    if(xx<0||xx>=EQ_COLS||yy<0||yy>=EQ_ROWS)continue;int n=yy*EQ_COLS+xx;if(!g->brick[n])continue;
    float left=EQ_BX+xx*EQ_DX,top=EQ_BY+yy*EQ_DY;
    if(b->x+7<left||b->x-7>left+EQ_BW||b->y+7<top||b->y-7>top+EQ_BH)continue;
    int steel=g->brick[n]==255;eq_hit(g,n,g->power[EQ_FIREBALL]!=0);hit=1;
    if(steel||!g->power[EQ_FIREBALL]){
     if(oy+7<=top){b->y=top-7;b->vy=-eq_abs(b->vy);}
     else if(oy-7>=top+EQ_BH){b->y=top+EQ_BH+7;b->vy=eq_abs(b->vy);}
     else if(ox<left){b->x=left-7;b->vx=-eq_abs(b->vx);}
     else{b->x=left+EQ_BW+7;b->vx=eq_abs(b->vx);}
    }
   }
  }
 }
 for(int i=0;i<EQ_SHOTS;i++){struct eq_shot*s=&g->shot[i];if(!s->active)continue;s->y-=12;if(s->y<0){s->active=0;continue;}for(int n=0;n<EQ_CELLS;n++)if(g->brick[n]&&s->x>=EQ_BX+(n%EQ_COLS)*EQ_DX&&s->x<=EQ_BX+EQ_BW+(n%EQ_COLS)*EQ_DX&&s->y<=EQ_BY+EQ_BH+(n/EQ_COLS)*EQ_DY&&s->y+12>=EQ_BY+(n/EQ_COLS)*EQ_DY){eq_hit(g,n,0);s->active=0;break;}}
 for(int i=0;i<EQ_DROPS;i++){struct eq_drop*d=&g->drop[i];if(!d->active)continue;d->y+=2.2f;if(d->y>EQ_PY-14&&d->y<EQ_PY+22&&eq_abs(d->x-g->paddle)<half+12){eq_apply(g,d->kind);d->active=0;}else if(d->y>EQ_H)d->active=0;}
 for(int i=0;i<EQ_PARTICLES;i++){struct eq_particle*p=&g->particle[i];if(p->life){p->life--;p->x+=p->vx;p->y+=p->vy;p->vy+=.1f;}}
 if(!g->remaining){g->cleared_levels++;g->transition=120;eq_sound(g,1400,16000);return;}
 if(!eq_count(g)){if(--g->lives<=0)g->over=1;else{for(int i=0;i<EQ_POWERS;i++)g->power[i]=0;for(int i=0;i<EQ_DROPS;i++)g->drop[i].active=0;for(int i=0;i<EQ_SHOTS;i++)g->shot[i].active=0;eq_serve(g);}eq_sound(g,100,14000);}
}
static void eq_audio(struct eq_game*g,int16_t*out,unsigned frames,float*gain,float target){
 for(unsigned i=0;i<frames;i++){*gain+=(target-*gain)*.002f;int value=0;if(g->audio_left>0){g->audio_phase+=(unsigned)g->audio_freq*(unsigned)(4294967296ULL/EQ_AUDIO_RATE);value=((g->audio_phase&0x80000000u)?1:-1)*1800*g->audio_left/g->audio_total;g->audio_left--;}out[2*i]=out[2*i+1]=(int16_t)(value**gain);}
}
#endif
