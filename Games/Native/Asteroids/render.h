#ifndef RX3_ASTEROIDS_RENDER_H
#define RX3_ASTEROIDS_RENDER_H
/* Shared neon renderer. Host provides ar_rect/ar_text; existing RX3 palette/font. */
static int ar_left=360,ar_top=70;
static void ar_plot(int x,int y,int w,int h,unsigned c){if(x<ar_left){w-=ar_left-x;x=ar_left;}if(y<ar_top){h-=ar_top-y;y=ar_top;}if(x+w>ar_left+AST_W)w=ar_left+AST_W-x;if(y+h>ar_top+AST_H)h=ar_top+AST_H-y;if(w>0&&h>0)ar_rect(x,y,w,h,c);}
static void ar_line(int x,int y,int a,int b,int weight,unsigned c){int dx=a>x?a-x:x-a,dy=b>y?b-y:y-b,sx=x<a?1:-1,sy=y<b?1:-1,e=dx-dy;for(;;){ar_plot(x-weight/2,y-weight/2,weight,weight,c);if(x==a&&y==b)break;int p=e*2;if(p>-dy){e-=dy;x+=sx;}if(p<dx){e+=dx;y+=sy;}}}
static void ar_polygon(int*xy,int count,unsigned fill,unsigned edge){int low=2000,high=-2000;for(int i=0;i<count;i++){int y=xy[2*i+1];if(y<low)low=y;if(y>high)high=y;}
 if(low<ar_top)low=ar_top;if(high>ar_top+AST_H)high=ar_top+AST_H;
 for(int y=low;y<high;y++){int hits[16],n=0;for(int i=0,j=count-1;i<count;j=i++){int ax=xy[2*j],ay=xy[2*j+1],bx=xy[2*i],by=xy[2*i+1];if((ay<=y&&by>y)||(by<=y&&ay>y))hits[n++]=ax+(y-ay)*(bx-ax)/(by-ay);}
 for(int i=1;i<n;i++){int v=hits[i],j=i;while(j&&hits[j-1]>v){hits[j]=hits[j-1];j--;}hits[j]=v;}for(int i=0;i+1<n;i+=2)ar_plot(hits[i],y,hits[i+1]-hits[i]+1,1,fill);}
 for(int i=0,j=count-1;i<count;j=i++)ar_line(xy[2*j],xy[2*j+1],xy[2*i],xy[2*i+1],3,edge);
}
static void ar_number(int x,int y,int scale,unsigned n,int digits,unsigned c){char label[16];label[digits]=0;for(int i=digits-1;i>=0;i--){label[i]='0'+n%10;n/=10;}ar_text(x,y,scale,label,c);}
static void ar_ship(float x,float y,int angle,int thrust,int invulnerable,unsigned t){int cx=ar_left+(int)x,cy=ar_top+(int)y,xy[8];const int a[]={0,102,128,154};const int r[]={20,16,7,16};for(int i=0;i<4;i++){xy[2*i]=cx+(int)(ast_sin(angle+a[i])*r[i]);xy[2*i+1]=cy-(int)(ast_sin(angle+a[i]+64)*r[i]);}
 if(invulnerable&&(t/7)%2)return;
 if(thrust){int len=28+(int)(t%13),tailx=cx-(int)(ast_sin(angle)*len),taily=cy+(int)(ast_sin(angle+64)*len);ar_line(cx,cy,tailx,taily,8,0x643c32);ar_line(cx,cy,tailx,taily,3,0xffcd55);}
 for(int i=0,j=3;i<4;j=i++)ar_line(xy[2*j],xy[2*j+1],xy[2*i],xy[2*i+1],7,0x243b50);ar_polygon(xy,4,0x375267,0x64ffe1);ar_line(cx,cy,cx+(int)(ast_sin(angle)*12),cy-(int)(ast_sin(angle+64)*12),2,0xe7fbff);
}
static void ar_rock(struct ast_body*b,int ox,int oy){int cx=ar_left+(int)b->x+ox,cy=ar_top+(int)b->y+oy,r=(int)ast_radius(b->size),xy[20];unsigned seed=b->shape;
 for(int i=0;i<10;i++){seed=seed*1664525u+1013904223u;int a=b->angle+i*256/10;float rr=r*(.70f+(seed>>28)*.02f);xy[2*i]=cx+(int)(ast_sin(a)*rr);xy[2*i+1]=cy-(int)(ast_sin(a+64)*rr);}
 unsigned edge=b->size==3?0x54bded:b->size==2?0xee8dd0:0xffcd55;for(int i=0,j=9;i<10;j=i++)ar_line(xy[2*j],xy[2*j+1],xy[2*i],xy[2*i+1],7,0x243b50);ar_polygon(xy,10,b->size==3?0x243b50:0x41314c,edge);int facet[]={xy[0],xy[1],xy[2],xy[3],xy[4],xy[5],cx+3,cy-2};ar_polygon(facet,4,0x42586b,0x465b70);
 ar_line(xy[0],xy[1],cx+3,cy-2,1,0x465b70);ar_line(cx+3,cy-2,xy[10],xy[11],1,0x465b70);ar_plot(cx-r/4,cy-r/4,r/5+2,r/5+2,0x080f1b);
}
static void ast_render(struct ast_game*g,int paused){ar_rect(0,0,1280,800,0x080f1b);ar_rect(346,56,908,728,0x243b50);ar_rect(349,59,902,722,0x080f1b);ar_rect(ar_left,ar_top,AST_W,AST_H,0x090d18);
 for(int i=0;i<110;i++){unsigned v=(unsigned)i*7919u+31;int x=ar_left+(int)(v%AST_W),y=ar_top+(int)((v*37u)%AST_H);ar_plot(x,y,i%9==0?2:1,i%9==0?2:1,i%9==0?0x65718a:0x253347);}
 for(int i=0;i<AST_ROCKS;i++)if(g->rocks[i].active){struct ast_body*b=&g->rocks[i];int r=(int)ast_radius(b->size);ar_rock(b,0,0);if(b->x<r)ar_rock(b,AST_W,0);if(b->x>AST_W-r)ar_rock(b,-AST_W,0);if(b->y<r)ar_rock(b,0,AST_H);if(b->y>AST_H-r)ar_rock(b,0,-AST_H);}
 for(int i=0;i<AST_PARTICLES;i++)if(g->particles[i].active){struct ast_body*p=&g->particles[i];unsigned c=p->size==1?0xff6278:p->size==2?0x64e8ee:0xffcd55;ar_plot(ar_left+(int)p->x,ar_top+(int)p->y,p->life>12?3:1,p->life>12?3:1,c);}
 for(int i=0;i<AST_SHOTS;i++)if(g->shots[i].active){struct ast_body*b=&g->shots[i];int x=ar_left+(int)b->x,y=ar_top+(int)b->y;ar_line(x,y,x-(int)b->vx,y-(int)b->vy,5,b->size?0x5e283b:0x294c51);ar_line(x,y,x-(int)b->vx,y-(int)b->vy,2,b->size?0xff6278:0xbaffed);ar_plot(x-1,y-1,3,3,0xffffff);}
 if(g->ufo.active){struct ast_body*b=&g->ufo;int x=ar_left+(int)b->x,y=ar_top+(int)b->y,r=b->size==1?14:24;int xy[]={x-r,y,x-r/2,y-r/3,x+r/2,y-r/3,x+r,y,x+r/2,y+r/3,x-r/2,y+r/3};ar_polygon(xy,6,0x2a1b33,0xee8dd0);ar_line(x-r/3,y-r/3,x-r/5,y-r/2,2,0x64e8ee);ar_line(x-r/5,y-r/2,x+r/4,y-r/2,2,0x64e8ee);ar_plot(x-2,y-1,4,3,0xffcd55);}
 if(g->ship.active){ar_ship(g->ship.x,g->ship.y,g->ship.angle,g->thrust,g->shield,g->ticks);if(g->ship.x<22)ar_ship(g->ship.x+AST_W,g->ship.y,g->ship.angle,g->thrust,g->shield,g->ticks);if(g->ship.x>AST_W-22)ar_ship(g->ship.x-AST_W,g->ship.y,g->ship.angle,g->thrust,g->shield,g->ticks);if(g->ship.y<22)ar_ship(g->ship.x,g->ship.y+AST_H,g->ship.angle,g->thrust,g->shield,g->ticks);if(g->ship.y>AST_H-22)ar_ship(g->ship.x,g->ship.y-AST_H,g->ship.angle,g->thrust,g->shield,g->ticks);}
 ar_text(30,35,6,"RX3",0x64ffe1);ar_text(30,90,3,"ARCADE",0xdce6ee);ar_text(30,140,4,"BASS ROCKS",0x64ffe1);
 ar_text(30,205,3,"SCORE",0x7796aa);ar_number(30,240,5,g->score,7,0xffffff);ar_text(30,302,2,"HIGH",0x7796aa);ar_number(100,302,2,g->high,7,0xffcd55);
 ar_text(30,355,3,"WAVE",0x7796aa);ar_number(165,355,3,g->wave,3,0xffcd55);ar_text(30,404,3,"LIVES",0x7796aa);ar_number(165,404,3,g->lives>0?(unsigned)g->lives:0,2,0x64ffe1);
 ar_rect(30,540,270,56,0x763044);ar_text(60,556,4,"MENU",0xffffff);
 ar_text(30,464,2,"FX A - FIRE",0x64ffe1);ar_text(30,493,2,"CRUSH / CUE - HYPER",0x8fa1b5);
 ar_rect(30,640,125,36,0x173044);ar_text(43,650,2,paused?"RESUME":"PAUSE",0x64ffe1);ar_rect(175,640,125,36,0x173044);ar_text(190,650,2,"RESTART",0x64ffe1);
 ar_text(30,707,2,"NOISE / FILTER - TURN",0x8fa1b5);ar_text(30,737,2,"DUB ECHO - THRUST",0x8fa1b5);
 ar_text(ar_left+12,ar_top+12,2,"NEON VECTOR FLIGHT",0x8fa1b5);
 if(g->wave_wait){ar_text(ar_left+250,ar_top+310,3,"NEXT WAVE",0x64ffe1);ar_number(ar_left+460,ar_top+355,5,g->wave+1,3,0xffcd55);}
 if(paused||g->game_over){ar_rect(ar_left+170,ar_top+265,540,130,0x14283d);ar_text(ar_left+205,ar_top+292,4,paused?"PAUSED":"GAME OVER",0x64ffe1);ar_text(ar_left+205,ar_top+353,2,g->game_over?"PRESS FX A OR RESTART":"TOUCH RESUME TO CONTINUE",0xffffff);}
}
#endif
