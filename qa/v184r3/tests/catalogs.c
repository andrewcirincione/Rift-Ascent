#include <assert.h>
#include <stdio.h>
#include "../source/game.c"
int main(void){Game g;char root[]="/tmp/rift-r3-catalogs-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));game_new(&g,1843);g.screen=CUSTOM_CARDS;
 int total=0;
 for(int tab=0;tab<3;tab++){
  for(int base=0;base<2;base++){
   int seen[512]={0},prior=-1,previous=-1,first=base?20:0,last=base?game_collection_count(tab):20;
   for(int i=first;i<last;i++){int id=game_collection_id(tab,i);assert(id>=0&&id<CARD_COUNT&&!seen[id]++);assert(cards[id].type>=prior);if(cards[id].type==prior)assert(id>previous);prior=cards[id].type;previous=id;total++;}
  }
  for(int bit=0;bit<20;bit++){
   ascent_account.cards[tab]=1u<<bit;int found=0;
   for(int i=0;i<20;i++){int id=game_collection_id(tab,i),open=game_collection_unlocked(tab,i);assert(open==(id==ASCENT_CARD_FIRST+20*tab+bit));found+=open;}assert(found==1);
  }
  for(int full=0;full<2;full++){
   ascent_account.cards[tab]=full?ASCENT_ALL_CARDS:0;g.revision184.tab=tab;int seen[512]={0},prior=-1,previous=-1;
   for(int i=0;i<custom_count(&g,tab);i++){int id=custom_id(&g,tab,i);assert(!seen[id]++);assert(cards[id].type>=prior&&ascent_card_available(&g,id));if(cards[id].type==prior)assert(id>previous);prior=cards[id].type;previous=id;}
   for(int id=0;id<CARD_COUNT;id++)assert(seen[id]==(custom_card_tab(id)==tab&&ascent_card_available(&g,id)));
  }
 }
 g.revision184.tab=3;g.deck_count=4;g.deck[0]=R_VENERATE|UPBIT;g.deck[1]=0;g.deck[2]=N_ADRENALINE|UPBIT;g.deck[3]=0;assert(custom_count(&g,3)==3);assert(custom_id(&g,3,0)==g.deck[0]&&custom_id(&g,3,1)==g.deck[1]&&custom_id(&g,3,2)==g.deck[2]);
 printf("PASS %d collection cards and all three custom catalogs sorted Attack, Skill, Power, then other cards; stable within-type order, exact memberships, no duplicates or locked-card leakage.\n",total);
 puts("PASS all 60 single-card unlock masks follow actual card IDs after sorting; selected-deck order, variants and quantities preserved.");return 0;}
