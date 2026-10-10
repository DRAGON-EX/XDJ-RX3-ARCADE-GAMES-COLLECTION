#define main launcher_main
#include "../native/launcher.c"
#undef main
#define CHECK(x) do{if(!(x)){printf("FAIL %d\n",__LINE__);return 1;}}while(0)
int main(void){
boot_progress_path="work/test-load-progress";
CHECK(boot_percent(0,0)==0&&boot_percent(50,100)==49&&boot_percent(100,100)==99);
int fd=open("work/test-copy-source",1|64|512,0600);char buf[40000];for(int i=0;i<40000;i++)buf[i]=(char)i;CHECK(write(fd,buf,40000)==40000);close(fd);
CHECK(!boot_copy("work/test-copy-source","work/test-copy-target",0,40000));
char got[40000];fd=open("work/test-copy-target",0);CHECK(read(fd,got,40000)==40000&&!memcmp(buf,got,40000));close(fd);
fd=open(boot_progress_path,0);char progress[16]={0};read(fd,progress,15);close(fd);CHECK(atoi(progress)==99);
CHECK(boot_copy("work/nonexistent-source","work/test-copy-target",0,1)==1);
printf("Byte copy, content integrity, errors and 99 percent readiness boundary passed\n");return 0;}
