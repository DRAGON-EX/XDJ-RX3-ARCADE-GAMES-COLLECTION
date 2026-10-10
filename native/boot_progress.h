#ifndef RX3_BOOT_PROGRESS_H
#define RX3_BOOT_PROGRESS_H
/* RAM-only progress; 100 is reserved for the completed, launchable payload. */
static const char*boot_progress_path="/tmp/rx3-launcher/load-progress";
static unsigned boot_percent(unsigned done,unsigned total){return !total?0:done>=total?99:(unsigned)((uint64_t)done*99/total);}
static void boot_progress_write(unsigned percent){char text[16];int n=snprintf(text,sizeof(text),"%u\n",percent);int fd=open(boot_progress_path,1|64|512,0600);if(fd>=0){write(fd,text,(unsigned)n);close(fd);}}
static int boot_copy(const char*source,const char*dest,unsigned done,unsigned total){
 int in=open(source,0);if(in<0)return 1;int out=open(dest,1|64|512,0600);if(out<0){close(in);return 1;}
 unsigned char block[16384];unsigned last=~0u;int failed=0;
 for(;;){long n=read(in,block,sizeof(block));if(n<0){failed=1;break;}if(!n)break;
  long pos=0;while(pos<n){long w=write(out,block+pos,(unsigned)(n-pos));if(w<=0){failed=1;break;}pos+=w;done+=(unsigned)w;}if(failed)break;
  unsigned percent=boot_percent(done,total);if(percent!=last){boot_progress_write(percent);last=percent;}
 }close(in);if(close(out)<0)failed=1;return failed;
}
static int boot_loading(const char*device){
 struct ready_banner b;banner_open(&b,now_ns());unsigned shown=0;uint64_t complete=0;
 while(active&&usb_present(device)){
  char value[16];int fd=open(boot_progress_path,0);if(fd>=0){long n=read(fd,value,sizeof(value)-1);close(fd);if(n>0){value[n]=0;int p=atoi(value);if(p<0)break;if(p<=100&&(unsigned)p>=shown)shown=(unsigned)p;}}
  banner_loading_progress(&b,shown);
  if(shown==100){if(!complete)complete=now_ns();if(now_ns()-complete>=1000000000ULL)break;}
  pause_tick();
 }banner_close(&b);return shown==100?0:1;
}
#endif
