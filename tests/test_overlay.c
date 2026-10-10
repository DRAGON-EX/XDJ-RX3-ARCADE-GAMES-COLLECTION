#include "../native/platform.h"
#include <stdarg.h>
static struct fb_var mode={.xres=1280,.yres=800,.xres_virtual=1280,.yres_virtual=800,.bpp=16,.red={11,5,0},.green={5,6,0},.blue={0,5,0}};
static int blank=4,maps,unblanks;static void*memory;
static int mock_open(const char*p,int f,...){(void)f;return !strcmp(p,"/dev/fb0")?10:!strcmp(p,"/dev/fb1")?11:-1;}
static int mock_close(int fd){(void)fd;return 0;}
static int mock_ioctl(int fd,unsigned long req,...){va_list ap;va_start(ap,req);int rc=0;
 if(req==0x4611){blank=va_arg(ap,int);if(!blank)unblanks++;}
 else{void*p=va_arg(ap,void*);if(req==0x4600)*(struct fb_var*)p=mode;else if(req==0x4601)mode=*(struct fb_var*)p;else if(req==0x8004462b)*(int*)p=blank;else if(req==0x4602){struct fb_fix*f=p;*f=(struct fb_fix){.smem_len=1280*800*2,.line_length=2560,.visual=2};memcpy(f->id,fd==11?"DISP3 FG":"DISP3 BG",9);}else if(req!=0x40084621&&req!=0x40084622&&req!=0xc0044624)rc=-1;}
 va_end(ap);return rc;
}
static void*mock_mmap(void*a,size_t n,int p,int flags,int fd,long off){(void)a;(void)p;(void)flags;(void)off;if(fd!=11)return(void*)-1;maps++;memory=malloc(n);return memory;}
static int mock_munmap(void*p,size_t n){(void)n;free(p);memory=0;return 0;}
#define open mock_open
#define close mock_close
#define ioctl mock_ioctl
#define mmap mock_mmap
#define munmap mock_munmap
#include "../native/ready_banner.h"
#define CHECK(x) do{if(!(x)){printf("FAIL line %d\n",__LINE__);return 1;}}while(0)
int main(void){struct ready_banner b;struct fb_var before=mode;
 banner_open(&b,1000000000);CHECK(b.fb&&b.overlay&&blank==0&&maps==1&&unblanks==1);
 banner_tick(&b,10999999999ULL);CHECK(b.fb&&blank==0);
 banner_tick(&b,11000000000ULL);CHECK(!b.fb&&blank==4&&!memory&&mode.xres==before.xres&&mode.yres==before.yres);
 blank=0;banner_open(&b,20000000000ULL);CHECK(!b.fb&&maps==1&&unblanks==1);
 printf("Overlay-only mapping, occupied-plane guard and exact 10-second cleanup passed\n");return 0;}
