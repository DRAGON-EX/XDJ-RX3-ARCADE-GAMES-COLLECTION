#ifndef RX3_PANEL_OUTPUT_H
#define RX3_PANEL_OUTPUT_H
/* Independently implemented RX3 EUP LED packet: 24 payload bytes, CRC-X25
 * over the first five words in bus order. Remaining transfer bytes are zero. */
static uint16_t led_crc_words(const unsigned char*p,int words){unsigned c=65535;for(int w=0;w<words;w++)for(int j=3;j>=0;j--){c^=p[w*4+j];for(int b=0;b<8;b++)c=(c>>1)^((c&1)?0x8408:0);}return(uint16_t)~c;}
static void tutorial_packet(unsigned char*p,int directions,int actions,int on){
 for(int i=0;i<100;i++)p[i]=0;
 p[1]=p[5]=0xe6;p[2]=p[6]=8;p[3]=p[7]=0x99;
 p[9]=0x80;p[10]=0xfd;
 p[19]=directions&&!on?0xe2:0xff; /* keep all six COLOR FX enabled outside the direction demonstration */
 if(on){if(directions)p[19]|=1|4|8|16; /* FILTER, DUB ECHO, NOISE, CRUSH */
 if(actions){p[19]|=64;}}else if(actions)p[19]&=~64; /* BEAT FX ON/OFF; CUE uses the separate SUB controller. */
 unsigned crc=led_crc_words(p,5);p[22]=crc;p[23]=crc>>8;
}
static void tutorial_cue_packet(unsigned char*p,int on){
 for(int i=0;i<104;i++)p[i]=0;
 for(int deck=0;deck<2;deck++)for(int led=0;led<12;led++){int pos=deck*48+led*4;/* Dim enabled RGB baseline; avoid the previous bright white fill. */p[pos]=1;p[pos+1]=6;p[pos+2]=6;p[pos+3]=4;}
 p[98]=p[99]=0xf0;if(!on)p[98]&=~32; /* SUB common: right deck CUE; EUP byte 7 bit 4 is RELOOP. */
 unsigned crc=led_crc_words(p,25);p[102]=crc;p[103]=crc>>8;
}
struct usb_stop_hold{uint64_t since;int armed,held;};
static int usb_stop_mask(const unsigned char*b,size_t n){if((n!=100&&n!=104)||b[0]||b[1]||b[2]||b[3]!=1)return -1;return ((b[94]&16)?1:0)|((b[94]&8)?2:0);}
static int usb_stop_hold(struct usb_stop_hold*s,int mask,int port,uint64_t now){
 if(mask<0){s->held=0;s->armed=0;return 0;}int pressed=mask&(port==2?2:1);
 if(!pressed){s->armed=1;s->held=0;return 0;}if(!s->armed)return 0;
 if(!s->held){s->held=1;s->since=now;}if(now-s->since<1000000000ULL)return 0;s->armed=0;s->held=0;return 1;
}
#endif
