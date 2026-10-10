#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../native/font.h"
#include "../Games/Native/DJ Wars/game.h"
#include "../Games/Native/DJ Wars/render.h"
static struct dw_game game;
static unsigned actual[1280*800],reference[1280*800];
static int check(int x,int y,int overview,int neon){
 dw_manual=1;dw_camera_x=x;dw_camera_y=y;dw_overview=overview;dw_camera_active=game.active;dw_camera_state=game.state;
 dw_render(&game,actual,0,neon);
 dw_cached_version=0; /* Reference redraws the entire static scene. */
 dw_render(&game,reference,0,neon);
 for(int i=0;i<1280*800;i++)if(actual[i]!=reference[i]){printf("Cache mismatch camera=%d,%d overview=%d neon=%d pixel=%d,%d: %x != %x\n",x,y,overview,neon,i%1280,i/1280,actual[i],reference[i]);return 1;}
 return 0;
}
int main(void){
 dw_reset(&game,0x5833);game.event_age=0;
 const int points[][2]={{0,0},{3,1},{20,13},{19,12},{25,9},{25,300},{530,310},{535,305},{880,700},{881,701},{870,692},{10,2},{879,699},{1759,899},{1760,900},{1735,899},{0,0},{1760,900}};
 for(unsigned i=0;i<sizeof(points)/sizeof(points[0]);i++)if(check(points[i][0],points[i][1],0,0))return 1;
 if(check(1200,600,0,1)||check(1207,605,0,1)||check(0,0,1,1)||check(0,0,1,0)||check(550,250,0,0))return 1;
 dw_explode(&game,800,500,76,42);if(check(556,254,0,0)||check(562,250,0,0))return 1;
 for(int i=0;i<80;i++){game.ticks++;if(check((i*137)%1761,(i*79)%901,0,i%19==0))return 1;}
 printf("DJ Wars scrolling cache matches full redraw: pan directions, jumps, edges, zoom, theme and crater changes\n");return 0;
}
