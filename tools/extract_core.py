from pathlib import Path
s=Path('vendor/pacman-upstream.c').read_text()
s=s[s.index('// config defines'):s.index('/*== GFX SUBSYSTEM')]
a=s.index('        // sokol-gfx resources'); b=s.index('    } gfx;',a);s=s[:a]+s[b:]
a=s.index('// forward-declared sound-effect register dumps');b=s.index('/*== GRAB BAG',a)
stubs=s[a:s.index('// forward declarations',a)]
stubs+='\nstatic void start(trigger_t* t);\nstatic bool now(trigger_t t);\nstatic void intro_tick(void);\nstatic void game_tick(void);\nstatic void input_enable(void);\nstatic void input_disable(void);\nstatic void snd_clear(void);\nstatic void snd_stop(int slot);\nstatic void snd_start(int slot,const sound_desc_t *d);\nstatic const uint8_t rom_wavetable[256];\n'
s=s[:a]+stubs+s[b:]
header='''/* Adapted from floooh/pacman.c, MIT. See vendor/LICENSE.pacman.
 * Gameplay only. Original graphics and Sokol backend removed; local ROM wavetable retained.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#define assert(x) do { if (!(x)) __builtin_trap(); } while(0)
void *memset(void *p,int c,size_t n) {unsigned char *q=p;while(n--)*q++=(unsigned char)c;return p;}
int abs(int x) {return x<0?-x:x;}
'''
audio=Path('vendor/pacman-upstream.c').read_text()
audio=audio[audio.index('// the snd_voice_tick()'):audio.index('// Pacman sprite ROM dump')]
audio=audio.replace('saudio_push(state.audio.sample_buffer, state.audio.num_samples);','audio_push(state.audio.sample_buffer, state.audio.num_samples);')
push='\nstatic float audio_output[1024];\nstatic unsigned audio_count;\nstatic void audio_push(const float *p,unsigned n) {\n for(unsigned i=0;i<n&&audio_count<1024;i++)audio_output[audio_count++]=p[i];\n}\n'
api='''
void arcade_restore_high(unsigned score) { if(score/10>state.game.hiscore)state.game.hiscore=score/10; }
void arcade_reset(void) {
 unsigned high=state.game.hiscore;
 memset(&state,0,sizeof(state));audio_count=0;state.audio.sample_duration_ns=1000000000/48000;state.audio.voice_tick_period=96000000/48000;
 state.game.hiscore=high;state.timing.tick=1;
 disable(&state.intro.started);start(&state.game.started);state.timing.tick++;state.gamestate=GAMESTATE_GAME;
}
void arcade_tick(unsigned keys) {
 audio_count=0;snd_tick();state.input.up=(keys&1)!=0;state.input.down=(keys&2)!=0;
 state.input.left=(keys&4)!=0;state.input.right=(keys&8)!=0;
 state.input.anykey=keys!=0;
 if(now(state.game.started))state.gamestate=GAMESTATE_GAME;
 if(now(state.intro.started))state.gamestate=GAMESTATE_INTRO;
 if(state.gamestate==GAMESTATE_GAME)game_tick();else intro_tick();
 snd_frame(16666667);audio_push(state.audio.sample_buffer,state.audio.num_samples);state.audio.num_samples=0;state.timing.tick++;
}
unsigned arcade_tile(unsigned x,unsigned y) {return x<28&&y<36?state.gfx.video_ram[y][x]:64;}
unsigned arcade_value(unsigned key) {
 switch(key){case 0:return state.game.score*10;case 1:return state.game.num_lives;
 case 2:return state.game.round+1;case 3:return state.game.freeze;
 case 4:return state.gamestate;case 5:return state.game.num_dots_eaten;
 case 6:return state.game.hiscore*10;case 7:return audio_count;
 default:return 0;}
}
int arcade_actor(unsigned index,unsigned field) {
 if(index>=5)return 0;
 actor_t *a=index==0?&state.game.pacman.actor:&state.game.ghost[index-1].actor;
 switch(field){case 0:return a->pos.x;case 1:return a->pos.y;case 2:return a->dir;
 case 3:return index?state.game.ghost[index-1].state:0;
 case 4:return state.gfx.sprite[index].enabled;default:return 0;}
}
'''
Path('Games/Native/Pacman/pacman_core.c').write_text(header+s+push+audio+'\n#include "pacman_sound_data.h"\n'+api+'\nfloat *arcade_audio(void){return audio_output;}\n')
