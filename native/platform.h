#ifndef RX3_PLATFORM_H
#define RX3_PLATFORM_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef long ssize_t;
typedef struct {long sec,nsec;} rx_timespec;
extern int open(const char*,int,...),close(int),ioctl(int,unsigned long,...);
extern ssize_t read(int,void*,size_t),write(int,const void*,size_t);
extern void *mmap(void*,size_t,int,int,int,long);
extern int munmap(void*,size_t),clock_gettime(int,rx_timespec*),nanosleep(const rx_timespec*,rx_timespec*);
extern void *malloc(size_t),*memcpy(void*,const void*,size_t);
extern void free(void*);
extern int memcmp(const void*,const void*,size_t);
extern int printf(const char*,...),snprintf(char*,size_t,const char*,...),strcmp(const char*,const char*),atoi(const char*);
struct rx_pollfd{int fd;short events,revents;};
extern int poll(struct rx_pollfd*,unsigned long,int);
extern int fflush(void*);
extern int *__errno_location(void);
extern void (*signal(int,void(*)(int)))(int);
extern int snd_pcm_open(void**,const char*,int,int),snd_pcm_close(void*),snd_pcm_set_params(void*,int,int,unsigned,unsigned,int,unsigned),snd_pcm_recover(void*,int,int);
extern long snd_pcm_writei(void*,const void*,unsigned long);
extern const char *snd_strerror(int);
struct fb_bits{uint32_t offset,length,msb_right;};
struct fb_var{uint32_t xres,yres,xres_virtual,yres_virtual,xoffset,yoffset,bpp,grayscale;struct fb_bits red,green,blue,transp;uint32_t nonstd,activate,height,width,accel_flags,pixclock,left,right,upper,lower,hsync,vsync,sync,vmode,rotate,colorspace,reserved[4];};
struct fb_fix{char id[16];unsigned long smem_start;uint32_t smem_len,type,type_aux,visual;uint16_t xpanstep,ypanstep,ywrapstep;uint32_t line_length;unsigned long mmio_start;uint32_t mmio_len,accel;uint16_t capabilities,reserved[2];};
struct input_event{long sec,usec;uint16_t type,code;int32_t value;};
struct absinfo{int32_t value,minimum,maximum,fuzz,flat,resolution;};
void arcade_reset(void),arcade_tick(unsigned),arcade_restore_high(unsigned);
unsigned arcade_tile(unsigned,unsigned),arcade_value(unsigned);
int arcade_actor(unsigned,unsigned);
float *arcade_audio(void);
#endif
