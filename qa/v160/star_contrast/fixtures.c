/* Reuse the release's combat setup; exercise only changed presentation. */
#define main prior_ui_fixture_main
#include "../ui_fixtures.c"
#undef main
static void capture(Game*g,const char*name){
 static uint32_t frame[W*H];char path[256];game_render_reset_fx();game_render_frame(g,frame,W,300);
 snprintf(path,sizeof(path),"qa/v160/star_contrast/evidence/native-%s.ppm",name);FILE*f=fopen(path,"wb");assert(f);
 fprintf(f,"P6\n480 272\n255\n");for(int i=0;i<W*H;i++){fputc(frame[i]&255,f);fputc(frame[i]>>8&255,f);fputc(frame[i]>>16&255,f);}fclose(f);
 snprintf(path,sizeof(path),"qa/v160/star_contrast/evidence/state-%s.dat",name);assert(game_save(g,path));
}
int main(void){
 Game g;int counts[]={0,1,3,15,99,100,999};char name[40];
 for(unsigned i=0;i<sizeof(counts)/sizeof(counts[0]);i++){
  arena(&g,CHAR_REGENT);g.regent.stars=counts[i];g.selection=2;snprintf(name,sizeof(name),"stars-%d",counts[i]);capture(&g,name);
 }
 arena(&g,CHAR_REGENT);g.regent.stars=2;g.selection=2;capture(&g,"last-stars");
 arena(&g,CHAR_REGENT);g.regent.stars=0;g.selection=0;capture(&g,"regain-stars");
 arena(&g,CHAR_REGENT);g.regent.stars=15;g.hand[2]=R_STARDUST;g.selection=2;assert(game_card_star_cost(&g,g.hand[2])==-1);capture(&g,"cost-x");
 arena(&g,CHAR_REGENT);g.regent.stars=0;g.hand[2]=R_FALLING_STAR|CARD_FREE;g.selection=2;capture(&g,"cost-free");
 arena(&g,CHAR_REGENT);g.regent.stars=15;g.hand[2]=R_FALLING_STAR|(MOD_ECHO<<CARD_MOD_SHIFT);g.selection=2;capture(&g,"modified");
 for(int c=0;c<2;c++){arena(&g,c);snprintf(name,sizeof(name),"character-%d",c);capture(&g,name);}
 arena(&g,CHAR_REGENT);g.screen=REST;g.selection=1;game_action(&g,BTN_OK);g.deck[0]=R_SEVEN_STARS;capture(&g,"upgrade");
 puts("PASS Stars render fixtures: 0/1/3/15/99/100/999, X, free cost, modification, upgrade and other characters");
 return 0;
}
