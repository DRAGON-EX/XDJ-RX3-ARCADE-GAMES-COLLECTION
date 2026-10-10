#ifndef RX3_DJ_WARS_AUDIO_H
#define RX3_DJ_WARS_AUDIO_H
/* Original 112 BPM sneaky bass / pizzicato soundtrack, shared by ARM and WASM.
   No samples, allocations or combat RNG: eight effect voices, six musical voices. */
#include <stdint.h>
#define DW_AUDIO_RATE 44100u
struct dw_voice {uint32_t phase,step;int delta,left,total,amp,wave,envelope,decay,attack;};
struct dw_audio_state {
 uint32_t serial,rng,rate,step_left,step_index,engine_phase,clock,foot_clock,buttons;
 struct dw_voice fx[8],music[6];
 int percussion_left,percussion_total,percussion_kind,percussion_amp,victory,fuse_beat;
 float fx_gain,music_gain;
};
static void dw_audio_reset(struct dw_audio_state*a){*a=(struct dw_audio_state){.rng=0x51a7,.rate=DW_AUDIO_RATE,.fx_gain=1,.music_gain=1,.victory=-1,.fuse_beat=-1};}
static void dw_audio_set_rate(struct dw_audio_state*a,unsigned rate){a->rate=rate>=8000&&rate<=192000?rate:DW_AUDIO_RATE;}
static uint32_t dw_audio_random(struct dw_audio_state*a){a->rng^=a->rng<<13;a->rng^=a->rng>>17;a->rng^=a->rng<<5;return a->rng;}
static void dw_note(struct dw_audio_state*a,struct dw_voice*v,int hz,int end,int ms,int amp,int wave){
 unsigned duration=a->rate*(unsigned)ms/1000u;if(!duration)duration=1;
 uint32_t unit=(uint32_t)(4294967296ULL/a->rate);
 *v=(struct dw_voice){.step=unit*(unsigned)hz,.delta=(int)((int64_t)(end-hz)*unit/duration),.left=(int)duration,.total=(int)duration,.amp=amp,.wave=wave,.envelope=16777216,.decay=(16777216+(int)duration-1)/(int)duration};
}
static void dw_fx_note(struct dw_audio_state*a,int hz,int end,int ms,int amp,int wave){
 int slot=0;for(int i=0;i<8;i++){if(!a->fx[i].left){slot=i;break;}if(a->fx[i].left<a->fx[slot].left)slot=i;}
 dw_note(a,&a->fx[slot],hz,end,ms,amp,wave);
}
static int dw_voice_sample(struct dw_audio_state*a,struct dw_voice*v){
 if(!v->left)return 0;
 v->phase+=v->step;v->step=(uint32_t)((int64_t)v->step+v->delta);
 int wave=(int)(v->phase>>16)-32768;
 if(v->wave==1)wave=wave<0?-24576:24576;
 else if(v->wave==2)wave=(int)(dw_audio_random(a)>>16)-32768;
 else {wave=wave<0?-wave:wave;wave=wave*2-32768;if(v->wave==3)wave=(wave+((int)(dw_audio_random(a)>>16)-32768))/2;}
 /* Short attack prevents clicks; a squared release keeps the plucks soft. */
 int envelope=v->envelope>>14;if(v->attack<envelope)envelope=v->attack;
 if(v->attack<1024)v->attack+=6;
 v->envelope-=v->decay;if(v->envelope<0)v->envelope=0;
 int sample=(int)((int64_t)wave*v->amp*envelope*envelope/34359738368LL);v->left--;return sample;
}
static void dw_audio_event(struct dw_audio_state*a,const struct dw_game*g){
 /* Movement and fuses are polled without adding gameplay events or RNG calls. */
 if(g->state==DW_AIM&&g->active>=0&&g->active<DW_UNITS){
  const struct dw_unit*u=&g->unit[g->active];
  if((g->old_buttons&DW_JUMP)&&!(a->buttons&DW_JUMP)&&u->vy<0)dw_fx_note(a,240,580,180,1200,0);
  if((g->old_buttons&(DW_LEFT|DW_RIGHT))&&u->ground&&a->clock-a->foot_clock>a->rate/5u){dw_fx_note(a,95,55,65,950,3);a->foot_clock=a->clock;}
 }
 a->buttons=g->old_buttons;
 int fuse=-1;for(int i=0;i<DW_SHOTS;i++)if(g->shot[i].live&&g->shot[i].kind==1&&(fuse<0||g->shot[i].life<fuse))fuse=g->shot[i].life;
 int beat=fuse<0?-1:fuse/(fuse<60?10:30);
 if(beat>=0&&beat!=a->fuse_beat)dw_fx_note(a,fuse<60?1350:950,fuse<60?1350:950,35,1050,1);
 a->fuse_beat=beat;
 if(a->serial==g->event_serial)return;
 a->serial=g->event_serial;
 switch(g->event){
  case 1:dw_fx_note(a,g->team?262:523,g->team?196:659,160,1600,0);break;
  case 2:dw_fx_note(a,95,24,800,6000,3);dw_fx_note(a,180,45,230,3800,2);break;
  case 3:dw_fx_note(a,190,65,1300,2400,3);dw_fx_note(a,1200,320,1000,1700,0);break;
  case 4:dw_fx_note(a,170,450,750,2600,3);dw_fx_note(a,660,1320,400,1500,0);break;
  case 5:dw_fx_note(a,330,80,800,2000,0);dw_fx_note(a,90,30,450,1900,2);break;
  case 6:if(g->weapon==1){dw_fx_note(a,650,200,120,1700,0);dw_fx_note(a,180,85,90,1800,3);}else {dw_fx_note(a,230,55,380,4000,3);dw_fx_note(a,g->weapon==3?1400:950,300,220,2200,2);}break;
  case 7:dw_fx_note(a,220,1760,550,2000,0);dw_fx_note(a,1760,440,750,1700,0);break;
  case 8:dw_fx_note(a,75,160,400,2100,3);break;
  case 9:dw_fx_note(a,130,35,260,3400,3);break;
  case 10:dw_fx_note(a,160,150,150,1400,1);break;
  case 11:dw_fx_note(a,392,392,650,1800,0);dw_fx_note(a,494,494,850,1600,0);dw_fx_note(a,587,784,1200,1900,0);break;
  case 12:dw_fx_note(a,523,1046,800,1600,0);dw_fx_note(a,659,1318,1100,1400,0);dw_fx_note(a,784,1568,1400,1400,0);break;
  case 13:dw_fx_note(a,1500,180,160,2500,3);dw_fx_note(a,100,28,380,4000,3);break;
  case 14:dw_fx_note(a,300,1500,300,1400,0);dw_fx_note(a,110,35,900,3000,3);break;
  case 15:dw_fx_note(a,480,100,350,2300,0);dw_fx_note(a,95,40,120,1400,2);break;
 }
}
static void dw_music_step(struct dw_audio_state*a,const struct dw_game*g){
 /* Eight-bar melodic phrases vary between stealthy minor and a warm reunion. */
 static const int roots[8]={110,87,130,82,110,146,87,82};
 static const int melody[64]={440,0,523,0,659,587,523,0, 349,0,440,0,523,0,494,0, 523,0,659,587,784,0,659,0, 494,0,415,0,330,392,415,0, 440,523,659,0,880,0,784,659, 587,0,698,659,587,0,523,0, 523,440,349,0,440,523,587,0, 659,0,622,0,494,0,415,0};
 static const int happy[16]={523,0,659,784,1046,0,784,659,587,0,698,880,784,659,587,523};
 unsigned step=a->step_index++%64u,bar=step/8u;int victory=(g->state==DW_OVER&&g->winner==0)||(g->state==DW_FINALE&&(!g->finale_assisted||g->finale_tick>=DW_CELEBRATE_TICK));
 if(victory!=a->victory){a->victory=victory;step=bar=0;a->step_index=1;}
 int root=victory?(bar%4u==3?98:bar%4u==2?87:130):roots[bar];
 if(!(step&1u))dw_note(a,&a->music[0],root,root,240,2200,0);
 int hz=victory?happy[step%16u]:melody[step];
 if(hz)dw_note(a,&a->music[1],hz,hz,step%8u==6?300:175,victory?1500:1200,0);
 if(!(step%4u)){
  dw_note(a,&a->music[2],root*2,root*2,520,750,0);
  dw_note(a,&a->music[3],victory?root*5/2:root*12/5,victory?root*5/2:root*12/5,420,550,0);
  dw_note(a,&a->music[4],root*3,root*3,450,550,0);
 }
 if(!(step&1u))dw_note(a,&a->music[5],90,35,150,1700,0);
 a->percussion_total=a->percussion_left=(int)(a->rate*(step%4u==2?90:35)/1000);
 a->percussion_kind=step%4u==2;a->percussion_amp=a->percussion_kind?650:240;
 a->step_left=a->rate*60u/(112u*2u);
}
static void dw_audio_sample(struct dw_audio_state*a,const struct dw_game*g,int*fx,int*music){
 if(!a->step_left)dw_music_step(a,g);
 a->step_left--;a->clock++;
 int effect=0,score=0;for(int v=0;v<8;v++)effect+=dw_voice_sample(a,&a->fx[v]);
 for(int v=0;v<6;v++)score+=dw_voice_sample(a,&a->music[v]);
 if(a->percussion_left){int noise=(int)(dw_audio_random(a)>>17)-16384;score+=(int)((int64_t)noise*a->percussion_amp*a->percussion_left/(16384LL*a->percussion_total));a->percussion_left--;}
 if(g->jet_ticks||g->charging){a->engine_phase+=(uint32_t)((4294967296ULL/a->rate)*(g->jet_ticks?70u:180u+(unsigned)g->power*3u));int wave=(int)(a->engine_phase>>17)-16384;
  effect+=g->jet_ticks?(wave+((int)(dw_audio_random(a)>>17)-16384))/18:wave/38;}
 *fx=effect;*music=score;
}
static int dw_audio_limit(int sample){return sample>30000?30000:sample< -30000?-30000:sample;}
/* CH1 effects, CH2 music, then master. Muted buses keep their musical position. */
static void dw_audio_mix(struct dw_audio_state*a,const struct dw_game*g,int16_t*out,unsigned frames,float*gain,float target,float fx,float music){
 dw_audio_event(a,g);for(unsigned i=0;i<frames;i++){
  int effect,score;dw_audio_sample(a,g,&effect,&score);*gain+=(target-*gain)*.002f;
  a->fx_gain+=(fx-a->fx_gain)*.002f;a->music_gain+=(music-a->music_gain)*.002f;
  int sample=dw_audio_limit((int)((effect*a->fx_gain+score*a->music_gain)**gain));out[2*i]=out[2*i+1]=(int16_t)sample;
 }
}
/* Compatibility for effect-only callers. Native and preview use the mixer/stems. */
static void dw_audio(struct dw_audio_state*a,const struct dw_game*g,int16_t*out,unsigned frames,float*gain,float target){a->music_gain=0;dw_audio_mix(a,g,out,frames,gain,target,1,0);}
#endif
