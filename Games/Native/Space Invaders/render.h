/* Software neon sprites; shared native and browser renderer. */
static const unsigned si_colors[]={0xe0a7ff,0x66c8ff,0x64ffe1,0xffcd55,0xffac64};
static const unsigned short si_sprites[3][2][8]={
 {{0x198,0x3fc,0x7fe,0xdbb,0xfff,0x666,0x924,0x408},{0x198,0x3fc,0x7fe,0xdbb,0xfff,0x666,0x492,0x204}},
 {{0x204,0x108,0x7fe,0xdbb,0xfff,0x5fa,0x492,0x906},{0x204,0x108,0x7fe,0xdbb,0xfff,0x5fa,0x294,0x462}},
 {{0x3fc,0x7fe,0xe07,0xdbb,0xfff,0x666,0x924,0x492},{0x3fc,0x7fe,0xe07,0xdbb,0xfff,0x666,0x492,0x924}}
};
static void si_number(int x,int y,int size,unsigned v,unsigned c){char a[16],b[16];int n=0;do{a[n++]='0'+v%10;v/=10;}while(v);for(int i=0;i<n;i++)b[i]=a[n-i-1];b[n]=0;si_text(x,y,size,b,c);}
static void si_alien(int x,int y,int kind,int anim,int armor,unsigned color){
 if(armor>1){si_rect(x-3,y-3,42,2,color);si_rect(x-3,y+25,42,2,color);}
 for(int r=0;r<8;r++)for(int c=0;c<12;c++)if(si_sprites[kind][anim][r]&(1<<(11-c))){si_rect(x+c*3,y+r*3,3,3,0x25445b);si_rect(x+c*3,y+r*3,2,2,color);}
}
static void si_render(struct si_game*g,int paused){
 si_rect(0,0,1280,800,0x080f1b);si_rect(24,27,294,3,0x64ffe1);si_text(24,49,4,"DROP",0x64ffe1);si_text(24,84,4,"DEFENSE",0x64ffe1);si_text(26,128,2,"DEFEND THE DANCEFLOOR",0x8fa1b5);
 si_text(26,183,2,"SCORE",0x7796aa);si_number(26,211,4,g->score,0xffffff);
 si_text(26,262,2,"HIGH",0x7796aa);si_number(127,260,3,g->best,0xffcd55);
 si_text(26,319,2,"WAVE",0x7796aa);si_number(127,315,3,g->wave,0x64ffe1);
 si_text(26,365,2,"LIVES",0x7796aa);for(int i=0;i<g->lives;i++){int x=123+i*29;si_rect(x,369,20,8,0x64ffe1);si_rect(x+7,364,6,5,0xffffff);}
 si_text(26,425,2,"HOLD A TO FIRE",0x64ffe1);si_text(26,463,1,"SHIELDS BREAK UNDER FIRE",0x8fa1b5);si_text(26,487,1,"UFO - MYSTERY BONUS",0x8fa1b5);si_text(26,511,1,"EXTRA LIFE AT 1500",0x8fa1b5);
 si_text(26,561,2,"NOISE / FILTER - MOVE",0x8fa1b5);si_text(26,591,2,"BEAT FX - FIRE",0x8fa1b5);
 si_rect(26,643,132,40,0x14283d);si_text(43,656,2,paused?"RESUME":"PAUSE",0x64ffe1);si_rect(177,643,138,40,0x14283d);si_text(201,656,2,"RESET",0x64ffe1);
 si_rect(26,711,289,57,0x763044);si_text(106,727,3,"MENU",0xffffff);
 const int x0=382,y0=80;
 si_rect(x0-4,y0-4,848,688,0x25445b);si_rect(x0,y0,840,680,0x0b1220);
 for(int i=0;i<58;i++){int x=(i*137+31)%830,y=(i*83+17)%670;si_rect(x0+x,y0+y,i%5?1:2,i%5?1:2,i%4?0x25445b:0x62738b);}
 si_rect(x0+10,y0+651,820,2,0x25445b);si_text(x0+20,y0+661,1,"KEEP THE INVADERS ABOVE YOUR LINE",0x7796aa);
 for(int i=0;i<SI_ALIENS;i++)if(g->alien[i]){int row=i/SI_COLS;si_alien(x0+g->formation_x+i%SI_COLS*58,y0+g->formation_y+row*39,row==0?0:row<3?1:2,(g->step/2)&1,g->alien[i],si_colors[row]);}
 if(g->ufo_active){int x=x0+(int)g->ufo_x,y=y0+32;si_rect(x-24,y,48,8,0xff6278);si_rect(x-17,y-6,34,6,0xe0a7ff);si_rect(x-8,y-10,16,4,0xff6278);for(int i=-16;i<=16;i+=8)si_rect(x+i,y+2,4,3,0xffffff);si_rect(x-16,y+8,6,4,0xffcd55);si_rect(x+10,y+8,6,4,0xffcd55);}
 if(g->ufo_notice){si_text(x0+20,y0+18,2,"UFO BONUS",0xff6278);si_number(x0+158,y0+18,2,g->ufo_points,0xffffff);}
 for(int b=0;b<4;b++)for(int y=0;y<SI_BUNKER_ROWS;y++)for(int x=0;x<SI_BUNKER_COLS;x++)if(g->bunker[b][y*SI_BUNKER_COLS+x]){int xx=x0+si_bunker_x[b]+x*5,yy=y0+500+y*5;si_rect(xx,yy,5,5,0x174841);si_rect(xx,yy,4,3,0x95ef8d);}
 if(!g->over&&(!g->invulnerable||(g->ticks/6)%2)){int x=x0+(int)g->player,y=y0+SI_PLAYER_Y;si_rect(x-22,y-3,44,14,0x25445b);si_rect(x-19,y-5,38,10,0x64ffe1);si_rect(x-11,y-10,22,7,0x64ffe1);si_rect(x-3,y-17,6,14,0xffffff);si_rect(x-12,y+6,6,4,0xffac64);si_rect(x+6,y+6,6,4,0xffac64);}
 for(int i=0;i<SI_PLAYER_SHOTS;i++)if(g->shot[i].active){int x=x0+(int)g->shot[i].x,y=y0+(int)g->shot[i].y;si_rect(x-3,y-5,6,12,0x25445b);si_rect(x-1,y-5,2,10,0xffffff);}
 for(int i=0;i<SI_ENEMY_SHOTS;i++)if(g->enemy[i].active){int x=x0+(int)g->enemy[i].x,y=y0+(int)g->enemy[i].y;si_rect(x-2,y-5,4,10,0xff6278);si_rect(x+((g->ticks/5)%2?-4:2),y-1,3,4,0xffcd55);}
 for(int i=0;i<48;i++){struct si_particle*p=&g->particle[i];if(p->life&&p->x>0&&p->x<840&&p->y>0&&p->y<680)si_rect(x0+(int)p->x,y0+(int)p->y,3,3,p->color);}
 if(paused||g->over||g->transition||g->life_pause){si_rect(505,402,600,125,0x10243c);si_rect(505,402,600,3,0x64ffe1);si_text(535,431,3,paused?"PAUSED":g->over?"GAME OVER":g->transition?"WAVE CLEARED":"SHIP LOST",0x64ffe1);si_text(535,485,2,paused?"P / TOUCH RESUME":g->over?"PRESS A TO DEFEND AGAIN":g->transition?"NEXT INVASION INCOMING":"GET READY - KEEP FIGHTING",0xffffff);}
}
