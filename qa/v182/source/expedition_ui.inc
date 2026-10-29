static void expedition_scene_view(const Game*g){
 const uint16_t*art=g->screen==ABOMINATION_BEFORE?expedition_god_before:g->screen==ABOMINATION_AFTER?expedition_god_after:g->screen==MYSTERY_STORY?expedition_mystery_scene:expedition_story_art[CLAMP(g->character,0,2)][expedition_art(g)];
 rect(0,0,W,H,RGB(0,0,0));art_blit(72,38,336,126,art,336,126);
 r134_text(72,175,expedition_title(g),GOLD,&r138_title10);
 r134_wrap(72,194,336,4,12,expedition_line(g),WHITE,&r134_f9,0);
 r134_blit(343,247,14,14,r135_cross,14,14,255);r134_text(364,250,"CONTINUE",MUTED,&r134_f8);
}
static void expedition_gift_view(const Game*g){
 ascent_heading("A GIFT FROM THE JOURNEY");
 if(g->expedition.gift_kind){
  int id=g->expedition.gift;r150b_relic_icon(210,51,60,id);
  r134_wrap(60,122,360,2,14,relic_defs[id].name,GOLD,&r138_title10,0);
  r134_wrap(60,158,360,4,12,relic_defs[id].desc,WHITE,&r134_f9,0);
 }else card_render(172,44,136,167,g->expedition.gift,1,1);
 button(73,215,145,27,"TAKE",g->selection==0);button(263,215,145,27,"LEAVE",g->selection==1);
 if(g->message[0])r134_center(240,246,g->message,GOLD,&r134_f8);
 footer("LEFT/RIGHT CHOOSE   X CONFIRM   O LEAVE");
}
static void expedition_mystery_view(const Game*g){
 ascent_heading("THE NAMELESS EXCHANGE");
 for(int i=0;i<3;i++){
  int id=CARD_ID(g->reward[i]),owner=id<NINJA_FIRST?0:id<NINJA_LAST?1:2;
  r162_card_choice=1;card_render(20+i*156,46,128,169,g->reward[i],g->selection==i,1);r162_card_choice=0;
  r134_center(84+i*156,222,ascent_names[owner],GOLD,&r134_f8);
 }
 button(171,236,138,23,"LEAVE",g->selection==3);
 if(g->message[0])r134_center(240,31,g->message,GOLD,&r134_f8);
 footer("LEFT/RIGHT CHOOSE   X TAKE ONE   O LEAVE");
}
