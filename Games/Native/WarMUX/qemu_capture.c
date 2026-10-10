#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <malloc.h>
static unsigned frames;
static void capture(SDL_Surface*s){const char*path=getenv("RX3_WARMUX_SNAPSHOT");if(!path||!s||!s->pixels)return;FILE*f=fopen(path,"wb");if(!f)return;fprintf(f,"P6\n%d %d\n255\n",s->w,s->h);SDL_LockSurface(s);for(int y=0;y<s->h;y++)for(int x=0;x<s->w;x++){unsigned p=0;unsigned char*r=(unsigned char*)s->pixels+y*s->pitch+x*s->format->BytesPerPixel;for(int k=0;k<s->format->BytesPerPixel;k++)p|=(unsigned)r[k]<<(k*8);unsigned char rgb[3];SDL_GetRGB(p,s->format,rgb,rgb+1,rgb+2);fwrite(rgb,1,3,f);}SDL_UnlockSurface(s);fclose(f);fprintf(stderr,"RX3 WarMUX frame=%u snapshot=%dx%d\n",frames,s->w,s->h);}
int SDL_Flip(SDL_Surface*s){static int(*flip)(SDL_Surface*);if(!flip)flip=dlsym(RTLD_NEXT,"SDL_Flip");frames++;const char*at=getenv("RX3_WARMUX_FRAME");unsigned n=at?atoi(at):100;if(frames<=5||frames==n)capture(s);if(at&&frames>n+5){struct mallinfo m=mallinfo();fprintf(stderr,"RX3 target heap used=%d free=%d mmap=%d frames=%u elapsed_ms=%u\n",m.uordblks,m.fordblks,m.hblkhd,frames,SDL_GetTicks());fflush(stdout);_exit(0);}return flip(s);}
int SDL_PollEvent(SDL_Event*e){static int(*poll)(SDL_Event*);if(!poll)poll=dlsym(RTLD_NEXT,"SDL_PollEvent");const char*at=getenv("RX3_WARMUX_FRAME");unsigned n=at?atoi(at):100;if(frames>n+5){e->type=SDL_QUIT;return 1;}return poll(e);}
