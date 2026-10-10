#ifndef RX3_CREDITS_ROBOT_H
#define RX3_CREDITS_ROBOT_H
/* A ceiling-suspended, articulated laboratory optic. The host supplies
   cr_rect, cr_wave and cr_dragon_shape. Every part stays in a 48 x 48 grid. */
static void cr_robot(int x,int y,int s,unsigned t){
 const unsigned ink=0x121119,edge=0x44434d,steel=0x777680;
 const unsigned shell=0xece7df,shade=0xaba7aa,light=0xfffaf0;
 int sway=cr_wave(t/4,2)-1,nod=cr_wave((t+900)/5,1);
 #define RB(X,Y,W,H,C) cr_rect(x+(X)*s,y+(Y)*s,(W)*s,(H)*s,(C))
 #define RP(C,...) do{const signed char p[]={__VA_ARGS__};cr_dragon_shape(x,y,s,p,sizeof(p)/2,C);}while(0)
 /* Fixed ceiling clamp; independently stepped cables follow the hanging mass. */
 RB(20,0,17,2,edge);RB(23,2,11,2,steel);RB(25,2,2,2,shade);
 RB(23,4,2,5,edge);RB(24,9,2,6,edge);RB(25+sway,15,2,8,edge);
 RB(29,4,3,9,ink);RB(29,4,1,9,steel);RB(29+sway,13,3,7,edge);
 RB(35,2,2,8,edge);RB(36,10,2,8,edge);RB(35+sway,18,2,10,edge);
 RB(36,12,1,5,steel);RB(24,10,1,4,steel);
 x+=sway*s;
 /* A dark flexible spine remains visible inside the curved porcelain shell. */
 RP(ink,27,12,32,13,35,18,35,25,32,31,27,35,22,34,23,29,28,25,29,20,26,17);
 RP(edge,28,15,30,15,32,20,31,26,27,31,24,31,25,28,29,25,30,20);
 RB(28,18,4,2,steel);RB(29,22,4,2,steel);RB(27,26,4,2,steel);
 RB(25,29,4,2,steel);
 /* Swept outer shoulder: broad at the top, tapering to the head coupling. */
 RP(ink,28,10,34,11,38,15,41,21,40,27,36,32,30,35,27,32,33,28,35,23,33,18,28,16);
 RP(shade,29,12,34,13,37,16,39,21,38,27,35,30,30,33,29,31,34,27,36,23,34,17,29,15);
 RP(shell,29,12,34,13,37,17,38,22,37,26,34,29,31,31,30,30,34,25,34,20,32,16,29,15);
 RP(light,29,12,33,13,35,16,34,17,31,15,29,15);
 RB(36,20,2,4,light);RB(34,27,2,2,light);
 /* Shell seams and one recessed actuator break up the continuous white arc. */
 RP(edge,34,16,38,18,38,20,34,18);
 RP(edge,34,27,37,28,35,30,32,30);
 RB(38,22,1,3,steel);RB(32,30,1,2,shade);
 /* Looping rear hoses connect beneath the shoulder rather than forming limbs. */
 RB(37,28,2,6,edge);RB(34,33,4,2,edge);RB(30,34,5,2,edge);
 RB(37,29,1,3,steel);RB(31,34,3,1,steel);
 y+=nod*s;
 /* Tilted, long optic pod. Its off-center eye faces toward the center stage. */
 RP(ink,12,28,22,26,28,27,32,30,32,35,29,39,22,43,15,44,10,41,8,36,9,32);
 RP(shade,13,29,22,27,28,28,30,31,30,35,27,39,21,42,15,42,11,39,10,35,11,32);
 RP(shell,13,29,22,27,27,28,29,30,28,32,22,31,16,32,12,35,11,37,10,35,11,32);
 RP(light,13,29,21,28,25,28,25,29,17,30,12,33,11,33);
 RP(shell,12,38,16,40,22,39,28,36,29,35,28,38,21,42,15,42,12,40);
 RP(light,16,41,21,40,24,39,23,41,20,42,16,42);
 RP(ink,13,33,18,30,24,30,29,32,29,35,25,38,18,40,13,38,11,36);
 RP(edge,14,33,18,31,23,31,26,32,25,33,19,32,15,34,13,36,12,35);
 /* Amber lens: dark bezel, copper glass and a scanning yellow aperture. */
 RB(16,33,7,5,0x594129);RB(17,32,5,7,0x594129);
 RB(17,33,5,5,0xd7932c);RB(18,32,3,1,0xb77522);
 int scan=cr_wave(t/3,2)-1;
 RB(18+scan,34,3,3,0xffd55f);RB(19+scan,34,1,2,0xfff5bc);
 RB(17,33,1,1,0xffe894);RB(22,35,1,2,0x8e561e);
 RB(25,33,2,1,steel);RB(25,34,2,2,ink);
 /* Small mechanical details keep the shell readable at four-times scale. */
 RB(11,38,1,2,steel);RB(27,29,1,1,light);RB(28,37,1,1,edge);
 #undef RP
 #undef RB
}
#endif
