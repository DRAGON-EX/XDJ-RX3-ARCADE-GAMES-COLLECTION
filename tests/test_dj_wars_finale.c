#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../Games/Native/DJ Wars/game.h"
#include "../native/font.h"
#include "../Games/Native/DJ Wars/render.h"
static struct dw_game g;
static unsigned pixels[1280*800];
static int render_sequence;
static unsigned terrain_hash(void){unsigned h=2166136261u;for(unsigned i=0;i<sizeof(g.terrain);i++)h=(h^g.terrain[i])*16777619u;return h;}
static void killed(int i){g.unit[i].hp=0;g.grave[i]=(struct dw_grave){g.unit[i].x,g.unit[i].y,0,1};}
static void advance(int tick){while(g.finale_tick<tick){dw_tick(&g,255);if(render_sequence&&g.finale_tick%4==0)dw_render(&g,pixels,0,0);}}
static void save_frame(const char*folder,const char*name){
 if(!folder)return;
 dw_render(&g,pixels,0,0);
 char path[1024];snprintf(path,sizeof(path),"%s/%s.ppm",folder,name);FILE*f=fopen(path,"wb");assert(f);
 fprintf(f,"P6\n1280 800\n255\n");for(int i=0;i<1280*800;i++){unsigned char c[3]={pixels[i]>>16,pixels[i]>>8,pixels[i]};fwrite(c,1,3,f);}fclose(f);
}
static void assert_revived(void){for(int i=0;i<5;i++){assert(g.unit[i].hp==g.unit[i].max_hp);assert(!g.grave[i].live);assert(!g.unit[i].fall&&!g.unit[i].vy&&!g.unit[i].blast_attack);}assert(dw_health(&g,0)==540);}
static void test_win(const char*folder){
 dw_reset(&g,71);for(int i=5;i<10;i++)killed(i);killed(0);killed(2);g.unit[0].y=DW_H+50;g.unit[1].hp=21;
 g.hit_pause=2;dw_tick(&g,0);assert(g.state!=DW_FINALE);dw_tick(&g,0);assert(g.state!=DW_FINALE);
 g.shot[0].live=1;g.jet_ticks=10;g.charging=1;dw_tick(&g,255);
 assert(g.state==DW_FINALE&&g.winner==-1&&!g.finale_assisted&&g.finale_result==0);assert(!g.charging&&!g.jet_ticks&&!g.shot[0].live);
 int remaining=g.remaining,turn=g.turn,power=g.power;unsigned terrain=g.terrain_version;
 advance(179);assert(g.unit[0].hp==0&&g.grave[0].live);assert(g.state==DW_FINALE&&g.winner==-1);
 advance(180);assert_revived();assert(g.unit[0].y<DW_H-24);assert(g.event==12);
 save_frame(folder,"dj-wars-victory-reunion");
 dw_choose_weapon(&g,2);dw_fire(&g);assert(!g.shot[0].live);dw_check_end(&g);assert(g.finale_tick==180);
 advance(720);assert(g.state==DW_FINALE&&g.winner==-1); /* Keep celebrating past the old 12-second boundary. */
 advance(2699);assert(g.state==DW_FINALE);
 advance(DW_VICTORY_TICKS-1);assert(g.state==DW_FINALE&&g.winner==-1);
 dw_tick(&g,255);assert(g.state==DW_OVER&&g.winner==0&&!g.finale_assisted);
 assert(g.remaining==remaining&&g.turn==turn&&g.power==power&&g.terrain_version==terrain);
 assert_revived();save_frame(folder,"dj-wars-victory-complete");
 for(int i=0;i<120;i++)dw_tick(&g,255);assert(g.state==DW_OVER&&g.finale_tick==DW_VICTORY_TICKS);
}
static void test_assist(const char*folder,int draw){
 dw_reset(&g,91);for(int i=0;i<5;i++)killed(i);if(draw)for(int i=5;i<10;i++)killed(i);
 dw_tick(&g,255);assert(g.state==DW_FINALE&&g.winner==-1&&g.finale_assisted&&g.finale_result==(draw?2:1));assert(g.event==11);
 int timer=g.remaining,turn=g.turn;unsigned terrain=g.terrain_version,hash=terrain_hash();
 float original_x[DW_UNITS],original_y[DW_UNITS];for(int i=0;i<DW_UNITS;i++){original_x[i]=g.unit[i].x;original_y[i]=g.unit[i].y;}
 advance(120);assert(g.event==15);if(!draw)save_frame(folder,"dj-wars-master-arrival");
 advance(DW_REVIVE_TICK-1);assert(!dw_alive(&g,0));
 advance(DW_REVIVE_TICK);assert_revived();assert(g.event==12);for(int i=0;i<5;i++){assert(g.unit[i].x==original_x[i]&&g.unit[i].y==original_y[i]);}if(!draw)save_frame(folder,"dj-wars-master-revival");
 advance(DW_BARRAGE_TICK-1);assert(dw_alive(&g,1)==(draw?0:5));
 for(int j=0;j<5;j++){
  advance(DW_BARRAGE_TICK+j*DW_BARRAGE_INTERVAL);
  assert(g.unit[5+j].hp==0&&g.winner==-1&&g.state==DW_FINALE);
  assert(g.finale_shots==(draw?0:j+1));assert_revived();
  if(!draw){assert(g.finale_target==5+j&&g.event==13);if(j==2)save_frame(folder,"dj-wars-master-precision");if(folder){char name[80];snprintf(name,sizeof(name),"dj-wars-master-shot-%d",j+1);save_frame(folder,name);}}
 }
 advance(DW_CELEBRATE_TICK);assert(g.event==14&&!dw_alive(&g,1));assert(g.winner==-1);
 if(!draw)save_frame(folder,"dj-wars-master-fireworks");
 advance(DW_CELEBRATE_TICK+720);assert(g.state==DW_FINALE&&g.winner==-1);
 advance(DW_CELEBRATE_TICK+2699);assert(g.state==DW_FINALE);
 advance(DW_ASSIST_TICKS-1);assert(g.state==DW_FINALE&&g.winner==-1);
 dw_tick(&g,255);assert(g.state==DW_OVER&&g.winner==0&&g.finale_assisted);
 assert(g.remaining==timer&&g.turn==turn&&g.terrain_version==terrain&&terrain_hash()==hash);assert_revived();
 for(int i=5;i<10;i++)assert(g.unit[i].x==original_x[i]&&g.unit[i].y==original_y[i]);
 assert(g.finale_x>=0&&g.finale_x<DW_W&&g.finale_y>0&&g.finale_y<DW_H);
 if(!draw)save_frame(folder,"dj-wars-assisted-complete");
 dw_reset(&g,55);assert(g.state==DW_AIM&&g.winner==-1&&!g.finale_tick&&!g.finale_assisted&&g.finale_result==-1&&g.finale_target==-1&&!g.finale_shots);assert(dw_alive(&g,0)==5&&dw_alive(&g,1)==5);
}
static void test_partial_enemies(void){
 dw_reset(&g,41);for(int i=0;i<5;i++)killed(i);killed(5);killed(7);dw_tick(&g,0);
 advance(DW_BARRAGE_TICK);assert(g.finale_target==-1&&!g.finale_shots&&dw_finale_beam_target(&g)==-1);
 advance(DW_BARRAGE_TICK+DW_BARRAGE_INTERVAL);assert(g.finale_target==6&&g.finale_shots==1&&dw_finale_beam_target(&g)==6);
 advance(DW_BARRAGE_TICK+2*DW_BARRAGE_INTERVAL);assert(g.finale_target==6&&g.finale_shots==1&&dw_finale_beam_target(&g)==-1); /* No repeated hit on an already dead target. */
 advance(DW_ASSIST_TICKS);
 assert(g.state==DW_OVER&&g.winner==0&&g.finale_shots==3);assert_revived();
}
static void test_drowned_revival(void){
 dw_reset(&g,42);for(int i=0;i<5;i++)killed(i);
 g.unit[1].y=DW_H+40;g.grave[1].y=DW_H-24;float x=g.unit[1].x;
 unsigned hash=terrain_hash();dw_tick(&g,0);advance(DW_REVIVE_TICK);
 assert_revived();assert(g.unit[1].x==x&&g.unit[1].y<DW_H-24);
 assert(dw_solid(&g,(int)x,(int)g.unit[1].y)&&!dw_solid(&g,(int)x,(int)g.unit[1].y-24));
 assert(terrain_hash()==hash);
}
int main(int argc,char**argv){
 const char*folder=argc>1?argv[1]:0;render_sequence=folder!=0;
 test_win(folder);test_assist(folder,0);test_assist(folder,1);test_partial_enemies();test_drowned_revival();
 puts("DJ Wars finales passed: delayed unlock, revival, 45-second victory celebration, 58.5-second assisted rescue, precise per-enemy shots, draw, partial enemies, reset and frozen controls.");return 0;
}
