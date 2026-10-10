#ifndef RX3_MASTER_STATE_H
#define RX3_MASTER_STATE_H
/* Shared last-known MASTER ADC value, stored only when it changes. */
static const char*master_state_path="/tmp/rx3-launcher/master-level";
static unsigned master_state_last=~0u;
static void master_state_store(unsigned raw){
 if(raw>1023||raw==master_state_last)return;
 int fd=open(master_state_path,1|64|512,0600);if(fd<0)return;
 if(write(fd,&raw,sizeof(raw))==(long)sizeof(raw))master_state_last=raw;
 close(fd);
}
static unsigned master_state_load(void){
 unsigned raw=0;int fd=open(master_state_path,0);if(fd<0)return 0;
 long n=read(fd,&raw,sizeof(raw));close(fd);return n==(long)sizeof(raw)&&raw<=1023?raw:0;
}
static void master_state_panel(const unsigned char*b,size_t n){
 if((n!=100&&n!=104)||b[0]||b[1]||b[2]||b[3]!=1)return;
 master_state_store(b[74]|((unsigned)b[75]<<8));
}
#endif
