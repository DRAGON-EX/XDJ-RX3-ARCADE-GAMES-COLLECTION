/* Independent libretro frontend. ROMs are supplied locally by the user. */
#include "../../../native/platform.h"
#include "nes.h"
#include "../../../work/libretro-fceumm-master/src/drivers/libretro/libretro-common/include/libretro.h"

static void *handle;
static void (*run_core)(void),(*unload_core)(void),(*deinit_core)(void);
int nes_video_enabled=1;
static unsigned buttons,format=RETRO_PIXEL_FORMAT_0RGB1555;
uint32_t nes_video[512*480];unsigned nes_width,nes_height,nes_audio_frames;
int16_t nes_audio[4096];uint64_t nes_period=20000000;
static void core_log(enum retro_log_level level,const char*fmt,...){(void)level;(void)fmt;}
static bool environment(unsigned cmd,void *p){
 switch(cmd){
 case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:((struct retro_log_callback*)p)->log=core_log;return true;
 case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:format=*(unsigned*)p;return format<=2;
 case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:*(const char**)p="/tmp/rx3-launcher";return true;
 case RETRO_ENVIRONMENT_GET_CAN_DUPE:*(bool*)p=true;return true;
 case RETRO_ENVIRONMENT_GET_VARIABLE:{struct retro_variable*v=p;
 v->value=!strcmp(v->key,"fceumm_sndrate_hint")?"48KHz":!strcmp(v->key,"fceumm_region")?"PAL":!strcmp(v->key,"fceumm_sndquality")?"Low":0;return v->value!=0;}
 case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:*(bool*)p=false;return true;
 case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:*(unsigned*)p=0;return true;
 case RETRO_ENVIRONMENT_SET_VARIABLES:case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:case RETRO_ENVIRONMENT_SET_GEOMETRY:return true;
 default:return false;
 }
}
static void video(const void *p,unsigned w,unsigned h,size_t pitch){
 if(!nes_video_enabled||!p||w>512||h>480)return;nes_width=w;nes_height=h;
 for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w;x++){
 const unsigned char *row=(const unsigned char*)p+y*pitch;uint32_t c;
 if(format==RETRO_PIXEL_FORMAT_XRGB8888)c=((const uint32_t*)row)[x]&0xffffff;
 else{unsigned v=((const uint16_t*)row)[x],r,g,b;if(format==RETRO_PIXEL_FORMAT_RGB565){r=v>>11;g=(v>>5)&63;b=v&31;g=(g<<2)|(g>>4);}else{r=(v>>10)&31;g=((v>>5)&31)*255/31;b=v&31;}c=(((r<<3)|(r>>2))<<16)|(g<<8)|(b<<3)|(b>>2);}
 nes_video[y*w+x]=c;}
}
static size_t audio_batch(const int16_t *p,size_t n){size_t take=n;if(take>2048-nes_audio_frames)take=2048-nes_audio_frames;memcpy(nes_audio+nes_audio_frames*2,p,take*4);nes_audio_frames+=take;return n;}
static void audio_sample(int16_t l,int16_t r){int16_t p[]={l,r};audio_batch(p,1);}
static void input_poll(void){}
static int16_t input_state(unsigned port,unsigned device,unsigned index,unsigned id){(void)index;return port==0&&device==RETRO_DEVICE_JOYPAD&&id<16&&((buttons>>id)&1);}
void nes_close(void){if(handle){unload_core();deinit_core();handle=0;}nes_width=nes_height=nes_audio_frames=0;}
int nes_open(const char*rom,const char*core){
 nes_close();int fd=open(rom,0);if(fd<0){printf("Road Fighter ROM unavailable: %s\n",rom);return 0;}unsigned char header[16];long n=read(fd,header,16);close(fd);
 if(n!=16||header[0]!='N'||header[1]!='E'||header[2]!='S'||header[3]!=26||header[4]!=1||header[5]!=1||header[6]&0xf4||header[7]&0xf0){printf("Expected a mapper-0 Road Fighter NES ROM\n");return 0;}
 (void)core;handle=(void*)1;
 #define LOAD(name) __typeof__(&name) p_##name=&name;
 LOAD(retro_set_environment) LOAD(retro_set_video_refresh) LOAD(retro_set_audio_sample) LOAD(retro_set_audio_sample_batch) LOAD(retro_set_input_poll) LOAD(retro_set_input_state) LOAD(retro_init) LOAD(retro_load_game) LOAD(retro_get_system_av_info) LOAD(retro_set_controller_port_device) LOAD(retro_run) LOAD(retro_unload_game) LOAD(retro_deinit)
 run_core=p_retro_run;unload_core=p_retro_unload_game;deinit_core=p_retro_deinit;
 p_retro_set_environment(environment);p_retro_set_video_refresh(video);p_retro_set_audio_sample(audio_sample);p_retro_set_audio_sample_batch(audio_batch);p_retro_set_input_poll(input_poll);p_retro_set_input_state(input_state);p_retro_init();
 struct retro_game_info game={rom,0,0,0};if(!p_retro_load_game(&game)){nes_close();return 0;}
 p_retro_set_controller_port_device(0,RETRO_DEVICE_JOYPAD);struct retro_system_av_info av;p_retro_get_system_av_info(&av);
 if(av.timing.sample_rate!=48000||av.timing.fps<49||av.timing.fps>61){printf("Unexpected NES audio/timing\n");nes_close();return 0;}
 nes_period=(uint64_t)(1000000000.0/av.timing.fps);printf("Road Fighter loaded: %.3f Hz, %.0f Hz audio\n",av.timing.fps,av.timing.sample_rate);return 1;
}
void nes_tick(unsigned b){buttons=b;nes_audio_frames=0;if(handle)run_core();}
