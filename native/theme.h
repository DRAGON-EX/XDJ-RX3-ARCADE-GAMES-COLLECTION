#ifndef RX3_THEME_H
#define RX3_THEME_H
/* Red & Black is the startup default; Neon preserves the original palette. */
static int theme;
static uint32_t theme_color(uint32_t c){
 if(theme)return c;
 switch(c){
 case 0x080f1b:return 0x090709;
 case 0x14283d:return 0x201014;
 case 0x10243c:return 0x251117;
 case 0x173044:return 0x32141d;
 case 0x243b50:return 0x39131e;
 case 0x64ffe1:return 0xff405c;
 case 0x54bded:return 0xed334d;
 case 0x1b6057:return 0x6c1428;
 case 0x763044:return 0x6c1428;
 case 0x8fa1b5:return 0xb7a1a8;
 case 0x7796aa:return 0xb7a1a8;
 default:return c;
 }
}
#endif
