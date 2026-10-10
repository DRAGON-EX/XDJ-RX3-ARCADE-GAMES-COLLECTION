#ifndef RX3_PANEL_CONTROLS_H
#define RX3_PANEL_CONTROLS_H
#include <stdint.h>
#include <stddef.h>
/* RX3 panel protocol facts, implemented independently: colour FX at wire 89;
 * browse counter at 84/85 and browse push at wire 93 bit 4. */
struct panel_controls { unsigned counter,buttons; int valid; };
struct panel_event { int steps,click; unsigned direction,held,action_a,master; int master_valid; unsigned select,start,gain[2]; int gain_valid[2]; };
static struct panel_event panel_decode(struct panel_controls*s,const unsigned char*b,size_t n){
 struct panel_event e={0};
 if((n!=100&&n!=104)||b[0]||b[1]||b[2]||b[3]!=1){s->valid=0;return e;}
 for(int c=0;c<2;c++){unsigned offset=c?54:38;e.gain[c]=b[offset]|((unsigned)b[offset+1]<<8);e.gain_valid[c]=e.gain[c]<=1023;}
 e.master=b[74]|((unsigned)b[75]<<8);e.master_valid=e.master<=1023;
 unsigned counter=b[84]|((unsigned)b[85]<<8),buttons=b[93]&16;
 if(s->valid){int delta=(int)((counter-s->counter)&65535);if(delta>32767)delta-=65536;if(delta>=-32&&delta<=32)e.steps=delta;e.click=buttons&&!(s->buttons&16);}
 e.held=((b[89]&64)?1:0)|((b[89]&4)?2:0)|((b[89]&128)?4:0)|((b[89]&16)?8:0);e.action_a=(b[91]&4)!=0;e.select=(b[91]&2)!=0;e.start=(b[91]&1)!=0;
 unsigned oldfx=s->buttons>>8,fx=b[89],pressed=s->valid?fx&~oldfx:0;
 if(pressed&64)e.direction=1; /* DUB ECHO: up */
 else if(pressed&128)e.direction=4; /* NOISE: left */
 else if(pressed&16)e.direction=8; /* FILTER: right */
 else if(pressed&4)e.direction=2; /* CRUSH: down */
 s->counter=counter;s->buttons=buttons|(fx<<8);s->valid=1;return e;
}
#endif
