#include <assert.h>
#include <stdio.h>
#include "../v186/source/game.c"
static void arena(Game*g,int c){game_new(g,185421);game_choose_character(g,c);g->relic_count=0;g->potion_count=0;g->ascent.cards=ASCENT_ALL_CARDS;g->act=2;g->floor=8;game_start_battle(g,NODE_FIGHT);g->hp=g->maxhp=10000;g->enemy_count=3;for(int i=0;i<3;i++){init_enemy(g,&g->enemies[i],i);g->enemies[i].hp=g->enemies[i].maxhp=100000;g->enemies[i].intent=1;g->enemies[i].intent_value=0;}g->draw_count=g->hand_count=g->discard_count=0;g->energy=50;g->regent.stars=50;g->selection=0;g->revision184.custom=1;}
static void card(Game*g,int raw){g->hand_count=1;g->hand[0]=regent_stamp(g,raw);g->energy=99;g->regent.stars=99;game_play_card(g,0,0);}
static void settle(Game*g){for(int n=0;n<80;n++){if(g->screen==ANCIENT_PICK){int pick=-1;for(int i=0;i<game_ancient_count(g);i++)if(game_ancient_eligible(g,i)){pick=i;break;}if(pick<0)game_action(g,BTN_BACK);else{g->selection=pick;game_action(g,BTN_OK);}}else if(g->screen==NINJA_CHOICE||g->screen==REGENT_CHOICE||g->screen==CARD_CHOICE){game_action(g,BTN_OK);game_action(g,BTN_BACK);}else break;}}
int main(void){Game g,h;char root[]="/tmp/r5-relics-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));
 for(int c=0;c<3;c++){
  arena(&g,c);add_relic(&g,SR_ICE_CREAM);g.energy=17;game_end_turn(&g);assert(g.energy==20);game_start_battle(&g,NODE_FIGHT);assert(g.energy==3);
  arena(&g,c);add_relic(&g,SR_ICE_CREAM);add_relic(&g,SR_PAELS_TEARS);add_relic(&g,SR_VELVET_CHOKER);g.energy=17;game_end_turn(&g);assert(g.energy==22);for(int i=0;i<6;i++)card(&g,c==0?0:c==1?N_STRIKE_SILENT:R_STRIKE_REGENT);int hp=g.enemies[0].hp;card(&g,0);assert(g.enemies[0].hp==hp&&SB(&g,SB_PLAYS)==6);
  arena(&g,c);add_relic(&g,SR_RUNIC_PYRAMID);g.hand_count=2;g.hand[0]=regent_stamp(&g,0);g.hand[1]=regent_stamp(&g,1);game_end_turn(&g);assert(g.hand_count==2);
  arena(&g,c);add_relic(&g,SR_SHURIKEN);add_relic(&g,SR_SHURIKEN);add_relic(&g,SR_KUNAI);add_relic(&g,SR_KUNAI);for(int i=0;i<3;i++)card(&g,c==0?0:c==1?N_STRIKE_SILENT:R_STRIKE_REGENT);assert(g.strength==2&&g.dexterity==2);
  arena(&g,c);add_relic(&g,SR_THROWING_AXE);int raw=c==0?0:c==1?N_STRIKE_SILENT:R_STRIKE_REGENT;CardPreview p;g.hand_count=1;g.hand[0]=regent_stamp(&g,raw);h=g;game_card_preview(&g,g.hand[0],0,&p);assert(!memcmp(&g,&h,sizeof(g)));int before=g.enemies[0].hp;game_play_card(&g,0,0);assert(before-g.enemies[0].hp==12&&p.total[0]==12);
  arena(&g,c);add_relic(&g,SR_CHARONS_ASHES);add_relic(&g,SR_CHARONS_ASHES);add_relic(&g,SR_FORGOTTEN_SOUL);int sum=0;for(int i=0;i<3;i++)sum+=g.enemies[i].hp;exhaust_card(&g,0);int after=0;for(int i=0;i<3;i++)after+=g.enemies[i].hp;assert(sum-after==19);
 }
 for(int c=0;c<3;c++){arena(&g,c);add_relic(&g,SR_THROWING_AXE);g.hand_count=1;g.hand[0]=regent_stamp(&g,R_VOID_FORM);g.energy=99;int turn=g.turn;game_play_card(&g,0,0);for(int i=0;i<8;i++){game_update(&g);settle(&g);}assert(g.turn==turn+1&&g.regent.task_count==0&&g.screen==COMBAT&&g.regent.powers[R_VOID_FORM-REGENT_FIRST]==2);}
 puts("PASS replayed Void Form on all three characters resolves both Powers and ends the turn exactly once; no sentinel deadlock.");
 puts("PASS three-engine stacking, Ice Cream income/reset, Tears, Choker cap, Pyramid, Shuriken/Kunai, exhaust and replay preview parity.");
 arena(&g,2);g.relic_count=0;add_relic(&g,SR_DIVINE_RIGHT);add_relic(&g,SR_TOUCH_OF_OROBAS);game_start_battle(&g,NODE_FIGHT);assert(g.regent.stars==6&&!relic(&g,SR_DIVINE_RIGHT));
 arena(&g,0);add_relic(&g,SR_TUNGSTEN_ROD);add_relic(&g,SR_TUNGSTEN_ROD);int hp=g.hp;lose_hp(&g,5);assert(hp-g.hp==3);add_relic(&g,SR_LIZARD_TAIL);g.hp=1;lose_hp(&g,100);assert(g.hp==g.maxhp/2&&!SC(&g,SR_LIZARD_TAIL));
 arena(&g,1);add_relic(&g,NR_TINGSHA);add_relic(&g,NR_TINGSHA);add_relic(&g,NR_BANDAGES);add_relic(&g,NR_BANDAGES);int sum=0;for(int i=0;i<3;i++)sum+=g.enemies[i].hp;g.block=0;ninja_discard_effects(&g,0,0);int after=0;for(int i=0;i<3;i++)after+=g.enemies[i].hp;assert(sum-after==6&&g.block==6);
 arena(&g,0);add_relic(&g,SR_ALCHEMICAL_COFFER);add_relic(&g,SR_POTION_BELT);assert(game_potion_capacity(&g)==9&&g.potion_count==4);sr_brew(&g,20,0);assert(g.potion_count==9);game_discard_potion(&g,0);assert(g.potion_count==8);assert(game_save(&g,"/tmp/r5-belt.dat")&&game_load(&h,"/tmp/r5-belt.dat")&&h.potion_count==8);
 /* Gold effects must observe each award even if the action also spends gold or RNG. */
 arena(&g,0);add_relic(&g,SR_BOWLER_HAT);add_relic(&g,SR_DRAGON_FRUIT);int oldgold=g.gold,maxhp=g.maxhp;sr_gain_gold(&g,40);assert(g.gold==oldgold+50&&g.maxhp==maxhp+1);add_relic(&g,SR_ECTOPLASM);oldgold=g.gold;maxhp=g.maxhp;sr_gain_gold(&g,40);assert(g.gold==oldgold&&g.maxhp==maxhp);
 arena(&g,0);add_relic(&g,SR_SOZU);g.screen=EVENT;g.effect_turn=19;g.gold=100;assert(!game_event_choice_available(&g,0));
 arena(&g,0);g.revision184.custom=0;int shop=sr_special_relic(&g,4,-1),event=sr_special_relic(&g,6,-1);int sm=0,em=0;for(unsigned i=0;i<sizeof(sr_catalog)/sizeof(*sr_catalog);i++){sm|=sr_catalog[i].id==shop&&sr_catalog[i].rarity==4;em|=sr_catalog[i].id==event&&sr_catalog[i].rarity==6;}assert(sm&&em);
 arena(&g,0);add_relic(&g,SR_NUTRITIOUS_SOUP);int beforedeck[MAX_DECK];memcpy(beforedeck,g.deck,sizeof(beforedeck));add_relic(&g,SR_PANDORAS_BOX);for(int i=0;i<g.deck_count;i++)if(sr_is_strike(beforedeck[i]))assert(g.deck[i]==beforedeck[i]);
 puts("PASS per-award gold triggers, Ectoplasm priority, Sozu event blocking, shop/event-exclusive acquisition, Soup-bound transformation protection.");
 puts("PASS starter upgrade, duplicate discard procs, mitigation/revival, expanded belt and save roundtrip.");
 /* Isolate offered-history weighting from owned-card weighting. */
 arena(&g,0);g.revision184.custom=0;g.deck_count=1;g.deck[0]=0;int fresh=5,seen=12,a=0,b=0;reward_mark(&g,seen);
 for(int i=0;i<180000;i++){int id=reward_of_rarity(&g,0,-1,-1);a+=id==fresh;b+=id==seen;assert(cards[id].rarity==0);}assert(a>b*2&&a<b*4);assert(b>0);
 g.battle_type=NODE_FIGHT;fill_rewards(&g);for(int i=0;i<3;i++)assert(reward_seen(&g,CARD_ID(g.reward[i])));unsigned history[13];memcpy(history,g.revision186.reward_seen,sizeof(history));begin_act(&g);assert(!memcmp(history,g.revision186.reward_seen,sizeof(history)));
 assert(game_save(&g,"/tmp/r5-variety.dat")&&game_load(&h,"/tmp/r5-variety.dat"));assert(!memcmp(history,h.revision186.reward_seen,sizeof(history)));
 /* Legacy standalone and alternating slot prefixes import empty history. */
 g.screen=MAP;g.selection=0;g.lane=0;unsigned check=hash_bytes((unsigned char*)&g,offsetof(Game,revision186));FILE*f=fopen("/tmp/r5-old.dat","wb");assert(f);fwrite(&check,4,1,f);fwrite(&g,offsetof(Game,revision186),1,f);fclose(f);assert(game_load(&h,"/tmp/r5-old.dat"));for(int i=0;i<13;i++)assert(!h.revision186.reward_seen[i]);
 assert(ascent_begin(&g,0,0,77186));g.screen=MAP;g.selection=g.scroll=0;reward_mark(&g,5);assert(ascent_write("ASCENT18_RUN_1",&g,offsetof(Game,revision186),10000));assert(ascent_resume(&h,0)&&h.ascent.id==g.ascent.id);for(int i=0;i<13;i++)assert(!h.revision186.reward_seen[i]);
 puts("PASS 180,000 same-rarity draws: soft unseen weighting, repeat availability, marks, act/save persistence and old-prefix migration.");
 for(int c=0;c<3;c++)for(int seed=1;seed<=600;seed++){game_new(&g,seed);game_choose_character(&g,c);g.act=1+seed%7;g.endless=g.act>3;offer_blessing(&g);assert(game_validate(&g));for(int i=0;i<3;i++){assert(g.reward[i]==SR_ICE_CREAM||game_relic_is_ancient(g.reward[i]));for(int j=0;j<i;j++)assert(g.reward[j]!=g.reward[i]);}game_action(&g,BTN_OK);settle(&g);assert(game_validate(&g));}
 puts("PASS 1,800 act-start boon offers across all characters and acts, legal claims and selection screens.");
 for(unsigned r=0;r<sizeof(sr_catalog)/sizeof(*sr_catalog);r++)for(int copies=1;copies<=2;copies++){
  arena(&g,r%3);g.screen=MAP;g.gold=1000;for(int i=0;i<copies;i++)add_relic(&g,sr_catalog[r].id);sr_open_pending(&g);settle(&g);assert(game_validate(&g));
  game_start_battle(&g,NODE_ELITE);g.hp=g.maxhp=10000;for(int e=0;e<g.enemy_count;e++){g.enemies[e].hp=g.enemies[e].maxhp=100000;g.enemies[e].intent=1;g.enemies[e].intent_value=0;}
  sr_open_pending(&g);settle(&g);
  for(int turn=0;turn<9&&g.screen==COMBAT;turn++){card(&g,0);settle(&g);card(&g,1);settle(&g);card(&g,8);settle(&g);if(g.screen==COMBAT)game_end_turn(&g);sr_open_pending(&g);settle(&g);}
  assert(game_validate(&g));assert(game_save(&g,"/tmp/r5-catalog.dat")&&game_load(&h,"/tmp/r5-catalog.dat"));
 }
 puts("PASS full 296-entry catalog: pickup, both copy counts, combat turns, choices, validation and save/load under sanitizers.");
 for(int act=5;act<=8;act++){arena(&g,0);g.act=act;g.endless=1;game_story_start(&g,1);if(act==6){assert(g.screen==ABOMINATION_BEFORE);for(int i=0;i<3;i++)game_action(&g,BTN_OK);assert(g.enemy_count==3&&g.enemies[0].art==45&&game_music_track(&g)==12);}else{assert(g.screen==COMBAT&&g.enemy_count==1&&g.enemies[0].art!=45&&game_music_track(&g)==0);}}
 puts("PASS Act 6 Abomination with two minions and dedicated track; acts 5, 7 and 8 remain random bosses.");
}
