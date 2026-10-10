#ifndef RX3_CAFE_STATE_H
#define RX3_CAFE_STATE_H
/* Visits, greetings and chatter are session RAM only. Pending arrivals survive closing early. */
struct cafe_state { unsigned seen,mask,seed,next,arrival_at,speaker,phrase,missing; int arrival,mode,won,reunion,victory; unsigned show_ms; int show_active; };
/* Native touch and the browser both hit the actual record on the right shelf. */
static int cafe_vinyl_hit(int x,int y){return x>=1008&&x<=1098&&y>=344&&y<=420;}
static unsigned cafe_random(struct cafe_state*c){unsigned x=c->seed?c->seed:7319;x^=x<<13;x^=x>>17;x^=x<<5;return c->seed=x;}
static void cafe_open(struct cafe_state*c,unsigned seen,unsigned mask,int won,unsigned seed){
 *c=(struct cafe_state){.seen=seen&mask&15,.mask=mask&15,.seed=seed,.arrival=-1,.won=won,.speaker=4};
}
static void cafe_update(struct cafe_state*c,unsigned ms){
 if(ms<c->next)return;
 if(c->arrival>=0){c->seen|=1u<<c->arrival;c->arrival=-1;}
 unsigned pending=c->mask&~c->seen;
 if(pending){unsigned cat=0;while(!(pending&(1u<<cat)))cat++;c->arrival=(int)cat;c->arrival_at=ms;c->mode=1;c->speaker=4;c->next=ms+5000;return;}
 if(c->mask==15&&!c->reunion){c->reunion=1;c->mode=2;c->speaker=4;c->next=ms+6000;return;}
 if(c->won&&!c->victory){c->victory=1;c->mode=3;c->speaker=4;c->next=ms+6000;return;}
 c->mode=0;unsigned speakers[12],n=0;for(unsigned i=0;i<4;i++)if(c->seen&(1u<<i))speakers[n++]=i;for(unsigned i=4;i<12;i++)speakers[n++]=i;c->speaker=speakers[cafe_random(c)%n];
 unsigned old=c->phrase;c->phrase=(old+1+cafe_random(c)%7)%8;
 unsigned missing[4],m=0;for(unsigned i=0;i<4;i++)if(!(c->mask&(1u<<i)))missing[m++]=i;
 c->missing=m?missing[cafe_random(c)%m]:4;
 c->next=ms+5200+cafe_random(c)%1200;
}
static const char*cafe_speech(const struct cafe_state*c){
 static const char*welcome[]={"YUKI IS HERE! MAKE ROOM AT THE DECKS!","PEACH IS HOME! YOU ROCK, LITTLE ONE!","COOKIE! OUR LITTLE GRID CHAMPION!","MOCHI HAS LANDED! GROUP HUG!"};
 static const char*hopes[]={"I HOPE YUKI COMES BACK SOON...","I HOPE PEACH COMES BACK SOON...","I HOPE COOKIE COMES BACK SOON...","I HOPE MOCHI COMES BACK SOON..."};
 static const char*quips[]={"I HATE THAT MOUSE!!!","THAT MOUSE OWES ME NINE APOLOGIES.","NO MORE MOUSE SETS. MY EARS NEED A NAP.","THIS CAFE HAS A STRICT NO-MOUSE POLICY.","MORE TUNA. LESS SYNC. THAT IS MY RIDER.","DJ DRAGON-X, DROP THE BASS AND THE TREATS!","I SCRATCH VINYL. AND SOMETIMES THE SOFA.","PAWS UP! THIS IS OUR KIND OF AFTERPARTY."};
 if(c->mode==1)return welcome[c->arrival];
 if(c->mode==2)return "ALL FOUR OF US ARE HOME! BEST CREW EVER!";
 if(c->mode==3)return "YOU WON DJ WARS! THE CATS OWN THE NIGHT!";
 if(c->mask==0){
  static const char*pleas[]={"PLEASE BRING YUKI BACK SOON!","PLEASE FIND PEACH. WE MISS HER SO MUCH!","BRING COOKIE HOME. HIS SOFA IS WAITING!","PLEASE HURRY! MOCHI NEEDS YOU!","THAT MOUSE TOOK ALL FOUR OF OUR FRIENDS!","IT IS TOO QUIET WITHOUT OUR FOUR FRIENDS.","PLEASE SAVE THEM, DJ. WE ARE COUNTING ON YOU!","WE WILL KEEP THEIR SPOTS WARM. BRING THEM HOME!"};
  return pleas[c->phrase%8];
 }
 if(c->missing<4&&(c->phrase==2||c->phrase==5))return hopes[c->missing];
 if(c->won&&c->phrase==0)return "MOUSE DEFEATED. FRIENDS REUNITED. PURRFECT!";
 if(c->mask==15&&c->phrase==2)return "YUKI, PEACH, COOKIE, MOCHI - ALL TOGETHER!";
 return quips[c->phrase];
}
#endif
