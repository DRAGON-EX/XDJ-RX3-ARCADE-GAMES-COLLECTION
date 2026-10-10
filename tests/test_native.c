#define main rx3_program_main
#include "../native/arcade.c"
#undef main
/* Keep display/touch regression independent of the external emulator core. */
int nes_open(const char*r,const char*c){(void)r;(void)c;return 0;}
void nes_close(void){}
int nes_video_enabled=1;
uint32_t nes_video[512*480];unsigned nes_width,nes_height;
#define CHECK(c) do {if(!(c)){printf("FAIL line %d\n",__LINE__);return 1;}}while(0)
static void sample(int x,int y,int down){int rx=tx_min+(width-x)*tx_factor*4/width,ry=ty_min+y*ty_factor*4/height;unsigned char b[6]={down,0,rx,rx>>8,ry,ry>>8};raw_touch_sample(b);}
static void fader_sample(unsigned raw){unsigned char b[104]={0,0,0,1};b[22]=raw;b[23]=raw>>8;nes_sub_decode(&steering,b,sizeof(b));}
static int test_asteroid_jog(void){
 unsigned char b[104]={0,0,0,1};unsigned counter=65000;uint64_t now=1000000000ULL;
 selected_game=2;menu=0;paused=0;reset_current_game();
 #define JOG(step) do{counter=(counter+(step))&65535u;b[14]=counter;b[15]=counter>>8;now+=16666667ULL;asteroid_jog_sample(b,sizeof(b),now);}while(0)
 JOG(0);CHECK(asteroid.ship.angle==0); /* First packet only establishes a baseline. */
 JOG(405);CHECK(asteroid.ship.angle==64);
 JOG(-405);CHECK(asteroid.ship.angle==0);
 for(int i=0;i<4;i++)JOG(405);CHECK(asteroid.ship.angle==0); /* Full turn, including counter wrap. */
 for(int i=0;i<4;i++)JOG(-405);CHECK(asteroid.ship.angle==0); /* Reverse wrap. */
 for(int i=0;i<30;i++){JOG(1);JOG(-1);}CHECK(asteroid.ship.angle==0);
 b[30]=255;b[31]=255;JOG(0);CHECK(asteroid.ship.angle==0); /* Right deck cannot steer. */
 JOG(2);JOG(2);JOG(3);CHECK(asteroid.ship.angle==1); /* Slow movement accumulates. */
 JOG(-7);CHECK(asteroid.ship.angle==0);
 asteroid.ship.angle=250;JOG(405);CHECK(asteroid.ship.angle==58);
 ast_tick(&asteroid,AST_LEFT|AST_UP|AST_FIRE);CHECK(asteroid.ship.angle==55&&asteroid.shots[0].active&&asteroid.thrust);
 ast_tick(&asteroid,AST_RIGHT);CHECK(asteroid.ship.angle==58); /* Buttons remain additive. */
 for(int gate=0;gate<6;gate++){
  int before=asteroid.ship.angle;
  if(gate==0)menu=1;else if(gate==1)paused=1;else if(gate==2)rescue.active=1;
  else if(gate==3)asteroid.ship.active=0;else if(gate==4)asteroid.game_over=1;else selected_game=1;
  JOG(405);CHECK(asteroid.ship.angle==before);
  menu=paused=rescue.active=asteroid.game_over=0;asteroid.ship.active=1;selected_game=2;
  JOG(405);CHECK(asteroid.ship.angle==before); /* No queued motion on resume. */
  JOG(405);CHECK(asteroid.ship.angle==((before+64)&255));
 }
 int before=asteroid.ship.angle;
 now+=300000000ULL;JOG(405);CHECK(asteroid.ship.angle==before);
 JOG(405);CHECK(asteroid.ship.angle==((before+64)&255));before=asteroid.ship.angle;
 JOG(4096);CHECK(asteroid.ship.angle==before); /* Corrupt/reset counters do not spin the ship. */
 JOG(405);CHECK(asteroid.ship.angle==((before+64)&255));before=asteroid.ship.angle;
 b[3]=2;JOG(405);CHECK(asteroid.ship.angle==before);b[3]=1;
 JOG(405);CHECK(asteroid.ship.angle==before);JOG(405);CHECK(asteroid.ship.angle==((before+64)&255));
 before=asteroid.ship.angle;asteroid_jog_sample(b,14,now);JOG(405);CHECK(asteroid.ship.angle==before);
 counter=(counter+405)&65535u;b[14]=counter;b[15]=counter>>8;now+=16666667ULL;asteroid_jog_sample(b,100,now);CHECK(asteroid.ship.angle==((before+64)&255));
 reset_current_game();JOG(405);CHECK(asteroid.ship.angle==0);
 back_to_menu();selected_game=2;choose_game();JOG(405);CHECK(asteroid.ship.angle==0);
 JOG(405);CHECK(asteroid.ship.angle==64);back_to_menu();
 #undef JOG
 printf("Bass Rocks left jog: proportional turns, both wraps, slow motion, jitter, deck isolation, button combination, pause/rescue/menu and reconnect guards passed\n");return 0;
}
static int test_crossfader_games(void){
 /* Exercise the exact input/step path used by the ARM loop in both games. */
 for(int game=1;game<=5;game+=4){
  selected_game=game;nes_held=nes_a=0;fader_sample(512);reset_current_game();
  int lo=game==1?72:22,hi=840-lo;
  for(int phase=0;phase<12;phase++){
   if(phase==1)fader_sample(514); /* ADC noise does not take control. */
   if(phase==2)fader_sample(0);
   if(phase==3)fader_sample(1023);
   if(phase==4)nes_held=4; /* Buttons win even at a fader end stop. */
   if(phase==5)nes_held=0;
   if(phase==6)fader_sample(512);
   if(phase==7){if(game==1)equalizarr_pointer=300;else invaders_pointer=300;}
   if(phase==8){equalizarr_pointer=invaders_pointer=-1;}
   if(phase==9)fader_sample(0);
   if(phase==10)fader_sample(1024); /* Invalid sample releases analog ownership. */
   if(phase==11)fader_sample(1023); /* Reconnection only establishes a baseline. */
   if(game==1)equalizarr_step();else invaders_step();
   float x=game==1?equalizarr.paddle:invaders.player;
   if(phase<=1||phase==6)CHECK(x==420);
   if(phase==2||phase>=9)CHECK(x==lo);
   if(phase==3)CHECK(x==hi);
   if(phase==4||phase==5)CHECK(x==hi-(game==1?9:5.5f));
   if(phase==7||phase==8)CHECK(x==300);
  }
  reset_current_game();if(game==1)equalizarr_step();else invaders_step();
  CHECK((game==1?equalizarr.paddle:invaders.player)==420); /* New game never jumps. */
 }
 selected_game=1;fader_sample(512);reset_current_game();equalizarr.power[EQ_WIDE]=100;
 fader_sample(0);equalizarr_step();CHECK(equalizarr.paddle==104);
 fader_sample(1023);equalizarr_step();CHECK(equalizarr.paddle==736);
 nes_a=1;equalizarr_step();CHECK(!equalizarr.ball[0].stuck); /* A still launches. */
 selected_game=5;reset_current_game();fader_sample(0);invaders_step();CHECK(invaders.shot[0].active); /* A still fires. */
 nes_a=0;back_to_menu();printf("Crossfader endpoints, noise, button/touch handover, invalid samples, reset and fire passed\n");return 0;
}
static int test_rescue_cafe(void){
 const unsigned ids[]={1,2,4,5},goals[]={10,4,8,4};session_rescued_cats=0;
 menu=1;help_page=0;menu_touch(600,720);CHECK(help_page==11&&cafe.mask==0);help_page=0;
 for(unsigned i=0;i<4;i++){
  selected_game=(int)ids[i];reset_current_game();unsigned before=session_rescued_cats;
  for(unsigned k=goals[i]-1;k<=goals[i];k++){
   if(i==0)equalizarr.cleared_levels=k;else if(i==1)asteroid.wave=k;else if(i==2)basslines.score[0]=k;else invaders.wave=(int)k;
   update_game_rescue();if(k<goals[i])CHECK(session_rescued_cats==before&&!rescue.active);
  }
  CHECK(rescue.active&&rescue.cat==i);rescue.ticks=77;update_game_rescue();CHECK(rescue.ticks==77);
  menu=0;paused=0;sample(65,650,1);CHECK(!paused);rescue.active=0;reset_current_game();CHECK(session_rescued_cats==(before|(1u<<i)));
 }
 menu=1;help_page=0;menu_touch(600,720);CHECK(help_page==11&&cafe.mask==15);menu_touch(1120,180);CHECK(!help_page&&session_rescued_cats==15);
 session_rescued_cats=0;rescue.active=0;printf("Native rescue boundaries, no repeat, input lock, RAM persistence and cafe gate passed\n");return 0;
}
static int test_dj_finale_gate(void){
 width=1280;height=800;selected_game=3;menu=0;paused=0;session_rescued_cats=15;session_dj_wars_won=0;
 music_count=2;music_tracks[0]=(struct music_track){0,100,"FIRST","",0};music_tracks[1]=(struct music_track){100,200,"SECRET","",2};
 for(int assisted=0;assisted<2;assisted++){
  menu=0;help_page=0;credits_completed=0;session_dj_wars_won=music_secret_unlocked=0;
  music_position=42;music_paused=assisted;music_seek_request=~0u;
  CHECK(dj_wars_reset());for(int i=assisted?0:5;i<(assisted?5:10);i++)dj_wars->unit[i].hp=0;
  dw_tick(dj_wars,0);dj_wars_result();CHECK(dj_wars->state==DW_FINALE&&!session_dj_wars_won);
  dj_wars_touch(750,250);CHECK(!menu);CHECK(!dj_wars_begin_credits(1)&&dj_wars);
  int duration=assisted?DW_ASSIST_TICKS:DW_VICTORY_TICKS;
  for(int i=0;i<duration-1;i++)dw_tick(dj_wars,0);
  CHECK(!dj_wars_begin_credits(2)&&!menu&&!credits_active);
  dw_tick(dj_wars,0);CHECK(dj_wars_begin_credits(1000));
  CHECK(session_dj_wars_won&&menu&&help_page==5&&!paused&&!dj_wars&&!dw_back);
  CHECK(credits_active&&!credits_completed&&!music_secret_unlocked&&music_credits_mode&&music_seek_request==100);
  CHECK(credits_saved_position==42&&credits_saved_pause==(unsigned)assisted);
  unsigned epoch=music_credits_epoch;CHECK(!dj_wars_begin_credits(2000)&&music_credits_epoch==epoch);
  CHECK(!credits_to_cafe()); /* Credits cannot finish before both full audio passes. */
  music_credits_position=music_credits_total;credits_update(3000,1);CHECK(!credits_to_cafe());
  music_credits_complete=1;credits_update(4000,1);CHECK(credits_completed&&music_secret_unlocked);
  CHECK(credits_to_cafe());CHECK(menu&&help_page==11&&!credits_active&&!music_credits_mode);
  CHECK(music_seek_request==42&&music_paused==(unsigned)assisted&&cafe.mask==15&&cafe.won);
  CHECK(!credits_to_cafe()); /* No automatic reopening after closing the cafe. */
  help_page=0;CHECK(!dj_wars_begin_credits(5000)&&!credits_to_cafe());
 }
 session_rescued_cats=session_dj_wars_won=music_secret_unlocked=0;selected_game=0;help_page=0;music_count=0;
 printf("Native normal/assisted finale -> credits -> cafe: immediate handoff, once-only, two full song passes, RAM gates and playlist restore passed\n");return 0;
}

int main(void){
 if(test_asteroid_jog())return 1;
 if(test_rescue_cafe()||test_dj_finale_gate())return 1;
 debug_unlocked=1;for(unsigned c=0;c<4;c++){menu=1;help_page=4;menu_touch(120+c*260,740);CHECK(rescue.active&&rescue.cat==c&&debug_rescue_test&&!menu);rescue.active=0;}debug_unlocked=debug_rescue_test=0;menu=1;help_page=0;
 float saved_gain=master_target;tick=100;volume_until=0;gain_seen[0]=gain_seen[1]=0;
 gain_display_update(0,0,1);CHECK(volume_until==0);gain_display_update(0,1023,1);CHECK(volume_kind==1&&gain_percent[0]==100&&volume_until==220);
 gain_display_update(1,0,1);gain_display_update(1,512,1);CHECK(volume_kind==2&&gain_percent[1]==50&&gain_percent[0]==100);
 gain_display_update(1,1024,1);gain_display_update(1,0,0);CHECK(gain_percent[1]==50&&master_target==saved_gain);volume_until=0;
 width=11;height=7;pixels=malloc(width*height*4);previous=malloc(width*height*4);packed_row=malloc(width*4);
 for(int bpp=16;bpp<=32;bpp+=8){
  struct fb_var v={.bpp=bpp,.xoffset=2,.yoffset=1};
  if(bpp==16){v.red=(struct fb_bits){11,5,0};v.green=(struct fb_bits){5,6,0};v.blue=(struct fb_bits){0,5,0};}
  else{v.red=(struct fb_bits){16,8,0};v.green=(struct fb_bits){8,8,0};v.blue=(struct fb_bits){0,8,0};if(bpp==32)v.transp=(struct fb_bits){24,8,0};}
  struct fb_fix f={.line_length=64};unsigned char fb[640],expect[640];
  for(int i=0;i<640;i++)fb[i]=expect[i]=0xa5;
  for(int i=0;i<width*height;i++)pixels[i]=(i%3==0)?0xff0000:(i%3==1)?0x00ff00:0x0000ff;
  first_blit=1;fb_bytes=0;
  for(int pass=0;pass<4;pass++){
   if(pass==2)pixels[4*width+5]=0xffffff;
   if(pass==3)for(int x=2;x<8;x++)pixels[2*width+x]=0;
   for(int y=0;y<height;y++)for(int x=0;x<width;x++){
    unsigned p=pixels[y*width+x];unsigned out=bpp==16?(((p>>19)&31)<<11)|(((p>>10)&63)<<5)|((p>>3)&31):p|(bpp==32?0xff000000:0);
    for(int byte=0;byte<bpp/8;byte++)expect[(y+1)*64+(x+2)*(bpp/8)+byte]=out>>(byte*8);
   }
   nes_active=1;menu=0;
   if(pass==2)mark_dirty(5,4,1,1);
   if(pass==3)mark_dirty(2,2,6,1);
   unsigned long long before=fb_bytes;blit(fb,&v,&f);
   for(int i=0;i<640;i++)CHECK(fb[i]==expect[i]);
   if(pass==1)CHECK(fb_bytes==before);
   if(pass==2)CHECK(fb_bytes-before==(unsigned)bpp/8);
  }
 }
 nes_active=0;selected_game=3;menu=0;
 {
  struct fb_var v={.bpp=16,.xoffset=2,.yoffset=1,.red={11,5,0},.green={5,6,0},.blue={0,5,0}};
  struct fb_fix f={.line_length=64};unsigned char fb[640],expect[640];
  for(int i=0;i<640;i++)fb[i]=expect[i]=0xa5;
  first_blit=1;fb_bytes=0;
  for(int pass=0;pass<6;pass++){
   if(pass==0||pass==4)for(int i=0;i<width*height;i++)pixels[i]=(unsigned)i*3197+pass*113;
   if(pass==1)pixels[4*width+5]=0xffffff;
   if(pass==2)pixels[width-1]=0xff3300;
   if(pass==3)for(int i=0;i<width*height;i++)pixels[i]=0;
   for(int y=0;y<height;y++)for(int x=0;x<width;x++){
    unsigned p=packed(pixels[y*width+x],&v),off=(y+1)*64+(x+2)*2;
    expect[off]=p;expect[off+1]=p>>8;
   }
   unsigned long long before=fb_bytes;blit(fb,&v,&f);
   for(int i=0;i<640;i++)CHECK(fb[i]==expect[i]);
   if(pass==5)CHECK(fb_bytes==before);
  }
 }
 selected_game=0;
 free(pixels);free(previous);free(packed_row);
 width=1280;height=800;menu=0;touch_down=0;paused=0;direction=0;running=1;
 sample(128,710,1);CHECK(direction==1);sample(0,0,0);
 sample(65,650,1);CHECK(paused==1);sample(65,650,1);CHECK(paused==1);sample(0,0,0);
 sample(165,565,1);CHECK(menu==1&&running==1);sample(0,0,0);
 sample(400,180,1);CHECK(menu==0);sample(0,0,0);
 sample(165,565,1);CHECK(menu==1);sample(0,0,0);
 sample(400,230,1);CHECK(menu==0&&selected_game==1&&!nes_active&&equalizarr.remaining>0);sample(0,0,0);back_to_menu();
 menu_touch(1085,55);CHECK(help_page==2);
 menu_touch(400,180);CHECK(menu==1&&help_page==2);
 menu_touch(400,260);CHECK(help_page==2);
 help_scroll(999);CHECK(help_offset==((help_count()-1)/10)*10);help_scroll(-999);CHECK(help_offset==0);
 menu_touch(1100,180);CHECK(help_page==0);
 menu_touch(1160,55);CHECK(help_page==3);
 menu_touch(400,260);CHECK(help_page==3);
 menu_touch(1100,200);CHECK(help_page==0);
 version_tap(1000000000ULL);version_tap(1200000000ULL);version_tap(1400000000ULL);version_tap(1600000000ULL);CHECK(help_page==0);
 version_tap(2200000000ULL);CHECK(debug_taps==1&&help_page==0);
 version_tap(2300000000ULL);version_tap(2400000000ULL);version_tap(2500000000ULL);version_tap(2600000000ULL);CHECK(help_page==4&&debug_data.captures==1);
 debug_update(debug_next_update-1);CHECK(debug_data.captures==1);
 debug_update(debug_next_update);CHECK(debug_data.captures==2);
 menu_touch(400,260);CHECK(help_page==4&&debug_data.captures==2);menu_touch(1100,180);CHECK(help_page==0&&debug_data.captures==2);
 debug_update(debug_next_update+1000000000ULL);CHECK(debug_data.captures==2);
 pixels=malloc((size_t)width*height*4);volume_percent=75;volume_until=120;tick=1;draw_menu();uint32_t before_volume=pixels[255*width+width/2];draw_volume();CHECK(pixels[255*width+width/2]!=before_volume);draw_menu();tick=120;draw_volume();CHECK(pixels[255*width+width/2]==before_volume);free(pixels);pixels=0;
 sample(150,720,1);CHECK(running==1&&menu&&help_page==10);sample(0,0,0);
 menu_touch(400,180);CHECK(running==1&&help_page==10);
 menu_frame_valid=0;CHECK(display_needs_redraw());CHECK(!display_needs_redraw());
 menu_touch(1100,180);CHECK(running==1&&help_page==0);
 pixels=malloc((size_t)width*height*4);nes_width=256;nes_height=240;nes_active=1;menu=0;paused=0;nes_background=nes_volume_was=nes_pause_was=0;volume_until=0;
 for(unsigned i=0;i<256*240;i++)nes_video[i]=0x123456;
 draw();int sc=(height-100)/240;if(sc>3)sc=3;int left=(width-256*sc)/2;CHECK(pixels[(45+10*sc)*width+left+10*sc]==0x123456);
 nes_video[10*256+10]=0xabcdef;draw();CHECK(pixels[(45+10*sc)*width+left+10*sc]==0xabcdef);CHECK(pixels[(45+11*sc)*width+left+10*sc]==0x123456);
 volume_until=tick+120;draw_volume();draw();volume_until=0;draw();CHECK(pixels[(45+10*sc)*width+left+10*sc]==0xabcdef);
 previous=malloc((size_t)width*height*4);packed_row=malloc(width*4);
 unsigned char *nes_fb=malloc((size_t)width*height*2);
 struct fb_var nv={.bpp=16,.red={11,5,0},.green={5,6,0},.blue={0,5,0}};
 struct fb_fix nf={.line_length=width*2};first_blit=1;
 for(int phase=0;phase<5;phase++){
  if(phase==1)nes_video[20*256+20]=0xff3344;
  volume_until=phase==2?tick+120:0;paused=phase==3;
  draw();draw_volume();blit(nes_fb,&nv,&nf);
  for(int i=0;i<width*height;i++)CHECK(((uint16_t*)nes_fb)[i]==packed(pixels[i],&nv));
 }
 free(nes_fb);free(previous);free(packed_row);previous=0;packed_row=0;
 free(pixels);pixels=0;nes_active=0;
 selected_game=3;menu=1;paused=0;session_rescued_cats=0;choose_game();CHECK(menu&&help_page==9&&!dj_wars);
 session_save_cat(0);CHECK(!session_rescued_cats);session_save_cat(1);session_save_cat(2);session_save_cat(4);choose_game();CHECK(menu&&help_page==9);
 session_save_cat(5);menu_touch(500,600);CHECK(!help_page);choose_game();CHECK(!menu&&dj_wars&&dw_back);
 CHECK(dj_wars_buttons(1,0)==DW_UP&&dj_wars_buttons(2,0)==DW_DOWN);
 CHECK(dj_wars_buttons(4,0)==DW_LEFT&&dj_wars_buttons(8,0)==DW_RIGHT);
 CHECK(dj_wars_buttons(0,1)==DW_FIRE);
 int old_power=dj_wars->power;for(int i=0;i<20;i++)dw_tick(dj_wars,dj_wars_buttons(0,1));
 CHECK(dj_wars->charging&&dj_wars->power>old_power);dw_tick(dj_wars,dj_wars_buttons(0,0));CHECK(dj_wars->state==DW_FLIGHT);
 dj_wars_touch(60,755);CHECK(paused);dj_wars_touch(60,755);CHECK(!paused);
 reset_current_game();dj_wars_touch(90,380);CHECK(dj_wars->weapon==1);
 reset_current_game();nes_held=nes_a=0;
 unsigned advances=0;int initial_timer=dj_wars->remaining;
 for(unsigned i=0;i<=15;i++)advances+=dj_wars_advance(1000000000ULL+i*66666668ULL,0);
 CHECK(advances==61&&dj_wars->remaining==initial_timer-61);
 CHECK(!dj_wars_advance(2000000000ULL,0));
 unsigned resets=dj_wars_clock_resets;CHECK(dj_wars_advance(3000000000ULL,0)==6&&dj_wars_clock_resets==resets+1);
 dj_wars_next_step=0;CHECK(dj_wars_advance(9000000000ULL,0)==1);
 CHECK(dj_wars_advance(0,1)==1);
 dj_wars_touch(235,755);CHECK(menu&&!dj_wars&&!dw_back);
 menu=1;help_page=0;volume_until=0;menu_frame_valid=0;
 CHECK(display_needs_redraw());CHECK(!display_needs_redraw());
 selected_game=2;CHECK(display_needs_redraw());CHECK(!display_needs_redraw());
 theme=!theme;CHECK(display_needs_redraw());CHECK(!display_needs_redraw());
 volume_until=tick+2;CHECK(display_needs_redraw());CHECK(!display_needs_redraw());
 volume_percent++;CHECK(display_needs_redraw());tick+=2;CHECK(display_needs_redraw());CHECK(!display_needs_redraw());
 help_page=3;CHECK(display_needs_redraw());CHECK(display_needs_redraw());help_page=0;CHECK(display_needs_redraw());
 menu=0;CHECK(display_needs_redraw());menu=1;CHECK(display_needs_redraw());
 selected_game=1;menu=1;choose_game();CHECK(!menu&&!nes_active&&equalizarr.remaining>0);
 equalizarr_touch(900,650,1,1);CHECK(equalizarr_pointer==518&&(equalizarr_keys&EQ_FIRE));
 equalizarr_touch(940,650,1,0);CHECK(equalizarr_pointer==558);
 equalizarr_touch(940,650,0,0);CHECK(equalizarr_pointer==-1&&!(equalizarr_keys&EQ_FIRE));
 equalizarr_touch(80,660,1,1);CHECK(paused);equalizarr_touch(80,660,1,1);CHECK(!paused);
 equalizarr_touch(120,730,1,1);CHECK(menu);
 selected_game=4;menu=1;choose_game();CHECK(!menu&&!nes_active&&basslines.round==1);
 basslines_touch(165,575,1,1);CHECK(basslines_touch_keys==BL_UP);
 basslines_touch(165,625,1,0);CHECK(basslines_touch_keys==BL_BOOST);
 basslines_touch(0,0,0,0);CHECK(!basslines_touch_keys);
 basslines_touch(80,495,1,1);CHECK(paused);basslines_touch(80,495,1,1);CHECK(!paused);
 basslines_touch(140,740,1,1);CHECK(menu&&selected_game==4);
 menu_touch(400,350);CHECK(!menu&&selected_game==4);back_to_menu();
 menu_touch(400,405);CHECK(!menu&&selected_game==5&&invaders.remaining==55);
 invaders_touch(900,660,1,1);CHECK(invaders_pointer==518&&(invaders_keys&SI_FIRE));invaders_touch(0,0,0,0);CHECK(!invaders_keys&&invaders_pointer==-1);back_to_menu();
 selected_game=ROAD_FIGHTER_GAME;back_to_menu();CHECK(selected_game==0);
 menu=1;help_page=7;help_offset=0;menu_frame_valid=0;
 CHECK(help_count()>20);CHECK(display_needs_redraw());CHECK(!display_needs_redraw());
 story_continue();CHECK(help_page==7&&help_offset==10);CHECK(display_needs_redraw());CHECK(!display_needs_redraw());
 help_scroll(-1);CHECK(help_offset==0);
 menu_touch(width-250,height-100);CHECK(help_offset==10);
 story_continue();CHECK(help_offset==20);while(help_page==7)story_continue();CHECK(help_page==0&&help_offset==0);CHECK(display_needs_redraw());
 help_page=7;menu_touch(width-160,180);CHECK(help_page==0);
 printf("Story pagination, skip and static redraw checks passed\n");
 printf("NES cached rows and overlay restoration passed\n");
 printf("Native framebuffer 16/24/32, padding, changed spans and RX3 touch controls passed\n");
 CHECK(test_crossfader_games()==0);
 return 0;
}
