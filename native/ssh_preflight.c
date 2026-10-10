#include <stdio.h>
#include "ssh_root_permissions.h"
int main(void){
 struct stat root,dev,dir,key;
 if(stat("/",&root)){puts("RX3 SSH: root metadata unavailable");return 0;}
 const char *reason=rx3_ram_key_root_reason("/","/dev/rx3-games-ssh",&root);
 printf("RX3 SSH RAM key validation: %s\n",reason?reason:"OK");
 printf("root mode=%03o uid=%u gid=%u\n",(unsigned)(root.st_mode&0777),(unsigned)root.st_uid,(unsigned)root.st_gid);
 if(!lstat("/dev",&dev))printf("dev mode=%03o uid=%u gid=%u separate=%d\n",(unsigned)(dev.st_mode&0777),(unsigned)dev.st_uid,(unsigned)dev.st_gid,dev.st_dev!=root.st_dev);
 if(!lstat("/dev/rx3-games-ssh",&dir))printf("key directory mode=%03o uid=%u gid=%u\n",(unsigned)(dir.st_mode&0777),(unsigned)dir.st_uid,(unsigned)dir.st_gid);
 if(!lstat("/dev/rx3-games-ssh/authorized_keys",&key))printf("public key mode=%03o uid=%u gid=%u\n",(unsigned)(key.st_mode&0777),(unsigned)key.st_uid,(unsigned)key.st_gid);
 return 0;
}
