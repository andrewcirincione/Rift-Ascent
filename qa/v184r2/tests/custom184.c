#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../source/game.c"
static void add(Game*g,int tab,int id,int copies){g->screen=CUSTOM_CARDS;g->revision184.tab=tab;int i=0;while(custom_id(g,tab,i)!=id){assert(i<400);i++;}g->selection=i;game_action(g,BTN_OK);assert(g->screen==CUSTOM_QUANTITY);g->revision184.quantity=copies;game_action(g,BTN_OK);assert(g->screen==CUSTOM_CARDS);}
static void item(Game*g,int tab,int id,int copies){g->screen=CUSTOM_ITEMS;g->revision184.tab=tab;int i=0;while(custom_id(g,tab,i)!=id){assert(i<100);i++;}g->selection=i;game_action(g,BTN_OK);assert(g->screen==CUSTOM_QUANTITY);g->revision184.quantity=copies;game_action(g,BTN_OK);assert(g->screen==CUSTOM_ITEMS);}
int main(void){char root[]="/tmp/rift184-custom-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g,h;RiftProfile p;profile_init(&p);
 assert(ascent_begin_mode(&g,1,0,184,1));assert(g.screen==CUSTOM_CARDS&&!g.deck_count&&g.revision184.building&&game_validate(&g));assert(ascent_slot(1)->custom);
 game_action(&g,BTN_START);assert(g.screen==CUSTOM_CARDS&&!g.deck_count);
 ascent_account.cards[2]=1u<<18;
 add(&g,0,0,4);add(&g,1,N_ADRENALINE,2);add(&g,2,R_VENERATE,3);add(&g,2,392,2);assert(g.deck_count==11);
 g.revision184.tab=3;g.selection=1;game_action(&g,BTN_OK);g.revision184.quantity=1;game_action(&g,BTN_OK);assert(g.deck_count==10&&custom_copies(&g,3,N_ADRENALINE)==1);
 game_action(&g,BTN_START);assert(g.screen==CUSTOM_ITEMS);
 item(&g,0,4,2);item(&g,0,63,1);item(&g,1,25,3);assert(g.relic_count==3&&g.potion_count==3&&g.maxhp==80);
 g.revision184.tab=1;g.selection=0;game_action(&g,BTN_OK);game_action(&g,BTN_OK);assert(g.screen==CUSTOM_QUANTITY&&g.potion_count==3);game_action(&g,BTN_BACK);
 g.revision184.tab=3;g.selection=0;game_action(&g,BTN_OK);g.revision184.quantity=2;game_action(&g,BTN_OK);assert(g.potion_count==1);item(&g,1,0,1);
 assert(ascent_checkpoint(&g)&&ascent_resume(&h,1)&&h.potion_count==2&&h.relic_count==3&&h.deck_count==10&&h.revision184.custom);
 game_action(&g,BTN_START);assert(g.screen==CUSTOM_CONFIRM);game_action(&g,BTN_OK);assert(!g.revision184.building&&g.maxhp==108&&g.screen==ACT_INTRO&&game_validate(&g));
 assert(ascent_checkpoint(&g)&&ascent_resume(&h,1)&&h.maxhp==108);
 AscentAccount account=ascent_account;g.act=3;g.screen=COMBAT;g.battle_type=NODE_BOSS;g.enemy_count=1;g.enemies[0].hp=0;check_win(&g);assert(g.screen==ASCENT_ENDING&&g.ascent.won&&!memcmp(&account,&ascent_account,sizeof(account)));for(int i=0;i<3;i++)game_action(&g,BTN_OK);assert(g.screen==VICTORY&&!profile_record(&p,&g));
 g.endless=1;g.act=4;assert(ascent_complete(&g,1));ascent_record_act(&g);assert(!memcmp(&account,&ascent_account,sizeof(account)));
 g.screen=DEFEAT;g.hp=0;assert(!profile_record(&p,&g));assert(ascent_checkpoint(&g));assert(!ascent_resume(&h,1)&&!ascent_slot(1)->present);assert(ascent_open(root)&&!ascent_resume(&h,1));
 char path[300];for(int side=0;side<2;side++){snprintf(path,sizeof(path),"%s/ASCENT18_RUN_2.%d",root,side);assert(access(path,F_OK));}
 /* Recreate an old generation: death receipt still prevents resurrection. */
 g.hp=g.maxhp;g.screen=MAP;assert(ascent_write("ASCENT18_RUN_2",&g,sizeof(g),900));assert(!ascent_resume(&h,1));assert(ascent_begin(&h,1,1,185)&&ascent_resume(&g,1)&&!g.revision184.custom);
 assert(ascent_set_fast(1)&&ascent_open(root)&&ascent_fast());assert(ascent_set_fast(0));
 puts("PASS custom catalogs, mixed/unlocked cards, quantities/removal, potion cap, draft resume, effects applied once, no account/profile records, death deletion/tombstone, normal slot reuse, persisted speed");
 /* Every card can be inspected and resolved by each custom character. */
 for(int c=0;c<3;c++)for(int id=0;id<CARD_COUNT;id++)if(custom_card_tab(id)>=0){
  game_new(&g,184+id);game_choose_character(&g,c);g.revision184.custom=1;g.ascent.cards=ASCENT_ALL_CARDS;g.ascent.relics=1023;g.ascent.potions=0xfffff;
  game_start_battle(&g,NODE_FIGHT);g.hp=g.maxhp=100000;for(int e=0;e<g.enemy_count;e++){g.enemies[e].hp=g.enemies[e].maxhp=100000;g.enemies[e].block=0;}
  g.hand_count=1;g.hand[0]=id;g.selection=0;g.energy=20;g.regent.stars=20;CardPreview cp;game_card_preview(&g,id,0,&cp);game_play_card(&g,0,0);
  if(!game_validate(&g)){fprintf(stderr,"invalid custom card %d character %d screen %d\n",id,c,g.screen);return 1;}
 }
 puts("PASS every supported card resolves and previews safely for all three custom characters");

 for(int character=0;character<3;character++){
  game_new(&g,184);game_choose_character(&g,character);g.revision184.custom=1;g.deck_count=2;g.deck[0]=0;g.deck[1]=R_CLOAK_OF_STARS;g.relic_count=0;game_start_battle(&g,NODE_FIGHT);
  assert(game_shows_stars(&g)==(character==CHAR_REGENT));assert(g.regent.stars==(character==CHAR_REGENT?3:0));
  if(character==CHAR_REGENT)continue;
  const int generators[]={R_VENERATE,R_CONVERGENCE,R_GENESIS,R_THE_SEALED_THRONE,R_KNOCKOUT_BLOW,374,379,391,393};
  const int nongenerators[]={0,N_ADRENALINE,R_CLOAK_OF_STARS,R_ALIGNMENT,R_BLACK_HOLE,R_CHILD_OF_THE_STARS,R_STARDUST,R_RADIATE,378,383,388,392};
  for(unsigned i=0;i<sizeof(generators)/sizeof(*generators);i++){g.deck[1]=generators[i]|UPBIT;assert(game_shows_stars(&g));}
  for(unsigned i=0;i<sizeof(nongenerators)/sizeof(*nongenerators);i++){g.deck[1]=nongenerators[i]|UPBIT;assert(!game_shows_stars(&g));}g.deck[1]=R_CLOAK_OF_STARS;
  int*piles[]={g.hand,g.draw,g.discard,g.exhaust};int*counts[]={&g.hand_count,&g.draw_count,&g.discard_count,&g.exhaust_count};
  for(int p=0;p<4;p++){for(int j=0;j<4;j++)*counts[j]=0;*counts[p]=1;piles[p][0]=R_VENERATE;assert(game_shows_stars(&g));piles[p][0]=R_CLOAK_OF_STARS;assert(!game_shows_stars(&g));}
  for(int j=0;j<4;j++)*counts[j]=0;g.regent.powers[R_GENESIS-REGENT_FIRST]=2;assert(game_shows_stars(&g));g.regent.powers[R_GENESIS-REGENT_FIRST]=0;
  g.deck[1]=R_VENERATE;g.hand_count=1;g.hand[0]=R_VENERATE;g.energy=9;game_play_card(&g,0,0);assert(g.regent.stars==2&&game_shows_stars(&g));
  g.hand_count=1;g.hand[0]=R_CLOAK_OF_STARS;game_play_card(&g,0,0);assert(g.regent.stars==1&&g.block>=7);for(int k=0;k<g.enemy_count;k++)g.enemies[k].intent=1;game_end_turn(&g);assert(g.regent.stars==1&&game_shows_stars(&g));
 }
 puts("PASS conditional Star HUD for Warden/Vesper across deck and combat piles, active generator powers, spending-only cards and upgraded generator/spender cards; Proxy always visible; actual gain/spend/carry-over; no unearned Proxy starter Stars");
 return 0;
}
