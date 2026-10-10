#include "../native/platform.h"
#include "../native/ready_banner.h"
#define CHECK(x) do{if(!(x)){printf("FAIL line %d\n",__LINE__);return 1;}}while(0)
int main(void){
 struct ready_banner b={.x=8,.y=6,.w=800,.h=150,.start=1000000000,.next=0};b.v.red=(struct fb_bits){11,5,0};b.v.green=(struct fb_bits){5,6,0};b.v.blue=(struct fb_bits){0,5,0};b.f.line_length=2048;
 b.fb=malloc(2048*200);b.saved=malloc(b.w*b.h*2);b.paint=malloc(b.w*b.h*2);
 for(int i=0;i<2048*200/2;i++)((uint16_t*)b.fb)[i]=0x1111;
 for(int i=0;i<b.w*b.h;i++)b.saved[i]=0x1111,b.paint[i]=0x2222;
 for(int y=0;y<b.h;y++)memcpy(banner_row(&b,y),b.paint+y*b.w,b.w*2);
 banner_row(&b,10)[10]=0xbeef;
 banner_tick(&b,6000000000ULL);
 CHECK(b.saved[10*b.w+10]==0xbeef);
 uint16_t fill=banner_color(&b,0x64ffe1),bg=banner_color(&b,0x243b50);
 CHECK(b.paint[134*b.w+30]==fill);CHECK(b.paint[134*b.w+600]==fill);
 banner_loading_progress(&b,25);CHECK(b.paint[134*b.w+30]==fill);CHECK(b.paint[134*b.w+600]==bg);
 banner_row(&b,11)[11]=0xabcd;
 banner_restore_pixels(&b);
 CHECK(banner_row(&b,10)[10]==0xbeef);CHECK(banner_row(&b,11)[11]==0xabcd);CHECK(banner_row(&b,20)[20]==0x1111);
 b.next=0;banner_farewell_progress(&b,b.start);CHECK(b.paint[140*b.w+30]==bg);
 banner_farewell_progress(&b,b.start+FAREWELL_DURATION_NS/2);CHECK(b.paint[140*b.w+30]==fill);CHECK(b.paint[140*b.w+300]==bg);
 banner_farewell_progress(&b,b.start+FAREWELL_DURATION_NS+1000000000);CHECK(b.paint[140*b.w+30]==fill);
 farewell_artwork_path="work/farewell-artwork.rgba";banner_artwork(&b);for(int y=10;y<150;y++)for(int x=b.w-416;x<b.w-16;x++)CHECK(b.paint[y*b.w+x]!=0);
 free(b.fb);free(b.saved);free(b.paint);printf("Ready banner percentage and concurrent DJ writes/restore passed\n");return 0;
}
