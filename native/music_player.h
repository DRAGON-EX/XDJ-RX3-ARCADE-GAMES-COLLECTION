#ifndef RX3_MUSIC_PLAYER_H
#define RX3_MUSIC_PLAYER_H
/* Metadata is immutable while audio runs; commands/state cross threads atomically. */
#include "music_progress.h"
#define MUSIC_MAX_TRACKS 32
#define MUSIC_MAX_BYTES 80000000u
struct music_track { unsigned start,end;char title[80],artist[64];unsigned locked; };
static struct music_track music_tracks[MUSIC_MAX_TRACKS];static unsigned music_count;static const char*music_index_path="/tmp/rx3-launcher/payload/music-index.tsv";
static unsigned music_position,music_seek_request=~0u,music_paused,music_level=100;
static int music_remaining,music_selected,music_browser_page;
/* Café playback is a scoped RAM-only override, never a new playlist selection. */
static unsigned music_cafe_mode,music_cafe_track,music_cafe_saved_position,music_cafe_saved_pause;

static unsigned char *music_cover;static int music_cover_track=-1;
static void music_copy(char*d,unsigned cap,const char*s){unsigned i=0;for(;s[i]&&i+1<cap;i++){char c=s[i];d[i]=c>='a'&&c<='z'?c-32:c>=32&&c<=126?c:' ';}d[i]=0;}
static unsigned music_parse_uint(const char*s){unsigned n=0;for(;*s>='0'&&*s<='9';s++){if(n>200000000)return ~0u;n=n*10+*s-'0';}return n;}
static void music_catalog_load(void){
 music_count=0;int fd=open(music_index_path,0);if(fd<0)return;char data[8192];long n=read(fd,data,sizeof(data)-1);close(fd);if(n<=0)return;data[n]=0;char*p=data;
 while(*p&&music_count<MUSIC_MAX_TRACKS){char*field[5]={p,0,0,0,0};int count=1;while(*p&&*p!='\n'){if(*p=='\t'&&count<5){*p=0;field[count++]=p+1;}p++;}if(*p)*p++=0;if(count<4)continue;unsigned locked=count==5?music_parse_uint(field[4]):0;if(locked>3)continue;
  unsigned start=music_parse_uint(field[0]),end=music_parse_uint(field[1]);if((locked==1?start!=end:locked>=2?start>end:start>=end)||end>MUSIC_MAX_BYTES||start%2||end%2||(music_count&&start!=music_tracks[music_count-1].end)||(!music_count&&start))continue;
  struct music_track*t=&music_tracks[music_count++];t->start=start;t->end=end;t->locked=locked;music_copy(t->title,sizeof(t->title),field[2]);music_copy(t->artist,sizeof(t->artist),field[3]);
 }
}
static int music_track_locked(unsigned i){return music_tracks[i].locked==1||(music_tracks[i].locked==2&&!__atomic_load_n(&music_secret_unlocked,__ATOMIC_ACQUIRE))||(music_tracks[i].locked==3&&!__atomic_load_n(&session_dj_wars_won,__ATOMIC_ACQUIRE));}
static int music_audio_allowed(unsigned i){return music_tracks[i].end>music_tracks[i].start&&(!music_track_locked(i)||(music_tracks[i].locked==2&&__atomic_load_n(&music_credits_mode,__ATOMIC_ACQUIRE)));}
/* Clamp each read at a track boundary and skip locked PCM, including direct seeks. */
static unsigned music_audio_next(unsigned pos,unsigned total,unsigned*end){
 if(!music_count){*end=total;return pos>=total?0:pos;}
 if(__atomic_load_n(&music_cafe_mode,__ATOMIC_ACQUIRE)){
  unsigned i=__atomic_load_n(&music_cafe_track,__ATOMIC_RELAXED);
  if(i<music_count&&music_tracks[i].end>music_tracks[i].start){*end=music_tracks[i].end;return pos>=music_tracks[i].start&&pos<*end?pos:music_tracks[i].start;}
 }

 if(__atomic_load_n(&music_credits_mode,__ATOMIC_ACQUIRE))for(unsigned i=0;i<music_count;i++)if(music_tracks[i].locked==2&&music_audio_allowed(i)){*end=music_tracks[i].end;return pos>=music_tracks[i].start&&pos<*end?pos:music_tracks[i].start;}
 unsigned first=0;while(first<music_count&&pos>=music_tracks[first].end)first++;
 for(unsigned n=0;n<music_count;n++){unsigned i=(first+n)%music_count;if(music_audio_allowed(i)){*end=music_tracks[i].end;return pos>=music_tracks[i].start&&pos<*end?pos:music_tracks[i].start;}}
 *end=0;return 0;
}
static unsigned music_current(void){unsigned p=__atomic_load_n(&music_position,__ATOMIC_RELAXED);for(unsigned i=0;i<music_count;i++)if(p<music_tracks[i].end)return i;return 0;}
static void music_time(char*b,unsigned cap,unsigned frames,int negative){unsigned ms=(unsigned)((uint64_t)frames*1000/44100);snprintf(b,cap,"%s%02u:%02u.%03u",negative?"-":"",ms/60000,ms/1000%60,ms%1000);}
static void music_seek(unsigned frame){__atomic_store_n(&music_seek_request,frame&~1u,__ATOMIC_RELEASE);}
static int music_cafe_begin(unsigned saved_position,unsigned saved_pause){
 for(unsigned i=0;i<music_count;i++)if(!strcmp(music_tracks[i].title,"PIXEL PLAINS")&&music_tracks[i].end>music_tracks[i].start){
  if(!__atomic_load_n(&music_cafe_mode,__ATOMIC_ACQUIRE)){music_cafe_saved_position=saved_position;music_cafe_saved_pause=saved_pause;}
  __atomic_store_n(&music_cafe_track,i,__ATOMIC_RELAXED);
  __atomic_store_n(&music_cafe_mode,1,__ATOMIC_RELEASE);music_seek(music_tracks[i].start);__atomic_store_n(&music_paused,0,__ATOMIC_RELAXED);return 1;
 }
 return 0;
}
static void music_cafe_end(void){
 if(!__atomic_load_n(&music_cafe_mode,__ATOMIC_ACQUIRE))return;
 music_seek(music_cafe_saved_position);__atomic_store_n(&music_paused,music_cafe_saved_pause,__ATOMIC_RELAXED);__atomic_store_n(&music_cafe_mode,0,__ATOMIC_RELEASE);
}
static void music_choose(int index){if(!music_count)return;if(index<0)index=(int)music_count-1;if(index>=(int)music_count)index=0;if(music_track_locked((unsigned)index)||music_tracks[index].start==music_tracks[index].end)return;music_selected=index;music_browser_page=index/4;music_seek(music_tracks[index].start);__atomic_store_n(&music_paused,0,__ATOMIC_RELAXED);}
static void music_adjacent(int current,int step){if(!music_count)return;for(unsigned n=0;n<music_count;n++){current=(current+step+(int)music_count)%(int)music_count;if(!music_track_locked((unsigned)current)&&music_tracks[current].start<music_tracks[current].end){music_choose(current);return;}}}
static void music_browse(int steps,int click){if(!music_count)return;if(steps){music_selected=(music_selected+steps)%(int)music_count;if(music_selected<0)music_selected+=music_count;music_browser_page=music_selected/4;}if(click)music_choose(music_selected);}
static void music_cover_release(void){free(music_cover);music_cover=0;music_cover_track=-1;}
static void music_cover_load(unsigned track){if(music_cover_track==(int)track)return;music_cover_release();music_cover_track=(int)track;char path[100];snprintf(path,sizeof(path),"/tmp/rx3-launcher/payload/music-cover-%u.rgb",track);int fd=open(path,0);if(fd<0)return;music_cover=malloc(180*180*3);unsigned got=0;while(music_cover&&got<180*180*3){long n=read(fd,music_cover+got,180*180*3-got);if(n<=0)break;got+=(unsigned)n;}close(fd);if(got!=180*180*3){free(music_cover);music_cover=0;}}
extern long lseek(int,long,int);
static const char*music_animation_path="/tmp/rx3-launcher/payload/no-cover-cat.rgb";
static int music_animation_fd=-1,music_animation_frame=-1,music_animation_failed;
static unsigned char*music_animation_pixels;static uint64_t music_animation_start;
static void music_animation_release(void){if(music_animation_fd>=0)close(music_animation_fd);music_animation_fd=-1;free(music_animation_pixels);music_animation_pixels=0;music_animation_frame=-1;music_animation_failed=0;}
static int music_animation_draw(void){
 if(!(session_rescued_cats&1u)){music_animation_release();return 0;}
 if(music_animation_failed)return 0;
 if(music_animation_fd<0){
  music_animation_fd=open(music_animation_path,0);if(music_animation_fd<0){music_animation_failed=1;return 0;}
  if(lseek(music_animation_fd,0,2)!=64*180*180*3){music_animation_release();music_animation_failed=1;return 0;}
  music_animation_pixels=malloc(180*180*3);if(!music_animation_pixels){music_animation_release();music_animation_failed=1;return 0;}
  music_animation_start=ns();
 }
 int frame=(int)(((ns()-music_animation_start)/60000000ULL)%64);
 if(frame!=music_animation_frame){
  if(lseek(music_animation_fd,frame*(180*180*3),0)<0)goto failed;
  unsigned got=0;while(got<180*180*3){long n=read(music_animation_fd,music_animation_pixels+got,180*180*3-got);if(n<=0)goto failed;got+=(unsigned)n;}
  music_animation_frame=frame;
 }
 for(int y=0;y<180;y++)for(int x=0;x<180;x++){const unsigned char*c=music_animation_pixels+3*(y*180+x);pixels[(235+y)*width+110+x]=(c[0]<<16)|(c[1]<<8)|c[2];}
 return 1;
failed:music_animation_release();music_animation_failed=1;return 0;
}
static void music_label(int x,int y,int scale,const char*s,unsigned limit,uint32_t color){char b[90];unsigned i=0;for(;s[i]&&i<limit&&i<sizeof(b)-1;i++)b[i]=s[i];b[i]=0;text(x,y,scale,b,color);}
static void draw_music_player(void){
 unsigned current=music_current(),position=__atomic_load_n(&music_position,__ATOMIC_RELAXED);struct music_track empty={0,0,"NO TRACKS AVAILABLE","IMPORT YOUR MUSIC",0};struct music_track*t=music_count?&music_tracks[current]:&empty;
 rect(110,235,180,180,0x14283d);music_cover_load(current);
 if(music_cover){music_animation_release();for(int y=0;y<180;y++)for(int x=0;x<180;x++){const unsigned char*c=music_cover+(y*180+x)*3;pixels[(235+y)*width+110+x]=(c[0]<<16)|(c[1]<<8)|c[2];}}
 else if(!music_animation_draw()){disk(200,320,65,0x080f1b);disk(200,320,46,0x243b50);disk(200,320,25,0x64ffe1);disk(200,320,6,0x080f1b);text(137,390,2,"NO COVER",0x8fa1b5);}
 text(325,242,2,"NOW PLAYING",0x64ffe1);music_label(325,280,2,t->title,35,0xffffff);music_label(325,320,2,t->artist,35,0x8fa1b5);
 text(325,365,2,__atomic_load_n(&music_paused,__ATOMIC_RELAXED)?"PAUSED":"PLAYING",0x64ffe1);char b[80];snprintf(b,sizeof(b),"TRACK %u / %u",music_count?current+1:0,music_count);text(325,393,2,b,0x8fa1b5);
 unsigned duration=t->end-t->start,elapsed=position>t->start?position-t->start:0;if(elapsed>duration)elapsed=duration;
 rect(110,453,645,12,0x243b50);int fill=duration?(int)((uint64_t)elapsed*645/duration):0;rect(110,453,fill,12,0x64ffe1);rect(107+fill,446,6,26,0xffffff);
 music_time(b,sizeof(b),music_remaining?duration-elapsed:elapsed,music_remaining);text(110,501,4,b,0xffffff);
 rect(555,490,200,48,0x14283d);text(570,506,2,music_remaining?"REMAINING":"ELAPSED",0x64ffe1);
 rect(110,568,90,62,0x14283d);text(127,591,2,"PREV",0x64ffe1);rect(215,568,270,62,0x64ffe1);text(270,588,3,__atomic_load_n(&music_paused,__ATOMIC_RELAXED)?"PLAY":"PAUSE",0x080f1b);rect(500,568,90,62,0x14283d);text(517,591,2,"NEXT",0x64ffe1);
 text(110,671,2,"VOLUME",0x8fa1b5);unsigned level=__atomic_load_n(&music_level,__ATOMIC_RELAXED);snprintf(b,sizeof(b),"%u%%",level);text(690,671,2,b,0x64ffe1);rect(225,675,425,10,0x243b50);rect(225,675,425*level/100,10,0x64ffe1);rect(222+425*level/100,667,6,26,0xffffff);
 text(805,242,2,"TRACK BROWSER",0x64ffe1);
 for(unsigned row=0;row<4;row++){unsigned i=music_browser_page*4+row;if(i>=music_count)break;int y=278+row*82;rect(805,y,365,72,0x14283d);if((int)i==music_selected){rect(805,y,3,72,0x64ffe1);rect(805,y,365,2,0x64ffe1);}if(music_tracks[i].locked>=2&&music_track_locked(i)){text(818,y+13,2,"SECRET SONG",0xff405c);text(818,y+43,2,music_tracks[i].locked==3?"WIN DJ WARS TO UNLOCK":"WATCH CREDITS TO UNLOCK",0x8fa1b5);}else{music_label(818,y+13,2,music_tracks[i].title,28,0xffffff);music_label(818,y+43,2,music_track_locked(i)?"NOT YET UNLOCKED":music_tracks[i].artist,23,0x8fa1b5);}if(i==current)rect(1155,y+46,5,10,0x64ffe1);}
 text(805,620,2,"BROWSE - PRESS TO PLAY",0x8fa1b5);rect(805,660,165,42,0x14283d);text(823,674,2,"PREVIOUS",0x64ffe1);rect(1005,660,165,42,0x14283d);text(1050,674,2,"NEXT",0x64ffe1);
}
static void music_touch(int x,int y){
 if(!music_count)return;unsigned current=music_current();struct music_track*t=&music_tracks[current];
 if(x>=110&&x<755&&y>=430&&y<482){music_seek(t->start+(unsigned)((uint64_t)(x-110)*(t->end-t->start)/645));return;}
 if(x>=555&&x<755&&y>=490&&y<538){music_remaining=!music_remaining;return;}
 if(y>=568&&y<630){if(x>=110&&x<200)music_adjacent((int)current,-1);else if(x>=215&&x<485)__atomic_fetch_xor(&music_paused,1,__ATOMIC_RELAXED);else if(x>=500&&x<590)music_adjacent((int)current,1);return;}
 if(x>=210&&x<=665&&y>=651&&y<710){int v=(x-225)*100/425;if(v<0)v=0;if(v>100)v=100;__atomic_store_n(&music_level,(unsigned)v,__ATOMIC_RELAXED);return;}
 if(x>=805&&x<1170){if(y>=278&&y<606){int row=(y-278)/82;if(row<4&&y<278+row*82+72){int i=music_browser_page*4+row;if(i<(int)music_count)music_choose(i);}}else if(y>=660&&y<702){int pages=(music_count+3)/4;music_browser_page+=(x<985?-1:1);if(music_browser_page<0)music_browser_page=0;if(music_browser_page>=pages)music_browser_page=pages-1;}}
}
#endif
