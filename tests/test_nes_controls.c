#include <assert.h>
#include <stdio.h>
#include "../Games/EMU/NES/RoadFighter/nes_controls.h"
#include "../native/panel_controls.h"
int main(void){unsigned char b[104]={0,0,0,1};struct nes_steering s={0};
 b[22]=0;b[23]=2;nes_sub_decode(&s,b,104);assert(!nes_buttons(0,0,0,&s));
 b[22]=0;b[23]=0;nes_sub_decode(&s,b,104);assert(nes_buttons(0,0,0,&s)==64);
 b[22]=255;b[23]=3;nes_sub_decode(&s,b,104);assert(nes_buttons(0,0,0,&s)==128);
 assert(nes_buttons(4,0,0,&s)==64); /* FX direction overrides crossfader */
 b[22]=0;b[23]=2;nes_sub_decode(&s,b,104);assert(!nes_buttons(0,0,0,&s));
 b[11]=128;b[7]=128;nes_sub_decode(&s,b,104);assert(nes_buttons(0,1,1,&s)==(256|8)); /* deck CUE no longer maps Select/Start */
 b[23]=4;nes_sub_decode(&s,b,104);assert(!nes_buttons(0,0,0,&s));
 b[23]=0;b[0]=1;nes_sub_decode(&s,b,104);assert(!s.valid);
 struct panel_controls p={0};b[0]=0;b[89]=128;b[91]=7;
 struct panel_event e=panel_decode(&p,b,104);assert(e.held==4&&e.action_a&&e.select&&e.start);
 e=panel_decode(&p,b,104);assert(e.held==4&&!e.direction);b[89]=0;b[91]=0;e=panel_decode(&p,b,104);assert(!e.held&&!e.action_a);
 puts("NES crossfader centre/dead zone, FX priority, A/B/Start, release and malformed frames passed");}
