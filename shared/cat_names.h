#ifndef RX3_CAT_NAMES_H
#define RX3_CAT_NAMES_H
/* Names follow sprite IDs so every scene refers to the same little friends. */
#define CAT_NAME_COUNT 11
static const char*const cat_names[CAT_NAME_COUNT]={
 "SUNNY","COOKIE","LUNA","PEACH","MOCHI","CHAI",
 "MISO","OREO","NALA","FUDGE","YUKI"
};
static const char*cat_name(unsigned id){return id<CAT_NAME_COUNT?cat_names[id]:"CAT";}
#endif
