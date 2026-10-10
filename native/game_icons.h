/* Small original game-specific pixel illustrations for the launcher cards. */
static void game_icon(int game,int x,int y){
 if(game==0){disk(x+16,y+16,18,0xffcd55);for(int i=0;i<13;i++)rect(x+16+i,y+16-i/2,21-i,i+1,0x14283d);return;}
 if(game==1){for(int r=0;r<2;r++)for(int c=0;c<3;c++){rect(x+c*12,y+r*8,10,6,r?0xffcd55:0x64ffe1);rect(x+c*12,y+r*8,10,1,0xffffff);}disk(x+23,y+23,3,0xffffff);rect(x+3,y+31,28,5,0x64ffe1);rect(x+7,y+31,20,2,0xffffff);}
 else if(game==2){const unsigned short rock[]={0x0f8,0x3fe,0x7ff,0xfff,0xfff,0xffe,0x7fc,0x1f8};for(int r=0;r<8;r++)for(int c=0;c<12;c++)if(rock[r]&(1<<(11-c)))rect(x+c*3,y+r*4,3,4,0xe0a7ff);rect(x+8,y+9,8,6,0x574372);rect(x+22,y+18,6,7,0x574372);rect(x+4,y+4,9,3,0xffffff);rect(x+33,y+29,3,8,0xffcd55);rect(x+30,y+32,9,3,0xffcd55);}
 else if(game==4){rect(x,y+3,27,4,0x64e8ee);rect(x+23,y+3,4,18,0x64e8ee);rect(x+4,y+28,31,4,0x64ffe1);rect(x+4,y+14,4,18,0x64ffe1);rect(x+1,y+10,10,9,0xffffff);rect(x+3,y+12,6,5,0x64ffe1);rect(x+21,y+19,8,8,0xffffff);}
 else if(game==5){const unsigned short alien[]={0x204,0x108,0x7fe,0xdbb,0xfff,0x5fa,0x492,0x906};for(int r=0;r<8;r++)for(int c=0;c<12;c++)if(alien[r]&(1<<(11-c)))rect(x+c*3,y+r*3+3,3,3,0x64ffe1);rect(x+9,y+12,3,3,0xffffff);rect(x+24,y+12,3,3,0xffffff);rect(x+16,y+31,3,5,0xffcd55);}
 else{rect(x+4,y+12,28,20,0xffcd55);rect(x+7,y+7,7,12,0xffcd55);rect(x+23,y+7,7,12,0xffcd55);rect(x+11,y+20,5,5,0x10243c);rect(x+23,y+20,5,5,0x10243c);rect(x+16,y+27,5,3,0x10243c);rect(x+7,y,4,6,0xffffff);rect(x+17,y-3,4,9,0xffffff);rect(x+27,y,4,6,0xffffff);}
}
