#include "platform.h"
#include "launcher_trigger.h"
#include "panel_output.h"
#include "master_state.h"
extern int umount(const char*);extern void sync(void);
#include "ready_banner.h"
extern int system(const char*),rmdir(const char*),unlink(const char*),fork(void),setsid(void),kill(int,int),waitpid(int,int*,int),chdir(const char*),dup2(int,int),setenv(const char*,const char*,int),execl(const char*,const char*,...);
extern void _exit(int);
static struct ready_banner*farewell_banner;
static void farewell_update(uint64_t now){banner_farewell_progress(farewell_banner,now);}
static volatile int active=1;
static void stopped(int s){(void)s;active=0;}
static uint64_t now_ns(void){rx_timespec t;clock_gettime(1,&t);return(uint64_t)t.sec*1000000000u+t.nsec;}
#include "farewell_audio.h"
static void pause_tick(void){rx_timespec d={0,100000000};nanosleep(&d,0);}
static int usb_present(const char*path){int fd=open(path,0);if(fd<0)return 0;close(fd);return 1;}
static void restore_dj(void){
 int marker=open("/tmp/rx3-launcher/restart-dj",0);if(marker<0)return;close(marker);
 if(system("pidof rbp >/dev/null 2>&1")!=0){int pid=fork();if(pid==0){setsid();int fd=open("/dev/null",2);if(fd>=0){dup2(fd,0);dup2(fd,1);dup2(fd,2);if(fd>2)close(fd);}if(chdir("/root/pdj")==0)execl("./rbp","rbp",(char*)0);_exit(127);}}
 unlink("/tmp/rx3-launcher/restart-dj");
}
static int ssh_start(void){
 int file=open("/tmp/rx3-launcher/ssh-start.sh",0);if(file<0)return 0;close(file);
 int pid=fork();if(pid==0){setsid();execl("/bin/sh","sh","/tmp/rx3-launcher/ssh-start.sh",(char*)0);_exit(127);}return pid>0?pid:0;
}
static void ssh_stop(int pid){
 if(!pid)return;kill(pid,15);int status;uint64_t end=now_ns()+8000000000ULL;
 while(now_ns()<end){if(waitpid(pid,&status,1)==pid)return;pause_tick();}
 kill(-pid,9);waitpid(pid,&status,0);
}
static void remove_files(void){
 const char*paths[]={"/tmp/rx3-launcher/secret-song-unlocked","/tmp/rx3-launcher/secret-song-unlocked.tmp","/tmp/rx3-launcher/master-level","/tmp/rx3-launcher/load-progress","/tmp/rx3-launcher/ssh-status","/tmp/rx3-launcher/ssh-start.sh","/tmp/rx3-launcher/ssh-stop.sh","/tmp/rx3-launcher/ssh-server.pid","/tmp/rx3-launcher/payload/rx3-arcade","/tmp/rx3-launcher/payload/road-fighter.nes","/tmp/rx3-launcher/payload/cat-cute.pcm","/tmp/rx3-launcher/payload/cat-meow-1.pcm","/tmp/rx3-launcher/payload/cat-meow-2.pcm","/tmp/rx3-launcher/payload/cat-meow-3.pcm","/tmp/rx3-launcher/payload/cat-purr-soft.pcm","/tmp/rx3-launcher/payload/cat-purr-warm.pcm","/tmp/rx3-launcher/payload/cafe-show.pcm","/tmp/rx3-launcher/payload/rescue-fanfare.pcm","/tmp/rx3-launcher/payload/menu-open.pcm","/tmp/rx3-launcher/payload/farewell.pcm","/tmp/rx3-launcher/payload/welcome.pcm","/tmp/rx3-launcher/payload/farewell-artwork.rgba","/tmp/rx3-launcher/payload/no-cover-cat.rgb","/tmp/rx3-launcher/payload/debug-artwork.rgba","/tmp/rx3-launcher/payload/menu-music.pcm","/tmp/rx3-launcher/payload/music-index.tsv","/tmp/rx3-launcher/road-fighter.nes","/tmp/rx3-launcher/start.sh","/tmp/rx3-launcher/stop-dj.sh","/tmp/rx3-launcher/eject-request","/tmp/rx3-launcher/rx3-arcade","/tmp/rx3-launcher/rx3-launcher","/tmp/rx3-launcher/restart-dj","/tmp/rx3-arcade","/tmp/rx3-launcher.pid","/tmp/rx3-arcade.log","/tmp/rx3-arcade-rbp.log"};
 for(unsigned i=0;i<sizeof(paths)/sizeof(paths[0]);i++)unlink(paths[i]);system("rm -rf /tmp/rx3-launcher/payload/ssh");chdir("/");int payload=open("/tmp/rx3-launcher/payload",0);if(payload>=0){close(payload);umount("/tmp/rx3-launcher/payload");}rmdir("/tmp/rx3-launcher/payload");rmdir("/tmp/rx3-arcade.lock");rmdir("/tmp/rx3-launcher");rmdir("/tmp/rx3-launcher.lock");
}
#include "boot_progress.h"
int main(int argc,char**argv){
 if(argc==6&&!strcmp(argv[1],"--copy"))return boot_copy(argv[2],argv[3],(unsigned)atoi(argv[4]),(unsigned)atoi(argv[5]));
 if(argc==3&&!strcmp(argv[1],"--loading")){signal(15,stopped);signal(2,stopped);return boot_loading(argv[2]);}
 signal(2,stopped);signal(15,stopped);
 if(argc!=4||!usb_present(argv[1])){printf("USB monitor unavailable; launcher cancelled\n");remove_files();return 1;}
 unlink(master_state_path);master_state_last=~0u;
 int ssh_child=ssh_start();
 struct ready_banner banner;banner_open(&banner,now_ns());
 struct hold_trigger trigger={0};int fd=-1,last=-2,child=0;unsigned errors=0;int welcome_child=0,welcome_started=0;int eject=0;struct usb_stop_hold stop_hold={0};int port=atoi(argv[3])==2?2:1;
 printf("RX3 launcher v5: RIGHT deck REV + IN + OUT, 5 seconds; USB monitor active\n");fflush(0);
 while(active&&usb_present(argv[1])){
  banner_tick(&banner,now_ns());
  int request=open("/tmp/rx3-launcher/eject-request",0);if(request>=0){close(request);eject=1;break;}
  if(child){int status;if(waitpid(child,&status,1)==child){child=0;int request=open("/tmp/rx3-launcher/eject-request",0);if(request>=0){close(request);eject=1;break;}restore_dj();trigger=(struct hold_trigger){0};last=-2;}pause_tick();continue;}
  if(fd<0)fd=open("/dev/subucom_spi1.0",04000);
  unsigned char frame[104];long n=fd<0?-1:read(fd,frame,sizeof(frame));
  if(n>0)master_state_panel(frame,(size_t)n);
  /* Wait for the first valid MASTER sample; play once without delaying input. */
  if(!welcome_started&&master_state_last<=1023){
   welcome_started=1;welcome_child=fork();
   if(welcome_child==0){signal(15,0);signal(2,0);if(fd>=0)close(fd);farewell_audio("/tmp/rx3-launcher/payload/welcome.pcm",master_state_load(),0);_exit(0);}
   if(welcome_child<0)welcome_child=0;
  }
  if(welcome_child){int status;if(waitpid(welcome_child,&status,1)==welcome_child)welcome_child=0;}
  if(usb_stop_hold(&stop_hold,n<0?-1:usb_stop_mask(frame,(size_t)n),port,now_ns())){eject=1;break;}
  int mask=n<0?-1:launcher_keys(frame,(size_t)n);
  if(mask!=last){printf("Panel combo mask=%d, bytes=%ld\n",mask,n);fflush(0);last=mask;}
  if(mask<0){if(++errors==100){printf("Panel unreadable; launcher stays inactive\n");fflush(0);}if(fd>=0){close(fd);fd=-1;}}else errors=0;
  if(launcher_hold(&trigger,mask,now_ns())){
   banner_close(&banner);close(fd);fd=-1;
   child=fork();if(child==0){setsid();setenv("RX3_MANAGED","1",1);setenv("RX3_USB_ROOT",argv[2],1);setenv("RX3_USB_PORT",argv[3],1);execl("/bin/sh","sh","/tmp/rx3-launcher/start.sh",(char*)0);_exit(127);}if(child<0)child=0;
  }
  pause_tick();
 }
 banner_close(&banner);if(fd>=0)close(fd);
 if(welcome_child){kill(welcome_child,15);int status;waitpid(welcome_child,&status,0);}
 if(child){kill(-child,15);uint64_t limit=now_ns()+3000000000ULL;int status,done=0;while(now_ns()<limit){if(waitpid(child,&status,1)==child){done=1;break;}pause_tick();}if(!done){kill(-child,9);waitpid(child,&status,0);}}
 /* The game releases framebuffer, touch and ALSA before the DJ app restarts. */
 ssh_stop(ssh_child);
 int eject_ok=1;
 int stopped_ok=system("/bin/sh /tmp/rx3-launcher/stop-dj.sh")==0;
 if(eject){
  fflush(0);int null=open("/dev/null",2);if(null>=0){dup2(null,1);dup2(null,2);if(null>2)close(null);}sync();
  eject_ok=0;if(stopped_ok){for(int i=0;i<30;i++){if(umount(argv[2])==0||*__errno_location()==22){eject_ok=1;break;}pause_tick();}}
 }
 uint64_t farewell_begin=now_ns();banner_farewell(&banner,farewell_begin);
 if(eject&&banner.fb){banner_rect(&banner,216,242,banner.w-656,86,banner_color(&banner,0x10243c));banner_text(&banner,216,242,4,eject_ok?"USB STOP COMPLETE":"EJECT FAILED",0xffffff);banner_text(&banner,216,284,3,eject_ok?"SAFE TO REMOVE STICK":"USE RX3 USB STOP AGAIN",0x8fa1b5);for(int y=0;y<banner.h;y++)memcpy(banner_row(&banner,y),banner.paint+y*banner.w,banner.w*2);}
 farewell_banner=&banner;banner.next=0;farewell_update(farewell_begin);
 if(stopped_ok&&(active||eject))farewell_audio("/tmp/rx3-launcher/payload/farewell.pcm",master_state_load(),farewell_update);
 uint64_t end=farewell_begin+FAREWELL_DURATION_NS;while(now_ns()<end){farewell_update(now_ns());pause_tick();}
 farewell_update(end);restore_dj();
 banner_close(&banner);remove_files();return 0;
}
