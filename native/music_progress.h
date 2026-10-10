#ifndef RX3_MUSIC_PROGRESS_H
#define RX3_MUSIC_PROGRESS_H
#include "../shared/rescue_goals.h"
/* Volatile process state. The optional persistent-save worker can snapshot this state. */
static unsigned session_dj_wars_won;
static unsigned session_rescued_cats;
/* Completed levels / arenas, or the wave reached; never a score threshold.
 * Game ids: Equalizarr=1, Bass Rocks=2, Basslines=4, Drop Defense=5. */
static void session_save_cat(int game){int cat=game==1?0:game==2?1:game==4?2:game==5?3:-1;if(cat>=0)session_rescued_cats|=1u<<cat;}
static void session_check_rescue(int game,unsigned progress){
 unsigned goal=game==1?RESCUE_EQUALIZARR_LEVELS:game==2?RESCUE_BASS_ROCKS_WAVE:game==4?RESCUE_BASSLINES_ARENAS:game==5?RESCUE_DROP_DEFENSE_WAVE:0;
 if(goal&&progress>=goal)session_save_cat(game);
}
static int session_finale_ready(void){return session_rescued_cats==15;}
static unsigned session_cat_count(void){unsigned n=0;for(unsigned i=0;i<4;i++)n+=(session_rescued_cats>>i)&1;return n;}
static unsigned music_secret_unlocked,music_credits_mode;
static unsigned music_credits_epoch,music_credits_position,music_credits_total,music_credits_complete;
static void music_unlock_secret(void){if(session_dj_wars_won)__atomic_store_n(&music_secret_unlocked,1,__ATOMIC_RELEASE);}
#endif
