#ifndef RX3_LEFT_JOG_H
#define RX3_LEFT_JOG_H
#include <stdint.h>
#include <stddef.h>
/* RX3 EUP protocol: deck 1's cumulative jog counter is little-endian at
 * wire 14/15, deck 2's at 30/31. It wraps at 65536; one turn is 1620 pulses.
 * These are protocol facts recovered from the local RX3 panel implementation.
 * Keep fractional ship angles so slow turns remain useful without amplifying
 * a one-pulse back-and-forth movement into visible jitter. */
struct left_jog { unsigned counter; int valid,enabled,fraction; uint64_t seen; };
static int left_jog_rotation(struct left_jog*s,const unsigned char*b,size_t n,uint64_t now,int enabled){
 if((n!=100&&n!=104)||b[0]||b[1]||b[2]||b[3]!=1){*s=(struct left_jog){0};return 0;}
 unsigned counter=b[14]|((unsigned)b[15]<<8);
 int fresh=s->valid&&now>=s->seen&&now-s->seen<=250000000ULL;
 int delta=(int)((counter-s->counter)&65535u);if(delta>32767)delta-=65536;
 int accept=fresh&&enabled&&s->enabled&&delta>=-512&&delta<=512;
 s->counter=counter;s->seen=now;s->valid=1;s->enabled=enabled;
 if(!accept){s->fraction=0;return 0;}
 s->fraction+=delta*256;
 int turn=s->fraction/1620;s->fraction-=turn*1620;
 return turn;
}
#endif
