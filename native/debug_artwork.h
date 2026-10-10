/* Artwork stays outside the executable and is released when debug closes. */
static const char *debug_artwork_path="/tmp/rx3-launcher/payload/debug-artwork.rgba";
static unsigned char *debug_artwork_pixels;static int debug_artwork_failed;
static void debug_artwork_release(void){free(debug_artwork_pixels);debug_artwork_pixels=0;debug_artwork_failed=0;}
static void draw_debug_artwork(void){
 const unsigned size=440*440*4;
 if(!debug_artwork_pixels&&!debug_artwork_failed){
  debug_artwork_failed=1;int fd=open(debug_artwork_path,0);
  if(fd>=0){unsigned char *p=malloc(size);unsigned used=0;
   if(p){while(used<size){long n=read(fd,p+used,size-used);if(n<=0)break;used+=(unsigned)n;}if(used==size)debug_artwork_pixels=p;else free(p);}close(fd);
  }
 }
 if(!debug_artwork_pixels)return;
 for(int yy=0;yy<440;yy++)for(int xx=0;xx<440;xx++){
  int x=730+xx,y=250+yy;if(x>=width||y>=height)continue;
  unsigned char *v=debug_artwork_pixels+4*(yy*440+xx);unsigned a=v[3];if(!a)continue;
  uint32_t *d=pixels+y*width+x;
  unsigned r=(v[0]*a+((*d>>16)&255)*(255-a))/255,g=(v[1]*a+((*d>>8)&255)*(255-a))/255,b=(v[2]*a+(*d&255)*(255-a))/255;
  *d=(r<<16)|(g<<8)|b;
 }
}
