#include <stdint.h>
#include "../../../native/font.h"
#include "../../../native/theme.h"
#include "asteroids.h"
static uint32_t framebuffer[1280*800];static struct ast_game game;static int16_t samples[1470];static float volume=1;
static void ar_rect(int x,int y,int w,int h,unsigned c){c=theme_color(c);uint32_t rgba=0xff000000u|((c&255)<<16)|(c&0xff00)|((c>>16)&255);if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x+w>1280)w=1280-x;if(y+h>800)h=800-y;for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)framebuffer[j*1280+i]=rgba;}
static void ar_text(int x,int y,int scale,const char*s,unsigned c){for(;*s;s++,x+=6*scale){int n=*s>='0'&&*s<='9'?*s-'0':*s>='A'&&*s<='Z'?*s-'A'+10:-1;if(n>=0)for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(font[n][yy]&(1<<(4-xx)))ar_rect(x+xx*scale,y+yy*scale,scale,scale,c);if(*s=='-')ar_rect(x,y+3*scale,5*scale,scale,c);if(*s=='/')for(int i=0;i<5;i++)ar_rect(x+(4-i)*scale,y+(i+1)*scale,scale,scale,c);}}
#include "render.h"
#define cr_rect ar_rect
#define cr_text ar_text
#include "../../../shared/cat_scenes.h"
#undef cr_rect
#undef cr_text
static struct rescue_playback rescue;
void game_rescue_begin(unsigned cat){if(cat<4)rescue_begin(&rescue,cat);}
void game_reset(unsigned seed){rescue.active=0;ast_reset(&game,seed);}
void game_tick(unsigned buttons){if(rescue.active){rescue_advance(&rescue);return;}if(game.game_over&&(buttons&AST_FIRE)&&!(game.old_buttons&AST_FIRE))ast_reset(&game,0x5833+game.ticks);ast_tick(&game,buttons);}
void game_frame(int neon,int paused){theme=neon;if(rescue.active){rescue_scene(rescue.cat,rescue.ticks*1000/60);return;}ast_render(&game,paused);}
uint32_t*game_buffer(void){return framebuffer;}
int16_t*game_sound(unsigned percent){if(rescue.active){rescue_audio(&rescue,samples,735,44100,&volume,percent/100.f);return samples;}ast_audio(&game,samples,735,&volume,percent/100.f);return samples;}
unsigned game_value(int id){if(id==8)return rescue.active;return id==0?game.score:id==1?game.wave:id==2?(unsigned)game.lives:id==3?(unsigned)ast_count(&game):id==4?game.game_over:id==5?sizeof(game):id==6?game.high:id==7?game.wave:0;}

void game_set_best(unsigned best){if(best>game.high)game.high=best;}
