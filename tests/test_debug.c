#include <assert.h>
#include <string.h>
#include "../native/platform.h"
#include "../native/debug_info.h"
static unsigned reads,stat_reads,pauses;
static int fixture(const char*path,char*b,unsigned n){reads++;const char*s="";
 if(!strcmp(path,"/proc/stat"))s=stat_reads++?"cpu 150 0 50 800 0 0 0 0\ncpu0 150 0 0 950 0 0 0 0\ncpu1 100 0 0 1000 0 0 0 0\ncpu2 200 0 0 900 0 0 0 0\ncpu3 125 0 0 975 0 0 0 0\n":"cpu 100 0 50 750 0 0 0 0\ncpu0 100 0 0 900 0 0 0 0\ncpu1 100 0 0 900 0 0 0 0\ncpu2 100 0 0 900 0 0 0 0\ncpu3 100 0 0 900 0 0 0 0\n";
 else if(!strcmp(path,"/proc/meminfo"))s="MemTotal: 524288 kB\nMemFree: 102400 kB\nBuffers: 10240 kB\nCached: 204800 kB\n";
 else if(!strcmp(path,"/proc/self/status"))s="VmRSS: 20480 kB\nThreads: 1\n";
 else if(!strcmp(path,"/proc/uptime"))s="1234.50 2000.00\n";
 else if(!strcmp(path,"/proc/loadavg"))s="0.25 0.30 0.40 1/10 99\n";
 snprintf(b,n,"%s",s);return *s!=0;
}
static void sample_pause(void){pauses++;}
int main(void){struct debug_snapshot d={0};assert(reads==0);debug_collect(&d,fixture,sample_pause);assert(d.captures==1&&pauses==1&&stat_reads==2);assert(strstr(d.line[0],"50%"));assert(strstr(d.line[1],"UNAVAILABLE"));assert(d.core_load[0]==50&&d.core_load[1]==0&&d.core_load[2]==100&&d.core_load[3]==25);assert(d.history_count==1);assert(strstr(d.line[3],"512 MB"));assert(strstr(d.line[4],"100 MB"));assert(strstr(d.line[5],"ESTIMATE: 310 MB"));assert(strstr(d.line[6],"20 MB"));assert(strstr(d.line[8],"1234"));unsigned saved=reads;for(int i=0;i<1000;i++)assert(d.captures==1);assert(reads==saved);debug_collect(&d,fixture,sample_pause);assert(pauses==1&&d.history_count==2);for(int c=0;c<4;c++)assert(d.core_load[c]==-1);debug_core_sample(&d,"cpu0 1 0 0 1 0 0 0 0\n");for(int c=0;c<4;c++)assert(d.core_load[c]==-1);printf("Four-core deltas, missing/reset counters, debug-only sampling passed\n");}
