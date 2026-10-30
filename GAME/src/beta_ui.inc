/* Reuse the game's existing gilded panels, card art and inventory typography. */
static void beta_forge_screen(const Game*g){
 header(g,"LATE ASCENT / FORGE INSTEAD OF A CARD");
 gilded_panel(10,30,460,223,0);
 if(g->screen==BETA_FORGE){
  const char*titles[]={"HONE / UPGRADE ONE CARD","TEMPER / MODIFY ONE CARD","BLOOD ETCH / ECHO ONE ATTACK"};
  for(int c=0;c<3;c++){
   int eligible=0;for(int i=0;i<g->deck_count;i++)eligible+=game_beta_eligible(g,c,g->deck[i]);
   int y=42+c*66;gilded_panel(24,y,432,59,g->selection==c);
   r134_text(38,y+9,titles[c],eligible?GOLD:MUTED,&r134_f10);
   char detail[200];
   if(c==0)snprintf(detail,sizeof(detail),"Upgrade any eligible card. No Gold or health cost.");
   else if(c==1)snprintf(detail,sizeof(detail),"%s No Gold or health cost.",game_mod_description(game_beta_modifier(g,c)));
   else snprintf(detail,sizeof(detail),"Lose 4 maximum HP. Echo costs +1 Energy; plays twice.");
   r134_wrap(38,y+26,399,2,11,eligible?detail:"Unavailable: no eligible card or insufficient maximum HP.",eligible?WHITE:MUTED,&r134_f9,0);
  }
  footer("X CHOOSE  O CARD REWARDS");return;
 }
 int page=g->selection/6*6;
 for(int i=page;i<g->deck_count&&i<page+6;i++){
  int x=26+(i-page)%3*102,y=64+(i-page)/3*89;
  card_render(x,y,82,85,g->deck[i],i==g->selection,1);
  if(!game_beta_eligible(g,g->effect_turn,g->deck[i]))dark_rect(x,y,82,85,150);
 }
 r134_text(27,39,g->effect_turn==2?"CONFIRM COST: 4 MAX HP":"CHOOSE A CARD / RESULT PREVIEW",GOLD,&r134_f9);
 r162_side_preview(game_beta_preview(g,g->effect_turn,g->deck[g->selection]),65);
 footer("X CONFIRM  O BACK  SELECT SAVE & QUIT");
}
static void end_screen(const Game*g){
 header(g,g->screen==VICTORY?"RUN RECAP / ASCENT COMPLETE":"RUN RECAP / DEFEAT");
 const char*tabs[]={"SUMMARY","FINAL DECK","RELICS"};int tab=CLAMP(g->scroll,0,2);
 for(int i=0;i<3;i++)button(18+i*151,35,143,24,tabs[i],tab==i);
 if(tab==1){
  int selected=CLAMP(g->master_index,0,CLAMP(g->deck_count-1,0,MAX_DECK-1)),page=selected/6*6;
  for(int i=page;i<g->deck_count&&i<page+6;i++)card_render(26+(i-page)%3*102,66+(i-page)/3*89,82,85,g->deck[i],i==selected,1);
  if(g->deck_count)r162_side_preview(g->deck[selected],65);
  footer("L R TABS  O SUMMARY");return;
 }
 if(tab==2){
  int selected=CLAMP(g->master_index,0,CLAMP(g->relic_count-1,0,MAX_RELICS-1)),page=selected/6*6;
  for(int i=page;i<g->relic_count&&i<page+6;i++){
   int id=game_relic_id(g,i),y=65+(i-page)*29;gilded_panel(22,y,258,28,i==selected);r150b_relic_icon(29,y+3,22,id);r134_text(59,y+8,relic_defs[id].name,i==selected?GOLD:WHITE,&r134_f9);
  }
  if(g->relic_count){int id=game_relic_id(g,selected);r150b_relic_icon(344,75,60,id);r134_wrap(302,147,151,2,12,relic_defs[id].name,GOLD,&r134_f10,0);r134_wrap(302,178,151,5,12,relic_defs[id].desc,WHITE,&r134_f9,0);}
  else r134_center(240,130,"NO RELICS COLLECTED",MUTED,&r134_f10);
  footer("L R TABS  O SUMMARY");return;
 }
 char b[160];int won=g->screen==VICTORY;
 snprintf(b,sizeof(b),"%s  /  %s",game_character_name(g),g->revision184.custom?"CUSTOM RUN":"STANDARD RUN");r134_center(240,70,b,GOLD,&r134_f10);
 snprintf(b,sizeof(b),"Acts cleared: %d   Reached Act %d, room %d",CLAMP(g->act-(won?0:1),0,100000),g->act,CLAMP(g->floor+1,1,MAP_ROWS+1));r134_center(240,91,b,WHITE,&r134_f9);
 snprintf(b,sizeof(b),"%d foes defeated   %d cards   %d relics   %d Gold",g->kill_count,g->deck_count,g->relic_count,g->gold);r134_center(240,109,b,WHITE,&r134_f9);
 int types[3]={0};for(int i=0;i<g->deck_count;i++){int t=cards[CARD_ID(g->deck[i])].type;if(t>=0&&t<3)types[t]++;}
 snprintf(b,sizeof(b),"Deck: %d Attacks / %d Skills / %d Powers",types[0],types[1],types[2]);r134_center(240,127,b,MUTED,&r134_f9);
 r134_wrap(34,149,414,2,12,won?"Your final deck and relics are preserved here to inspect before continuing.":g->subtitle[0]?g->subtitle:"Fell during the ascent.",won?GOLD:RED,&r134_f10,0);
 if(won){button(28,206,205,34,"CONTINUE ENDLESS",g->selection==0);button(248,206,205,34,"RETURN TO MENU",g->selection==1);}
 else button(132,206,216,34,"RETURN TO MENU",1);
 footer("L R TABS  SQ DECK  TRI RELICS  X CONFIRM");
}
