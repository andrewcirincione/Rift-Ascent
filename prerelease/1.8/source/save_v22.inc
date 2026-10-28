/* v22/v21 used 8-bit card IDs. Widen IDs without changing upgrade/mod flags,
 * and remap live instance tags one-to-one into 9 bits. Frozen struct prefixes
 * stay byte-identical: the Regent state is appended after the complete v22 Game. */
static int migrate_card22(int raw,int map[1024],int*next){
 if(raw<0)return raw;int instance=(raw>>21)&1023,tag=0;
 if(instance){if(!map[instance]){if(*next>=511)return -1;map[instance]=++*next;}tag=map[instance];}
 return (raw&255)|((raw&0x1fff00)<<1)|(tag<<CARD_INSTANCE_SHIFT);
}
static int migrate_raw22(Game*g){
 int map[1024]={0},next=0;
 if(g->deck_count<0||g->deck_count>MAX_DECK||g->hand_count<0||g->hand_count>MAX_HAND||g->draw_count<0||g->draw_count>MAX_PILE||g->discard_count<0||g->discard_count>MAX_PILE||g->exhaust_count<0||g->exhaust_count>MAX_PILE||g->ninja.task_count<0||g->ninja.task_count>NINJA_TASK_MAX||g->ninja.choice_count<0||g->ninja.choice_count>MAX_HAND||g->ninja.nightmare_count<0||g->ninja.nightmare_count>MAX_HAND)return 0;
#define CV(c) do{(c)=migrate_card22((c),map,&next);if((c)<0)return 0;}while(0)
 for(int i=0;i<g->deck_count;i++)CV(g->deck[i]);
 for(int i=0;i<g->hand_count;i++)CV(g->hand[i]);
 for(int i=0;i<g->draw_count;i++)CV(g->draw[i]);
 for(int i=0;i<g->discard_count;i++)CV(g->discard[i]);
 for(int i=0;i<g->exhaust_count;i++)CV(g->exhaust[i]);
 for(int i=0;i<g->ninja.task_count;i++)CV(g->ninja.tasks[i].raw);
 for(int i=0;i<g->ninja.choice_count;i++)CV(g->ninja.choice_raws[i]);
 for(int i=0;i<g->ninja.nightmare_count;i++)CV(g->ninja.nightmare_raw[i]);
 if(g->ninja.choice_kind)CV(g->ninja.choice_source);
 if(g->ninja.v[NS_LAST_DRAW]>0){int c=g->ninja.v[NS_LAST_DRAW]-1;CV(c);g->ninja.v[NS_LAST_DRAW]=c+1;}
 int screen=g->screen;
 if(screen==REWARD||screen==REWARD_PREVIEW||(screen==DECK_VIEW&&g->prior_screen==REWARD))for(int i=0;i<3;i++)CV(g->reward[i]);
 if(screen==EVENT&&(g->effect_turn==2||g->effect_turn==6))for(int i=0;i<2;i++)CV(g->reward[i]);
 if(screen==EVENT_RESULT&&g->reward[0]>=0)CV(g->reward[0]);
 if(screen==SHOP||((screen==DECK_VIEW||screen==POTION_VIEW)&&g->prior_screen==SHOP))for(int i=0;i<3;i++)if(g->shop_cards[i]>=0)CV(g->shop_cards[i]);
 if(screen==CARD_CHOICE&&g->next_screen==25)CV(g->ic[IC_PENDING_CARD]);
 if(g->ic[46]>0){int c=g->ic[46]-1;CV(c);g->ic[46]=c+1;}
 g->ninja.next_instance=next;
#undef CV
 return 1;
}
