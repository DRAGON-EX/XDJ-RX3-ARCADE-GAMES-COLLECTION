#ifndef RX3_DEBUG_INFO_H
#define RX3_DEBUG_INFO_H
/* Collect only while the debug page is open. No background polling. */
#define DEBUG_LINES 10
#define DEBUG_CORES 4
#define DEBUG_HISTORY 30
struct debug_snapshot { char line[DEBUG_LINES][88]; unsigned captures; unsigned long long cpu_total,cpu_idle; int cpu_valid; unsigned long long core_total[DEBUG_CORES],core_idle[DEBUG_CORES]; int core_valid[DEBUG_CORES],core_load[DEBUG_CORES]; unsigned char history[DEBUG_CORES][DEBUG_HISTORY]; unsigned history_head,history_count; };
typedef int (*debug_reader)(const char*,char*,unsigned);
static unsigned long long debug_number(const char **s){unsigned long long n=0;while(**s==' '||**s=='\t')(*s)++;while(**s>='0'&&**s<='9'){n=n*10+(*(*s)++-'0');}return n;}
static unsigned long long debug_field(const char*s,const char*key){
 for(;*s;s++){if(s!=0){const char*a=s,*b=key;while(*b&&*a==*b){a++;b++;}if(!*b)return debug_number(&a);}while(*s&&*s!='\n')s++;if(!*s)break;}return 0;
}
static int debug_cpu_values(const char*s,unsigned long long*total,unsigned long long*idle){
 *total=0;*idle=0;for(int i=0;i<8;i++){unsigned long long n=debug_number(&s);*total+=n;if(i==3||i==4)*idle+=n;}return *total!=0;
}
static int debug_cpu(const char*s,unsigned long long*total,unsigned long long*idle){
 if(s[0]!='c'||s[1]!='p'||s[2]!='u'||s[3]!=' ')return 0;return debug_cpu_values(s+4,total,idle);
}
static int debug_core(const char*s,int core,unsigned long long*total,unsigned long long*idle){
 while(*s){if(s[0]=='c'&&s[1]=='p'&&s[2]=='u'&&s[3]=='0'+core&&s[4]==' ')return debug_cpu_values(s+5,total,idle);while(*s&&*s!='\n')s++;if(*s)s++;}return 0;
}
static void debug_core_baseline(struct debug_snapshot*d,const char*b){
 for(int c=0;c<DEBUG_CORES;c++)d->core_valid[c]=debug_core(b,c,&d->core_total[c],&d->core_idle[c]);
}
static void debug_core_sample(struct debug_snapshot*d,const char*b){
 for(int c=0;c<DEBUG_CORES;c++){
  unsigned long long total=0,idle=0;int present=debug_core(b,c,&total,&idle);
  unsigned long long dt=total-d->core_total[c],di=idle-d->core_idle[c];
  int valid=present&&d->core_valid[c]&&total>d->core_total[c]&&idle>=d->core_idle[c]&&di<=dt;
  d->core_load[c]=valid?(int)(100*(dt-di)/dt):-1;
  d->history[c][d->history_head]=valid?(unsigned char)d->core_load[c]:255;
  d->core_total[c]=total;d->core_idle[c]=idle;d->core_valid[c]=present;
 }
 d->history_head=(d->history_head+1)%DEBUG_HISTORY;if(d->history_count<DEBUG_HISTORY)d->history_count++;
}
static int debug_read_file(const char*path,char*b,unsigned cap){int fd=open(path,0);if(fd<0){b[0]=0;return 0;}long n=read(fd,b,cap-1);close(fd);if(n<0)n=0;b[n]=0;return n>0;}
static void debug_pause(void){rx_timespec delay={0,200000000};nanosleep(&delay,0);}
static void debug_collect(struct debug_snapshot*d,debug_reader reader,void(*pause_sample)(void)){
 char b[8192];unsigned long long a=d->cpu_total,ai=d->cpu_idle,z=0,zi=0;int valid=d->cpu_valid;if(!valid){valid=reader("/proc/stat",b,sizeof(b))&&debug_cpu(b,&a,&ai);debug_core_baseline(d,valid?b:"");pause_sample();}int sampled=reader("/proc/stat",b,sizeof(b))&&debug_cpu(b,&z,&zi);debug_core_sample(d,sampled?b:"");valid=valid&&sampled&&z>a&&zi>=ai&&zi-ai<=z-a;d->cpu_total=z;d->cpu_idle=zi;d->cpu_valid=sampled;
 if(valid)snprintf(d->line[0],88,"CPU LOAD: %llu%%",100*(z-a-(zi-ai))/(z-a));else snprintf(d->line[0],88,"CPU LOAD: UNAVAILABLE");
 if(reader("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq",b,sizeof(b))){const char*p=b;snprintf(d->line[1],88,"CPU CLOCK: %llu MHZ",debug_number(&p)/1000);}else snprintf(d->line[1],88,"CPU CLOCK: UNAVAILABLE");
 snprintf(d->line[2],88,"ARCHITECTURE: ARMV7 / SOFTFP");
 if(reader("/proc/meminfo",b,sizeof(b))){unsigned long long total=debug_field(b,"MemTotal:"),available=debug_field(b,"MemAvailable:"),free=debug_field(b,"MemFree:");if(!available)available=free+debug_field(b,"Buffers:")+debug_field(b,"Cached:");snprintf(d->line[3],88,"RAM TOTAL: %llu MB",total/1024);snprintf(d->line[4],88,"RAM FREE: %llu MB",free/1024);snprintf(d->line[5],88,"RAM AVAILABLE%s: %llu MB",debug_field(b,"MemAvailable:")?"":" ESTIMATE",available/1024);}else for(int i=3;i<=5;i++)snprintf(d->line[i],88,"RAM: UNAVAILABLE");
 if(reader("/proc/self/status",b,sizeof(b))){snprintf(d->line[6],88,"GAME PROCESS RAM: %llu MB",debug_field(b,"VmRSS:")/1024);snprintf(d->line[7],88,"PROCESS THREADS: %llu",debug_field(b,"Threads:"));}else {snprintf(d->line[6],88,"PROCESS RAM: UNAVAILABLE");snprintf(d->line[7],88,"PROCESS THREADS: UNAVAILABLE");}
 if(reader("/proc/uptime",b,sizeof(b))){const char*p=b;snprintf(d->line[8],88,"UPTIME: %llu SECONDS",debug_number(&p));}else snprintf(d->line[8],88,"UPTIME: UNAVAILABLE");
 if(reader("/proc/loadavg",b,sizeof(b))){int i=0;while(b[i]&&b[i]!=' '&&i<16)i++;b[i]=0;snprintf(d->line[9],88,"LOAD AVERAGE 1 MIN: %s",b);}else snprintf(d->line[9],88,"LOAD AVERAGE: UNAVAILABLE");
 d->captures++;
}
#endif
