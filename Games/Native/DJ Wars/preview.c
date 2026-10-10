#include <stdint.h>
#include "../../../native/font.h"
#include "game.h"
#include "render.h"
#include "audio.h"
static struct dw_game game;
static struct dw_audio_state soundtrack;
static float sound_stems[4096];
static unsigned framebuffer[1280*800];
void game_reset(unsigned seed){dw_reset(&game,seed);dw_audio_reset(&soundtrack);dw_cached_version=0;dw_minimap_version=0;dw_camera_x=(int)game.unit[0].x-440;dw_camera_y=(int)game.unit[0].y-410;dw_overview=dw_manual=0;dw_camera_active=-1;dw_camera_limit();}
void game_tick(unsigned buttons){dw_tick(&game,buttons);}
void game_frame(int neon,int paused){dw_render(&game,framebuffer,paused,neon);for(unsigned i=0;i<1280*800;i++){unsigned c=framebuffer[i];framebuffer[i]=0xff000000u|((c&255)<<16)|(c&0xff00)|((c>>16)&255);}}
unsigned*game_buffer(void){return framebuffer;}
void game_weapon(int w){dw_choose_weapon(&game,w);}
void game_target(int x,int y){dw_mark(&game,dw_camera_x+x*dw_divisor(),dw_camera_y+(y-dw_yoffset())*dw_divisor());}
void game_camera(int action,int x,int y){dw_camera(action,x,y);}
int game_value(int id){return id==0?game.state:id==1?game.team:id==2?game.active:id==3?game.remaining:id==4?game.event:id==5?game.event_age:id==6?game.turn:id==7?dw_alive(&game,0):id==8?dw_alive(&game,1):id==9?game.weapon:id==10?game.power:id==11?game.winner:id==12?sizeof(game):id==13?dw_health(&game,0):id==14?dw_health(&game,1):id==15?(int)game.event_serial:id==16?game.hit_pause:id==17?game.finale_assisted:id==18?game.finale_tick:id==19?((game.state==DW_FINALE||(game.state==DW_OVER&&game.finale_result>=0))&&(!game.finale_assisted||game.finale_tick>=DW_CELEBRATE_TICK)):0;}

/* Two mono buses: effects then score, each with a fixed 2048-frame capacity. */
float*game_audio_buffer(void){return sound_stems;}
void game_audio_render(unsigned frames,unsigned rate){
 if(frames>2048)frames=2048;if(rate<8000||rate>192000)rate=44100;
 dw_audio_set_rate(&soundtrack,rate);dw_audio_event(&soundtrack,&game);
 for(unsigned i=0;i<frames;i++){int fx,music;dw_audio_sample(&soundtrack,&game,&fx,&music);sound_stems[i]=dw_audio_limit(fx)/32768.f;sound_stems[2048+i]=dw_audio_limit(music)/32768.f;}
}

/* Explicit browser demonstrations affect only this ephemeral preview instance. */
void game_demo(int result){
 game_reset(0xD1A6);for(int i=0;i<10;i++)if(result?i<5:i>=5||i==2){game.unit[i].hp=0;game.grave[i]=(struct dw_grave){game.unit[i].x,game.unit[i].y,0,1};}
 dw_check_end(&game);
}
