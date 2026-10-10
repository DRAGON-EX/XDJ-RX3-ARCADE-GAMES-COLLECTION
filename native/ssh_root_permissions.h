/* Permit the RX3's writable root only for our isolated RAM key store.
 * All key files/directories remain root-owned and non-writable to other users.
 * Never relax checks for arbitrary Dropbear authorized-key paths. */
#ifndef RX3_SSH_ROOT_PERMISSIONS_H
#define RX3_SSH_ROOT_PERMISSIONS_H
#include <sys/stat.h>
#include <sys/vfs.h>
#include <string.h>
static const char *rx3_ram_key_root_reason(const char *path,const char *keydir,const struct stat *root){
 struct stat dev,dir,key;struct statfs fs;
 if(strcmp(path,"/")||strcmp(keydir,"/dev/rx3-games-ssh"))return "unexpected authorized-key path";
 if(lstat("/dev",&dev)||!S_ISDIR(dev.st_mode)||dev.st_uid!=0||(dev.st_mode&(S_IWGRP|S_IWOTH)))return "unsafe /dev ownership or permissions";
 if(dev.st_dev==root->st_dev||statfs("/dev",&fs))return "/dev is not a separate filesystem or statfs failed";
 if((unsigned long)fs.f_type!=0x01021994UL&&(unsigned long)fs.f_type!=0x858458f6UL)return "/dev is not a RAM filesystem";
 if(lstat(keydir,&dir)||!S_ISDIR(dir.st_mode)||dir.st_uid!=0||(dir.st_mode&0777)!=0700||dir.st_dev!=dev.st_dev)return "unsafe RAM key directory ownership, mode or filesystem";
 if(lstat("/dev/rx3-games-ssh/authorized_keys",&key)||!S_ISREG(key.st_mode)||key.st_uid!=0||(key.st_mode&0777)!=0600||key.st_dev!=dev.st_dev)return "unsafe public-key file ownership, mode or filesystem";
 return 0;
}
static int rx3_ram_key_root_ok(const char *path,const char *keydir,const struct stat *root){
 return rx3_ram_key_root_reason(path,keydir,root)==0;
}
#endif
