/* New relic effects are orthogonal to card resolver and character identity. */
static void r151_self_loss(Game*g){
 if(g->screen!=COMBAT||g->hp<=0||!g->ic[IC_OWN_TURN]||!relic(g,53)||g->v151.relic[0])return;
 g->v151.relic[0]=1;gain_energy(g,1);relic_block(g,4);
}
static void r151_exhaust(Game*g){
 if((g->screen!=COMBAT&&g->screen!=CARD_CHOICE)||!relic(g,56)||g->v151.relic[3]>=3)return;
 if(++g->v151.relic[3]==3){if(g->screen==CARD_CHOICE)g->v151.relic[11]=1;else{gain_energy(g,1);draw_card(g);draw_card(g);}}
}
static void r151_played(Game*g,int type){
 if(g->screen==CARD_CHOICE){g->v151.relic[10]=type+1;return;}
 if(g->screen!=COMBAT)return;
 if(type==2)g->v151.relic[4]=MIN(3,g->v151.relic[4]+1);
 if(type==0&&relic(g,55)&&g->ic[IC_ATTACKS_THIS_TURN]%3==0)ninja_poison_all(g,1);
 if(type>=0&&type<=2&&relic(g,62)&&!(g->v151.relic[9]&8)){
  g->v151.relic[9]|=1<<type;
  if(g->v151.relic[9]==7){g->v151.relic[9]|=8;gain_energy(g,2);draw_card(g);draw_card(g);}
 }
}
static void r151_status_snapshot(const Game*g,int out[6]){
 for(int i=0;i<2;i++){out[3*i]=g->enemies[i].weak;out[3*i+1]=g->enemies[i].vulnerable;out[3*i+2]=g->enemies[i].poison;}
}
static void r151_after_effect(Game*g,const int before[6]){
 if(g->screen!=COMBAT||!g->ic[IC_OWN_TURN]||!relic(g,58)||g->v151.relic[5])return;
 int now[6];r151_status_snapshot(g,now);
 for(int i=0;i<g->enemy_count*3;i++)if(now[i]>before[i]){g->v151.relic[5]=1;draw_card(g);break;}
}
static void r151_skip_reward(Game*g){
 if(relic(g,27))heal(g,6);
 if(!relic(g,60))return;g->gold+=8;
 if(++g->v151.relic[7]>=3){
  g->v151.relic[7]=0;int choices[MAX_DECK],n=0;
  for(int i=0;i<g->deck_count;i++)if(game_card_is_upgradeable(g->deck[i]))choices[n++]=i;
  if(n){int i=choices[rnd(g,n)];g->deck[i]|=UPBIT;snprintf(g->message,sizeof(g->message),"Grave Ledger upgraded %s.",cards[CARD_ID(g->deck[i])].name);}
 }
}

static void r151_finish_choice(Game*g){
 if(g->v151.relic[11]){g->v151.relic[11]=0;gain_energy(g,1);draw_card(g);draw_card(g);}
 if(g->v151.relic[10]){int type=g->v151.relic[10]-1;g->v151.relic[10]=0;r151_played(g,type);}
}
