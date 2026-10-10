#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <string.h>
static struct stat root,dev,dir,key;
static struct statfs fs;
static int fake_lstat(const char *p,struct stat *s){
 if(!strcmp(p,"/dev"))*s=dev;
 else if(!strcmp(p,"/dev/rx3-games-ssh"))*s=dir;
 else if(!strcmp(p,"/dev/rx3-games-ssh/authorized_keys"))*s=key;
 else return -1;return 0;
}
static int fake_statfs(const char *p,struct statfs *s){if(strcmp(p,"/dev"))return -1;*s=fs;return 0;}
#define lstat fake_lstat
#define statfs(p,s) fake_statfs(p,s)
#include "../native/ssh_root_permissions.h"
static int valid(void){return rx3_ram_key_root_ok("/","/dev/rx3-games-ssh",&root);}
int main(void){
 root.st_mode=S_IFDIR|0775;root.st_dev=1;
 dev.st_mode=S_IFDIR|0755;dev.st_dev=2;
 dir.st_mode=S_IFDIR|0700;dir.st_dev=2;
 key.st_mode=S_IFREG|0600;key.st_dev=2;fs.f_type=0x01021994;
 assert(valid());
 assert(!rx3_ram_key_root_ok("/root","/dev/rx3-games-ssh",&root));
 assert(!rx3_ram_key_root_ok("/","/tmp/keys",&root));
 root.st_mode|=S_IWOTH;assert(valid());root.st_mode&=~S_IWOTH;
 root.st_gid=100;assert(valid());root.st_gid=0;
 root.st_uid=100;assert(valid());root.st_uid=0;
 dev.st_mode|=S_IWGRP;assert(!valid());dev.st_mode&=~S_IWGRP;
 dev.st_dev=1;assert(!valid());dev.st_dev=2;
 fs.f_type=0xef53;assert(!valid());fs.f_type=0x01021994;
 dir.st_mode=S_IFLNK|0700;assert(!valid());dir.st_mode=S_IFDIR|0700;
 dir.st_uid=1;assert(!valid());dir.st_uid=0;
 key.st_mode|=S_IRGRP;assert(!valid());key.st_mode&=~S_IRGRP;
 key.st_dev=1;assert(!valid());key.st_dev=2;
 assert(valid());puts("RX3 root permission exception restricted to secure owned RAM keys; unsafe cases rejected");
}
