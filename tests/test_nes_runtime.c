#include "../native/platform.h"
#include "../Games/EMU/NES/nes.h"
int main(int argc,char**argv){if(argc<2||!nes_open(argv[1],0))return 1;
 unsigned long long energy=0;unsigned checksum=0,previous=0,changes=0,frames=0;
 for(unsigned t=0;t<1800;t++){unsigned b=(t>=300&&t<310)||(t>=450&&t<460)?8:t>1000?256:0;if(t>1100&&t<1170)b|=128;if(t>1200&&t<1270)b|=64;nes_tick(b);
 if(!nes_width||!nes_height||nes_width>512||nes_height>480||nes_audio_frames>2048)return 2;
 frames+=nes_audio_frames;for(unsigned i=0;i<nes_audio_frames*2;i++)energy+=nes_audio[i]<0?-(int)nes_audio[i]:nes_audio[i];
 checksum=0;for(unsigned i=0;i<nes_width*nes_height;i++)checksum=(checksum*33)^nes_video[i];if(checksum!=previous)changes++;previous=checksum;}
 printf("NES 1800 frames: %ux%u, audio_frames=%u, energy=%llu, changed_frames=%u, period=%llu ns\n",nes_width,nes_height,frames,energy,changes,(unsigned long long)nes_period);
 if(argc>2){int f=open(argv[2],1|64|512,0644);char h[64];int n=snprintf(h,64,"P6\n%u %u\n255\n",nes_width,nes_height);write(f,h,n);for(unsigned i=0;i<nes_width*nes_height;i++){unsigned char p[]={nes_video[i]>>16,nes_video[i]>>8,nes_video[i]};write(f,p,3);}close(f);}
 int good=frames>1700000&&energy>1000000&&changes>300&&nes_period>19000000&&nes_period<21000000;nes_close();return good?0:3;}
