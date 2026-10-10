/* Shared software renderer: RX3 palette/font, original Equalizarr visuals. */
static const unsigned eq_colors[]={0x64ffe1,0xff6278,0xe0a7ff,0xffcd55,0x66c8ff,0x95ef8d,0xffac64,0xffffff};
static void eq_number(int x,int y,int scale,unsigned v,unsigned color){char a[16];int n=0;do{a[n++]='0'+v%10;v/=10;}while(v);char b[16];for(int i=0;i<n;i++)b[i]=a[n-i-1];b[n]=0;eq_text(x,y,scale,b,color);}
static void eq_orb(int x,int y,unsigned color){eq_rect(x-4,y-7,8,14,color);eq_rect(x-7,y-4,14,8,color);eq_rect(x-3,y-4,4,4,0xffffff);}
static void eq_render(struct eq_game*g,int paused){
 eq_rect(0,0,1280,800,0x080f1b);eq_rect(24,30,294,3,0x64ffe1);
 eq_text(24,54,4,"EQUALIZARR",0x64ffe1);eq_text(26,97,2,"BREAK THE FREQUENCY",0x8fa1b5);
 eq_text(26,144,2,"SCORE",0x8fa1b5);eq_number(26,173,4,g->score,0xffffff);
 eq_text(26,221,2,"LEVEL",0x8fa1b5);eq_number(112,217,3,g->level+1,0xffcd55);eq_text(158,217,3,"/ 50",0xffcd55);
 eq_text(26,256,2,eq_level_names[g->level],0x64ffe1);
 eq_text(26,283,2,"HIGH SCORE",0x7796aa);eq_number(174,283,2,g->best,0xffcd55);
 eq_text(26,303,2,"LIVES",0x8fa1b5);for(int i=0;i<g->lives;i++)eq_rect(110+i*20,304,15,10,0xff6278);
 eq_text(26,346,2,"POWER UPS",0x8fa1b5);
 int line=0;for(int i=0;i<EQ_POWERS;i++)if(g->power[i]){eq_text(26,378+line*30,2,eq_power_names[i],eq_colors[i]);eq_number(264,378+line*30,2,i==EQ_SHIELD?g->power[i]:(g->power[i]+59)/60,eq_colors[i]);line++;}
 if(!line)eq_text(26,380,2,"CATCH FALLING BONUSES",0x7796aa);
 eq_text(26,616,1,"X STEEL - CLEAR COLORED BRICKS",0x7796aa);
 eq_text(26,559,2,"NOISE / FILTER - MOVE",0x8fa1b5);eq_text(26,590,2,"BEAT FX - LAUNCH / FIRE",0x8fa1b5);
 eq_rect(26,643,132,40,0x14283d);eq_text(43,656,2,paused?"RESUME":"PAUSE",0x64ffe1);
 eq_rect(177,643,138,40,0x14283d);eq_text(201,656,2,"RESET",0x64ffe1);
 eq_rect(26,711,289,57,0x763044);eq_text(106,727,3,"MENU",0xffffff);
 const int x0=382,y0=80;
 eq_rect(x0-4,y0-4,848,688,0x25445b);eq_rect(x0,y0,840,680,0x0b1220);
 // Sparse static grid and stage meter; no per-pixel noise or expensive effects.
 for(int x=0;x<840;x+=EQ_DX)eq_rect(x0+x,y0,1,680,0x132033);
 for(int y=0;y<680;y+=EQ_DY)eq_rect(x0,y0+y,840,1,0x132033);
 eq_text(x0+22,y0+20,2,eq_level_names[g->level],0x7796aa);
 eq_text(1016,101,2,"BRICKS",0x7796aa);eq_number(1110,101,2,g->remaining,0xffffff);
 for(int n=0;n<EQ_CELLS;n++)if(g->brick[n]){
  int x=x0+EQ_BX+n%EQ_COLS*EQ_DX,y=y0+EQ_BY+n/EQ_COLS*EQ_DY;
  unsigned color=g->kind[n]==1?0xffac64:eq_colors[(n%EQ_COLS/3+n/EQ_COLS+g->level)%7];
  if(g->kind[n]==2){
   eq_rect(x,y,EQ_BW,EQ_BH,0x62738b);eq_rect(x+3,y+3,EQ_BW-6,EQ_BH-6,0x263141);
   eq_rect(x+5,y+5,3,3,0xc1d2e3);eq_rect(x+EQ_BW-8,y+EQ_BH-8,3,3,0xc1d2e3);
   eq_text(x+16,y+7,2,"X",0x9aaabd);continue;
  }
  eq_rect(x,y+3,EQ_BW,EQ_BH,0x05070f);eq_rect(x,y,EQ_BW,EQ_BH,0x25445b);
  eq_rect(x,y,EQ_BW,3,color);eq_rect(x,y,3,EQ_BH,color);eq_rect(x+4,y+4,EQ_BW-8,EQ_BH-8,color);
  for(int j=1;j<g->brick[n];j++)eq_rect(x+13*j,y+9,3,10,0x080f1b);
  if(g->kind[n]==1)eq_text(x+16,y+7,2,"+",0x080f1b);
 }
 if(g->power[EQ_SHIELD]){eq_rect(x0+6,y0+EQ_H-16,828,3,0x64ffe1);for(int x=0;x<828;x+=20)eq_rect(x0+6+x,y0+EQ_H-12,12,2,0x25445b);}
 for(int i=0;i<EQ_PARTICLES;i++)if(g->particle[i].life){struct eq_particle*p=&g->particle[i];if(p->x>0&&p->x<840&&p->y>0&&p->y<680)eq_rect(x0+(int)p->x,y0+(int)p->y,3,3,p->color);}
 for(int i=0;i<EQ_DROPS;i++)if(g->drop[i].active){struct eq_drop*d=&g->drop[i];int x=x0+(int)d->x,y=y0+(int)d->y;eq_rect(x-13,y-11,26,22,eq_colors[d->kind]);eq_rect(x-11,y-9,22,18,0x10243c);eq_text(x-5,y-7,2,eq_power_letters[d->kind],eq_colors[d->kind]);}
 for(int i=0;i<EQ_SHOTS;i++)if(g->shot[i].active){eq_rect(x0+(int)g->shot[i].x-2,y0+(int)g->shot[i].y,4,14,0xff6278);eq_rect(x0+(int)g->shot[i].x-1,y0+(int)g->shot[i].y,2,10,0xffffff);}
 int px=x0+(int)g->paddle,py=y0+EQ_PY,half=eq_half(g);
 eq_rect(px-half-3,py-8,half*2+6,23,0x25445b);eq_rect(px-half,py-7,half*2,18,g->power[EQ_STICKY]?0x95ef8d:0x64ffe1);eq_rect(px-half+10,py-3,half*2-20,10,0x10243c);
 for(int i=-3;i<=3;i++)eq_rect(px+i*12,py-2,6,6,0xffffff);
 if(g->power[EQ_LASER]){eq_rect(px-half+3,py-15,8,18,0xff6278);eq_rect(px+half-11,py-15,8,18,0xff6278);}
 for(int i=0;i<EQ_BALLS;i++)if(g->ball[i].active){struct eq_ball*b=&g->ball[i];for(int j=3;j>=0;j--)if(b->trail_y[j]>0&&b->trail_y[j]<680)eq_rect(x0+(int)b->trail_x[j]-2,y0+(int)b->trail_y[j]-2,4,4,g->power[EQ_FIREBALL]?0x763044:0x25445b);eq_orb(x0+(int)b->x,y0+(int)b->y,g->power[EQ_FIREBALL]?0xffac64:0xffffff);}
 if(g->notice){eq_rect(550,542,500,43,0x10243c);eq_text(590,556,2,eq_power_names[g->notice_kind],eq_colors[g->notice_kind]);}
 if(g->ball[0].stuck&&!paused&&!g->over&&!g->won&&!g->transition)eq_text(560,665,2,"PRESS A / SPACE TO LAUNCH",0x8fa1b5);
 if(paused||g->over||g->won||g->transition){eq_rect(505,440,600,104,0x10243c);eq_rect(505,440,600,3,0x64ffe1);eq_text(535,464,3,paused?"PAUSED":g->won?"ALL 50 LEVELS CLEARED!":g->over?"GAME OVER":"LEVEL COMPLETE",0x64ffe1);eq_text(535,507,2,g->won||g->over?"PRESS A TO PLAY AGAIN":g->transition?"NEXT FREQUENCY INCOMING":"PRESS P / TOUCH RESUME",0xffffff);}
}
