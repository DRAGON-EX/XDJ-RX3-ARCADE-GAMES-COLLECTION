/* Original RX3 fixed-screen alien shooter. No ROMs or original game assets. */
#ifndef SPACE_INVADERS_GAME_H
#define SPACE_INVADERS_GAME_H
#include <stdint.h>
#define SI_LEFT 4
#define SI_RIGHT 8
#define SI_FIRE 16
#define SI_COLS 11
#define SI_ROWS 5
#define SI_ALIENS 55
#define SI_PLAYER_SHOTS 1
#define SI_ENEMY_SHOTS 3
#define SI_W 840
#define SI_H 680
#define SI_PLAYER_Y 620
#define SI_BUNKER_COLS 17
#define SI_BUNKER_ROWS 10
#define SI_BUNKER_CELLS (SI_BUNKER_COLS*SI_BUNKER_ROWS)
#ifndef SI_AUDIO_RATE
#define SI_AUDIO_RATE 48000u
#endif
static const int si_bunker_x[]={108,287,466,645};
struct si_shot {float x,y,vy;int active;};
struct si_particle {float x,y,vx,vy;int life;unsigned color;};
struct si_game {
 unsigned rng,ticks,old_buttons,score,best,extra_at;
 unsigned char alien[SI_ALIENS],bunker[4][SI_BUNKER_CELLS];
 int wave,lives,remaining,over,transition,life_pause,invulnerable,reload;
 int formation_x,formation_y,dir,march,step,enemy_fire;
 int ufo_active,ufo_dir,ufo_wait,ufo_notice,ufo_points;float player,ufo_x;
 struct si_shot shot[SI_PLAYER_SHOTS],enemy[SI_ENEMY_SHOTS];struct si_particle particle[48];
 unsigned audio_phase,march_phase,ufo_phase;int march_left;int audio_left,audio_total,audio_freq;
};
static unsigned si_rand(struct si_game*g){g->rng^=g->rng<<13;g->rng^=g->rng>>17;g->rng^=g->rng<<5;return g->rng;}
static float si_abs(float x){return x<0?-x:x;}
static void si_sound(struct si_game*g,int hz,int samples){g->audio_freq=hz;g->audio_left=g->audio_total=samples;}
static void si_spark(struct si_game*g,float x,float y,unsigned color){for(int j=0;j<8;j++)for(int i=0;i<48;i++)if(!g->particle[i].life){g->particle[i]=(struct si_particle){x,y,((int)(si_rand(g)%81)-40)/20.f,((int)(si_rand(g)%71)-50)/20.f,28,color};break;}}
static void si_award(struct si_game*g,unsigned points){g->score+=points;if(g->score>g->best)g->best=g->score;if(g->extra_at&&g->score>=g->extra_at){g->lives++;g->extra_at=0;si_sound(g,1400,6000);}}
static void si_wave(struct si_game*g,int wave){
 g->wave=wave;g->remaining=SI_ALIENS;g->formation_x=78;g->formation_y=74+(wave-1)%5*12;g->dir=1;g->march=0;g->step=0;g->enemy_fire=80;g->transition=0;g->reload=0;
 g->ufo_active=0;g->ufo_notice=0;g->ufo_wait=480+(int)(si_rand(g)%240);g->player=SI_W/2;
 for(int i=0;i<SI_ALIENS;i++)g->alien[i]=1;
 for(int b=0;b<4;b++)for(int y=0;y<SI_BUNKER_ROWS;y++)for(int x=0;x<SI_BUNKER_COLS;x++)g->bunker[b][y*SI_BUNKER_COLS+x]=(unsigned char)(!(y<2&&(x<2-y||x>=SI_BUNKER_COLS-2+y))&&!(y>=6&&x>=5&&x<=11));
 for(int i=0;i<SI_PLAYER_SHOTS;i++)g->shot[i].active=0;for(int i=0;i<SI_ENEMY_SHOTS;i++)g->enemy[i].active=0;
 for(int i=0;i<48;i++)g->particle[i].life=0;
}
static void si_reset(struct si_game*g,unsigned seed){unsigned best=g->best;*g=(struct si_game){.rng=seed?seed:1978,.lives=3,.best=best,.extra_at=1500};si_wave(g,1);}
static void si_crater(struct si_game*g,float x,float y,int radius){for(int b=0;b<4;b++){
 if(x+radius<si_bunker_x[b]||x-radius>si_bunker_x[b]+85||y+radius<500||y-radius>550)continue;
 for(int yy=0;yy<SI_BUNKER_ROWS;yy++)for(int xx=0;xx<SI_BUNKER_COLS;xx++){float dx=si_bunker_x[b]+xx*5+2-x,dy=502+yy*5-y;if(dx*dx+dy*dy<=radius*radius)g->bunker[b][yy*SI_BUNKER_COLS+xx]=0;}
}}
static int si_shield(struct si_game*g,float x,float y){
 if(y<500||y>=550)return 0;
 for(int b=0;b<4;b++)if(x>=si_bunker_x[b]&&x<si_bunker_x[b]+85){int col=(int)(x-si_bunker_x[b])/5,row=(int)(y-500)/5;if(g->bunker[b][row*SI_BUNKER_COLS+col]){si_crater(g,x,y,10);si_spark(g,x,y,0x95ef8d);return 1;}}
 return 0;
}
static void si_hit_player(struct si_game*g){
 if(g->invulnerable||g->over)return;
 si_spark(g,g->player,SI_PLAYER_Y,0xff6278);si_sound(g,100,18000);
 for(int i=0;i<SI_ENEMY_SHOTS;i++)g->enemy[i].active=0;
 if(--g->lives<=0){g->over=1;return;}
 g->player=SI_W/2;g->life_pause=75;g->invulnerable=180;
}
static void si_enemy_shoot(struct si_game*g){
 int columns[SI_COLS],bottom[SI_COLS],count=0;
 for(int col=0;col<SI_COLS;col++){int row=SI_ROWS-1;while(row>=0&&!g->alien[row*SI_COLS+col])row--;if(row>=0){columns[count]=col;bottom[count++]=row;}}
 if(!count)return;
 int pick=(int)(si_rand(g)%(unsigned)count);
 if(si_rand(g)%4==0){float distance=9999;for(int i=0;i<count;i++){float d=si_abs(g->formation_x+columns[i]*58+17-g->player);if(d<distance){distance=d;pick=i;}}}
 for(int i=0;i<SI_ENEMY_SHOTS;i++)if(!g->enemy[i].active){float speed=3.0f+g->wave*.13f;if(speed>5.5f)speed=5.5f;g->enemy[i]=(struct si_shot){g->formation_x+columns[pick]*58+17.f,g->formation_y+bottom[pick]*39+26.f,speed,1};break;}
}
static void si_march(struct si_game*g){
 int left=9999,right=-1,bottom=0;
 for(int n=0;n<SI_ALIENS;n++)if(g->alien[n]){int x=g->formation_x+n%SI_COLS*58,y=g->formation_y+n/SI_COLS*39;if(x<left)left=x;if(x+34>right)right=x+34;if(y+24>bottom)bottom=y+24;}
 if(right<0)return;
 if(left+g->dir*8<16||right+g->dir*8>SI_W-16){g->dir=-g->dir;g->formation_y+=18;bottom+=18;}else g->formation_x+=g->dir*8;
 g->step++;g->march_left=1100;
 if(bottom>=500)for(int n=0;n<SI_ALIENS;n++)if(g->alien[n])si_crater(g,g->formation_x+n%SI_COLS*58+17,g->formation_y+n/SI_COLS*39+20,25);
 if(bottom>=SI_PLAYER_Y-18){g->over=1;g->lives=0;si_sound(g,90,24000);}
}
static void si_tick(struct si_game*g,unsigned buttons,int pointer){
 unsigned pressed=buttons&~g->old_buttons;g->old_buttons=buttons;
 if(g->over){if(pressed&SI_FIRE)si_reset(g,g->rng);return;}
 g->ticks++;
 if(g->invulnerable)g->invulnerable--;if(g->ufo_notice)g->ufo_notice--;
 for(int i=0;i<48;i++)if(g->particle[i].life){struct si_particle*p=&g->particle[i];p->life--;p->x+=p->vx;p->y+=p->vy;p->vy+=.08f;}
 if(g->transition){if(!--g->transition)si_wave(g,g->wave+1);return;}
 if(g->life_pause){g->life_pause--;return;}
 if(pointer>=0)g->player=pointer;if(buttons&SI_LEFT)g->player-=5.5f;if(buttons&SI_RIGHT)g->player+=5.5f;if(g->player<22)g->player=22;if(g->player>SI_W-22)g->player=SI_W-22;
 if(g->reload)g->reload--;
 if((buttons&SI_FIRE)&&!g->reload)for(int i=0;i<SI_PLAYER_SHOTS;i++)if(!g->shot[i].active){g->shot[i]=(struct si_shot){g->player,SI_PLAYER_Y-18,-9,1};g->reload=14;si_sound(g,1050,1200);break;}
 int interval=4+g->remaining*36/100-(g->wave>15?15:g->wave)/3;if(interval<4)interval=4;
 if(++g->march>=interval){g->march=0;si_march(g);if(g->over)return;}
 if(!--g->enemy_fire){si_enemy_shoot(g);int delay=58-g->wave*3;if(delay<22)delay=22;g->enemy_fire=delay+(int)(si_rand(g)%35);}
 if(g->ufo_active){g->ufo_x+=g->ufo_dir*2.2f;if(g->ufo_x< -50||g->ufo_x>SI_W+50){g->ufo_active=0;g->ufo_wait=480+(int)(si_rand(g)%300);}}
 else if(!--g->ufo_wait){g->ufo_active=1;g->ufo_dir=si_rand(g)%2?1:-1;g->ufo_x=g->ufo_dir>0?-40:SI_W+40;}
 for(int i=0;i<SI_PLAYER_SHOTS;i++){struct si_shot*s=&g->shot[i];if(!s->active)continue;
  for(int sub=0;sub<2&&s->active;sub++){
   s->y+=s->vy*.5f;if(s->y<0){s->active=0;break;}
   if(si_shield(g,s->x,s->y-4)){s->active=0;break;}
   if(g->ufo_active&&si_abs(s->x-g->ufo_x)<25&&s->y>=22&&s->y<=43){g->ufo_active=0;g->ufo_wait=600;g->ufo_points=(int[]){50,100,150,300}[si_rand(g)%4];g->ufo_notice=100;si_award(g,g->ufo_points);si_spark(g,g->ufo_x,32,0xff6278);si_sound(g,1300,7000);s->active=0;break;}
   for(int n=0;n<SI_ALIENS;n++)if(g->alien[n]){int x=g->formation_x+n%SI_COLS*58,y=g->formation_y+n/SI_COLS*39;if(s->x>=x-2&&s->x<=x+36&&s->y>=y-5&&s->y<=y+28){s->active=0;if(!--g->alien[n]){g->remaining--;si_award(g,n<SI_COLS?30:n<SI_COLS*3?20:10);si_spark(g,x+17,y+12,0xffcd55);}si_sound(g,420+n/SI_COLS*80,2500);break;}}
   // Player fire can intercept incoming shots.
   if(s->active)for(int j=0;j<SI_ENEMY_SHOTS;j++)if(g->enemy[j].active&&si_abs(s->x-g->enemy[j].x)<5&&si_abs(s->y-g->enemy[j].y)<10){s->active=g->enemy[j].active=0;si_spark(g,s->x,s->y,0x66c8ff);break;}
  }
 }
 for(int i=0;i<SI_ENEMY_SHOTS;i++){struct si_shot*s=&g->enemy[i];
  for(int sub=0;sub<2&&s->active;sub++){
   s->y+=s->vy*.5f;if(s->y>SI_H){s->active=0;break;}
   if(si_shield(g,s->x,s->y+4)){s->active=0;break;}
   if(si_abs(s->x-g->player)<22&&si_abs(s->y-SI_PLAYER_Y)<15){s->active=0;si_hit_player(g);}
  }
 }
 if(!g->remaining&&!g->over){g->transition=120;si_sound(g,1400,12000);}
}
static void si_audio(struct si_game*g,int16_t*out,unsigned frames,float*gain,float target){
 for(unsigned i=0;i<frames;i++){
  *gain+=(target-*gain)*.002f;int v=0;
  if(g->audio_left){g->audio_phase+=(unsigned)g->audio_freq*(unsigned)(4294967296ULL/SI_AUDIO_RATE);v=((g->audio_phase&0x80000000u)?1:-1)*1900*g->audio_left/g->audio_total;g->audio_left--;}
  if(g->march_left){g->march_phase+=(180-(g->step%4)*20)*(unsigned)(4294967296ULL/SI_AUDIO_RATE);v+=((g->march_phase&0x80000000u)?1:-1)*500*g->march_left/1100;g->march_left--;}
  if(g->ufo_active&&!g->over&&!g->life_pause){g->ufo_phase+=(unsigned)(380+(g->ticks%60)*5)*(unsigned)(4294967296ULL/SI_AUDIO_RATE);v+=((g->ufo_phase&0x80000000u)?1:-1)*180;}
  out[2*i]=out[2*i+1]=(int16_t)(v**gain);
 }
}
#endif
