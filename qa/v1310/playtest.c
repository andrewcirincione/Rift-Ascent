/* Deterministic full-run heuristic player. Uses real game rules; no HP, RNG,
 * reward or deck cheats. Simulation is a calibration tool, not human telemetry. */
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
static int mode;
static float quality(const Game*g,int raw){
 static const float q[92]={2,2,4,7,9,9,5,8,10,8,8,9,8,5,7,6,7,6,5,5,6,5,7,7,9,8,7,8,8,8,4,6,8,7,6,6,9,3,9,9,8,5,7,6,7,5,7,10,9,6,0,0,0,0,10,6,12,7,7,7,9,7,6,0,8,5,9,3,6,8,8,7,8,6,6,9,6,2,10,9,7,7,8,3,8,0,5,8,7,7,6,3};
 int id=CARD_ID(raw);float v=q[id]+(CARD_UP(raw)?1.5f:0);
 int powers=0,str=0,exh=0;for(int i=0;i<g->deck_count;i++){int c=CARD_ID(g->deck[i]);powers+=cards[c].type==2;str+=c==8||c==48||c==66;exh+=c==17||c==18||c==42||c==43||c==45||c==59;}
 if(id==22||id==27||id==36||id==10||id==87)v+=MIN(3,str);
 if(id==20||id==62||id==13||id==37)v+=MIN(3,exh);
 if(cards[id].type==2){if(powers>3)v-=2;for(int i=0;i<g->deck_count;i++)if(CARD_ID(g->deck[i])==id)v-=2;}
 return v;
}
static void choice(Game*g){
 if(g->screen!=CARD_CHOICE)return;
 int best=0,count=g->next_screen==25?g->discard_count:g->hand_count;float score=-1e9;
 for(int i=0;i<count;i++){int raw=g->next_screen==25?g->discard[i]:g->hand[i];float v=quality(g,raw);
  if(g->next_screen!=25&&g->next_screen!=23)v=-v;
  if(g->next_screen==23&&CARD_UP(raw))v=-100;
  if(v>score){score=v;best=i;}}
 g->selection=best;game_action(g,BTN_OK);
}
static int threat(const Game*g){int n=0;for(int e=0;e<g->enemy_count;e++)if(g->enemies[e].hp>0)n+=game_enemy_intent_damage(g,e);return n;}
static int endhp(const Game*g){Game c=*g;for(int i=0;i<3&&c.screen==CARD_CHOICE;i++)choice(&c);if(c.screen==COMBAT)game_end_turn(&c);return c.hp;}
static float powers(const Game*g){
 return g->strength*2.4f+g->dexterity*1.8f+g->heal_after_battle*5.0f+g->shop_cards[2]*1.7f+(g->used_potion&65535)*1.0f
 +g->ic[IC_CRIMSON]*1.5f+g->ic[IC_PYRE]*9+g->ic[IC_AGGRESSION]*6+g->ic[IC_HELLRAISER]*9+g->ic[IC_JUGGERNAUT]*2
 +g->ic[IC_STAMPEDE]*6+g->ic[IC_PLATING]*2+g->ic[IC_DARK_EMBRACE]*4+g->ic[IC_BARRICADE]*5+g->ic[IC_CRUELTY]*.14f
 +g->ic[IC_CORRUPTION]*6+g->ic[IC_JUGGLING]*4+g->ic[IC_VICIOUS]*2+g->ic[IC_NEXT_ATTACK_REPEAT]*4+(g->reward_potion>>8)*2;
}
static int play_turn(Game*g){
 for(int p=g->potion_count-1;p>=0;p--){int id=g->potions[p];
  if((id==0&&g->maxhp-g->hp>=18)||(id==1&&(g->battle_type!=NODE_FIGHT||g->turn==1))||(id==2&&threat(g)-g->block>=12)||(id==3&&(g->enemy_count==2||g->hp<28))||(id==4&&g->energy==0&&g->hand_count>=2)||(id==5&&g->energy>=2&&g->hand_count<=2))game_use_potion(g,p);
 }
 if(g->screen!=COMBAT)return 0;
 int best=-1,target=0,before=endhp(g);float bestv=.05f;
 for(int i=0;i<g->hand_count;i++){
  int raw=g->hand[i],cost=game_card_cost_now(g,raw);if(!game_card_is_playable(raw)||cost< -1||cost>g->energy)continue;
  for(int e=0;e<g->enemy_count;e++)if(g->enemies[e].hp>0){
   Game c=*g;game_play_card(&c,i,e);for(int j=0;j<3&&c.screen==CARD_CHOICE;j++)choice(&c);
   if(c.screen==DEFEAT)continue;
   Game normalized=c;memcpy(normalized.message,g->message,sizeof(g->message));memcpy(normalized.subtitle,g->subtitle,sizeof(g->subtitle));normalized.selection=g->selection;normalized.target=g->target;if(!memcmp(&normalized,g,sizeof(*g)))continue;
   float v=0;int dealt=0,kills=0;for(int k=0;k<g->enemy_count;k++){dealt+=MAX(0,g->enemies[k].hp-MAX(0,c.enemies[k].hp));kills+=g->enemies[k].hp>0&&c.enemies[k].hp<=0;
    if(c.enemies[k].hp>0)v+=(c.enemies[k].vulnerable-g->enemies[k].vulnerable)*1.8f+(c.enemies[k].weak-g->enemies[k].weak)*1.7f;}
   v+=dealt*(mode==2?1.1f:.85f)+kills*8+(endhp(&c)-before)*(mode==1?2.4f:1.7f);
   v+=(powers(&c)-powers(g))*(g->battle_type==NODE_BOSS?1.6f:1.f)+(c.energy-g->energy)*2.5f+(c.hand_count-g->hand_count+1)*1.4f;
   v+=(c.maxhp-g->maxhp)*2.0f;
   if(c.screen==REWARD||c.screen==RELIC_RESULT)v+=100;
   if(v>bestv){bestv=v;best=i;target=e;}
  }
 }
 if(best<0){game_end_turn(g);return 1;}game_play_card(g,best,target);return 0;
}
static float room(const Game*g,int n){switch(n){case NODE_REST:return 4+12.f*(g->maxhp-g->hp)/g->maxhp;case NODE_SHOP:return g->gold>=130?8:1;case NODE_EVENT:return 6;case NODE_TREASURE:return 12;case NODE_ELITE:return mode==2?5:g->hp>g->maxhp*.85?3:-4;default:return 3;}}
static int route(const Game*g){int best=g->lane;float value=-1e9;
 for(int c=0;c<3;c++)if(game_route_available(g,c)){
  float v=room(g,g->map[g->floor][c]);unsigned m=game_route_mask(g,g->floor,c);
  if(g->floor+1<MAP_ROWS){float next=-100;for(int j=0;j<3;j++)if(m&(1u<<j))next=MAX(next,room(g,g->map[g->floor+1][j]));v+=next*.5f;}
  v+=((g->seed>>(c*4))&15)*.01f;
  if(v>value){value=v;best=c;}}
 return best;
}
static void outside(Game*g){
 switch(g->screen){
 case BLESSING:g->selection=0;game_action(g,BTN_OK);break;
 case TITLE:game_action(g,BTN_OK);break;
 case MAP:if(g->floor==MAP_ROWS)game_action(g,BTN_OK);else game_choose_node(g,route(g));break;
 case CARD_CHOICE:choice(g);break;
 case REWARD:{int best=3;float v=g->deck_count<18?4.f:g->deck_count<25?7.f:9.f;for(int i=0;i<3;i++){float q=quality(g,g->reward[i]);if(q>v){v=q;best=i;}}g->selection=best;game_action(g,BTN_OK);break;}
 case RELIC_RESULT:g->selection=0;game_action(g,BTN_OK);break;
 case REST:{int eligible=0;for(int j=0;j<g->deck_count;j++)eligible+=game_card_is_upgradeable(g->deck[j]);g->selection=!eligible||g->hp<g->maxhp*(mode==1?.82f:.68f)?0:1;game_action(g,BTN_OK);break;}
 case DECK_VIEW:{int best=0;float score=-1e9;for(int i=0;i<g->deck_count;i++){float q=quality(g,g->deck[i]);if(g->prior_screen==SHOP)q=-q;else if(!game_card_is_upgradeable(g->deck[i]))q=-100;if(q>score){score=q;best=i;}}g->selection=best;game_action(g,BTN_OK);break;}
 case SHOP:{int item=6;float q=8;for(int i=0;i<3;i++)if(g->shop_cards[i]>=0){float v=quality(g,g->shop_cards[i]);if(v>q&&g->gold>=50+25*cards[g->shop_cards[i]].rarity){item=i;q=v;}}
  if(item==6&&g->shop_relic>=0&&g->gold>=155)item=3;
  if(item==6&&g->gold>=60){for(int i=0;i<g->deck_count;i++)if(quality(g,g->deck[i])<3&&g->deck_count>12){item=4;break;}}
  g->selection=item;game_action(g,BTN_OK);break;}
 case EVENT:{int id=g->effect_turn;g->selection=0;if((id==0&&g->hp<22)||(id==4&&g->hp<20)||(id==6)||(id==7&&g->hp<38)||(id==8&&g->hp>g->maxhp-23)||(id==9&&g->hp<40)||(id==11&&g->deck_count<15))g->selection=1;game_action(g,BTN_OK);break;}
 case EVENT_RESULT:case BOSS_REWARD:game_action(g,BTN_OK);break;
 default:game_action(g,BTN_BACK);break;
 }
}
int main(int argc,char**argv){int runs=argc>1?atoi(argv[1]):300,start=argc>2?atoi(argv[2]):1;mode=argc>3?atoi(argv[3]):0;
 int wins=0,stalls=0,deaths[3]={0},reached[3]={0},fights[3]={0},pairs[3]={0},turns=0;long floors=0;
 for(int i=0;i<runs;i++){Game g;game_new(&g,(unsigned)(start+i)*2654435761u);int steps=0,lastscreen=-1,lastact=-1,lastfloor=-1;reached[0]++;
  while(g.screen!=VICTORY&&g.screen!=DEFEAT&&steps++<6000){
   if(g.act>lastact&&lastact>0)reached[g.act-1]++;
   if(g.screen==COMBAT&&(lastscreen!=COMBAT||g.floor!=lastfloor||g.act!=lastact)){fights[g.act-1]++;pairs[g.act-1]+=g.enemy_count==2;}
   lastscreen=g.screen;lastact=g.act;lastfloor=g.floor;
   if(g.screen==COMBAT){if(g.turn>80){steps=6001;break;}turns+=play_turn(&g);}else outside(&g);
  }
  if(g.screen==VICTORY)wins++;else if(g.screen==DEFEAT)deaths[g.act-1]++;else{stalls++;fprintf(stderr,"STALL seed=%u screen=%d act=%d floor=%d turn=%d hand=%d hp=%d\n",g.seed,g.screen,g.act,g.floor,g.turn,g.hand_count,g.hp);}
  floors+=(g.act-1)*15+g.floor;
 }
 printf("runs=%d seed_start=%d policy=%d wins=%d rate=%.3f stalled=%d mean_floor=%.2f turns=%d\n",runs,start,mode,wins,(double)wins/runs,stalls,(double)floors/runs,turns);
 for(int a=0;a<3;a++)printf("act=%d reached=%d deaths=%d fights=%d pairs=%d pair_rate=%.3f\n",a+1,reached[a],deaths[a],fights[a],pairs[a],fights[a]?(double)pairs[a]/fights[a]:0);
 return stalls?1:0;
}
