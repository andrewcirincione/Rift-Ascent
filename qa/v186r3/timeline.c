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
 puts("PASS all 12 scene/character image mappings: normal victory uses resolution artwork, never the secret-boss confrontation; other scenes retain their intended images.");return 0;
}
