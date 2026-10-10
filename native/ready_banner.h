#ifndef RX3_READY_BANNER_H
#define RX3_READY_BANNER_H
#include "font.h"
#include "theme.h"
#include "dragon_logo.h"

struct ready_banner{int fd,x,y,w,h;struct fb_var v;struct fb_fix f;unsigned char*fb;uint16_t*saved,*paint;uint64_t start,next;int overlay,configured,old_blank;struct fb_var original;};
static uint16_t banner_color(struct ready_banner*b,uint32_t c){c=theme_color(c);return(uint16_t)((((c>>16)&255)>>(8-b->v.red.length))<<b->v.red.offset|(((c>>8)&255)>>(8-b->v.green.length))<<b->v.green.offset|((c&255)>>(8-b->v.blue.length))<<b->v.blue.offset);}
static void banner_rect(struct ready_banner*b,int x,int y,int w,int h,uint16_t c){for(int yy=y;yy<y+h&&yy<b->h;yy++)for(int xx=x;xx<x+w&&xx<b->w;xx++)if(xx>=0&&yy>=0)b->paint[yy*b->w+xx]=c;}
static void banner_text(struct ready_banner*b,int x,int y,int scale,const char*s,uint32_t color){uint16_t c=banner_color(b,color);for(;*s;s++,x+=6*scale){int n=*s>='0'&&*s<='9'?*s-'0':*s>='A'&&*s<='Z'?*s-'A'+10:-1;if(*s=='+'){banner_rect(b,x,y+3*scale,5*scale,scale,c);banner_rect(b,x+2*scale,y,scale,7*scale,c);continue;}if(*s=='-'){banner_rect(b,x,y+3*scale,5*scale,scale,c);continue;}if(n<0)continue;for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(font[n][yy]&(1<<(4-xx)))banner_rect(b,x+xx*scale,y+yy*scale,scale,scale,c);}}
static void banner_logo_scaled(struct ready_banner*b,int top,int size){
 for(int y=0;y<size;y++)for(int x=0;x<size;x++){
  int dx=22+x,dy=top+y;if(dx>=b->w||dy>=b->h)continue;
  const unsigned char*v=dragon_logo+4*((y*112/size)*112+x*112/size);unsigned a=v[3];uint16_t*d=b->paint+dy*b->w+dx;
  unsigned r=(v[0]*a+((*d>>11)*255/31)*(255-a))/255,g=(v[1]*a+(((*d>>5)&63)*255/63)*(255-a))/255,z=(v[2]*a+((*d&31)*255/31)*(255-a))/255;
  *d=(uint16_t)(((r>>3)<<11)|((g>>2)<<5)|(z>>3));if(!*d)*d=1;
 }
}
static void banner_logo(struct ready_banner*b){banner_logo_scaled(b,16,84);}
static uint16_t*banner_row(struct ready_banner*b,int y){return(uint16_t*)(b->fb+(y+b->y+b->v.yoffset)*b->f.line_length)+(b->x+b->v.xoffset);}
static void banner_restore_pixels(struct ready_banner*b){
 /* Restore only pixels still occupied by us; preserve the DJ application's writes. */
 for(int y=0;y<b->h;y++){uint16_t*r=banner_row(b,y);for(int x=0;x<b->w;x++)if(r[x]==b->paint[y*b->w+x])r[x]=b->saved[y*b->w+x];}
}
static void banner_close(struct ready_banner*b){
 if(!b->fb)return;
 if(b->overlay){ioctl(b->fd,0x4611,4);for(unsigned y=0;y<b->v.yres;y++){uint16_t*r=(uint16_t*)(b->fb+y*b->f.line_length);for(unsigned x=0;x<b->v.xres;x++)r[x]=0;}}
 else banner_restore_pixels(b);
 munmap(b->fb,b->f.smem_len);
 if(b->configured){ioctl(b->fd,0x4601,&b->original);ioctl(b->fd,0x4611,b->old_blank);}
 close(b->fd);free(b->saved);free(b->paint);b->fb=0;
}
static void banner_open_height(struct ready_banner*b,uint64_t now,int height){
 *b=(struct ready_banner){.fd=-1};
 /* Never paint onto the DJ application's background framebuffer. */
 int bg=open("/dev/fb0",0);struct fb_var display;
 if(bg<0)return;int rc=ioctl(bg,0x4600,&display);close(bg);if(rc||display.xres<800||display.xres>4096||display.yres<480||display.yres>2160)return;
 for(int i=1;i<=2;i++){
  char path[24];snprintf(path,sizeof(path),"/dev/fb%d",i);int fd=open(path,2);if(fd<0)continue;
  struct fb_fix fix;int blank=0;
  if(!ioctl(fd,0x4602,&fix)&&!ioctl(fd,0x8004462b,&blank)&&blank!=0){fix.id[15]=0;if(!strcmp(fix.id,"DISP3 FG")){b->fd=fd;b->old_blank=blank;break;}}
  close(fd);
 }
 if(b->fd<0){printf("Ready banner skipped: no unused DISP3 FG overlay\n");fflush(0);return;}
 if(ioctl(b->fd,0x4600,&b->original))goto failed;
 b->v=b->original;b->v.xres=display.xres;b->v.yres=display.yres;b->v.xres_virtual=display.xres;b->v.yres_virtual=display.yres;
 b->v.xoffset=b->v.yoffset=0;b->v.bpp=16;b->v.nonstd=b->v.grayscale=0;b->v.activate=0;
 b->v.red=(struct fb_bits){11,5,0};b->v.green=(struct fb_bits){5,6,0};b->v.blue=(struct fb_bits){0,5,0};b->v.transp=(struct fb_bits){0};
 b->configured=1;if(ioctl(b->fd,0x4601,&b->v)||ioctl(b->fd,0x4600,&b->v)||ioctl(b->fd,0x4602,&b->f))goto failed;
 if(b->v.bpp!=16||b->f.type||b->f.visual!=2||b->v.xres!=display.xres||b->v.yres!=display.yres||b->v.xoffset||b->v.yoffset||b->v.red.length!=5||b->v.green.length!=6||b->v.blue.length!=5||b->v.red.offset!=11||b->v.green.offset!=5||b->v.blue.offset||b->v.xres*2>b->f.line_length||(uint64_t)b->v.yres*b->f.line_length>b->f.smem_len)goto failed;
 b->overlay=1;b->x=40;b->y=b->v.yres-height-40;b->w=b->v.xres-80;b->h=height;
 b->saved=malloc(b->w*b->h*2);b->paint=malloc(b->w*b->h*2);
 if(!b->saved||!b->paint)goto failed;
 b->fb=mmap(0,b->f.smem_len,3,1,b->fd,0);if(b->fb==(void*)-1){b->fb=0;goto failed;}
 /* Black colour key makes the rest of the foreground plane transparent. */
 for(unsigned y=0;y<b->v.yres;y++){uint16_t*r=(uint16_t*)(b->fb+y*b->f.line_length);for(unsigned x=0;x<b->v.xres;x++)r[x]=0;}
 struct {int enable;uint32_t key;} key={1,0};struct{int enable,alpha;} alpha={1,255};struct{uint16_t x,y;} pos={0,0};
 if(ioctl(b->fd,0x40084621,&alpha)||ioctl(b->fd,0x40084622,&key)||ioctl(b->fd,0xc0044624,&pos))goto failed;
 banner_rect(b,0,0,b->w,b->h,banner_color(b,0x10243c));
 banner_rect(b,0,0,b->w,3,banner_color(b,0x64ffe1));
 banner_logo(b);
 banner_text(b,130,20,b->w>=1000?4:2,"DJ DRAGON-X RETRO GAMES LAUNCHER READY",0x64ffe1);
 banner_text(b,130,73,4,"HAVE FUN",0xffffff);
 banner_text(b,360,81,b->w>=1000?3:2,"RIGHT DECK REV + IN + OUT",0x8fa1b5);
 banner_text(b,360,106,2,"HOLD 5 SECONDS",0x8fa1b5);
 banner_rect(b,24,121,b->w-48,9,banner_color(b,0x64ffe1));
 for(int y=0;y<b->h;y++)memcpy(banner_row(b,y),b->paint+y*b->w,b->w*2);
 if(ioctl(b->fd,0x4611,0))goto failed;
 b->start=now;b->next=now+200000000ULL;printf("DRAGON-X ready banner: ready status, 100 percent\n");fflush(0);return;
failed:if(b->fb){munmap(b->fb,b->f.smem_len);b->fb=0;}if(b->configured){ioctl(b->fd,0x4611,4);ioctl(b->fd,0x4601,&b->original);ioctl(b->fd,0x4611,b->old_blank);}if(b->fd>=0)close(b->fd);free(b->saved);free(b->paint);b->fb=0;printf("Ready banner skipped: unsupported/unavailable framebuffer\n");fflush(0);
}
static void banner_open(struct ready_banner*b,uint64_t now){banner_open_height(b,now,150);}
static const char*farewell_artwork_path="/tmp/rx3-launcher/payload/farewell-artwork.rgba";
static void banner_artwork(struct ready_banner*b){
 int fd=open(farewell_artwork_path,0);if(fd<0)return;
 unsigned char*artwork=malloc(400*400*4);unsigned got=0;
 while(artwork&&got<400*400*4){long n=read(fd,artwork+got,400*400*4-got);if(n<=0)break;got+=(unsigned)n;}close(fd);
 if(got!=400*400*4){free(artwork);return;}
 for(int y=0;y<400;y++)for(int x=0;x<400;x++){
  int dx=b->w-416+x,dy=10+y;if(dx<0||dx>=b->w||dy>=b->h)continue;
  const unsigned char*v= artwork+4*(y*400+x);unsigned a=v[3];if(!a)continue;
  uint16_t*d=b->paint+dy*b->w+dx;
  unsigned r=(v[0]*a+((*d>>11)*255/31)*(255-a))/255,g=(v[1]*a+(((*d>>5)&63)*255/63)*(255-a))/255,z=(v[2]*a+((*d&31)*255/31)*(255-a))/255;
  *d=(uint16_t)(((r>>3)<<11)|((g>>2)<<5)|(z>>3));
  if(!*d)*d=1; /* Keep opaque image black distinct from the plane colour key. */
 }
 free(artwork);
}
static void banner_farewell(struct ready_banner*b,uint64_t now){
 banner_open_height(b,now,420);if(!b->fb)return;
 banner_rect(b,0,0,b->w,b->h,banner_color(b,0x10243c));
 banner_rect(b,0,0,b->w,3,banner_color(b,0x64ffe1));banner_logo_scaled(b,154,168);
 banner_text(b,24,76,4,"DJ DRAGON-X LAUNCHER IS CLOSING",0x64ffe1);
 banner_text(b,216,176,5,"THANKS FOR PLAYING",0xffffff);
 banner_text(b,216,242,4,"RESTORING ORIGINAL RX3",0x8fa1b5);banner_artwork(b);
 for(int y=0;y<b->h;y++)memcpy(banner_row(b,y),b->paint+y*b->w,b->w*2);
}
#define FAREWELL_DURATION_NS 11100000000ULL
static void banner_farewell_progress(struct ready_banner*b,uint64_t now){
 if(!b->fb)return;
 uint64_t elapsed=now>b->start?now-b->start:0;
 if(elapsed>FAREWELL_DURATION_NS)elapsed=FAREWELL_DURATION_NS;
 if(now<b->next&&elapsed<FAREWELL_DURATION_NS)return;
 b->next=now+100000000ULL;
 int fill=(int)((uint64_t)(b->w-464)*elapsed/FAREWELL_DURATION_NS);
 banner_rect(b,24,b->h-14,b->w-464,8,banner_color(b,0x243b50));
 banner_rect(b,24,b->h-14,fill,8,banner_color(b,0x64ffe1));
 for(int y=b->h-14;y<b->h-6;y++)memcpy(banner_row(b,y),b->paint+y*b->w,b->w*2);
}
static void banner_loading_progress(struct ready_banner*b,unsigned percent){
 if(!b->fb)return;if(percent>100)percent=100;
 for(int y=0;y<b->h;y++){uint16_t*r=banner_row(b,y);for(int x=0;x<b->w;x++)if(r[x]!=b->paint[y*b->w+x])b->saved[y*b->w+x]=r[x];}
 banner_rect(b,0,0,b->w,b->h,banner_color(b,0x10243c));banner_rect(b,0,0,b->w,3,banner_color(b,0x64ffe1));banner_logo(b);
 banner_text(b,130,20,b->w>=1000?4:2,percent==100?"DJ DRAGON-X - READY TO PLAY":"DJ DRAGON-X - LOADING GAMES",0x64ffe1);
 char label[20];snprintf(label,sizeof(label),"%u",percent);banner_text(b,130,72,5,label,0xffffff);
 int x=130;for(const char*p=label;*p;p++)x+=30;
 /* Pixel percent sign. */
 banner_rect(b,x,72,8,8,banner_color(b,0xffffff));banner_rect(b,x+17,99,8,8,banner_color(b,0xffffff));
 for(int i=0;i<7;i++)banner_rect(b,x+24-i*4,72+i*5,4,4,banner_color(b,0xffffff));
 banner_text(b,360,76,b->w>=1000?3:2,percent==100?"RIGHT DECK REV + IN + OUT":"COPYING DATA INTO RAM",0x8fa1b5);
 banner_text(b,360,104,2,percent==100?"HOLD 5 SECONDS":"PLEASE KEEP THE USB STICK CONNECTED",0x8fa1b5);
 banner_rect(b,24,130,b->w-48,9,banner_color(b,0x243b50));banner_rect(b,24,130,(b->w-48)*(int)percent/100,9,banner_color(b,0x64ffe1));
 for(int y=0;y<b->h;y++)memcpy(banner_row(b,y),b->paint+y*b->w,b->w*2);
}
static void banner_tick(struct ready_banner*b,uint64_t now){
 if(!b->fb)return;if(now-b->start>=10000000000ULL){banner_close(b);return;}
 if(now<b->next)return;b->next=now+200000000ULL;banner_loading_progress(b,100);
}
#endif
