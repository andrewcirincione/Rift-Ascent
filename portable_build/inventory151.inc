/* Overlay owns its cursor; opening/closing it never changes the underlying menu. */
int game_inventory_action(Game*g,int key,int*open,int*tab,int*scroll,int*potion){
 if(!game_inventory_available(g)){*open=0;return 0;}
 if(key==BTN_START){*open=!*open;if(*open){*tab=g->screen==COMBAT?2:3;*scroll=0;*potion=0;}return 1;}
 if(!*open){
  if(key==BTN_ALT&&g->screen!=COMBAT){*open=1;*tab=5;*scroll=0;return 1;}
  if(key==BTN_POTION&&(g->screen==COMBAT||g->screen==MAP||g->screen==SHOP)){*open=1;*tab=3;*potion=0;return 1;}
  return 0;
 }
 if(key==BTN_BACK){*open=0;return 1;}
 if(key==BTN_ALT){*tab=5;*scroll=0;return 1;}
 if(key==BTN_LEFT||key==BTN_RIGHT){*tab=(*tab+(key==BTN_LEFT?5:1))%6;*scroll=0;return 1;}
 if(key==BTN_UP||key==BTN_DOWN){
  int d=key==BTN_UP?-1:1;
  if(*tab==3)*potion=MAX(0,MIN(g->potion_count-1,*potion+d));
  else if(*tab==1&&g->screen==COMBAT&&g->enemy_count)g->target=(g->target+g->enemy_count+d)%g->enemy_count;
  else if(*tab==2)*scroll=MAX(0,MIN(MAX(0,game_status_count(g)-7),*scroll+d));
  else if(*tab==4)*scroll=MAX(0,MIN(g->relic_count-1,*scroll+d));
  else if(*tab==5)*scroll=MAX(0,MIN(g->deck_count-1,*scroll+d));
  return 1;
 }
 if(*tab==3&&g->potion_count>0&&(key==BTN_OK||key==BTN_POTION)){
  int count=g->potion_count,in_combat=g->screen==COMBAT;
  if(key==BTN_POTION)game_discard_potion(g,*potion);else game_use_potion(g,*potion);
  *potion=MAX(0,MIN(g->potion_count-1,*potion));
  if(key==BTN_OK&&g->potion_count<count){if(in_combat||!game_inventory_available(g))*open=0;return 2;}
  return key==BTN_POTION?3:1;
 }
 return 1;
}
