#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../source/game.c"
static void battle(Game*g,int c){assert(ascent_begin(g,c,c,160+c));g->screen=MAP;game_start_battle(g,NODE_FIGHT);for(int i=0;i<g->enemy_count;i++)g->enemies[i].hp=g->enemies[i].maxhp=10000;g->energy=20;}
int main(void){char root[]="/tmp/rift-storage-XXXXXX";assert(mkdtemp(root));assert(ascent_open(root));Game g[4],a;
 for(int i=0;i<4;i++){assert(ascent_begin(&g[i],i,i%3,10+i));g[i].gold=100+i;assert(ascent_checkpoint(&g[i]));}
 assert(ascent_open(root));for(int i=0;i<4;i++){assert(ascent_resume(&a,i));assert(a.gold==100+i&&a.ascent.id==g[i].ascent.id);}
 /* Corrupt newest generation: recover the untouched preceding checkpoint. */
 unsigned gen=ascent_slot(2)->generation;char path[320];snprintf(path,sizeof(path),"%s/ASCENT18_RUN_3.%u",root,gen&1u);FILE*f=fopen(path,"wb");fputs("torn write",f);fclose(f);
 assert(ascent_resume(&a,2));assert(a.ascent.id==g[2].ascent.id&&a.gold!=102);
 assert(ascent_set_debug(1));assert(ascent_open(root)&&ascent_debug());assert(ascent_set_debug(0));
 /* A failed save cannot grant account progress or lose a run in memory. */
 AscentAccount old=ascent_account;char good[256];strcpy(good,ascent_root);strcpy(ascent_root,"/nonexistent/rift-write-failure");a=g[0];a.act=3;assert(!ascent_complete(&a,0));assert(!memcmp(&old,&ascent_account,sizeof(old)));strcpy(ascent_root,good);
 /* Custom Vesper skills respect Pounce, Burst, Afterimage and Sly replay. */
 battle(&a,1);a.hand_count=1;a.hand[0]=355;a.ninja.v[NS_POUNCE]=1;a.ninja.v[NS_BURST]=1;a.ninja.v[NS_AFTERIMAGE]=2;int energy=a.energy;
 game_play_card(&a,0,0);assert(a.energy==energy&&a.turn_played==2&&a.block==18&&a.ninja.v[NS_SKILLS_TURN]==2&&!a.ninja.v[NS_BURST]);
 battle(&a,1);a.hand_count=1;a.hand[0]=355;a.ninja.v[NS_PLANNER]=1;game_play_card(&a,0,0);assert(a.discard_count>0&&a.discard[a.discard_count-1]&CARD_SLY);
 int raw=a.discard[--a.discard_count];a.block=0;ninja_push(&a,NT_PLAY,raw|CARD_FREE,0,0,0,0);ninja_process(&a);assert(a.block==7);
 battle(&a,1);a.hand_count=2;a.hand[0]=364;a.hand[1]=355|CARD_SLY;a.block=0;game_play_card(&a,0,0);ninja_process(&a);assert(a.block==7&&a.ninja.v[NS_DISCARDS_TURN]==1);
 /* Created/replayed Proxy cards resolve through their custom effects. */
 battle(&a,2);int stars=a.regent.stars;regent_push(&a,RT_PLAY,374|CARD_FREE,0,0,0,0);regent_process(&a);assert(a.turn_played==1&&a.regent.stars>stars);
 /* Locked cards cannot leak into generated colorless offers. */
 for(int i=0;i<100;i++){a.regent.task_count=0;regent_offer(&a,R_HEIRLOOM_HAMMER,0,0,0,0);for(int j=0;j<a.regent.option_count;j++)assert(CARD_ID(a.regent.options[j])>=COLORLESS_FIRST&&CARD_ID(a.regent.options[j])<EXTRA_LAST);}
 /* Every rarity distribution still finishes on ten wins across many seeds. */
 for(int seed=0;seed<500;seed++){memset(ascent_account.cards,0,sizeof(ascent_account.cards));Game pair;game_new(&pair,seed+1);pair.character=seed%3;for(int win=0;win<10;win++){int p[2];ascent_unlock_cards(&pair,p);assert(p[0]!=p[1]&&cards[p[0]].rarity!=cards[p[1]].rarity);assert(ascent_popcount(ascent_account.cards[pair.character])==2*(win+1));}}
 /* Stable v23 snapshots import once and leave the original file intact. */
 char legacy[]="/tmp/rift-legacy-XXXXXX";assert(mkdtemp(legacy));game_new(&a,17);game_choose_character(&a,2);a.screen=MAP;a.gold=777;a.version=23;size_t bytes=offsetof(Game,ascent);unsigned sum=hash_bytes((void*)&a,bytes);snprintf(path,sizeof(path),"%s/SAVE.DAT",legacy);f=fopen(path,"wb");assert(f);fwrite(&sum,4,1,f);fwrite(&a,bytes,1,f);fclose(f);
 assert(ascent_open(legacy)&&ascent_resume(&a,0));assert(a.gold==777&&a.character==2&&a.version==GAME_VERSION&&a.ascent.id);unsigned imported=a.ascent.id;assert(ascent_open(legacy)&&ascent_resume(&a,0)&&a.ascent.id==imported);f=fopen(path,"rb");assert(f);unsigned stored;assert(fread(&stored,4,1,f)==1&&stored==sum);fclose(f);
 puts("PASS four independent slots; torn-write recovery; global debug; failed-write rollback; Vesper/Proxy synergies and discard triggers; locked generation; 500 ten-win rarity schedules; stable-save import");return 0;}
