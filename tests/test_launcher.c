#include <stdio.h>
#include "../native/launcher_trigger.h"
#define CHECK(x) do{if(!(x)){printf("FAIL line %d\n",__LINE__);return 1;}}while(0)
static void chord(unsigned char*b,int mask){b[34]=(mask&1)?2:0;b[35]=((mask&2)?2:0)|((mask&4)?1:0);}
int main(void){
 unsigned char b[104]={0};b[3]=1;
 CHECK(launcher_keys(b,104)==0);b[18]=2;b[19]=3;CHECK(launcher_keys(b,104)==0);
 for(int mask=0;mask<8;mask++){chord(b,mask);CHECK(launcher_keys(b,104)==mask);CHECK(launcher_keys(b,100)==mask);}
 CHECK(launcher_keys(b,99)==-1);b[3]=0;CHECK(launcher_keys(b,104)==-1);
 struct hold_trigger t={0};uint64_t now=0;
 for(int i=0;i<80;i++,now+=100000000)CHECK(!launcher_hold(&t,7,now));
 CHECK(!launcher_hold(&t,0,now));now+=100000000;
 for(int i=0;i<50;i++,now+=100000000)CHECK(!launcher_hold(&t,7,now));
 CHECK(launcher_hold(&t,7,now));now+=100000000;
 for(int i=0;i<80;i++,now+=100000000)CHECK(!launcher_hold(&t,7,now));
 CHECK(!launcher_hold(&t,0,now));now+=100000000;
 for(int i=0;i<45;i++,now+=100000000)CHECK(!launcher_hold(&t,7,now));
 CHECK(!launcher_hold(&t,3,now));now+=100000000;
 for(int i=0;i<45;i++,now+=100000000)CHECK(!launcher_hold(&t,7,now));
 CHECK(!launcher_hold(&t,-1,now));now+=100000000;
 for(int i=0;i<80;i++,now+=100000000)CHECK(!launcher_hold(&t,7,now));
 CHECK(!launcher_hold(&t,0,now));now+=100000000;
 for(int i=0;i<45;i++,now+=100000000)CHECK(!launcher_hold(&t,7,now));
 now+=1000000000;CHECK(!launcher_hold(&t,7,now));now+=100000000;
 for(int i=0;i<49;i++,now+=100000000)CHECK(!launcher_hold(&t,7,now));
 CHECK(launcher_hold(&t,7,now));
 puts("Right-deck mapping, 5s hold, release, invalid frames, gaps and rearm passed");return 0;
}
