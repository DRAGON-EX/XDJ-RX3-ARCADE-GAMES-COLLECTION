#include <stdint.h>
#include "../../../native/font.h"
#include "../../../native/theme.h"
#include "game.h"
static uint32_t framebuffer[1280*800];static struct eq_game game;static int16_t samples[1600];static float volume;
static void eq_rect(int x,int y,int w,int h,unsigned c){c=theme_color(c);unsigned rgba=0xff000000u|((c&255)<<16)|(c&0xff00)|((c>>16)&255);if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x+w>1280)w=1280-x;if(y+h>800)h=800-y;for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)framebuffer[j*1280+i]=rgba;}
static void eq_text(int x,int y,int size,const char*s,unsigned c){for(;*s;s++,x+=6*size){int n=*s>='0'&&*s<='9'?*s-'0':*s>='A'&&*s<='Z'?*s-'A'+10:-1;if(n>=0)for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(font[n][yy]&(1<<(4-xx)))eq_rect(x+xx*size,y+yy*size,size,size,c);if(*s=='-'||*s=='+')eq_rect(x,y+3*size,5*size,size,c);if(*s=='+')eq_rect(x+2*size,y,size,7*size,c);if(*s=='/')for(int i=0;i<5;i++)eq_rect(x+(4-i)*size,y+(i+1)*size,size,size,c);if(*s=='!'){eq_rect(x+2*size,y,size,5*size,c);eq_rect(x+2*size,y+6*size,size,size,c);}}}
#include "render.h"
#define cr_rect eq_rect
#define cr_text eq_text
#include "../../../shared/cat_scenes.h"
#undef cr_rect
#undef cr_text
static struct rescue_playback rescue;
void game_rescue_begin(unsigned cat){if(cat<4)rescue_begin(&rescue,cat);}
void game_reset(unsigned seed,int level){rescue.active=0;eq_reset(&game,seed,level);}
void game_tick(unsigned buttons,int pointer){if(rescue.active){rescue_advance(&rescue);return;}eq_tick(&game,buttons,pointer);}
void game_frame(int neon,int paused){theme=neon;if(rescue.active){rescue_scene(rescue.cat,rescue.ticks*1000/60);return;}eq_render(&game,paused);}
uint32_t*game_buffer(void){return framebuffer;}
int16_t*game_sound(unsigned percent){if(rescue.active){rescue_audio(&rescue,samples,800,48000,&volume,percent/100.f);return samples;}eq_audio(&game,samples,800,&volume,percent/100.f);return samples;}
unsigned game_value(int id){if(id==8)return rescue.active;return id==0?game.score:id==1?game.level+1:id==2?(unsigned)game.lives:id==3?(unsigned)game.remaining:id==4?game.over||game.won:id==5?sizeof(game):id==6?game.best:id==7?game.cleared_levels:0;}

void game_set_best(unsigned best){if(best>game.best)game.best=best;}
