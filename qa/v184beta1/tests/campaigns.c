/* Seeded heuristic player. Uses legal game actions, ordinary starter decks,
   choices, rewards, shops, campfires and potions. No debug kills/stat boosts.
   This is a comparative balance probe, not a prediction of human win rate. */
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#ifdef BALANCE_SOURCE
#include BALANCE_SOURCE
#elif defined(BASELINE)
#include "../../v184r4/source/game.c"
#else
#include "../source/game.c"
#endif
static double rating(const Game*g,int raw){int id=CARD_ID(raw);if(id>=CARD_COUNT||cards[id].type==3||!game_card_is_playable(raw))return -30;
 if(id==0||id==1||id==N_STRIKE_SILENT||id==N_DEFEND_SILENT||id==R_STRIKE_REGENT||id==R_DEFEND_REGENT)return 1+2*CARD_UP(raw);
 if(id==N_GRAND_FINALE||id==R_ROYALTIES||id==R_DEBRIS||id==N_FLANKING||id==N_SNEAKY)return -20;
 int attack=0,skill=0,powers=0;for(int i=0;i<g->deck_count;i++){int t=cards[CARD_ID(g->deck[i])].type;attack+=t==0;skill+=t==1;powers+=t==2;}
 double v=8+2*MIN(cards[id].rarity,3)+2*CARD_UP(raw);int cost=game_card_cost(raw);if(cost==0)v+=2;if(cost>=3)v-=2;
 if(cards[id].type==0&&attack*100<g->deck_count*40)v+=3;if(cards[id].type==1&&skill*100<g->deck_count*30)v+=3;
 if(cards[id].type==2)v+=powers<3?2:-3;
 return v;}
static double relic_rating(int id){if(id==11||id==25)return 28;if(id==1||id==13||id==37)return 22;if(id==38||id==4||id==31)return 18;return 13;}
static int incoming(const Game*g){int n=0;for(int i=0;i<g->enemy_count;i++)n+=game_enemy_intent_damage(g,i);return n;}
static void choices(Game*g){for(int steps=0;steps<128;steps++){
 int best=-1;double value=-1e9;
 if(g->screen==NINJA_CHOICE){for(int i=0;i<g->hand_count;i++)if(game_ninja_choice_eligible(g,i)){double v=rating(g,g->hand[i]);if(g->ninja.choice_kind==NC_DISCARD)v=-v;if(v>value){value=v;best=i;}}}
 else if(g->screen==REGENT_CHOICE){int remove=g->regent.choice_kind==RC_EXHAUST||g->regent.choice_kind==RC_TRANSFORM_HAND||g->regent.choice_kind==RC_TRANSFORM_DRAW||g->regent.choice_kind==RC_GUARDS||g->regent.choice_kind==RC_ENTROPY;
  for(int i=0;i<game_regent_choice_count(g);i++)if(game_regent_choice_eligible(g,i)){double v=rating(g,game_regent_choice_raw(g,i));if(remove)v=-v;if(v>value){value=v;best=i;}}}
 else if(g->screen==CARD_CHOICE){int n=g->next_screen==25?g->discard_count:g->hand_count;for(int i=0;i<n;i++){int raw=g->next_screen==25?g->discard[i]:g->hand[i];if(g->next_screen==23&&!game_card_is_upgradeable(raw))continue;double v=rating(g,raw);if(g->next_screen!=25&&g->next_screen!=23)v=-v;if(v>value){value=v;best=i;}}}
 else break;
 if(best<0){game_action(g,BTN_BACK);break;}g->selection=best;game_action(g,BTN_OK);
 }}
static double combat_value(const Game*a,const Game*b,int raw,int potion){
 if(b->hp<=0||b->screen==DEFEAT)return -1e6;
 double v=2.0*(b->hp-a->hp),defense=2.0+0.5*(a->hp*2<a->maxhp);
 int ia=incoming(a),ib=incoming(b);v+=defense*(MAX(0,ia-a->block)-MAX(0,ib-b->block));
 for(int e=0;e<a->enemy_count;e++){v+=MAX(0,a->enemies[e].hp-MAX(0,b->enemies[e].hp));if(a->enemies[e].hp>0&&b->enemies[e].hp<=0)v+=12;
  if(b->enemies[e].hp>0){v+=1.7*(b->enemies[e].poison-a->enemies[e].poison)+1.5*(b->enemies[e].vulnerable-a->enemies[e].vulnerable);}}
 v+=6*(b->strength-a->strength)+6*(b->dexterity-a->dexterity)+2*(b->regent.stars-a->regent.stars)+.5*(b->regent.forge-a->regent.forge);
 v+=3.8*(b->energy-a->energy)+1.6*(b->hand_count-a->hand_count+(potion?0:1));
 if(!potion&&cards[CARD_ID(raw)].type==2)v+=a->turn<6?10:5;
 if(potion)v-=5;
 if(b->screen!=COMBAT&&b->screen!=NINJA_CHOICE&&b->screen!=REGENT_CHOICE&&b->screen!=CARD_CHOICE)v+=1000;
 return v;
}
static double follow_value(const Game*origin,const Game*g){
 if(g->screen!=COMBAT)return 0;double best=0;Game h;
 for(int i=0;i<g->hand_count;i++){
  int raw=g->hand[i],known=0;for(int j=0;j<origin->hand_count;j++)known|=CARD_ID(origin->hand[j])==CARD_ID(raw);
  if(!known||!game_card_is_playable(raw)||game_card_cost_now(g,raw)>g->energy||game_card_star_cost(g,raw)>g->regent.stars)continue;
  for(int e=0;e<g->enemy_count;e++)if(g->enemies[e].hp>0){h=*g;game_play_card(&h,i,e);choices(&h);if(h.turn_played==g->turn_played)continue;double v=combat_value(g,&h,raw,0);if(v>best)best=v;}
 }return best;
}
static int combat_step(Game*g){double best=.05;int pick=-1,target=0,potion=-1;Game h;
 for(int i=0;i<g->hand_count;i++){int raw=g->hand[i];if(!game_card_is_playable(raw)||game_card_cost_now(g,raw)>g->energy||game_card_star_cost(g,raw)>g->regent.stars)continue;
  for(int e=0;e<g->enemy_count;e++)if(g->enemies[e].hp>0){h=*g;game_play_card(&h,i,e);choices(&h);if(h.turn_played==g->turn_played&&h.screen==g->screen)continue;double v=combat_value(g,&h,raw,0)+.9*follow_value(g,&h);if(v>best){best=v;pick=i;target=e;potion=-1;}}}
 for(int i=0;i<g->potion_count;i++){h=*g;game_use_potion(&h,i);choices(&h);double v=combat_value(g,&h,0,1);if(v>best){best=v;potion=i;pick=-1;}}
 if(potion>=0)game_use_potion(g,potion);else if(pick>=0)game_play_card(g,pick,target);else game_end_turn(g);choices(g);return pick>=0||potion>=0;
}
static double inventory(const Game*g){double v=2*g->hp+g->maxhp+.10*g->gold+15*g->relic_count+5*g->potion_count+10*g->permanent_strength+20*g->max_energy;
 for(int i=0;i<g->deck_count;i++)v+=rating(g,g->deck[i])*.55;
 const int*n=g->revision182.next_combat;return v+3*n[0]+.5*n[1]+2*n[2]-3*n[3]-3*n[4];}
static void event_card(Game*g){int best=-1;double value=-1e9;for(int i=0;i<g->deck_count;i++)if(game_event_card_eligible(g,g->deck[i])){double v=rating(g,g->deck[i]);if(g->next_screen==EVENT_REMOVE||g->next_screen==EVENT_TRANSFORM)v=-v;if(v>value){value=v;best=i;}}if(best>=0){g->selection=best;game_action(g,BTN_OK);}else game_action(g,BTN_BACK);}
static double node_value(const Game*g,int row,int lane,int depth){if(row>=MAP_ROWS)return 0;int type=g->map[row][lane];double value=type==NODE_REST?(g->hp*100<g->maxhp*70?10:4):type==NODE_SHOP?(g->gold>=120?5:-2):type==NODE_TREASURE?9:type==NODE_EVENT?4:type==NODE_ELITE?(g->hp*100>=g->maxhp*80&&g->deck_count>=15?2:-7):0;
 if(depth>0&&row+1<MAP_ROWS){unsigned edges=route_layer(g,row,lane);double next=-1e9;for(int i=0;i<3;i++)if(edges&(1u<<i)){double v=node_value(g,row+1,i,depth-1);if(v>next)next=v;}if(next> -1e8)value+=.75*next;}return value;}
static int act(Game*g){Game h;int best=0;double value=-1e9;
 switch(g->screen){
 case COMBAT:combat_step(g);return 1;
 case NINJA_CHOICE:case REGENT_CHOICE:case CARD_CHOICE:choices(g);return 1;
 case MAP:
  for(int i=0;i<g->potion_count;i++)if(g->potions[i]==0&&g->maxhp-g->hp>=18){game_use_potion(g,i);break;}
  if(g->floor<MAP_ROWS)for(int i=0;i<3;i++)if(game_route_available(g,i)){double v=node_value(g,g->floor,i,5);if(v>value){value=v;best=i;}}
  g->scroll=0;g->selection=best;game_action(g,BTN_OK);return 1;
 case REWARD:
  if(!g->v151.card_done){for(int i=0;i<3;i++){double v=rating(g,g->reward[i]);if(v>value){value=v;best=i;}}g->selection=value>=(g->deck_count>25?14:g->deck_count>19?11:6)?best:5;}
  else g->selection=g->v151.potion_offer>=0&&g->potion_count<MAX_POTIONS?4:5;game_action(g,BTN_OK);return 1;
 case BLESSING:case BOSS_REWARD:
  for(int i=0;i<3;i++){int id=g->reward[i];double v=id>=1000?rating(g,id-1000):relic_rating(id);if(v>value){value=v;best=i;}}g->selection=best;game_action(g,BTN_OK);return 1;
 case RELIC_RESULT:case EXPEDITION_GIFT:g->selection=0;game_action(g,BTN_OK);return 1;
 case REST:g->selection=g->hp*100<g->maxhp*72||!count_upgradeable(g)?0:1;game_action(g,BTN_OK);return 1;
 case DECK_VIEW:
  for(int i=0;i<g->deck_count;i++)if((g->prior_screen==SHOP&&!g->deck_readonly)||game_card_is_upgradeable(g->deck[i])){double v=rating(g,g->deck[i]);if(g->prior_screen==SHOP)v=-v;if(v>value){value=v;best=i;}}
  if(value< -1e8)game_action(g,BTN_BACK);else{g->selection=best;game_action(g,BTN_OK);}return 1;
 case SHOP:
  for(int i=0;i<6;i++){h=*g;h.selection=i;game_action(&h,BTN_OK);if(h.screen==DECK_VIEW){int low=0;for(int j=1;j<h.deck_count;j++)if(rating(&h,h.deck[j])<rating(&h,h.deck[low]))low=j;h.selection=low;game_action(&h,BTN_OK);}
   double v=inventory(&h)-inventory(g);if(h.deck_count<g->deck_count)v+=5;for(int j=0;j<h.deck_count;j++)if(j>=g->deck_count&&rating(g,h.deck[j])<8)v-=10;if(v>value&&h.gold<g->gold){value=v;best=i;}}
  if(value>.5){g->selection=best;game_action(g,BTN_OK);}else game_action(g,BTN_BACK);return 1;
 case EVENT:
  for(int i=0;i<game_event_choice_count(g);i++)if(game_event_choice_available(g,i)){h=*g;h.selection=i;game_action(&h,BTN_OK);if(h.screen==EVENT_CARD)event_card(&h);double v=inventory(&h)-inventory(g);if(h.screen==EVENT_CARD)v=-1e8;if(v>value){value=v;best=i;}}
  g->selection=best;game_action(g,BTN_OK);return 1;
 case EVENT_CARD:event_card(g);return 1;
 case MYSTERY_CARDS:for(int i=0;i<3;i++){double v=rating(g,g->reward[i]);if(v>value){value=v;best=i;}}g->selection=best;game_action(g,BTN_OK);return 1;
 case TITLE:case CHARACTER_SELECT:case CUSTOM_CARDS:case ASCENT_SAVE_ERROR:return 0;
 default:game_action(g,BTN_OK);return 1;
 }}
int main(int argc,char**argv){int runs=argc>1?atoi(argv[1]):100,offset=argc>2?atoi(argv[2]):0;char root[]="/tmp/rift-campaigns-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));int total=0,wins=0,stalled=0;printf("character,seed,result,act,floor,hp,actions,combats,turns\n");
 for(int c=0;c<3;c++)for(int seed=offset+1;seed<=offset+runs;seed++){Game g;ascent_account.cards[c]=argc>3&&atoi(argv[3])?ASCENT_ALL_CARDS:0;ascent_account.potions=argc>3&&atoi(argv[3])?0xfffff:0;ascent_account.relics=argc>3&&atoi(argv[3])?1023:0;assert(ascent_begin(&g,0,c,seed*7919u+18440u));int actions=0,battles=0,turns=0,lastturn=0,lastscreen=-1,result=0,spam=0;
  for(;actions<15000;actions++){
   if(g.screen==ASCENT_ENDING||g.screen==ASCENT_CARDS||g.screen==VICTORY||g.act>3){result=1;break;}
   if(g.screen==DEFEAT||g.hp<=0)break;
   if(g.screen==COMBAT){if(g.turn==lastturn)spam++;else spam=0;if(spam>40){game_end_turn(&g);choices(&g);spam=0;continue;}if(lastscreen!=COMBAT&&g.turn==1) battles++;if(g.turn!=lastturn){turns++;lastturn=g.turn;}}
   lastscreen=g.screen;if(!act(&g)){result=-1;break;}
  }
  if(actions>=15000)result=-1;total++;wins+=result==1;stalled+=result<0;printf("%d,%d,%d,%d,%d,%d,%d,%d,%d\n",c,seed,result,g.act,g.floor,g.hp,actions,battles,turns);fflush(stdout);
 }
 fprintf(stderr,"Campaigns %d; wins %d (%.1f%%); policy stalls %d\n",total,wins,100.0*wins/total,stalled);return stalled?2:0;}
