/* Artwork stays outside the executable and is released when debug closes. */
static const char *story_artwork_path="/tmp/rx3-launcher/payload/story-artwork.rgba";
static unsigned char *story_artwork_pixels;static int story_artwork_failed;
static void story_artwork_release(void){free(story_artwork_pixels);story_artwork_pixels=0;story_artwork_failed=0;}
static void draw_story_artwork(void){
 const unsigned size=384*256*4;
 if(!story_artwork_pixels&&!story_artwork_failed){
  story_artwork_failed=1;int fd=open(story_artwork_path,0);
  if(fd>=0){unsigned char *p=malloc(size);unsigned used=0;
   if(p){while(used<size){long n=read(fd,p+used,size-used);if(n<=0)break;used+=(unsigned)n;}if(used==size)story_artwork_pixels=p;else free(p);}close(fd);
  }
 }
 if(!story_artwork_pixels)return;
 for(int yy=0;yy<256;yy++)for(int xx=0;xx<384;xx++){
  int x=790+xx,y=315+yy;if(x>=width||y>=height)continue;
  unsigned char *v=story_artwork_pixels+4*(yy*384+xx);unsigned a=v[3];if(!a)continue;
  uint32_t *d=pixels+y*width+x;
  unsigned r=(v[0]*a+((*d>>16)&255)*(255-a))/255,g=(v[1]*a+((*d>>8)&255)*(255-a))/255,b=(v[2]*a+(*d&255)*(255-a))/255;
  *d=(r<<16)|(g<<8)|b;
 }
}
