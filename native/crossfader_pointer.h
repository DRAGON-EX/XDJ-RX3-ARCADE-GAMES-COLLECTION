#ifndef RX3_CROSSFADER_POINTER_H
#define RX3_CROSSFADER_POINTER_H
/* Position control from the existing 10-bit SUB ADC sample. No extra device
 * reads: a small deadband rejects noise; buttons/touch take ownership until
 * the crossfader is moved again, avoiding a snap back after button release. */
struct crossfader_pointer {unsigned raw;int seen,active;};
static void crossfader_reset(struct crossfader_pointer*s,unsigned raw,int valid){
 *s=(struct crossfader_pointer){.raw=raw,.seen=valid&&raw<=1023};
}
static int crossfader_position(struct crossfader_pointer*s,unsigned raw,int valid,
                              unsigned directions,int touch,int left,int right){
 if(!valid||raw>1023){crossfader_reset(s,0,0);return touch;}
 if(!s->seen||directions||touch>=0){crossfader_reset(s,raw,1);return touch;}
 unsigned delta=raw>s->raw?raw-s->raw:s->raw-raw;
 if(delta>=4||(delta&&(raw==0||raw==1023))){s->raw=raw;s->active=1;}
 return s->active?left+(int)((s->raw*(unsigned)(right-left)+511)/1023):-1;
}
#endif
