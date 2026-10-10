#include "platform.h"
#include "theme.h"
#include "panel_controls.h"
#include "master_state.h"
#include "panel_output.h"
#include "../Games/EMU/NES/nes.h"
#include "../Games/EMU/NES/RoadFighter/nes_controls.h"
#include "crossfader_pointer.h"
static struct crossfader_pointer game_crossfader;
#include "debug_info.h"
static struct debug_snapshot debug_data;
static uint64_t debug_next_update,debug_last_tap;static unsigned debug_taps;
static uint64_t ns(void);
extern int sched_setaffinity(int,size_t,const void*),sched_getaffinity(int,size_t,void*);
static void allow_online_cpus(void){
 unsigned long mask=1;for(int c=1;c<4;c++){char path[96],value[8]={0};snprintf(path,sizeof(path),"/sys/devices/system/cpu/cpu%d/online",c);int fd=open(path,0);if(fd>=0){long n=read(fd,value,sizeof(value));close(fd);if(n>0&&value[0]=='1')mask|=1ul<<c;}}
 if(sched_setaffinity(0,sizeof(mask),&mask)==0)printf("Application CPU affinity: online RX3 cores mask=%lu\n",mask);else printf("Application CPU affinity unchanged, errno=%d\n",*__errno_location());
}

static void menu_sound_trigger(void);

static int nes_active,nes_background,nes_volume_was,nes_pause_was;static uint32_t nes_draw_cache[512*480];static const char *nes_rom="/tmp/rx3-launcher/payload/road-fighter.nes",*nes_core="/tmp/rx3-launcher/fceumm_libretro.so";
static unsigned nes_held,nes_a,nes_start,nes_select,nes_cue_start,test_nes_buttons;static struct nes_steering steering;static uint64_t sub_seen,eup_seen;
extern char *getenv(const char*);
static unsigned volume_percent,volume_until;static int volume_kind;static unsigned gain_percent[2];static int gain_seen[2];static int volume_seen,usb_port=1,led_state=-1;static struct usb_stop_hold eject_hold;
static volatile int running=1;
static uint32_t *pixels;static int width=1280,height=800,cell,bx,by;
static int selected_game,dummy_notice,help_page,help_offset;

static void debug_update(uint64_t now){if(help_page==4&&now>=debug_next_update){debug_collect(&debug_data,debug_read_file,debug_pause);debug_next_update=now+1000000000ULL;}}
static void version_tap(uint64_t now){
 if(!debug_taps||now-debug_last_tap>500000000ULL)debug_taps=0;
 debug_last_tap=now;
 if(++debug_taps==5){menu_sound_trigger();debug_taps=0;help_page=4;debug_data=(struct debug_snapshot){0};debug_collect(&debug_data,debug_read_file,debug_pause);debug_next_update=ns()+1000000000ULL;}
}
#include "help_text.h"
static int help_count(void){return help_page==7?(int)(sizeof(help_story)/sizeof(*help_story)):help_page==5?(int)(sizeof(help_credits)/sizeof(*help_credits)):help_page==1?(int)(sizeof(help_about)/sizeof(*help_about)):(int)(sizeof(help_controls)/sizeof(*help_controls));}
static void help_scroll(int steps){if(help_page!=1&&help_page!=2&&help_page!=7)return;help_offset+=steps*10;int max=((help_count()-1)/10)*10;if(help_offset<0)help_offset=0;if(help_offset>max)help_offset=max;}
static void story_continue(void){if(help_offset+10<help_count())help_scroll(1);else help_page=0,help_offset=0;}
#define MENU_GAME_COUNT 6
#define ROAD_FIGHTER_GAME 6
static const char*game_names[]={"NEON MAZE","EQUALIZARR","BASS ROCKS","DJ WARS","BASSLINES","DROP DEFENSE","ROAD FIGHTER"};
static const int menu_order[]={0,1,2,4,5,3};
static void menu_move(int steps){int pos=0;while(pos<MENU_GAME_COUNT&&menu_order[pos]!=selected_game)pos++;int next=(pos+steps)%MENU_GAME_COUNT;if(next<0)next+=MENU_GAME_COUNT;selected_game=menu_order[next];}
static unsigned tick,direction;static int paused,menu=1;static const uint32_t colors[]={0xffcd55,0xff6278,0xee8dd0,0x64e8ee,0xffac64};
static void stop(int sig){(void)sig;running=0;}
/* NES writes identify changed scanline ranges, avoiding a full-screen compare. */
#define DIRTY_ROWS 2048
static unsigned short dirty_left[DIRTY_ROWS],dirty_right[DIRTY_ROWS];
static void mark_dirty(int x,int y,int w,int h){
 if(!nes_active||menu||height>DIRTY_ROWS||w<=0||h<=0)return;
 if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}
 if(x+w>width)w=width-x;if(y+h>height)h=height-y;
 if(w<=0||h<=0)return;
 for(int row=y;row<y+h;row++){
  if(!dirty_right[row]||x<dirty_left[row])dirty_left[row]=x;
  if(x+w>dirty_right[row])dirty_right[row]=x+w;
 }
}
static void rect(int x,int y,int w,int h,uint32_t c){c=theme_color(c);if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x+w>width)w=width-x;if(y+h>height)h=height-y;mark_dirty(x,y,w,h);for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)pixels[yy*width+xx]=c;}
static void disk(int x,int y,int r,uint32_t c){for(int yy=-r;yy<=r;yy++)for(int xx=-r;xx<=r;xx++)if(xx*xx+yy*yy<=r*r)rect(x+xx,y+yy,1,1,c);}
#include "font.h"
#include "dragon_logo.h"
static void logo(int x,int y){for(int yy=0;yy<112;yy++)for(int xx=0;xx<112;xx++){if(x+xx<0||x+xx>=width||y+yy<0||y+yy>=height)continue;const unsigned char *v=dragon_logo+4*(yy*112+xx);unsigned a=v[3];uint32_t *d=pixels+(y+yy)*width+x+xx;unsigned r=(v[0]*a+((*d>>16)&255)*(255-a))/255,g=(v[1]*a+((*d>>8)&255)*(255-a))/255,b=(v[2]*a+(*d&255)*(255-a))/255;*d=(r<<16)|(g<<8)|b;}}
static void text(int x,int y,int scale,const char*s,uint32_t c){for(;*s;s++,x+=6*scale){int n=*s>='0'&&*s<='9'?*s-'0':*s>='A'&&*s<='Z'?*s-'A'+10:-1;if(*s=='&'){const unsigned char amp[]={12,18,20,8,21,18,13};for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(amp[yy]&(1<<(4-xx)))rect(x+xx*scale,y+yy*scale,scale,scale,c);continue;}if(*s=='%'){rect(x,y,scale,scale,c);rect(x+4*scale,y+6*scale,scale,scale,c);for(int i=0;i<5;i++)rect(x+(4-i)*scale,y+(i+1)*scale,scale,scale,c);continue;}if(*s=='!'){rect(x+2*scale,y,scale,5*scale,c);rect(x+2*scale,y+6*scale,scale,scale,c);continue;}if(*s=='~'){rect(x,y+2*scale,2*scale,scale,c);rect(x+3*scale,y+scale,2*scale,scale,c);rect(x+scale,y+5*scale,3*scale,scale,c);rect(x+4*scale,y+4*scale,scale,scale,c);continue;}if(*s=='^'){rect(x,y+2*scale,4*scale,3*scale,c);rect(x+scale,y,scale,2*scale,c);rect(x+3*scale,y+scale,2*scale,scale,c);rect(x+2*scale,y+5*scale,2*scale,scale,c);continue;}if(*s=='\''||*s==','){rect(x+2*scale,y+(*s==','?6:0)*scale,scale,scale,c);continue;}if(*s==':'||*s=='/'){if(*s==':'){rect(x+2*scale,y+2*scale,scale,scale,c);rect(x+2*scale,y+5*scale,scale,scale,c);}else for(int i=0;i<5;i++)rect(x+(4-i)*scale,y+(i+1)*scale,scale,scale,c);continue;}if(*s=='+'){rect(x,y+3*scale,5*scale,scale,c);rect(x+2*scale,y,scale,7*scale,c);continue;}if(*s=='?'){const unsigned char q[]={14,17,1,2,4,0,4};for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(q[yy]&(1<<(4-xx)))rect(x+xx*scale,y+yy*scale,scale,scale,c);continue;}if(*s=='.'){rect(x+2*scale,y+6*scale,scale,scale,c);continue;}if(*s=='_'){rect(x,y+6*scale,5*scale,scale,c);continue;}if(*s=='-'){rect(x,y+3*scale,5*scale,scale,c);continue;}if(n<0)continue;for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(font[n][yy]&(1<<(4-xx)))rect(x+xx*scale,y+yy*scale,scale,scale,c);}}
#include "music_player.h"
#include "cafe_show_audio.h"
#include "cafe_show_leds.h"
static struct cafe_show_led_state cafe_led_show;
static unsigned cafe_show_requested;
static const char*cafe_show_path="/tmp/rx3-launcher/payload/cafe-show.pcm";
static void cafe_show_toggle(void){
 unsigned on=__atomic_load_n(&cafe_show_requested,__ATOMIC_ACQUIRE);
 __atomic_store_n(&cafe_show_requested,!on,__ATOMIC_RELEASE);
}
#include "debug_artwork.h"
#include "story_artwork.h"
#include "credits.h"
#define cr_rect rect
#define cr_text text
#include "../shared/cat_scenes.h"
#undef cr_rect
#undef cr_text
static struct rescue_playback rescue;
static int debug_unlocked,debug_rescue_test;
static struct cafe_state cafe;
static unsigned cafe_seen,cafe_started;
static void open_cafe(void){
 music_cafe_begin(credits_active?credits_saved_position:__atomic_load_n(&music_position,__ATOMIC_RELAXED),credits_active?credits_saved_pause:__atomic_load_n(&music_paused,__ATOMIC_RELAXED));
 cafe_open(&cafe,cafe_seen,session_rescued_cats,session_dj_wars_won,tick+1);cafe_started=tick;help_page=11;
}
static int credits_to_cafe(void){
 if(!menu||help_page!=5||!credits_active||!credits_completed)return 0;
 /* Publish the café range first: even an in-flight audio seek cannot expose
  * the previous playlist between the drained credits and Pixel Plains. */
 open_cafe();credits_end();
 if(__atomic_load_n(&music_cafe_mode,__ATOMIC_ACQUIRE)){music_seek(music_tracks[music_cafe_track].start);__atomic_store_n(&music_paused,0,__ATOMIC_RELAXED);}
 return 1;
}
#include "../Games/Native/Asteroids/asteroids.h"
#include "left_jog.h"
static struct ast_game asteroid;static unsigned asteroid_keys,asteroid_touch;
static struct left_jog asteroid_jog;
static void asteroid_jog_sample(const unsigned char*frame,size_t n,uint64_t now){
 int active=selected_game==2&&!menu&&!paused&&!rescue.active&&asteroid.ship.active&&!asteroid.game_over;
 int turn=left_jog_rotation(&asteroid_jog,frame,n,now,active);
 if(turn)asteroid.ship.angle=(asteroid.ship.angle+turn)&255;
}
#define ar_rect rect
#define ar_text text
#include "../Games/Native/Asteroids/render.h"
#undef ar_rect
#undef ar_text
#include "dj_wars.h"
#define EQ_AUDIO_RATE 44100u
#include "../Games/Native/Equalizarr/game.h"
static struct eq_game equalizarr;static unsigned equalizarr_keys;static int equalizarr_pointer=-1;
#define eq_rect rect
#define eq_text text
#include "../Games/Native/Equalizarr/render.h"
#undef eq_rect
#undef eq_text
#define BL_AUDIO_RATE 44100u
#include "../Games/Native/Basslines/game.h"
static struct bl_game basslines;static unsigned basslines_keys,basslines_touch_keys;
#define bl_rect rect
#define bl_text text
#include "../Games/Native/Basslines/render.h"
#undef bl_rect
#undef bl_text
#define SI_AUDIO_RATE 44100u
#include "../Games/Native/Space Invaders/game.h"
static struct si_game invaders;static unsigned invaders_keys;static int invaders_pointer=-1;
#define si_rect rect
#define si_text text
#include "../Games/Native/Space Invaders/render.h"
#undef si_rect
#undef si_text
#include "persistent_save.h"
static void persistent_restore(void){
 struct save_values v={0};char*enabled=getenv("RX3_PERSISTENT_SAVE");
 if(!save_begin(getenv("RX3_USB_ROOT"),enabled&&!strcmp(enabled,"1"),&v))return;
 arcade_restore_high(v.word[0]);equalizarr.best=v.word[1];asteroid.high=v.word[2];basslines.best=v.word[3];invaders.best=v.word[4];
 session_rescued_cats=v.word[5];session_dj_wars_won=v.word[6];music_secret_unlocked=v.word[7];cafe_seen=v.word[8];
 for(unsigned i=0;i<music_count&&i<32;i++)if((v.word[9]&(1u<<i))&&music_tracks[i].locked==1)music_tracks[i].locked=0;
}
static void persistent_update(void){
 if(!persistent.started||debug_unlocked)return;
 struct save_values v={{arcade_value(6),equalizarr.best,asteroid.high,basslines.best,invaders.best,session_rescued_cats,session_dj_wars_won,__atomic_load_n(&music_secret_unlocked,__ATOMIC_ACQUIRE),cafe_seen,0}};
 for(unsigned i=0;i<music_count&&i<32;i++)if(!music_tracks[i].locked)v.word[9]|=1u<<i;
 save_queue(&v);
}
static void update_game_rescue(void){
 unsigned before=session_rescued_cats;
 if(selected_game==1)session_check_rescue(1,equalizarr.cleared_levels);
 else if(selected_game==2)session_check_rescue(2,asteroid.wave);
 else if(selected_game==4)session_check_rescue(4,basslines.score[0]);
 else if(selected_game==5)session_check_rescue(5,(unsigned)invaders.wave);
 unsigned added=session_rescued_cats&~before;
 if(added){unsigned cat=0;while(!(added&(1u<<cat)))cat++;rescue_begin(&rescue,cat);equalizarr_keys=invaders_keys=basslines_keys=basslines_touch_keys=asteroid_keys=asteroid_touch=0;equalizarr_pointer=invaders_pointer=-1;}
}
static void equalizarr_step(void){
 unsigned keys=equalizarr_keys|(nes_held&(EQ_LEFT|EQ_RIGHT))|(nes_a?EQ_FIRE:0);
 int half=eq_half(&equalizarr);
 int pointer=crossfader_position(&game_crossfader,steering.raw,steering.valid,keys&(EQ_LEFT|EQ_RIGHT),equalizarr_pointer,half+10,EQ_W-half-10);
 eq_tick(&equalizarr,keys,pointer);
}
static void invaders_step(void){
 unsigned keys=invaders_keys|(nes_held&(SI_LEFT|SI_RIGHT))|(nes_a?SI_FIRE:0);
 int pointer=crossfader_position(&game_crossfader,steering.raw,steering.valid,keys&(SI_LEFT|SI_RIGHT),invaders_pointer,22,SI_W-22);
 si_tick(&invaders,keys,pointer);
}
static void reset_current_game(void){rescue.active=0;asteroid_jog=(struct left_jog){0};crossfader_reset(&game_crossfader,steering.raw,steering.valid);if(selected_game==5){si_reset(&invaders,1978+tick);invaders_keys=0;invaders_pointer=-1;}else if(selected_game==4){bl_reset(&basslines,2401+tick);basslines_keys=basslines_touch_keys=0;}else if(selected_game==1){eq_reset(&equalizarr,1729+tick,0);equalizarr_keys=0;equalizarr_pointer=-1;}else if(selected_game==3){dj_wars_reset();}else if(selected_game==2)ast_reset(&asteroid,0x5833u+tick);else arcade_reset();direction=0;}
static void start_game(void){reset_current_game();paused=0;menu=0;}
static void choose_game(void){if(selected_game==3&&!session_finale_ready()){help_page=9;return;}if(selected_game==0||selected_game==1||selected_game==4||selected_game==5)start_game();else if(selected_game==ROAD_FIGHTER_GAME){if(nes_open(nes_rom,nes_core)){nes_active=1;nes_background=0;nes_volume_was=nes_pause_was=0;nes_held=nes_a=nes_start=0;steering=(struct nes_steering){0};paused=0;menu=0;}else dummy_notice=1;}else if(selected_game==2){ast_reset(&asteroid,0x5833u+tick);asteroid_jog=(struct left_jog){0};asteroid_keys=asteroid_touch=0;menu=0;paused=0;}else if(selected_game==3){if(dj_wars_reset()){menu=0;paused=0;}else dummy_notice=1;}else dummy_notice=1;}
static void back_to_menu(void){rescue.active=0;dj_wars_release();if(nes_active){nes_close();nes_active=0;}menu=1;paused=0;direction=0;invaders_keys=0;invaders_pointer=-1;equalizarr_keys=0;equalizarr_pointer=-1;basslines_keys=basslines_touch_keys=0;if(selected_game==ROAD_FIGHTER_GAME)selected_game=0;}
static int inside(int x,int y,int left,int top,int w,int h){return x>=left&&x<left+w&&y>=top&&y<top+h;}
static void basslines_touch(int x,int y,int down,int first){
 basslines_touch_keys=0;if(!down)return;
 if(first&&inside(x,y,26,721,289,47)){back_to_menu();return;}
 if(first&&inside(x,y,26,480,132,40)){paused=!paused;return;}
 if(first&&inside(x,y,177,480,138,40)){reset_current_game();paused=0;return;}
 if(inside(x,y,126,560,84,38))basslines_touch_keys=BL_UP;
 else if(inside(x,y,26,608,84,38))basslines_touch_keys=BL_LEFT;
 else if(inside(x,y,226,608,88,38))basslines_touch_keys=BL_RIGHT;
 else if(inside(x,y,126,656,84,38))basslines_touch_keys=BL_DOWN;
 else if(inside(x,y,126,608,84,38))basslines_touch_keys=BL_BOOST;
}
static void invaders_touch(int x,int y,int down,int first){
 if(!down){invaders_keys&=~SI_FIRE;invaders_pointer=-1;return;}
 if(first&&inside(x,y,26,711,289,57)){back_to_menu();return;}
 if(first&&inside(x,y,26,643,132,40)){paused=!paused;return;}
 if(first&&inside(x,y,177,643,138,40)){reset_current_game();return;}
 if(x>=382&&x<1222&&y>=80&&y<760){invaders_pointer=x-382;if(first)invaders_keys|=SI_FIRE;}
}
static void equalizarr_touch(int x,int y,int down,int first){
 if(!down){equalizarr_keys&=~EQ_FIRE;equalizarr_pointer=-1;return;}
 if(first&&inside(x,y,26,711,289,57)){back_to_menu();return;}
 if(first&&inside(x,y,26,643,132,40)){paused=!paused;return;}
 if(first&&inside(x,y,177,643,138,40)){reset_current_game();return;}
 if(x>=382&&x<1222&&y>=80&&y<760){equalizarr_pointer=x-382;if(first)equalizarr_keys|=EQ_FIRE;}
}
static void dj_wars_touch(int x,int y){
 if(!dj_wars)return;
 if(dj_wars->state==DW_OVER||dj_wars->state==DW_FINALE)return;
 if(inside(x,y,177,742,138,33)){back_to_menu();return;}
 if(inside(x,y,25,742,132,33)){paused=!paused;return;}
 if(x>=24&&x<316&&y>=336&&y<560){dw_choose_weapon(dj_wars,(y-336)/32);return;}
 if(inside(x,y,25,695,290,17)){dj_wars_pulse|=DW_JUMP;return;}
 if(inside(x,y,25,713,290,20)){dw_camera(0,0,0);return;}
 if(x>=1003&&x<1233&&y>=79&&y<225){dw_camera(3,(x-1008)*12,(y-86)*12);return;}
 if(x>=360&&x<1240&&y>=70&&y<770)dw_mark(dj_wars,dw_camera_x+(x-360)*dw_divisor(),dw_camera_y+(y-70-dw_yoffset())*dw_divisor());
}
static void menu_touch(int x,int y){
 if(help_page){
  if(help_page==11){if(inside(x,y,1080,165,120,44)){help_page=0;__atomic_store_n(&cafe_show_requested,0,__ATOMIC_RELEASE);}else if(cafe_vinyl_hit(x,y))cafe_show_toggle();return;}
  if(help_page==10){if(inside(x,y,width-210,160,100,46))help_page=0;return;}
  if(help_page==7){if(inside(x,y,width-210,160,100,46))help_page=0,help_offset=0;else if(inside(x,y,110,height-118,180,42))help_scroll(-1);else if(inside(x,y,width-340,height-118,230,42))story_continue();return;}
  if(help_page==8||help_page==9){if(inside(x,y,420,582,440,56))help_page=0;return;}
  if(help_page==6){if(inside(x,y,240,500,340,64))help_page=credits_confirm_return;else if(inside(x,y,700,500,340,64)&&++credits_confirm_step==3)help_page=5;return;}
  if(help_page==5){if(inside(x,y,1090,168,140,46)){help_page=0;credits_end();}else if(credits_completed&&inside(x,y,442,615,396,48)){help_page=3;credits_end();for(unsigned i=0;i<music_count;i++)if(music_tracks[i].locked==2){music_choose((int)i);break;}}return;}
  if(help_page==3){if(inside(x,y,width-210,180,100,46)){help_page=0;music_cover_release();}else music_touch(x,y);return;}
  if(help_page==4){if(debug_unlocked&&y>=720&&y<778){for(unsigned cat=0;cat<4;cat++)if(inside(x,y,110+cat*260,720,245,58)){rescue_begin(&rescue,cat);debug_rescue_test=1;menu=0;paused=0;return;}}if(inside(x,y,width-210,160,100,46))help_page=0;return;}
  if(inside(x,y,width-210,160,100,46))help_page=0;
  else if(help_page!=3&&inside(x,y,110,240,230,46))help_page=1,help_offset=0;
  else if(help_page!=3&&inside(x,y,360,240,270,46))help_page=2,help_offset=0;
  else if(help_page!=3&&inside(x,y,650,240,230,46))credits_request();
  else if(inside(x,y,110,height-118,180,42))help_scroll(-1);
  else if(inside(x,y,width-290,height-118,180,42))help_scroll(1);
  return;
 }
 if(inside(x,y,width-164,height-110,124,64)){version_tap(ns());return;}
 debug_taps=0;
 if(inside(x,y,width-236,height-110,56,44))return;
 if(inside(x,y,width-640,35,220,44)){theme=0;return;}
 if(inside(x,y,width-400,35,100,44)){theme=1;return;}
 if(inside(x,y,width-280,35,56,44)){help_page=1;help_offset=0;return;}
 if(inside(x,y,width-208,35,56,44)){help_page=2;help_offset=0;return;}
 if(inside(x,y,width-136,35,56,44)){help_page=3;return;}
 for(int i=0;i<MENU_GAME_COUNT;i++)if(inside(x,y,60,150+i*58,width-120,52)){selected_game=menu_order[i];dummy_notice=0;choose_game();return;}
 if(inside(x,y,60,height-110,360,64))help_page=10;
 if(inside(x,y,450,height-110,360,64))open_cafe();
}
/* Small pixel icons: chat, audio waveform, video play and globe. */
static void social_icon(int x,int y,int kind){
 uint32_t c=0x64ffe1,b=0x14283d;
 if(kind==0){rect(x,y,22,17,c);rect(x+3,y+3,16,11,b);rect(x+4,y+17,5,5,c);rect(x+8,y+6,3,6,c);rect(x+14,y+6,3,6,c);}
 else if(kind==1){for(int i=0;i<6;i++){int h=i==2?22:i==3?18:i==1?14:i==4?12:6;rect(x+i*4,y+(22-h)/2,2,h,c);}}
 else if(kind==2){rect(x,y+2,24,18,c);for(int i=0;i<8;i++)rect(x+8+i,y+6+i/2,1,10-i,b);}
 else{disk(x+11,y+11,11,c);disk(x+11,y+11,8,b);rect(x,y+10,22,2,c);rect(x+10,y+1,2,20,c);rect(x+3,y+5,16,2,c);rect(x+3,y+15,16,2,c);}
}
#include "game_icons.h"
static void draw_menu(void){
 for(int y=0;y<height;y++)for(int x=0;x<width;x++)pixels[y*width+x]=theme_color(0x080f1b);
 int scale=width>=1200?5:4;
 logo(40,15);text(170,30,scale,"DJ DRAGON-X",0x64ffe1);text(170,85,3,"GAMES COLLECTION",0xdce6ee);
 text(width-260,95,3,"GAMES MENU",0x8fa1b5);
 rect(width-640,35,220,44,theme==0?0x64ffe1:0x14283d);rect(width-638,37,216,40,0x14283d);
 text(width-622,49,2,"RED - BLACK",theme==0?0x64ffe1:0x8fa1b5);
 rect(width-400,35,100,44,theme==1?0x64ffe1:0x14283d);rect(width-398,37,96,40,0x14283d);
 text(width-382,49,2,"NEON",theme==1?0x64ffe1:0x8fa1b5);
 rect(width-280,35,56,44,0x14283d);text(width-261,46,3,"!",0x64ffe1);
 rect(width-208,35,56,44,0x14283d);text(width-189,46,3,"?",0x64ffe1);
 rect(width-136,35,56,44,0x14283d);
 int mx=width-120,my=45;rect(mx+7,my,3,22,0x64ffe1);rect(mx+24,my-3,3,22,0x64ffe1);rect(mx+7,my-3,20,4,0x64ffe1);disk(mx+3,my+22,5,0x64ffe1);disk(mx+20,my+19,5,0x64ffe1);

 const char*descriptions[]={"COLLECT DOTS AND DODGE GHOSTS THROUGH THE NEON MAZE","SAVE YUKI WHILE SMASHING BRICKS AND CATCHING POWER-UPS","SAVE PEACH WHILE BLASTING ROCKS AND ENEMY SHIPS","BRING YOUR CREW TOGETHER AND WIN THE FINAL SHOWDOWN","SAVE COOKIE WHILE OUTRIDING RIVALS AND DODGING TRAILS","SAVE MOCHI WHILE HOLDING BACK ENDLESS ALIEN WAVES"};
 for(int row=0;row<MENU_GAME_COUNT;row++){
  int i=menu_order[row],y=150+row*58;rect(60,y,width-120,52,i==3?0x10243c:0x14283d);
  if(i==selected_game||i==3){unsigned c=i==3?0xffcd55:0x64ffe1;rect(60,y,width-120,2,c);rect(60,y+50,width-120,2,c);rect(60,y,i==selected_game?6:3,52,c);}
  game_icon(i,92,y+9);
  text(155,y+6,3,i==3?"DJ WARS - FINAL BATTLE":game_names[i],i==3?0xffcd55:0xffffff);text(155,y+32,2,descriptions[i],0x8fa1b5);
  if(i==3){char count[16];snprintf(count,sizeof(count),"%u / 4 CATS",session_cat_count());text(width-195,y+8,2,session_finale_ready()?"PLAY":"LOCKED",0xffcd55);text(width-195,y+31,1,count,0xffffff);}
  else text(width-190,y+21,2,"PLAY",0x64ffe1);
 }
 text(60,510,2,dummy_notice?(selected_game==ROAD_FIGHTER_GAME?"ROAD FIGHTER ROM UNAVAILABLE":"GAME MEMORY OR DISPLAY UNAVAILABLE"):"BROWSE TO SELECT - PRESS TO PLAY",0x8fa1b5);
 text(width-210,510,2,"SOCIALS",0x64ffe1);
 const char*labels[]={"TWITCH","MIXCLOUD","YOUTUBE","WEBSITE"};
 const char*names[]={"FIR3DRAGONX","DRAGON-X","FI3EDRAGONX","DJDRAGONX.COM"};
 int col=(width-120)/4;
 for(int i=0;i<4;i++){int x=60+i*col;rect(x,height-265,col-16,125,0x14283d);social_icon(x+14,height-252,i);text(x+48,height-248,2,labels[i],0x64ffe1);text(x+14,height-216,2,names[i],0xffffff);text(x+14,height-183,2,i==0?"DJ_DRAGONX":"FOLLOW DJ DRAGON-X",i==0?0xffffff:0x8fa1b5);}
 rect(60,height-110,360,64,0x763044);text(85,height-90,3,"BACK TO DJ",0xffffff);text(width-156,height-85,2,"V0.4.7",0x8fa1b5);
 {rect(450,height-110,360,64,0x14283d);rect(450,height-110,360,3,0x64ffe1);text(495,height-90,3,"PURR HOUSE CAFE",0x64ffe1);rect(750,height-96,3,3,0x64ffe1);rect(753,height-99,3,3,0x64ffe1);}
 /* Settings placeholder: deliberately no touch action yet. */
 rect(width-236,height-110,56,44,0x14283d);int gx=width-208,gy=height-88;rect(gx-10,gy-7,20,14,0x64ffe1);rect(gx-7,gy-10,14,20,0x64ffe1);rect(gx-13,gy-2,26,4,0x64ffe1);rect(gx-2,gy-13,4,26,0x64ffe1);disk(gx,gy,5,0x14283d);

}
static void debug_graph_segment(int x,int y,int xx,int yy,uint32_t color){
 int dx=xx-x,dy=yy-y,steps=dx<0?-dx:dx;if((dy<0?-dy:dy)>steps)steps=dy<0?-dy:dy;
 for(int i=0;i<=steps;i++)rect(x+(steps?dx*i/steps:0),y+(steps?dy*i/steps:0),2,2,color);
}
static void draw_debug_graphs(void){
 const uint32_t accents[]={0x64ffe1,0xffcd55,0xee8dd0,0x64e8ee};
 int w=280;
 for(int c=0;c<DEBUG_CORES;c++){
  int x=110+(c%2)*300,y=570+(c/2)*74;rect(x,y,w,64,0x14283d);char label[48];
  if(debug_data.core_load[c]<0)snprintf(label,sizeof(label),"CORE %d - UNAVAILABLE",c+1);
  else snprintf(label,sizeof(label),"CORE %d - %d%%",c+1,debug_data.core_load[c]);
  text(x+12,y+8,2,label,accents[c]);int left=x+12,top=y+30,gw=w-24,gh=25;
  for(int level=0;level<=2;level++)rect(left,top+level*gh/2,gw,1,0x25445b);
  int old_x=0,old_y=0,old_valid=0;
  unsigned count=debug_data.history_count;
  for(unsigned i=0;i<count;i++){
   unsigned index=(debug_data.history_head+DEBUG_HISTORY-count+i)%DEBUG_HISTORY;
   int value=debug_data.history[c][index];int px=left+(DEBUG_HISTORY-count+i)*(gw-2)/(DEBUG_HISTORY-1);
   if(value>100){old_valid=0;continue;}int py=top+gh-value*gh/100;
   if(old_valid)debug_graph_segment(old_x,old_y,px,py,accents[c]);else rect(px,py,2,2,accents[c]);
   old_x=px;old_y=py;old_valid=1;
  }
 }
}
static void draw_help(void){
 if(help_page==11){cafe.show_active=cafe_show_audio_active();cafe.show_ms=cafe_show_audio_ms();cat_cafe_scene(&cafe,(tick-cafe_started)*1000/60);cafe_seen=cafe.seen;return;}
 if(help_page==10){
  rect(80,130,width-160,height-190,0x10243c);rect(80,130,width-160,3,0x64ffe1);
  text(110,170,4,"BACK TO DJ",0x64ffe1);
  rect(width-210,160,100,46,0x763044);text(width-196,174,2,"CLOSE",0xffffff);
  text(130,292,5,"JUST UNPLUG THE USB STICK.",0x64ffe1);
  text(130,370,3,"THE ORIGINAL RX3 INTERFACE WILL RETURN.",0xffffff);
  text(130,466,3,"THE POWER BUTTON IS SOFTWARE-CONTROLLED.",0x8fa1b5);
  text(130,511,3,"UNPLUG FIRST TO RESTORE NORMAL",0x8fa1b5);
  text(130,551,3,"POWER CONTROL.",0x8fa1b5);
  text(130,645,3,"THANKS FOR PLAYING!",0xffcd55);return;
 }
 if(help_page==7){
  rect(80,130,width-160,height-190,0x10243c);rect(80,130,width-160,3,0x64ffe1);
  text(110,170,4,"CAT RESCUE SAGA",0x64ffe1);
  rect(width-210,160,100,46,0x763044);text(width-194,174,2,"SKIP",0xffffff);
  for(int i=0;i<10&&help_offset+i<help_count();i++)text(110,270+i*32,2,help_story[help_offset+i],0xffffff);
  rect(110,height-118,180,42,0x14283d);text(135,height-105,2,"PREVIOUS",0x64ffe1);
  rect(width-340,height-118,230,42,0x14283d);text(width-320,height-105,2,help_offset+10<help_count()?"NEXT":"CONTINUE",0x64ffe1);
  draw_story_artwork();char label[48];snprintf(label,sizeof(label),"PAGE %d OF %d",help_offset/10+1,(help_count()+9)/10);text(450,height-105,2,label,0x8fa1b5);return;
 }

 if(help_page==8){credits_locked_render();return;}if(help_page==9){rescue_locked_render(session_rescued_cats);return;}
 if(help_page==6){credits_confirm_render(credits_confirm_step);return;}
 if(help_page==5){draw_credits();return;}
 if(help_page==3){rect(0,130,width,height-130,0x10243c);rect(20,150,width-40,3,0x64ffe1);text(110,190,4,"DJ DRAGON-X - MUSIC",0x64ffe1);rect(width-210,180,100,46,0x763044);text(width-196,194,2,"CLOSE",0xffffff);draw_music_player();return;}
 rect(80,130,width-160,height-190,0x10243c);rect(80,130,width-160,3,0x64ffe1);
 text(110,170,4,help_page==4?"DEBUG INFORMATION":help_page==3?"DJ DRAGON-X - MUSIC":"DJ DRAGON-X - HELP",0x64ffe1);
 rect(width-210,160,100,46,0x763044);text(width-196,174,2,"CLOSE",0xffffff);
 if(help_page==4){for(int i=0;i<DEBUG_LINES;i++)text(110,245+i*32,2,debug_data.line[i],0xffffff);draw_debug_graphs();draw_debug_artwork();if(debug_unlocked){text(110,693,2,"DEBUG - TEST RESCUE + FANFARE",0xffcd55);for(int c=0;c<4;c++){rect(110+c*260,720,245,58,0x14283d);text(128+c*260,740,3,rescue_names[c],0x64ffe1);}}return;}
 if(help_page==3){draw_music_player();return;}

 rect(110,240,230,46,0x14283d);text(132,253,3,"ABOUT",help_page==1?0x64ffe1:0x8fa1b5);
 rect(360,240,270,46,0x14283d);text(382,253,3,"HOW TO PLAY",help_page==2?0x64ffe1:0x8fa1b5);
 rect(650,240,230,46,0x14283d);text(672,253,3,"CREDITS",help_page==5?0x64ffe1:0x8fa1b5);
 const char**lines=help_page==5?help_credits:help_page==1?help_about:help_controls;int count=help_count();
 for(int i=0;i<10&&help_offset+i<count;i++)text(110,310+i*32,2,lines[help_offset+i],0xffffff);
 rect(110,height-118,180,42,0x14283d);text(135,height-105,2,"PREVIOUS",help_offset?0x64ffe1:0x8fa1b5);
 rect(width-290,height-118,180,42,0x14283d);text(width-250,height-105,2,"NEXT",help_offset+10<count?0x64ffe1:0x8fa1b5);
 char page[48];snprintf(page,sizeof(page),"PAGE %d OF %d - TURN BROWSE",help_offset/10+1,(count+9)/10);text(340,height-105,2,page,0x8fa1b5);

}
static void gain_display_update(int channel,unsigned raw,int valid){
 if(channel<0||channel>1||!valid||raw>1023)return;
 unsigned percent=(raw*100+511)/1023;
 if(gain_seen[channel]&&percent!=gain_percent[channel]){volume_kind=channel+1;volume_until=tick+120;}
 gain_percent[channel]=percent;gain_seen[channel]=1;
}
static void draw_volume(void){
 if(tick>=volume_until)return;
 int w=width*3/5,h=300,x=(width-w)/2,y=(height-h)/2;
 rect(x,y,w,h,0x10243c);rect(x,y,w,3,0x64ffe1);
 const char*title=volume_kind==1?"GAMES VOLUME":volume_kind==2?"MUSIC VOLUME":"MASTER VOLUME";int title_len=0;while(title[title_len])title_len++;text((width-title_len*6*4)/2,y+30,4,title,0x64ffe1);
 unsigned displayed=volume_kind?gain_percent[volume_kind-1]:volume_percent;
 char label[24];snprintf(label,sizeof(label),"%u%%",displayed);int len=0;while(label[len])len++;
 text((width-len*6*9)/2,y+90,9,label,0xffffff);
 int step=(w-80)/20;
 for(unsigned i=0;i<20;i++)rect(x+40+i*step,y+200,step-6,64,i*5<displayed?0x64ffe1:0x763044);
}

static int wall(int x,int y){return x>=0&&x<28&&y>=3&&y<=33&&arcade_tile(x,y)>=0xd0;}
static void draw(void){if(help_page!=7)story_artwork_release();if(help_page!=4)debug_artwork_release();if(help_page!=3)music_animation_release();if(nes_active&&!menu){
 int repaint=!nes_background||nes_volume_was||nes_pause_was;
 if(!nes_background){rect(0,0,width,height,0x080f1b);nes_background=1;}
 nes_pause_was=paused;
 if(nes_volume_was)rect(width/5,(height-300)/2,width*3/5,300,0x080f1b);
 nes_volume_was=tick<volume_until;
 int scale=nes_height?(height-100)/(int)nes_height:1;
 if(nes_width&&scale>width/(int)nes_width)scale=width/(int)nes_width;
 if(scale>3)scale=3;if(scale<1)scale=1;
 int left=(width-(int)nes_width*scale)/2,top=45;
 for(unsigned y=0;y<nes_height;y++){
  uint32_t *source=nes_video+y*nes_width,*cache=nes_draw_cache+y*nes_width;
  if(!repaint&&!memcmp(source,cache,nes_width*4))continue;
  memcpy(cache,source,nes_width*4);
  mark_dirty(left,top+y*scale,nes_width*scale,scale);
  uint32_t *row=pixels+(top+y*scale)*width+left;
  for(unsigned x=0;x<nes_width;x++){uint32_t c=nes_video[y*nes_width+x];for(int sx=0;sx<scale;sx++)row[x*scale+sx]=c;}
  for(int sy=1;sy<scale;sy++)memcpy(row+sy*width,row,nes_width*scale*4);
 }
 rect(20,20,190,50,0x763044);text(45,35,3,"MENU",0xffffff);rect(20,90,190,50,0x763044);text(35,105,3,"START",0xffffff);
 text(30,height-45,2,"FX A - CH1 CUE SELECT - CH2 CUE START",0x64ffe1);if(paused)text(width/2-80,height/2,4,"PAUSE",0xffffff);return;}
 if(rescue.active){rescue_scene(rescue.cat,rescue.ticks*1000/60);return;}
 if(menu){draw_menu();if(help_page)draw_help();return;}if(selected_game==3&&dj_wars){dw_render(dj_wars,pixels,paused,theme);if(dj_wars->state<DW_OVER){dw_rect(24,695,292,38,0x080b14);dw_text(26,701,1,"COLOR FX MOVE / AIM - TAP TO JUMP",0x919ab3);dw_text(26,717,1,"BEAT FX FIRE - TOUCH WEAPON / MAP",0x919ab3);}return;}if(selected_game==5){si_render(&invaders,paused);return;}if(selected_game==4){bl_render(&basslines,paused);return;}if(selected_game==1){eq_render(&equalizarr,paused);return;}if(selected_game==2){ast_render(&asteroid,paused);return;}for(int y=0;y<height;y++)for(int x=0;x<width;x++)pixels[y*width+x]=theme_color(0x080f1b);
for(int y=3;y<=33;y++)for(int x=0;x<28;x++){unsigned t=arcade_tile(x,y);int px=bx+x*cell,py=by+y*cell,cx=px+cell/2,cy=py+cell/2;if(wall(x,y)){rect(px,py,cell,cell,0x10243c);if(!wall(x-1,y))rect(px,py,2,cell,0x54bded);if(!wall(x+1,y))rect(px+cell-2,py,2,cell,0x54bded);if(!wall(x,y-1))rect(px,py,cell,2,0x54bded);if(!wall(x,y+1))rect(px,py+cell-2,cell,2,0x54bded);}else if(t==16)disk(cx,cy,cell/8+1,0xe8d9af);else if(t==20){disk(cx,cy,cell/3+1,0x174841);disk(cx,cy,cell/4+((tick/15)%2),0xbaffed);}else if(t==207)rect(px,cy,cell,2,0xee8dd0);}
for(unsigned i=0;i<5;i++){if(!arcade_actor(i,4))continue;int x=bx+arcade_actor(i,0)*cell/8,y=by+arcade_actor(i,1)*cell/8,r=cell*7/10,dir=arcade_actor(i,2),s=arcade_actor(i,3);uint32_t c=s==3?0x456de5:colors[i];if(i==0){disk(x,y,r+2,0x5a431d);disk(x,y,r,colors[0]);int mouth=(tick/5)%2;for(int yy=-r;yy<=r;yy++)for(int xx=-r;xx<=r;xx++){int f=dir==0?xx:dir==1?yy:dir==2?-xx:-yy,side=dir%2?xx:yy;if(side<0)side=-side;if(f>0&&side*(mouth?2:5)<f)rect(x+xx,y+yy,1,1,0x080f1b);}}else{if(s!=4){disk(x,y-r/4,r,c);rect(x-r,y-r/4,2*r,r+r/4,c);for(int z=-r;z<r;z+=r/2+1)disk(x+z,y+r/2,r/3,c);}for(int ex=-1;ex<=1;ex+=2){disk(x+ex*r/3,y-r/4,r/3,0xf6f7ff);disk(x+ex*r/3+(dir==0?2:dir==2?-2:0),y-r/4+(dir==1?2:dir==3?-2:0),r/6+1,0x192b50);}}}
int scale=width/400;if(scale<2)scale=2;int left=30;
text(left,35,scale*2,"RX3",0x64ffe1);text(left,35+scale*19,scale,"ARCADE",0xdce6ee);text(left,135,scale,"NEON MAZE",0x64ffe1);char n[24];text(left,205,scale,"SCORE",0x7796aa);snprintf(n,sizeof(n),"%06u",arcade_value(0));text(left,230,scale*2,n,0xf7faff);text(left,283,2,"HIGH",0x7796aa);snprintf(n,sizeof(n),"%06u",arcade_value(6));text(left+65,283,2,n,0xffcd55);text(left,310,scale,"LEVEL",0x7796aa);snprintf(n,sizeof(n),"%02u",arcade_value(2));text(left+scale*42,310,scale,n,0xffcd55);text(left,345,scale,"LIVES",0x7796aa);unsigned lives=arcade_value(1)+1;if(lives>3)lives=3;for(unsigned j=0;j<lives;j++)disk(left+scale*45+j*25,352,8,0xffcd55);
rect(30,height-260,270,56,0x763044);text(60,height-244,4,"MENU",0xffffff);
text(left,height-185,scale,paused?"PAUSED":"CLASSIC SOUND",0x64ffe1);rect(30,height-160,80,26,0x173044);rect(120,height-160,80,26,0x173044);rect(210,height-160,80,26,0x173044);text(40,height-152,2,"PAUSE",0x8fa1b5);text(130,height-152,2,"RESET",0x8fa1b5);text(225,height-152,2,"EXIT",0x8fa1b5);
/* Visible touch pads: up,left,down,right + pause/reset/exit row. */
const int pad[4][2]={{100,height-105},{35,height-62},{100,height-62},{165,height-62}};const char*lab[]={"U","L","D","R"};for(int i=0;i<4;i++){rect(pad[i][0],pad[i][1],56,36,0x173044);text(pad[i][0]+20,pad[i][1]+10,2,lab[i],0x64ffe1);}text(bx,by+10,2,"NEON MAZE",0x8fa1b5);if(paused)text(bx+cell*8,by+cell*17,3,"PAUSE",0x64ffe1);
}
static int ppm(const char*path){int fd=open(path,1|64|512,0644);if(fd<0)return -1;char head[64];int n=snprintf(head,sizeof(head),"P6\n%d %d\n255\n",width,height);write(fd,head,n);unsigned char*row=malloc(width*3);if(!row){close(fd);return -1;}for(int y=0;y<height;y++){for(int x=0;x<width;x++){unsigned p=pixels[y*width+x];row[x*3]=p>>16;row[x*3+1]=p>>8;row[x*3+2]=p;}write(fd,row,width*3);}free(row);close(fd);return 0;}
static uint32_t packed(uint32_t c,struct fb_var*v){return (((c>>16)&255)>>(8-v->red.length))<<v->red.offset|(((c>>8)&255)>>(8-v->green.length))<<v->green.offset|((c&255)>>(8-v->blue.length))<<v->blue.offset;}
/* Compare cached RAM, then write only changed spans to device memory. */
static uint32_t *previous;static unsigned char *packed_row;static int first_blit=1;
static unsigned long long fb_bytes;
static void blit(unsigned char*fb,struct fb_var*v,struct fb_fix*f){
 unsigned bytes=v->bpp/8;
 int fast565=bytes==2&&v->red.offset==11&&v->green.offset==5&&v->blue.offset==0&&v->red.length==5&&v->green.length==6&&v->blue.length==5;
 /* The existing scalar blit is faster for this viewport in measured tests. */
 uint32_t alpha=v->transp.length?((1u<<v->transp.length)-1)<<v->transp.offset:0;
 for(int y=0;y<height;y++){
  uint32_t *src=pixels+y*width,*old=previous+y*width;
  int begin_row=0,end_row=width;
  if(!first_blit&&nes_active&&!menu&&height<=DIRTY_ROWS){
   begin_row=dirty_left[y];end_row=dirty_right[y];
   if(!end_row)continue;
  }
  if(y<DIRTY_ROWS)dirty_right[y]=0;
  if(!first_blit&&!memcmp(src+begin_row,old+begin_row,(end_row-begin_row)*4))continue;
  unsigned char *dst=fb+(y+v->yoffset)*f->line_length+v->xoffset*bytes;
  int x=begin_row;
  while(x<end_row){
   while(x<end_row&&!first_blit&&src[x]==old[x])x++;
   int begin=x;
   while(x<end_row&&(first_blit||src[x]!=old[x])){
    uint32_t c=fast565?((src[x]>>8)&0xf800)|((src[x]>>5)&0x7e0)|((src[x]>>3)&31):packed(src[x],v)|alpha;
    if(bytes==4)((uint32_t*)packed_row)[x]=c;
    else if(bytes==2)((uint16_t*)packed_row)[x]=c;
    else{packed_row[3*x]=c;packed_row[3*x+1]=c>>8;packed_row[3*x+2]=c>>16;}
    old[x]=src[x];x++;
   }
   if(x>begin){size_t n=(size_t)(x-begin)*bytes;memcpy(dst+begin*bytes,packed_row+begin*bytes,n);fb_bytes+=n;}
  }
 }
 first_blit=0;
}
static uint64_t ns(void){rx_timespec t;clock_gettime(1,&t);return (uint64_t)t.sec*1000000000u+t.nsec;}
struct indev{int fd,x,y,sx,sy,down,ax,ay;struct absinfo xr,yr;};
static int mapaxis(int val,struct absinfo*a,int extent){if(a->maximum<=a->minimum)return val;long p=(long)(val-a->minimum)*(extent-1)/(a->maximum-a->minimum);return p<0?0:p>=extent?extent-1:(int)p;}
static void input(struct indev*d){struct input_event ev;while(read(d->fd,&ev,sizeof(ev))==sizeof(ev)){
if(ev.type==1){if(rescue.active)continue;if(!menu&&selected_game==5){
 if(ev.code==330){invaders_touch(d->x,d->y,ev.value,ev.value&&!d->down);d->down=ev.value;continue;}
 unsigned mask=ev.code==105||ev.code==30?SI_LEFT:ev.code==106||ev.code==32?SI_RIGHT:ev.code==57||ev.code==28?SI_FIRE:0;
 if(mask){if(ev.value)invaders_keys|=mask;else invaders_keys&=~mask;continue;}
 }if(!menu&&selected_game==4){
 if(ev.code==330){basslines_touch(d->x,d->y,ev.value,ev.value&&!d->down);d->down=ev.value;continue;}
 unsigned mask=ev.code==103||ev.code==17?BL_UP:ev.code==108||ev.code==31?BL_DOWN:ev.code==105||ev.code==30?BL_LEFT:ev.code==106||ev.code==32?BL_RIGHT:ev.code==57||ev.code==28?BL_BOOST:0;
 if(mask){if(ev.value)basslines_keys|=mask;else basslines_keys&=~mask;continue;}
 }if(!menu&&selected_game==1){
 if(ev.code==330){equalizarr_touch(d->x,d->y,ev.value,ev.value&&!d->down);d->down=ev.value;continue;}
 unsigned mask=ev.code==105||ev.code==30?EQ_LEFT:ev.code==106||ev.code==32?EQ_RIGHT:ev.code==57||ev.code==28?EQ_FIRE:0;
 if(mask){if(ev.value)equalizarr_keys|=mask;else equalizarr_keys&=~mask;continue;}
 }if(!menu&&selected_game==3){if(ev.code==330){if(ev.value&&!d->down)dj_wars_touch(d->x,d->y);d->down=ev.value;continue;}unsigned mask=ev.code==103?DW_UP:ev.code==108?DW_DOWN:ev.code==105?DW_LEFT:ev.code==106?DW_RIGHT:ev.code==57?DW_FIRE:ev.code==28?DW_JUMP:0;if(mask){if(ev.value)dj_wars_keys|=mask;else dj_wars_keys&=~mask;continue;}}if(!menu&&selected_game==2&&ev.code!=330){unsigned mask=ev.code==103||ev.code==17?AST_UP:ev.code==108||ev.code==31?AST_HYPER:ev.code==105||ev.code==30?AST_LEFT:ev.code==106||ev.code==32?AST_RIGHT:ev.code==57?AST_FIRE:0;if(mask){if(ev.value)asteroid_keys|=mask;else asteroid_keys&=~mask;return;}}if(ev.code==330){if(!menu&&selected_game==2){if(!ev.value)asteroid_touch=0;else if(!d->down){if(inside(d->x,d->y,30,540,270,56))back_to_menu();else if(inside(d->x,d->y,30,640,125,36))paused=!paused;else if(inside(d->x,d->y,175,640,125,36))reset_current_game();else if(inside(d->x,d->y,30,450,270,38))asteroid_touch=AST_FIRE;else if(inside(d->x,d->y,30,488,270,40))asteroid_touch=AST_HYPER;}d->down=ev.value;return;}if(ev.value&&menu)menu_touch(d->x,d->y);if(ev.value&&!menu&&d->x>=30&&d->x<300&&d->y>=height-260&&d->y<height-204)back_to_menu();if(!ev.value&&d->down&&d->x<290&&d->y>=height-160&&d->y<height-134){if(d->x<110)paused=!paused;else if(d->x<200){reset_current_game();}else running=0;}d->down=ev.value;if(d->down){d->sx=d->x;d->sy=d->y;}}else if(ev.value==1){switch(ev.code){case 103:case 17:direction=1;break;case 108:case 31:direction=2;break;case 105:case 30:direction=4;break;case 106:case 32:direction=8;break;case 1:if(menu)running=0;else back_to_menu();break;case 28:if(menu){if(help_page==7)story_continue();else if(!help_page)choose_game();}break;case 25:case 57:paused=!paused;break;case 19:reset_current_game();break;default:break;}}}
else if(ev.type==3){if(ev.code==d->ax)d->x=mapaxis(ev.value,&d->xr,width);if(ev.code==d->ay)d->y=mapaxis(ev.value,&d->yr,height);if(ev.code==57){d->down=ev.value>=0;if(d->down){d->sx=d->x;d->sy=d->y;}}}
else if(ev.type==0&&d->down){if(!menu&&selected_game==5){invaders_touch(d->x,d->y,1,0);continue;}if(!menu&&selected_game==4){basslines_touch(d->x,d->y,1,0);continue;}if(!menu&&selected_game==1){equalizarr_touch(d->x,d->y,1,0);continue;}int dx=d->x-d->sx,dy=d->y-d->sy;if(d->x<225&&d->y>=height-110){if(d->y<height-70&&d->x>=100&&d->x<156)direction=1;else if(d->y>=height-70){if(d->x<92)direction=4;else if(d->x<160)direction=2;else direction=8;}}else if(dx*dx+dy*dy>400){direction=(dx*dx>dy*dy)?(dx>0?8:4):(dy>0?2:1);d->sx=d->x;d->sy=d->y;}}
}}
/* Pioneer tsc2007 character device: 6 bytes, status/u16 x/u16 y.
 * ABI/mode/calibration formula recovered from the user's rbp binary. */
static int touchfd=-1,touch_down,old_mode=-1;static unsigned short old_resistance;
static int tx_min=37,ty_min=72,tx_factor=994,ty_factor=964;
static unsigned touch_events;
static void load_touch_calibration(void){
 const char*paths[]={"/root/settings/TouchCalib_User.dat","/root/settings/TouchCalib_Factory.dat"};
 for(unsigned f=0;f<2;f++){
  int fd=open(paths[f],0);if(fd<0)continue;char buf[128];int n=(int)read(fd,buf,127);close(fd);if(n<=0)continue;buf[n]=0;
  int a[6],k=0;char*p=buf;while(*p&&k<6){while(*p==' '||*p=='\n'||*p=='\r'||*p=='\t')p++;if(!*p)break;a[k++]=atoi(p);while(*p&&*p!=' '&&*p!='\n'&&*p!='\r'&&*p!='\t')p++;}
  if(k==6&&a[0]>=0&&a[1]>=0&&a[2]>0&&a[2]<=1024&&a[3]>0&&a[3]<=1024&&a[4]==1280&&a[5]==800){tx_min=a[0];ty_min=a[1];tx_factor=a[2];ty_factor=a[3];printf("Touch calibration: %s (%d %d %d %d)\n",paths[f],tx_min,ty_min,tx_factor,ty_factor);return;}
 }
 printf("Touch calibration: firmware defaults\n");
}
static int clamp(int n,int size){return n<0?0:n>=size?size-1:n;}
static void raw_touch_sample(const unsigned char*b){
 if(rescue.active){touch_down=b[0]!=0;return;}
 int down=b[0]!=0;if(!down)asteroid_touch=0;int rx=b[2]|b[3]<<8,ry=b[4]|b[5]<<8;
 if(rx<=tx_min||ry<=ty_min)down=0;
 int x=clamp(width-(rx-tx_min)*width/(tx_factor*4),width);
 int y=clamp((ry-ty_min)*height/(ty_factor*4),height);
 if(!menu&&selected_game==5){invaders_touch(x,y,down,down&&!touch_down);touch_down=down;return;}
 if(!menu&&selected_game==4){basslines_touch(x,y,down,down&&!touch_down);touch_down=down;return;}
 if(!menu&&selected_game==1){equalizarr_touch(x,y,down,down&&!touch_down);touch_down=down;return;}
 if(down){
  if(touch_events++<12){printf("Touch raw=%d,%d screen=%d,%d\n",rx,ry,x,y);fflush(0);}
  if(!touch_down){
   if(menu){menu_touch(x,y);touch_down=down;return;}
   if(selected_game==3){dj_wars_touch(x,y);touch_down=down;return;}
   if(selected_game==2){if(!touch_down){if(inside(x,y,30,540,270,56))back_to_menu();else if(inside(x,y,30,640,125,36))paused=!paused;else if(inside(x,y,175,640,125,36))reset_current_game();}if(inside(x,y,30,450,270,38))asteroid_touch=AST_FIRE;else if(inside(x,y,30,488,270,40))asteroid_touch=AST_HYPER;touch_down=down;return;}
   if(nes_active){if(inside(x,y,20,20,190,50))back_to_menu();else if(inside(x,y,20,90,190,50))nes_start=1;touch_down=down;return;}
   if(x>=30&&x<300&&y>=height-260&&y<height-204)back_to_menu();
   else if(x>=30&&x<290&&y>=height-160&&y<height-134){if(x<110)paused=!paused;else if(x<200){reset_current_game();}else back_to_menu();}
  }
  if(menu&&help_page==3&&touch_down&&((y>=430&&y<482)||(y>=651&&y<710)))music_touch(x,y);
  if(!menu&&x<225&&y>=height-110){if(y<height-70&&x>=100&&x<156)direction=1;else if(y>=height-70){if(x<92)direction=4;else if(x<160)direction=2;else direction=8;}}
 }
 touch_down=down;
}
static void raw_touch_input(void){
 if(touchfd<0)return;
 for(int i=0;i<32;i++){
  struct rx_pollfd p={touchfd,1,0};if(poll(&p,1,0)<=0||!(p.revents&1))break;
  unsigned char b[6];if(read(touchfd,b,6)!=6)break;raw_touch_sample(b);
 }
}
static void raw_touch_open(void){
 touchfd=open("/dev/tsc2007_2-0048",2|04000);
 if(touchfd<0){printf("Native touch unavailable, errno=%d\n",*__errno_location());return;}
 load_touch_calibration();
 int mode=3;unsigned short resistance=3900;
 if(ioctl(touchfd,0x80046b00,&old_mode)<0||ioctl(touchfd,0x80026b01,&old_resistance)<0||ioctl(touchfd,0x40046b00,&mode)<0||ioctl(touchfd,0x40026b01,&resistance)<0){printf("Touch configuration failed, errno=%d\n",*__errno_location());if(old_mode>=0)ioctl(touchfd,0x40046b00,&old_mode);close(touchfd);touchfd=-1;return;}
 printf("Native RX3 touch active: /dev/tsc2007_2-0048\n");
}
static void raw_touch_close(void){if(touchfd>=0){ioctl(touchfd,0x40046b00,&old_mode);ioctl(touchfd,0x40026b01,&old_resistance);close(touchfd);}}
static float master_gain=0,master_target=0;
/* Optional user-provided sound, kept outside source and publication. */
static int16_t *menu_sound;static int menu_music_fd=-1;
extern long lseek(int,long,int);static unsigned menu_sound_frames,menu_sound_pos,menu_music_frames,menu_music_pos;
/* User playlist is mono 22.05 kHz; duplicate samples for the measured
 * RX3 44.1 kHz output clock (the driver reports nominal 48 kHz). */
extern int snd_pcm_drop(void*),snd_pcm_prepare(void*),snd_pcm_delay(void*,long*),snd_pcm_drain(void*);
static void menu_music_load(void){
 menu_music_fd=open("/tmp/rx3-launcher/payload/menu-music.pcm",0);if(menu_music_fd<0)return;
 long bytes=lseek(menu_music_fd,0,2);if(bytes<=0||bytes>(long)MUSIC_MAX_BYTES||bytes%2||lseek(menu_music_fd,0,0)<0){close(menu_music_fd);menu_music_fd=-1;return;}
 menu_music_frames=(unsigned)bytes;menu_music_pos=0;__atomic_store_n(&music_position,0,__ATOMIC_RELAXED);if(music_count&&music_tracks[music_count-1].end>menu_music_frames)music_count=0;
}
static void menu_sound_load(void){
 int fd=open("/tmp/rx3-launcher/payload/menu-open.pcm",0);if(fd<0)return;
 menu_sound=malloc(384000);unsigned used=0;while(menu_sound&&used<384000){long n=read(fd,(unsigned char*)menu_sound+used,384000-used);if(n<=0)break;used+=(unsigned)n;}close(fd);
 menu_sound_frames=used/4;menu_sound_pos=menu_sound_frames;
}
/* Sound owns PCM only while the menu is open; join before game audio/exit. */
extern int pthread_create(unsigned long*,const void*,void*(*)(void*),void*),pthread_join(unsigned long,void**);
static unsigned long menu_sound_thread;static int menu_sound_active,menu_sound_pending;
static unsigned menu_sound_stop,menu_sound_done,menu_sound_gain,menu_sound_kick;
static unsigned menu_music_channel=100,menu_fx_channel=100;
#include "game_music.h"
static void menu_sound_trigger(void){menu_sound_pending=1;}
static void *menu_sound_worker(void*pcm){
 unsigned recoveries=0,last_pause=0,credits_epoch=0,credits_written=0,credits_limit=0,credits_drained=0;
 while(!__atomic_load_n(&menu_sound_stop,__ATOMIC_RELAXED)){
  unsigned show=__atomic_load_n(&cafe_show_requested,__ATOMIC_ACQUIRE);
  if(show&&!cafe_show_audio_active()){
   if(cafe_show_audio_begin(cafe_show_path)){snd_pcm_drop(pcm);snd_pcm_prepare(pcm);}
   else {__atomic_store_n(&cafe_show_requested,0,__ATOMIC_RELEASE);printf("Cafe lightshow audio unavailable\n");}
  }else if(!show&&cafe_show_audio_active()){
   cafe_show_audio_stop();snd_pcm_drop(pcm);snd_pcm_prepare(pcm);
  }
  if(cafe_show_audio_active()){
   int16_t samples[1600];float gain=__atomic_load_n(&menu_sound_gain,__ATOMIC_RELAXED)/1000.0f;
   gain*=__atomic_load_n(&menu_music_channel,__ATOMIC_RELAXED)/100.0f;
   gain*=__atomic_load_n(&music_level,__ATOMIC_RELAXED)/100.0f;
   int n=cafe_show_audio_read(samples,800,gain);
   if(n>0){
    unsigned done=0;while(done<(unsigned)n&&!__atomic_load_n(&menu_sound_stop,__ATOMIC_RELAXED)){
     long wrote=snd_pcm_writei(pcm,samples+2*done,(unsigned)n-done);
     if(wrote<0){if(++recoveries>8||snd_pcm_recover(pcm,(int)wrote,1)<0)goto finished;}
     else if(!wrote)goto finished;else done+=(unsigned)wrote;
    }
    long queued=0;snd_pcm_delay(pcm,&queued);cafe_show_audio_commit(done,queued);
   }else{
    if(n==0){if(snd_pcm_drain(pcm)<0)goto finished;cafe_show_audio_drained();}
    else snd_pcm_drop(pcm);
    snd_pcm_prepare(pcm);__atomic_store_n(&cafe_show_requested,0,__ATOMIC_RELEASE);
   }
   continue;
  }
  if(__atomic_exchange_n(&menu_sound_kick,0,__ATOMIC_RELAXED))menu_sound_pos=0;
  unsigned seek=__atomic_exchange_n(&music_seek_request,~0u,__ATOMIC_ACQUIRE),hold=__atomic_load_n(&music_paused,__ATOMIC_RELAXED);
  if(seek!=~0u&&menu_music_frames){if(seek>=menu_music_frames)seek=menu_music_frames-2;if(lseek(menu_music_fd,(long)seek,0)>=0){menu_music_pos=seek;snd_pcm_drop(pcm);snd_pcm_prepare(pcm);__atomic_store_n(&music_position,seek,__ATOMIC_RELAXED);}}
  if(hold!=last_pause){snd_pcm_drop(pcm);snd_pcm_prepare(pcm);last_pause=hold;}
  unsigned credits=__atomic_load_n(&music_credits_mode,__ATOMIC_ACQUIRE),epoch=__atomic_load_n(&music_credits_epoch,__ATOMIC_ACQUIRE);
  if(credits&&epoch!=credits_epoch){credits_epoch=epoch;credits_written=credits_drained=0;credits_limit=__atomic_load_n(&music_credits_total,__ATOMIC_RELAXED);}
  if(credits&&credits_limit&&credits_written>=credits_limit&&!credits_drained){
   /* Drain the final queued samples before exposing completion; never start pass 3. */
   if(snd_pcm_drain(pcm)<0)goto finished;snd_pcm_prepare(pcm);credits_drained=1;
   if(__atomic_load_n(&music_credits_mode,__ATOMIC_ACQUIRE)&&__atomic_load_n(&music_credits_epoch,__ATOMIC_ACQUIRE)==epoch){__atomic_store_n(&music_credits_position,credits_limit,__ATOMIC_RELEASE);__atomic_store_n(&music_credits_complete,1,__ATOMIC_RELEASE);}
  }
  unsigned track_end=menu_music_frames;
  if(menu_music_frames&&!hold){unsigned next=music_audio_next(menu_music_pos,menu_music_frames,&track_end);if(next!=menu_music_pos){if(lseek(menu_music_fd,(long)next,0)<0)track_end=0;else menu_music_pos=next;}}
  unsigned music_left=hold||track_end<menu_music_pos?0:track_end-menu_music_pos,sound_left=menu_sound_frames-menu_sound_pos;
  if(credits&&credits_limit&&credits_written>=credits_limit)music_left=0;
  unsigned n=music_left>sound_left?music_left:sound_left;if(!n&&((hold&&menu_music_frames)||(credits&&credits_drained)))n=800;if(!n)break;if(n>800)n=800;if(music_left&&n>music_left)n=music_left;int16_t samples[1600],music[400];
  if(music_left){unsigned bytes=n;unsigned got=0;while(got<bytes){long r=read(menu_music_fd,(unsigned char*)music+got,bytes-got);if(r<=0)break;got+=(unsigned)r;}if(got!=bytes){menu_music_frames=menu_music_pos=0;music_left=0;}}
  float gain=__atomic_load_n(&menu_sound_gain,__ATOMIC_RELAXED)/1000.0f;unsigned music_gain=__atomic_load_n(&music_level,__ATOMIC_RELAXED);
  int cat_sample=0;for(unsigned i=0;i<n;i++)for(unsigned c=0;c<2;c++){
   int v=i<music_left?(music[i/2]*(int)music_gain*(int)__atomic_load_n(&menu_music_channel,__ATOMIC_RELAXED))/10000:0;
   if(i<sound_left)v+=menu_sound[2*(menu_sound_pos+i)+c]*(int)__atomic_load_n(&menu_fx_channel,__ATOMIC_RELAXED)/100;
   if(c==0)cat_sample=cat_audio_sample(__atomic_load_n(&cafe_audio_enabled,__ATOMIC_RELAXED));v+=cat_sample*(int)__atomic_load_n(&menu_fx_channel,__ATOMIC_RELAXED)/100;
   if(v>32767)v=32767;if(v<-32768)v=-32768;samples[2*i+c]=(int16_t)(v*gain);
  }
  unsigned done=0;while(done<n&&!__atomic_load_n(&menu_sound_stop,__ATOMIC_RELAXED)){
   long wrote=snd_pcm_writei(pcm,samples+2*done,n-done);
   if(wrote<0){recoveries++;if(recoveries>8||snd_pcm_recover(pcm,(int)wrote,1)<0)goto finished;}
   else if(!wrote)goto finished;else done+=(unsigned)wrote;
  }menu_sound_pos+=done<sound_left?done:sound_left;menu_music_pos+=done<music_left?done:music_left;
  if(credits&&music_left&&credits_limit){credits_written+=done<music_left?done:music_left;long queued=0;snd_pcm_delay(pcm,&queued);unsigned heard=credits_written;if(queued>0)heard=(unsigned long)queued<heard?heard-(unsigned)queued:0;if(heard>credits_limit)heard=credits_limit;__atomic_store_n(&music_credits_position,heard,__ATOMIC_RELEASE);}
  if(!hold){long delay=0;snd_pcm_delay(pcm,&delay);unsigned shown=menu_music_pos;if(delay>0&&(unsigned long)delay<shown)shown-=(unsigned)delay;__atomic_store_n(&music_position,shown,__ATOMIC_RELAXED);}
 }
finished:
 if(cafe_show_audio_active()){snd_pcm_drop(pcm);snd_pcm_prepare(pcm);}
 cafe_show_audio_stop();__atomic_store_n(&cafe_show_requested,0,__ATOMIC_RELEASE);
 printf("Menu sound: frames=%u/%u recoveries=%u\n",menu_sound_pos,menu_sound_frames,recoveries);fflush(0);
 __atomic_store_n(&menu_sound_done,1,__ATOMIC_RELEASE);return 0;
}
static void menu_sound_finish(void){
 if(menu_sound_active){__atomic_store_n(&menu_sound_stop,1,__ATOMIC_RELAXED);pthread_join(menu_sound_thread,0);menu_sound_active=0;}
}
static void menu_sound_service(void*pcm){
 static int was_menu,was_cafe;static int last_arrival=-1;
 int in_cafe=menu&&help_page==11;if(in_cafe&&!was_cafe&&!music_cafe_mode)music_cafe_begin(__atomic_load_n(&music_position,__ATOMIC_RELAXED),__atomic_load_n(&music_paused,__ATOMIC_RELAXED));if(!in_cafe){__atomic_store_n(&cafe_show_requested,0,__ATOMIC_RELEASE);music_cafe_end();}was_cafe=in_cafe;__atomic_store_n(&cafe_audio_enabled,in_cafe,__ATOMIC_RELAXED);
 if(in_cafe&&cafe.arrival>=0&&cafe.arrival!=last_arrival)__atomic_store_n(&cafe_audio_greeting,1,__ATOMIC_RELAXED);last_arrival=in_cafe?cafe.arrival:-1;
 __atomic_store_n(&menu_music_channel,gain_seen[1]?gain_percent[1]:100,__ATOMIC_RELAXED);
 __atomic_store_n(&menu_fx_channel,gain_seen[0]?gain_percent[0]:100,__ATOMIC_RELAXED);
 __atomic_store_n(&menu_sound_gain,(unsigned)(master_target*1000),__ATOMIC_RELAXED);
 if(menu_sound_active&&(!menu||__atomic_load_n(&menu_sound_done,__ATOMIC_ACQUIRE)))menu_sound_finish();
 if(!menu){if(was_menu)game_music_handoff();if(menu_music_fd<0)menu_music_load();was_menu=0;menu_sound_pending=0;return;}
 if(!was_menu){if(menu_music_fd<0)menu_music_load();else game_music_handoff();was_menu=1;if(!menu_sound_pending)menu_sound_pending=2;}
 if(in_cafe&&__atomic_load_n(&cafe_show_requested,__ATOMIC_ACQUIRE)&&!menu_sound_active){if(pcm)menu_sound_pending=2;else __atomic_store_n(&cafe_show_requested,0,__ATOMIC_RELEASE);}
 if(menu_sound_pending){int sound=menu_sound_pending==1;menu_sound_pending=0;
  if(menu_sound_active){if(sound)__atomic_store_n(&menu_sound_kick,1,__ATOMIC_RELAXED);return;}
  menu_sound_pos=sound?0:menu_sound_frames;
  if(pcm&&((menu_sound&&menu_sound_pos<menu_sound_frames)||(menu_music_fd>=0&&menu_music_pos<menu_music_frames)||__atomic_load_n(&cafe_show_requested,__ATOMIC_ACQUIRE))){__atomic_store_n(&menu_sound_stop,0,__ATOMIC_RELAXED);__atomic_store_n(&menu_sound_done,0,__ATOMIC_RELAXED);__atomic_store_n(&menu_sound_kick,0,__ATOMIC_RELAXED);if(!pthread_create(&menu_sound_thread,0,menu_sound_worker,pcm))menu_sound_active=1;else {__atomic_store_n(&cafe_show_requested,0,__ATOMIC_RELEASE);printf("Menu sound worker unavailable\n");}}
 }
}

static int cue_led_fd=-1;
static void led_send(int fd,unsigned char*packet,unsigned n){struct{unsigned char mode,pad,command,option;unsigned length;unsigned char*data;unsigned reserved;}cmd={0,0,0,1,n,packet,0};if(fd>=0)ioctl(fd,0x40107000,&cmd);}
static int panel_fd=-1;static struct panel_controls panel_state;
static void cafe_led_service(int shutdown){
 unsigned char eup[100],sub[104];
 int active=!shutdown&&menu&&help_page==11&&cafe_show_audio_active();
 unsigned changed=cafe_show_led_update(&cafe_led_show,active,cafe_show_audio_ms(),ns(),eup,sub);
 if(changed&CAFE_SHOW_LED_EUP)led_send(panel_fd,eup,100);
 if(changed&CAFE_SHOW_LED_SUB)led_send(cue_led_fd,sub,104);
 if(changed&&!active)led_state=0;
}
static void panel_input(void){
 uint64_t now=ns();if(cue_led_fd>=0){unsigned char sub[104];long got=read(cue_led_fd,sub,104);if(got>0){nes_sub_decode(&steering,sub,(size_t)got);sub_seen=now;}}
 if(now-sub_seen>250000000ULL){steering.valid=0;steering.cue=steering.select=0;}if(now-eup_seen>250000000ULL){nes_held=nes_a=nes_select=nes_cue_start=0;}
 if(panel_fd<0)return;unsigned char frame[104];long n=read(panel_fd,frame,sizeof(frame));if(n<0)return;
 asteroid_jog_sample(frame,(size_t)n,now);
 struct panel_event e=panel_decode(&panel_state,frame,(size_t)n);nes_held=e.held;nes_a=e.action_a;nes_select=e.select;nes_cue_start=e.start;eup_seen=now;
 for(int c=0;c<2;c++)gain_display_update(c,e.gain[c],e.gain_valid[c]);
 if(e.master_valid){master_state_store(e.master);master_target=(float)e.master/1023.0f;unsigned p=(e.master*100+511)/1023;if(volume_seen&&p!=volume_percent){volume_kind=0;volume_until=tick+120;}volume_percent=p;volume_seen=1;}
 if(usb_stop_hold(&eject_hold,usb_stop_mask(frame,(size_t)n),usb_port,ns())){int f=open("/tmp/rx3-launcher/eject-request",1|64|512,0600);if(f>=0){close(f);running=0;}}
 int directions=0,actions=0;
 if(menu&&help_page==2){for(int i=help_offset;i<help_offset+10&&i<help_count();i++){const char*l=help_controls[i];if(l[0]=='D'&&l[1]=='U'&&l[2]=='B')directions=1;if(l[0]=='N'&&l[1]=='O'&&l[2]=='I')directions=1;if(l[0]=='F'&&l[1]=='I'&&l[2]=='L')directions=1;if(l[0]=='C'&&l[1]=='R'&&l[2]=='U')directions=1;if(l[0]=='B'&&l[1]=='E'&&l[2]=='A')actions=1;if(l[0]=='R'&&l[1]=='I'&&l[2]=='G'&&l[3]=='H')actions=1;}}
 int state=(directions?1:0)|(actions?2:0);if(state)state|=((tick/30)%2)?4:0;
 if(!cafe_led_show.active&&!cafe_show_audio_active()&&(state||led_state>0)&&state!=led_state){unsigned char packet[100];tutorial_packet(packet,directions,state?actions:1,state?(state&4):1);struct{unsigned char mode,pad,command,option;unsigned length;unsigned char*data;unsigned reserved;} cmd={0,0,0,1,100,packet,0};if(ioctl(panel_fd,0x40107000,&cmd)<0&&led_state<0)printf("Tutorial LED output unavailable\n");if(actions&&cue_led_fd<0)cue_led_fd=open("/dev/subucom_spi2.0",2|04000);
 if(cue_led_fd>=0){unsigned char cue[104];tutorial_cue_packet(cue,!state||(actions&&(state&4)));led_send(cue_led_fd,cue,104);}led_state=state;}

 if(rescue.active)return;
 if(menu&&help_page==3){music_browse(e.steps,e.click);}
 else if(menu&&help_page){help_scroll(e.steps);if(help_page==7&&e.click)story_continue();}
 else if(menu){if(e.steps){menu_move(e.steps);dummy_notice=0;}if(e.click)choose_game();}
 else if(selected_game==3&&dj_wars){if(e.steps)dw_choose_weapon(dj_wars,(dj_wars->weapon+e.steps%7+7)%7);if(e.click)dj_wars_pulse|=DW_JUMP;}else if(nes_active){nes_start|=e.click;}else if(e.direction)direction=e.direction;
}
/* The plain launcher is static until a control or volume overlay changes.
 * Animated help/music/credits and games retain the 30 Hz display schedule. */
static int menu_frame_valid,menu_frame_key[14];
static int display_needs_redraw(void){
 if(!menu||(help_page&&help_page!=7&&help_page!=8&&help_page!=9&&help_page!=10)){menu_frame_valid=0;return 1;}
 int key[14]={theme,selected_game,dummy_notice,width,height,tick<volume_until,volume_kind,(int)volume_percent,(int)gain_percent[0],(int)gain_percent[1],paused,help_page*10000+help_offset,(int)session_rescued_cats,(int)(__atomic_load_n(&persistent.busy,__ATOMIC_ACQUIRE)|(__atomic_load_n(&persistent.failed,__ATOMIC_ACQUIRE)<<1))};
 if(menu_frame_valid&&!memcmp(key,menu_frame_key,sizeof(key)))return 0;
 memcpy(menu_frame_key,key,sizeof(key));menu_frame_valid=1;return 1;
}
static const char*performance_scene(void){
 if(rescue.active)return "CAT RESCUED";
 if(!menu)return game_names[selected_game];
 return help_page==11?"PURR HOUSE CAFE":help_page==7?"CAT RESCUE STORY":help_page==1?"ABOUT":help_page==2?"HOW TO PLAY":help_page==3?"MUSIC PLAYER":help_page==4?"DEBUG":help_page==5?"CREDITS":help_page==6?"CREDITS CONFIRM":help_page==8?"CREDITS LOCKED":help_page==9?"FINAL BATTLE LOCKED":help_page==10?"UNPLUG TO RETURN":"LAUNCHER";
}
int main(int argc,char**argv){const char*fbpath="/dev/fb0",*pcmname="plughw:1,0",*snapshot=0;int headless=0,probe=0,frames=-1,noaudio=0,preview_cafe=-1,preview_rescue=-1;for(int i=1;i<argc;i++){if(!strcmp(argv[i],"--debug-unlocked")){debug_unlocked=1;}else if(!strcmp(argv[i],"--cafe-preview")&&i+1<argc)preview_cafe=atoi(argv[++i]);else if(!strcmp(argv[i],"--rescue-preview")&&i+1<argc)preview_rescue=atoi(argv[++i]);else if(!strcmp(argv[i],"--dj-wars")){selected_game=3;menu=0;}else if(!strcmp(argv[i],"--asteroids")){selected_game=2;menu=0;}else if(!strcmp(argv[i],"--equalizarr")){selected_game=1;menu=0;}else if(!strcmp(argv[i],"--space-invaders")){selected_game=5;menu=0;}else if(!strcmp(argv[i],"--basslines")){selected_game=4;menu=0;}else if(!strcmp(argv[i],"--road-fighter")){selected_game=ROAD_FIGHTER_GAME;menu=0;}else if(!strcmp(argv[i],"--rom")&&i+1<argc)nes_rom=argv[++i];else if(!strcmp(argv[i],"--nes-core")&&i+1<argc)nes_core=argv[++i];else if(!strcmp(argv[i],"--nes-buttons")&&i+1<argc)test_nes_buttons=atoi(argv[++i]);else if(!strcmp(argv[i],"--neon"))theme=1;else if(!strcmp(argv[i],"--selected")&&i+1<argc){selected_game=atoi(argv[++i]);if(selected_game<0||selected_game>=MENU_GAME_COUNT)selected_game=0;}else if(!strcmp(argv[i],"--help-offset")&&i+1<argc)help_offset=atoi(argv[++i])*10;else if((!strcmp(argv[i],"--games-volume-preview")||!strcmp(argv[i],"--music-volume-preview"))&&i+1<argc){volume_kind=!strcmp(argv[i],"--games-volume-preview")?1:2;int p=atoi(argv[++i]);if(p<0)p=0;if(p>100)p=100;gain_percent[volume_kind-1]=(unsigned)p;volume_until=120;}else if(!strcmp(argv[i],"--volume-preview")&&i+1<argc){volume_percent=atoi(argv[++i]);if(volume_percent>100)volume_percent=100;volume_until=120;}else if(!strcmp(argv[i],"--debug-preview")){help_page=4;const char*sample[]={"CPU LOAD: 24%","CPU CLOCK: 1000 MHZ","ARCHITECTURE: ARMV7 / SOFTFP","RAM TOTAL: 993 MB","RAM FREE: 570 MB","RAM AVAILABLE ESTIMATE: 670 MB","GAME PROCESS RAM: 20 MB","PROCESS THREADS: 1","UPTIME: 1234 SECONDS","LOAD AVERAGE 1 MIN: 0.25"};for(int j=0;j<DEBUG_LINES;j++)snprintf(debug_data.line[j],88,"%s",sample[j]);debug_data.history_count=DEBUG_HISTORY;for(int c=0;c<DEBUG_CORES;c++){for(int j=0;j<DEBUG_HISTORY;j++)debug_data.history[c][j]=(j*7+c*13)%65+5;debug_data.core_load[c]=debug_data.history[c][DEBUG_HISTORY-1];}debug_next_update=~(uint64_t)0;}else if(!strcmp(argv[i],"--debug-artwork")&&i+1<argc)debug_artwork_path=argv[++i];else if(!strcmp(argv[i],"--debug"))help_page=4;else if(!strcmp(argv[i],"--credits-screen"))help_page=5;else if(!strcmp(argv[i],"--credits-at")&&i+1<argc){help_page=5;credits_preview_ms=atoi(argv[++i]);}else if(!strcmp(argv[i],"--music-index")&&i+1<argc)music_index_path=argv[++i];else if(!strcmp(argv[i],"--music-screen"))help_page=3;else if(!strcmp(argv[i],"--story-artwork")&&i+1<argc)story_artwork_path=argv[++i];else if(!strcmp(argv[i],"--story-screen"))help_page=7;else if(!strcmp(argv[i],"--back-to-dj-screen"))help_page=10;else if(!strcmp(argv[i],"--help-screen"))help_page=1;else if(!strcmp(argv[i],"--controls-screen"))help_page=2;else if(!strcmp(argv[i],"--game"))menu=0;else if(!strcmp(argv[i],"--headless"))headless=1;else if(!strcmp(argv[i],"--probe"))probe=1;else if(!strcmp(argv[i],"--no-audio"))noaudio=1;else if(!strcmp(argv[i],"--frames")&&i+1<argc)frames=atoi(argv[++i]);else if(!strcmp(argv[i],"--snapshot")&&i+1<argc)snapshot=argv[++i];else if(!strcmp(argv[i],"--fb")&&i+1<argc)fbpath=argv[++i];else if(!strcmp(argv[i],"--audio")&&i+1<argc)pcmname=argv[++i];else{printf("Usage: rx3-arcade [--probe] [--headless --frames N --snapshot file.ppm] [--fb /dev/fb0] [--audio plughw:1,0] [--no-audio]\n");return 2;}}
if((preview_cafe>=0||preview_rescue>=0)&&!headless){printf("Cat scene previews require --headless\n");return 2;}
if(preview_cafe>=0){session_rescued_cats=(unsigned)preview_cafe&15;session_dj_wars_won=((preview_cafe&16)!=0)&&session_finale_ready();cafe_seen=session_rescued_cats;open_cafe();cafe_started=tick-150;}
if(preview_rescue>=0&&preview_rescue<4){rescue_begin(&rescue,(unsigned)preview_rescue);rescue.ticks=108;}
if(!menu&&selected_game==3&&!debug_unlocked&&!session_finale_ready()){menu=1;help_page=9;}
printf("RX3 ARCADE / Neon Maze / ARMv7 softfp / classic WSG sound\n");struct fb_var v={0};struct fb_fix f={0};unsigned char*fb=0;int fbfd=-1;if(!headless){fbfd=open(fbpath,2);if(fbfd<0||ioctl(fbfd,0x4600,&v)||ioctl(fbfd,0x4602,&f)){printf("Framebuffer unavailable: %s, errno=%d\n",fbpath,*__errno_location());return 3;}printf("Framebuffer %.16s: %ux%u, %u bpp, pitch %u, offset %u,%u\n",f.id,v.xres,v.yres,v.bpp,f.line_length,v.xoffset,v.yoffset);if(v.xres<640||v.yres<480||v.xres>4096||v.yres>2160||v.nonstd||v.grayscale||f.type!=0||f.visual!=2||(v.bpp!=16&&v.bpp!=24&&v.bpp!=32)||!v.red.length||!v.green.length||!v.blue.length||v.red.length>8||v.green.length>8||v.blue.length>8||v.transp.length>8||v.xoffset+v.xres>v.xres_virtual||v.yoffset+v.yres>v.yres_virtual||(uint64_t)(v.xoffset+v.xres)*(v.bpp/8)>f.line_length||(uint64_t)(v.yoffset+v.yres)*f.line_length>f.smem_len){printf("Unsupported framebuffer layout; refusing to write\n");close(fbfd);return 4;}width=v.xres;height=v.yres;if(!probe){fb=mmap(0,f.smem_len,3,1,fbfd,0);if(fb==(void*)-1){printf("Framebuffer mmap failed\n");return 5;}}}
struct indev dev[16];int count=0;for(int i=0;i<16;i++){char path[48],name[128]={0};snprintf(path,sizeof(path),"/dev/input/event%d",i);int fd=open(path,04000);if(fd<0)continue;ioctl(fd,0x80804506,name);struct indev*d=&dev[count++];*d=(struct indev){.fd=fd,.ax=0,.ay=1};if(ioctl(fd,0x80184540,&d->xr)||ioctl(fd,0x80184541,&d->yr)){d->ax=53;d->ay=54;ioctl(fd,0x80184575,&d->xr);ioctl(fd,0x80184576,&d->yr);}printf("Input %s: %s, ranges %d..%d / %d..%d\n",path,name,d->xr.minimum,d->xr.maximum,d->yr.minimum,d->yr.maximum);}
if(probe){for(int i=0;i<count;i++)close(dev[i].fd);if(fbfd>=0)close(fbfd);return 0;}
pixels=malloc((size_t)width*height*4);previous=malloc((size_t)width*height*4);packed_row=malloc((size_t)width*4);if(!pixels||!previous||!packed_row)return 6;cell=(height-40)/36;int cw=(width-380)/28;if(cw<cell)cell=cw;if(cell<8)cell=8;bx=width-cell*28-20;by=(height-cell*36)/2;
if(!headless)allow_online_cpus();void*pcm=0;if(!headless&&!noaudio){int rc=snd_pcm_open(&pcm,pcmname,0,0);if(rc>=0)rc=snd_pcm_set_params(pcm,2,3,2,48000,1,160000);if(rc<0){printf("Audio unavailable: %s (%s); continuing silently\n",pcmname,snd_strerror(rc));if(pcm)snd_pcm_close(pcm);pcm=0;}else printf("Audio active: %s, S16 stereo 48kHz\n",pcmname);}
if(!headless&&menu&&!help_page){help_page=7;help_offset=0;}music_catalog_load();if(!headless&&!debug_unlocked)persistent_restore();if(debug_unlocked){session_rescued_cats=15;session_dj_wars_won=1;music_secret_unlocked=1;for(unsigned i=0;i<music_count;i++)if(music_tracks[i].locked==1)music_tracks[i].locked=0;printf("DEBUG UNLOCKED: all cats, finale and music unlocked in RAM\n");}if(!headless){menu_sound_load();}char*port=getenv("RX3_USB_PORT");if(port&&atoi(port)==2)usb_port=2;signal(2,stop);signal(15,stop);if(!headless){raw_touch_open();panel_fd=open("/dev/subucom_spi1.0",2|04000);if(panel_fd<0)printf("Panel game controls unavailable\n");}if(!headless)cue_led_fd=open("/dev/subucom_spi2.0",2|04000);arcade_reset();if(!menu&&selected_game==3&&!dj_wars_reset())return 6;if(!menu&&selected_game==2)ast_reset(&asteroid,0x5833);if(!menu&&selected_game==1)eq_reset(&equalizarr,1729,0);if(!menu&&selected_game==5)si_reset(&invaders,1978);if(!menu&&selected_game==4)bl_reset(&basslines,2401);if(!menu&&selected_game==ROAD_FIGHTER_GAME){if(!nes_open(nes_rom,nes_core))return 1;nes_active=1;nes_background=0;}uint64_t deadline=ns(),report=deadline,next_render=deadline;unsigned rendered=0,late=0,underruns=0,nes_steps=0,idle_skips=0,sim_steps=0;uint64_t emu_time=0,audio_time=0,render_time=0,sim_time=0,draw_time=0,blit_time=0,render_peak=0;const char*last_scene=0;printf("Renderer: target 30 FPS; Neon Maze 60 Hz / NES native timing; audio-paced NES\n");fflush(0);while(running&&(frames<0||(int)tick<frames)){for(int i=0;i<count;i++)input(&dev[i]);raw_touch_input();panel_input();cafe_led_service(0);if(debug_rescue_test&&!rescue.active){debug_rescue_test=0;menu=1;help_page=4;help_offset++;}credits_update(ns(),pcm!=0);credits_to_cafe();menu_sound_service(pcm);if(help_page==4)debug_update(ns());const char*scene=performance_scene();if(scene!=last_scene){printf("Scene: %s\n",scene);last_scene=scene;fflush(0);}int render_due=!headless&&ns()>=next_render;uint64_t sim_begin=ns();int sim_active=!paused&&!menu&&!rescue.active;unsigned step_count=sim_active?1:0;if(paused||menu)dj_wars_next_step=0;if(rescue.active){
 if(pcm){int16_t samples[1470];game_mix(samples,735,1);unsigned pos=0;while(pos<735){long n=snd_pcm_writei(pcm,samples+2*pos,735-pos);if(n<0){underruns++;if(snd_pcm_recover(pcm,(int)n,1)<0)break;}else if(!n)break;else pos+=(unsigned)n;}}
 rescue_advance(&rescue);
 }else if(!paused&&!menu&&nes_active){nes_video_enabled=headless||render_due;uint64_t emu_begin=ns();nes_tick(nes_buttons(nes_held,nes_a,nes_start|nes_cue_start,&steering)|(nes_select?4:0)|test_nes_buttons);emu_time+=ns()-emu_begin;nes_steps++;nes_start=0;uint64_t audio_begin=ns();if(pcm){for(unsigned i=0;i<nes_audio_frames;i++){master_gain+=(master_target-master_gain)*0.002f;nes_audio[2*i]=(int16_t)(nes_audio[2*i]*master_gain);nes_audio[2*i+1]=(int16_t)(nes_audio[2*i+1]*master_gain);}game_mix(nes_audio,nes_audio_frames,0);unsigned pos=0;while(pos<nes_audio_frames){long n=snd_pcm_writei(pcm,nes_audio+2*pos,nes_audio_frames-pos);if(n<0){underruns++;if(snd_pcm_recover(pcm,(int)n,1)<0)break;}else if(!n)break;else pos+=n;}}audio_time+=ns()-audio_begin; }else if(!paused&&!menu&&selected_game==3&&dj_wars){step_count=dj_wars_advance(sim_begin,headless);if(dj_wars_begin_credits(sim_begin)){menu_sound_service(pcm);}else if(pcm&&step_count){int16_t samples[1470];for(unsigned block=0;block<step_count;block++){dj_wars_audio_block(samples,735,&master_gain,master_target,(gain_seen[0]?gain_percent[0]:100)/100.f,(gain_seen[1]?gain_percent[1]:100)/100.f*__atomic_load_n(&music_level,__ATOMIC_RELAXED)/100.f);unsigned pos=0;while(pos<735){long n=snd_pcm_writei(pcm,samples+2*pos,735-pos);if(n<0){underruns++;if(snd_pcm_recover(pcm,(int)n,1)<0)break;}else if(!n)break;else pos+=(unsigned)n;}}}}else if(!paused&&!menu&&selected_game==5){
 invaders_step();
 if(pcm){int16_t samples[1470];si_audio(&invaders,samples,735,&master_gain,master_target);game_mix(samples,735,0);unsigned pos=0;while(pos<735){long n=snd_pcm_writei(pcm,samples+2*pos,735-pos);if(n<0){underruns++;if(snd_pcm_recover(pcm,(int)n,1)<0)break;}else if(!n)break;else pos+=(unsigned)n;}}
 }else if(!paused&&!menu&&selected_game==4){
 bl_tick(&basslines,basslines_keys|basslines_touch_keys|nes_held|(nes_a?BL_BOOST:0));
 if(pcm){int16_t samples[1470];bl_audio(&basslines,samples,735,&master_gain,master_target);game_mix(samples,735,0);unsigned pos=0;while(pos<735){long n=snd_pcm_writei(pcm,samples+2*pos,735-pos);if(n<0){underruns++;if(snd_pcm_recover(pcm,(int)n,1)<0)break;}else if(!n)break;else pos+=(unsigned)n;}}
 }else if(!paused&&!menu&&selected_game==1){
 equalizarr_step();
 if(pcm){int16_t samples[1470];eq_audio(&equalizarr,samples,735,&master_gain,master_target);game_mix(samples,735,0);unsigned pos=0;while(pos<735){long n=snd_pcm_writei(pcm,samples+2*pos,735-pos);if(n<0){underruns++;if(snd_pcm_recover(pcm,(int)n,1)<0)break;}else if(!n)break;else pos+=(unsigned)n;}}
 }else if(!paused&&!menu&&selected_game==2){unsigned buttons=asteroid_keys|asteroid_touch|nes_held|(nes_a?AST_FIRE:0)|((nes_cue_start||steering.cue)?AST_HYPER:0);if(asteroid.game_over&&(buttons&AST_FIRE)&&!(asteroid.old_buttons&AST_FIRE))ast_reset(&asteroid,0x5833u+tick);ast_tick(&asteroid,buttons);if(pcm){int16_t samples[1470];ast_audio(&asteroid,samples,735,&master_gain,master_target);game_mix(samples,735,0);unsigned pos=0;while(pos<735){long n=snd_pcm_writei(pcm,samples+2*pos,735-pos);if(n<0){underruns++;if(snd_pcm_recover(pcm,(int)n,1)<0)break;}else if(!n)break;else pos+=(unsigned)n;}}}else if(!paused&&!menu){arcade_tick(direction);if(arcade_value(4)==0){reset_current_game();}if(pcm){unsigned n=arcade_value(7);int16_t samples[2048];float*a=arcade_audio();for(unsigned i=0;i<n;i++){master_gain+=(master_target-master_gain)*0.002f;float z=a[i]*32767*master_gain;if(z>32767)z=32767;if(z<-32768)z=-32768;samples[2*i]=samples[2*i+1]=(int16_t)z;}game_mix(samples,n,0);unsigned pos=0;while(pos<n){long wrote=snd_pcm_writei(pcm,samples+2*pos,n-pos);if(wrote<0){underruns++;if(snd_pcm_recover(pcm,(int)wrote,1)<0)break;}else if(wrote==0)break;else pos+=(unsigned)wrote;}}}if(sim_active){update_game_rescue();sim_time+=ns()-sim_begin;sim_steps+=step_count;}tick++;persistent_update();
if(render_due||(snapshot&&frames>0&&(int)tick==frames)){
 if((snapshot&&frames>0&&(int)tick==frames)||display_needs_redraw()){
  uint64_t render_begin=ns();draw();draw_volume();if(persistent.started){if(__atomic_load_n(&persistent.busy,__ATOMIC_ACQUIRE)){rect(width-280,4,276,22,0x301010);text(width-272,10,1,"SAVING - DO NOT REMOVE USB",0xffcc66);}else if(__atomic_load_n(&persistent.failed,__ATOMIC_ACQUIRE)){rect(width-280,4,276,22,0x301010);text(width-272,10,1,"SAVE FAILED - SESSION ONLY",0xff6666);}}uint64_t draw_end=ns();
  if(fb)blit(fb,&v,&f);uint64_t render_end=ns(),elapsed=render_end-render_begin;
  draw_time+=draw_end-render_begin;blit_time+=render_end-draw_end;render_time+=elapsed;
  if(elapsed>render_peak)render_peak=elapsed;rendered++;
 }else idle_skips++;
 next_render+=33333333;uint64_t render_now=ns();if(render_now>next_render+33333333)next_render=render_now;
}if(!headless){deadline+=nes_active?nes_period:16666667;uint64_t now=ns();if(pcm&&nes_active&&!paused&&!menu){deadline=now;}else if(deadline>now){uint64_t delay=deadline-now;rx_timespec t={(long)(delay/1000000000),(long)(delay%1000000000)};nanosleep(&t,0);}else{late++;if(now-deadline>100000000)deadline=now;}if(now-report>=5000000000ULL){printf("Perf ticks=%u renders=%u late=%u audio_recover=%u fb_bytes=%llu interval_ms=%u\n",tick,rendered,late,underruns,fb_bytes,(unsigned)((now-report)/1000000));printf("Timing nes=%d emu_avg_us=%llu audio_avg_us=%llu render_avg_us=%llu display_fps=%u\n",nes_active,(unsigned long long)(nes_steps?emu_time/nes_steps/1000:0),(unsigned long long)(nes_steps?audio_time/nes_steps/1000:0),(unsigned long long)(rendered?render_time/rendered/1000:0),(unsigned)(rendered*1000000000ULL/(now-report)));printf("Detail scene=%s game=%d sim_steps=%u step_avg_us=%llu draw_avg_us=%llu blit_avg_us=%llu render_peak_us=%llu idle_frames_skipped=%u clock_resets=%u\n",performance_scene(),menu?-1:selected_game,sim_steps,(unsigned long long)(sim_steps?sim_time/sim_steps/1000:0),(unsigned long long)(rendered?draw_time/rendered/1000:0),(unsigned long long)(rendered?blit_time/rendered/1000:0),(unsigned long long)(render_peak/1000),idle_skips,dj_wars_clock_resets);fflush(0);rendered=late=underruns=nes_steps=0;emu_time=audio_time=render_time=sim_time=draw_time=blit_time=render_peak=0;idle_skips=sim_steps=0;fb_bytes=0;report=now;}}}
save_end();dj_wars_release();credits_end();debug_artwork_release();story_artwork_release();menu_sound_finish();cafe_show_audio_stop();cafe_led_service(1);music_cover_release();music_animation_release();cat_audio_close();if(rescue_pcm_fd>=0)close(rescue_pcm_fd);free(menu_sound);if(menu_music_fd>=0)close(menu_music_fd);nes_close();raw_touch_close();if(cue_led_fd>=0){unsigned char cue[104];tutorial_cue_packet(cue,1);led_send(cue_led_fd,cue,104);close(cue_led_fd);}if(panel_fd>=0){if(led_state>=0){unsigned char packet[100];tutorial_packet(packet,0,1,1);struct{unsigned char mode,pad,command,option;unsigned length;unsigned char*data;unsigned reserved;} cmd={0,0,0,1,100,packet,0};ioctl(panel_fd,0x40107000,&cmd);}close(panel_fd);}if(snapshot&&ppm(snapshot))printf("Snapshot write failed\n");printf("Stopped after %u ticks, score=%u\n",tick,arcade_value(0));if(pcm)snd_pcm_close(pcm);if(fb)munmap(fb,f.smem_len);if(fbfd>=0)close(fbfd);for(int i=0;i<count;i++)close(dev[i].fd);free(pixels);free(previous);free(packed_row);return 0;}
