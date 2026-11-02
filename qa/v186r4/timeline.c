/* Retain the full r2 frame regression and independently assert scene artwork. */
#define main previous_timeline_main
#include "../v186r2/timeline.c"
#undef main
int main(void){
 assert(previous_timeline_main()==0);Game g;
 const int screens[]={ASCENT_ENDING,ASCENT_REVELATION,ASCENT_BOSS,ASCENT_TRUE_END};
 for(int c=0;c<3;c++)for(int kind=0;kind<4;kind++){
  game_new(&g,18603);g.character=c;g.act=kind>=2?4:3;g.endless=g.act>3;g.screen=screens[kind];g.selection=0;game_render_reset_fx();
  game_render_frame(&g,frame,480,0);
  pix=expected;sw=480;rect(0,0,480,272,0);
  /* Explicit semantic oracle: ordinary endings use the return-to-city art;
     secret revelations and confrontations retain their distinct images. */
  art_blit(72,38,336,126,ascent_scene_art[c][kind==0?3:kind],336,126);
  for(int y=38;y<164;y++)assert(!memcmp(frame+y*480+72,expected+y*480+72,336*sizeof(*frame)));
  assert(!rift_asset_errors);
 }
 for(int page=0;page<ASCENT_FINALE_PAGES;page++){
  char rows[24][192];assert(r134_lines(ascent_finale_dialogue[page],336,&r134_f9,rows)<=4);
  game_new(&g,18604);g.act=4;g.endless=1;g.ascent.ascended=1;g.screen=ASCENT_FINALE;g.selection=page;game_render_reset_fx();game_render_frame(&g,frame,480,0);
  pix=expected;sw=480;rect(0,0,480,272,0);art_blit(72,38,336,126,ascent_finale_art,336,126);
  for(int y=38;y<164;y++)assert(!memcmp(frame+y*480+72,expected+y*480+72,336*sizeof(*frame)));
  assert(game_story_kind(&g)==1&&game_music_silent(&g)&&!rift_asset_errors);
 }
 game_render_reset_fx();g.screen=ASCENT_FINALE;g.selection=0;Game before=g;before.screen=ASCENT_TRUE_END;before.selection=2;
 game_render_feedback(&before,&g,BTN_OK,500);assert(game_render_transition_busy(500));
 game_render_frame(&g,frame,480,516);assert(!brightness(frame));
 game_render_frame(&g,frame,480,532);assert(!game_render_transition_busy(532));
 for(int y=38;y<164;y++)assert(!memcmp(frame+y*480+72,expected+y*480+72,336*sizeof(*frame)));
 puts("PASS individual Act 4 ending transitions once through black into the shared finale artwork.");
 AscentAccount*account=(AscentAccount*)ascent_profile();
 for(int c=0;c<3;c++){
  account->wins[c]=4;account->ascended=7;pix=expected;sw=480;rect(0,0,480,272,0);ascent_badges(0,0,c);
  r135_shadow_text(64,31,"A",RGB(255,210,89),&r134_f18b);
  account->wins[c]=5;account->ascended=0;pix=frame;rect(0,0,480,272,0);ascent_badges(0,0,c);
  for(int y=31;y<55;y++)assert(!memcmp(frame+y*480+64,expected+y*480+64,18*sizeof(*frame)));
  account->wins[c]=4;pix=frame;rect(0,0,480,272,0);ascent_badges(0,0,c);int different=0;
  for(int y=31;y<55;y++)different|=memcmp(frame+y*480+64,expected+y*480+64,18*sizeof(*frame));assert(different);
 }
 puts("PASS all five finale pages use shared artwork; golden A absent at four wins even with an Act 4 clear, present at five before any Act 4 clear.");
 puts("PASS all 12 scene/character image mappings: normal victory uses resolution artwork, never the secret-boss confrontation; other scenes retain their intended images.");return 0;
}
