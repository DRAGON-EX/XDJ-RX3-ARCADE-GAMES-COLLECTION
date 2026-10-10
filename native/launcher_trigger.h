#ifndef RX3_LAUNCHER_TRIGGER_H
#define RX3_LAUNCHER_TRIGGER_H
#include <stdint.h>
#include <stddef.h>
/* Right deck common data starts at wire byte 20. REV is byte 34 bit 1;
 * IN / OUT are byte 35 bits 1 / 0. rbp checkDeckData + keyCodeAsText. */
static int launcher_keys(const unsigned char *b,size_t n){
 if((n!=100&&n!=104)||b[0]||b[1]||b[2]||b[3]!=1)return -1;
 return ((b[34]&2)?1:0)|((b[35]&2)?2:0)|((b[35]&1)?4:0);
}
struct hold_trigger{uint64_t since,last;int armed,holding;};
static int launcher_hold(struct hold_trigger*t,int mask,uint64_t now){
 if(mask<0){t->holding=0;t->armed=0;t->last=now;return 0;}
 if(now<t->last||now-t->last>500000000ULL)t->holding=0;
 t->last=now;
 if(mask==0)t->armed=1;
 if(mask!=7){t->holding=0;return 0;}
 if(!t->armed)return 0;
 if(!t->holding){t->since=now;t->holding=1;return 0;}
 if(now-t->since<5000000000ULL)return 0;
 t->holding=0;t->armed=0;return 1;
}
#endif
