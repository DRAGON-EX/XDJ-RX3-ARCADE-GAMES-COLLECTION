#include <assert.h>
#include <stdio.h>
#include "../native/panel_controls.h"
#include "../native/panel_output.h"
int main(void){
 struct panel_controls s={0};unsigned char b[104]={0};b[3]=1;
 b[38]=255;b[39]=3;b[54]=0;b[55]=2;struct panel_event gains=panel_decode(&s,b,104);assert(gains.gain_valid[0]&&gains.gain[0]==1023&&gains.gain_valid[1]&&gains.gain[1]==512);b[55]=4;assert(!panel_decode(&s,b,104).gain_valid[1]);b[55]=0;
 assert(!panel_decode(&s,b,104).click);
 assert(panel_decode(&s,b,104).master_valid);
 b[74]=255;b[75]=3;struct panel_event e=panel_decode(&s,b,104);assert(e.master_valid&&e.master==1023);
 b[75]=4;assert(!panel_decode(&s,b,104).master_valid);b[74]=b[75]=0;
 const unsigned bits[]={64,128,16,4},dirs[]={1,4,8,2};
 for(int i=0;i<4;i++){b[89]=bits[i];assert(panel_decode(&s,b,104).direction==dirs[i]);assert(!panel_decode(&s,b,104).direction);b[89]=0;panel_decode(&s,b,104);}
 b[84]=1;assert(panel_decode(&s,b,104).steps==1);b[84]=0;assert(panel_decode(&s,b,104).steps==-1);
 b[89]=64;panel_decode(&s,b,104);b[93]=16;assert(panel_decode(&s,b,104).click);assert(!panel_decode(&s,b,104).click);b[93]=0;assert(!panel_decode(&s,b,104).click);
 b[3]=2;assert(!panel_decode(&s,b,104).click);b[3]=1;b[93]=16;assert(!panel_decode(&s,b,104).click);
 b[84]=255;b[85]=255;s.valid=0;panel_decode(&s,b,104);b[84]=b[85]=0;assert(panel_decode(&s,b,104).steps==1);
 b[84]=120;assert(!panel_decode(&s,b,104).steps);
 struct usb_stop_hold h={0};assert(!usb_stop_hold(&h,1,1,0)); /* held at entry is ignored */
 assert(!usb_stop_hold(&h,0,1,1));assert(!usb_stop_hold(&h,2,1,2)); /* other port */
 assert(!usb_stop_hold(&h,1,1,100));assert(!usb_stop_hold(&h,1,1,1000000099ULL));assert(usb_stop_hold(&h,1,1,1000000100ULL));assert(!usb_stop_hold(&h,1,1,2000000200ULL));
 b[94]=16;assert(usb_stop_mask(b,104)==1);b[94]=8;assert(usb_stop_mask(b,104)==2);b[3]=0;assert(usb_stop_mask(b,104)==-1);
 unsigned char out[100];tutorial_packet(out,1,0,1);assert(out[19]==255&&out[1]==0xe6&&out[7]==0x99);
 tutorial_packet(out,1,0,0);assert(out[19]==0xe2&&out[1]==0xe6);
 tutorial_packet(out,0,1,0);assert(out[19]==0xbf);
 tutorial_packet(out,0,1,1);assert(out[19]==255);
 unsigned char cue[104];tutorial_cue_packet(cue,1);assert(cue[98]==0xf0&&cue[99]==0xf0);assert(cue[0]==1&&cue[1]==6&&cue[95]==4);tutorial_cue_packet(cue,0);assert(cue[98]==0xd0&&cue[99]==0xf0&&cue[1]==6);
 assert((out[22]|out[23]<<8)==led_crc_words(out,5));assert((cue[102]|cue[103]<<8)==led_crc_words(cue,25));
 puts("COLOR FX restoration and separate right CUE output passed");
 puts("USB STOP port isolation, one-second hold and tutorial LED packet masks passed");
 puts("Panel FX directions, browse turn/push, held buttons, invalid frames and wrap passed");
}
