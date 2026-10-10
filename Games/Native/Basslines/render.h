/* Original neon circuits, rendered directly into the shared framebuffer. */
static const unsigned bl_colors[]={0x64ffe1,0x66c8ff,0xffcd55,0xe0a7ff};
static void bl_number(int x,int y,int scale,unsigned v,unsigned color){char a[16],b[16];int n=0;do{a[n++]='0'+v%10;v/=10;}while(v);for(int i=0;i<n;i++)b[i]=a[n-i-1];b[n]=0;bl_text(x,y,scale,b,color);}
static void bl_render(struct bl_game*g,int paused){
 bl_rect(0,0,1280,800,0x080f1b);bl_rect(24,30,294,3,0x64ffe1);bl_text(24,54,4,"BASSLINES",0x64ffe1);bl_text(26,99,2,"OWN THE GRID",0x8fa1b5);
 bl_text(26,139,2,"SCORE",0x7796aa);bl_number(128,137,3,g->points,0xffffff);
 bl_text(26,174,2,"HIGH SCORE",0x7796aa);bl_number(174,174,2,g->best,0xffcd55);
 bl_text(26,210,1,"ROUND",0x7796aa);bl_number(80,207,2,g->round,0xffffff);bl_text(136,210,1,"CLEAR ALL 8 ARENAS",0x8fa1b5);
 for(int i=0;i<4;i++){int y=249+i*37;bl_rect(26,y,8,18,bl_colors[i]);bl_text(47,y,2,bl_names[i],bl_colors[i]);bl_number(228,y,2,g->score[i],0xffffff);if(!g->rider[i].alive)bl_text(263,y+3,1,"OUT",0x7796aa);}
 bl_text(26,417,2,"BOOST",0x8fa1b5);bl_rect(116,416,198,17,0x25445b);bl_rect(119,419,g->rider[0].fuel*192/240,11,0x64ffe1);
 bl_text(26,452,1,"HOLD A / BEAT FX - BOOST",0x8fa1b5);
 bl_rect(26,480,132,40,0x14283d);bl_text(43,493,2,paused?"RESUME":"PAUSE",0x64ffe1);bl_rect(177,480,138,40,0x14283d);bl_text(201,493,2,"RESET",0x64ffe1);
 bl_text(26,540,1,"COLOR FX - STEER",0x8fa1b5);
 bl_rect(126,560,84,38,0x14283d);bl_text(148,571,2,"UP",0x64ffe1);
 bl_rect(26,608,84,38,0x14283d);bl_text(40,619,2,"LEFT",0x64ffe1);
 bl_rect(226,608,88,38,0x14283d);bl_text(237,619,2,"RIGHT",0x64ffe1);
 bl_rect(126,656,84,38,0x14283d);bl_text(141,667,2,"DOWN",0x64ffe1);
 bl_rect(126,608,84,38,0x763044);bl_text(139,621,1,"BOOST",0xffffff);
 bl_rect(26,721,289,47,0x763044);bl_text(106,734,3,"MENU",0xffffff);
 const int x0=394,y0=114;
 bl_text(x0,62,3,bl_arenas[g->arena],0x64ffe1);bl_text(x0,91,1,"AVOID WALLS AND EVERY TRAIL - NO U-TURNS",0x8fa1b5);
 bl_rect(x0-4,y0-4,808,616,0x25445b);bl_rect(x0,y0,800,608,0x0b1220);
 for(int x=0;x<=800;x+=32)bl_rect(x0+x,y0,1,608,0x132033);for(int y=0;y<=608;y+=32)bl_rect(x0,y0+y,800,1,0x132033);
 for(int y=0;y<BL_H;y++)for(int x=0;x<BL_W;x++){
  int v=g->grid[y*BL_W+x];if(!v)continue;int px=x0+x*8,py=y0+y*8;
  if(v==5){bl_rect(px,py,8,8,0x3a5067);bl_rect(px+1,py+1,6,6,0x18283c);continue;}
  unsigned color=bl_colors[v-1];bl_rect(px,py,8,8,0x14283d);bl_rect(px+2,py+2,4,4,color);
  if(g->links[y*BL_W+x]&4)bl_rect(px,py+2,4,4,color);
  if(g->links[y*BL_W+x]&1)bl_rect(px+4,py+2,4,4,color);
  if(g->links[y*BL_W+x]&8)bl_rect(px+2,py,4,4,color);
  if(g->links[y*BL_W+x]&2)bl_rect(px+2,py+4,4,4,color);
 }
 for(int i=0;i<4;i++){struct bl_rider*r=&g->rider[i];int x=x0+r->x*8+4,y=y0+r->y*8+4;
  if(r->alive){bl_rect(x-5,y-5,10,10,bl_colors[i]);bl_rect(x-2,y-2,4,4,0xffffff);bl_rect(x+bl_dx[r->dir]*6-2,y+bl_dy[r->dir]*6-2,4,4,0xffffff);}
  else if(r->crash){int size=3+(45-r->crash)/3;bl_rect(x-size,y-1,2*size,2,bl_colors[i]);bl_rect(x-1,y-size,2,2*size,bl_colors[i]);}
 }
 bl_text(x0,753,2,g->rider[0].alive?"A / SPACE - BOOST":"CRASHED - WATCH THE ROUND",0x8fa1b5);
 bl_number(1110,753,2,g->survival/60,0xffffff);bl_text(1150,753,2,"SEC",0x7796aa);
 if(paused||g->state!=BL_PLAY){
  bl_rect(504,332,580,150,0x10243c);bl_rect(504,332,580,3,0x64ffe1);
  if(paused){bl_text(538,362,4,"PAUSED",0x64ffe1);bl_text(538,423,2,"P / TOUCH RESUME",0xffffff);}
  else if(g->state==BL_COUNTDOWN){bl_text(538,356,3,"GET READY",0x64ffe1);bl_number(938,354,5,(g->countdown+59)/60,0xffffff);bl_text(538,421,2,"TURN WITH COLOR FX / ARROWS",0x8fa1b5);}
  else{bl_text(538,356,3,g->state==BL_MATCH?(g->winner==0?"YOU WIN THE MATCH":"MATCH COMPLETE"):g->winner==0?"ROUND WON":g->winner<0?"DRAW":"ROUND LOST",g->winner>=0?bl_colors[g->winner]:0xffffff);
   if(g->winner>=0){bl_text(538,394,2,bl_names[g->winner],bl_colors[g->winner]);bl_text(645,394,2,"OWNS THE GRID",0xffffff);}
   bl_text(538,442,2,g->state==BL_MATCH?"PRESS A TO PLAY AGAIN":"PRESS A FOR NEXT ROUND",0x8fa1b5);}
 }
}
