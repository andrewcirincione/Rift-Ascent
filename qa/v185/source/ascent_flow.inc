int ascent_story_screen(int s){return s==ASCENT_ENDING||s==ASCENT_REVELATION||s==ASCENT_BOSS||s==ASCENT_TRUE_END;}
static int ascent_action(Game*g,int b){
 if(b==BTN_PAUSE&&(ascent_story_screen(g->screen)||g->screen==ASCENT_CARDS||g->screen==ASCENT_POTIONS||g->screen==ASCENT_RELIC))return 0;
 if(ascent_debug()&&(b==BTN_DEBUG_KILL||b==BTN_DEBUG_ACT)){
  if(b==BTN_DEBUG_KILL&&g->screen==COMBAT){for(int e=0;e<g->enemy_count;e++)g->enemies[e].hp=0;check_win(g);}
  else if(b==BTN_DEBUG_ACT&&g->ascent.id&&(g->screen==COMBAT||g->screen==MAP||g->screen==REST||g->screen==SHOP||g->screen==EVENT||g->screen==REWARD)){
   g->act++;g->endless=g->act>3;g->floor=0;g->lane=1;g->hp=g->maxhp;g->win_screen=g->relic_pending=0;map_generate(g);begin_act(g);
  }return 1;
 }
 switch(g->screen){
 case ASCENT_SLOTS_VIEW:
  if(b==BTN_UP)g->selection=(g->selection+3)%4;
  else if(b==BTN_DOWN)g->selection=(g->selection+1)%4;
  else if(b==BTN_BACK){g->screen=TITLE;g->selection=g->scroll;}
  else if(b==BTN_OK){int slot=g->selection;const AscentSlot*info=ascent_slot(slot);
   if(!g->scroll){Game saved;if(!info->present||!ascent_resume(&saved,slot))msg(g,"This slot has no valid saved run.");else {*g=saved;msg(g,"");}}
   else{g->master_index=slot;g->screen=info->present?ASCENT_OVERWRITE:CHARACTER_SELECT;g->selection=info->present?0:g->character;}
  }return 1;
 case ASCENT_OVERWRITE:
  if(b==BTN_UP||b==BTN_DOWN)g->selection=1-g->selection;
  else if(b==BTN_BACK||(b==BTN_OK&&!g->selection)){g->screen=ASCENT_SLOTS_VIEW;g->scroll=1+g->revision184.custom;g->selection=g->master_index;}
  else if(b==BTN_OK){g->screen=CHARACTER_SELECT;g->selection=g->character;}
  return 1;
 case ASCENT_OPTIONS:
  if(b==BTN_UP||b==BTN_DOWN)g->selection=1-g->selection;
  if(b==BTN_OK){int ok=g->selection?ascent_set_debug(!ascent_debug()):ascent_set_fast(!ascent_fast());if(!ok)msg(g,"Could not save Options. Try again.");}
  if(b==BTN_BACK){g->screen=TITLE;g->selection=4;}return 1;
 case ASCENT_COLLECTION:
  if(b==BTN_ALT)g->selection=g->selection>=game_collection_unlock_count(g->scroll)?0:game_collection_unlock_count(g->scroll);
  if(b==BTN_TAB_LEFT){g->scroll=(g->scroll+4)%5;g->selection=0;}
  if(b==BTN_TAB_RIGHT){g->scroll=(g->scroll+1)%5;g->selection=0;}
  if(b==BTN_LEFT&&g->selection>0)g->selection--;
  if(b==BTN_RIGHT)g->selection=MIN(game_collection_count(g->scroll)-1,g->selection+1);
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  if(b==BTN_DOWN)g->selection=MIN(game_collection_count(g->scroll)-1,g->selection+3);
  if(b==BTN_OK)g->selection=(g->selection+1)%game_collection_count(g->scroll);
  if(b==BTN_BACK){g->screen=TITLE;g->selection=3;}return 1;
 case ASCENT_SAVE_ERROR:
  if(b==BTN_OK&&ascent_complete(g,g->selection)){g->screen=g->selection?ASCENT_TRUE_END:ASCENT_ENDING;g->selection=0;}
  return 1;
 case ASCENT_ENDING:
  if(b==BTN_OK&&++g->selection>=3){g->screen=g->revision184.custom?(g->ascent.secret?ASCENT_REVELATION:VICTORY):ASCENT_CARDS;g->selection=0;if(g->screen==VICTORY)g->scroll=g->master_index=0;}return 1;
 case ASCENT_CARDS:
  if(b==BTN_OK){g->screen=ASCENT_POTIONS;g->selection=0;if(g->screen==VICTORY)g->scroll=g->master_index=0;}return 1;
 case ASCENT_POTIONS:
  if(b==BTN_OK){g->screen=ASCENT_RELIC;g->selection=0;if(g->screen==VICTORY)g->scroll=g->master_index=0;}return 1;
 case ASCENT_RELIC:
  if(b==BTN_OK){g->screen=g->ascent.secret?ASCENT_REVELATION:VICTORY;g->selection=0;if(g->screen==VICTORY)g->scroll=g->master_index=0;}return 1;
 case ASCENT_REVELATION:
  if(b==BTN_OK&&++g->selection>=3){
   /* The unlocked fourth act is part of this character's ending. There is
    * no endless-mode choice until the Unmaker has actually been defeated. */
   g->endless=1;g->act=4;g->hp=g->maxhp;g->floor=0;g->lane=1;
   g->win_screen=g->relic_pending=0;map_generate(g);begin_act(g);
  }return 1;
 case ASCENT_BOSS:
  if(b==BTN_OK&&++g->selection>=3)game_start_battle(g,NODE_BOSS);return 1;
 case ASCENT_TRUE_END:
  if(b==BTN_OK&&++g->selection>=3){g->screen=VICTORY;g->selection=0;if(g->screen==VICTORY)g->scroll=g->master_index=0;}return 1;
 default:return 0;
 }
}
