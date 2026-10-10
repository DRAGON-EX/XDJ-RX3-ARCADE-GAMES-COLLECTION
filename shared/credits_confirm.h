/* Same retro confirmation screen on the RX3 and in the browser. */
static void credits_confirm_render(int step){
 cr_rect(0,130,1280,670,0x080f1b);
 cr_rect(160,240,960,360,0x10243c);cr_rect(160,240,960,4,0x64ffe1);
 cr_text(220,280,5,"CREDITS",0x64ffe1);
 if(step==0){cr_text(220,360,3,"DO YOU REALLY WANT TO PUT YOURSELF",0xffffff);cr_text(220,400,3,"THROUGH THIS VOLUNTARILY",0xffffff);}
 else if(step==1)cr_text(220,370,5,"ARE YOU SURE",0xffffff);
 else cr_text(220,365,7,"REAAAAALLY",0x64ffe1);
 cr_rect(240,500,340,64,0x14283d);cr_text(300,520,3,"NO - GO BACK",0xffffff);
 cr_rect(700,500,340,64,0x763044);cr_text(780,520,3,"YES",0xffffff);
}

static void credits_locked_render(void){
 cr_rect(0,130,1280,670,0x080f1b);
 cr_rect(100,210,1080,460,0x10243c);cr_rect(100,210,1080,4,0x64ffe1);
 cr_text(145,251,5,"HEY DJ!",0x64ffe1);
 cr_text(145,328,3,"YOU STILL HAVEN'T SAVED ALL THE CATS.",0xffffff);
 cr_text(145,373,3,"HOW DARE YOU WATCH THE CREDITS",0xffffff);
 cr_text(145,418,3,"WHEN YOU'RE NOT DONE YET?",0xffffff);
 cr_text(145,478,3,"THE POOR FELLAS NEED YOU.",0xffcd55);
 cr_text(145,526,2,"WIN DJ WARS TO UNLOCK THE CREDITS.",0x8fa1b5);
 cr_rect(420,582,440,56,0x763044);cr_text(466,599,3,"BACK TO THE RESCUE",0xffffff);
}

static void rescue_locked_render(unsigned mask){
 cr_rect(0,130,1280,670,0x080f1b);
 cr_rect(100,190,1080,490,0x10243c);cr_rect(100,190,1080,4,0xffcd55);
 cr_text(145,226,5,"DJ WARS - FINAL BATTLE",0xffcd55);
 cr_text(145,291,3,"RESCUE ALL FOUR CATS TO ENTER.",0xffffff);
 const char*names[]={"YUKI - EQUALIZARR","PEACH - BASS ROCKS","COOKIE - BASSLINES","MOCHI - DROP DEFENSE"};
 for(unsigned i=0;i<4;i++){cr_text(145,354+i*45,3,names[i],0xffffff);cr_text(865,358+i*45,2,(mask&(1u<<i))?"RESCUED":"NEEDS YOU",(mask&(1u<<i))?0x64ffe1:0xffcd55);}
 cr_rect(420,582,440,56,0x763044);cr_text(466,599,3,"BACK TO THE RESCUE",0xffffff);
}
