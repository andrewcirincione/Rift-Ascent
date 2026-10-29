#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../source/game.c"
#include "../source/render.c"
static uint32_t pixels[W*H];
static void output(Game*g,const char*name){
 game_render_reset_fx();game_render_frame(g,pixels,W,300);assert(rift_asset_errors==0);
 if(!name)return;char p[320];snprintf(p,sizeof(p),"qa/v184r2/evidence/native-%s.ppm",name);FILE*f=fopen(p,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){uint32_t v=pixels[i];unsigned char b[3]={v,v>>8,v>>16};fwrite(b,3,1,f);}fclose(f);
}
int main(void){rift_assets_set_file("qa/v184r2/evidence/EBOOT.PBP");assert(rift_assets_open());char root[]="/tmp/rift-184r2-render-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g;char name[90],rows[24][192];game_new(&g,181);output(&g,"title");
 for(int c=0;c<3;c++)for(int id=0;id<20;id++)for(int page=0;page<3;page++){
  g.character=c;g.screen=EXPEDITION_STORY;g.expedition.story_id=id;g.selection=page;assert(r134_lines(expedition_line(&g),336,&r134_f9,rows)<=4);assert(r134_width(expedition_title(&g),&r138_title10)<=336);
  snprintf(name,sizeof(name),"story-%d-%d",c,id);output(&g,page==0&&id%5==0?name:0);
  if(page==2){expedition_prepare_gift(&g);g.screen=EXPEDITION_GIFT;g.selection=0;snprintf(name,sizeof(name),"gift-%d-%d",c,id);output(&g,id<2?name:0);}
 }
 for(int c=0;c<3;c++){g.character=c;expedition_mystery(&g);g.selection=0;output(&g,c==0?"mystery-scene":0);g.screen=MYSTERY_CARDS;snprintf(name,sizeof(name),"mystery-%d",c);output(&g,name);}
 for(int id=74;id<94;id++){g.screen=RELIC_RESULT;g.reward[0]=id;g.relic_pending=1;g.selection=0;snprintf(name,sizeof(name),"relic-%d",id);output(&g,name);}
 g.screen=ASCENT_COLLECTION;g.scroll=4;for(int id=0;id<20;id++){g.selection=10+60+id;snprintf(name,sizeof(name),"collection-%d",id);output(&g,id%6==0?name:0);}
 for(int s=ABOMINATION_BEFORE;s<=ABOMINATION_AFTER;s++)for(int page=0;page<3;page++){g.screen=s;g.selection=page;assert(r134_lines(expedition_line(&g),336,&r134_f9,rows)<=4);snprintf(name,sizeof(name),"god-scene-%d-%d",s,page);output(&g,name);}
 g.act=7;g.endless=1;game_start_battle(&g,NODE_BOSS);output(&g,"god-combat");
 Game before=g;game_combat_fx_begin();g.enemies[0].hp=0;check_win(&g);game_combat_fx_end();game_render_feedback(&before,&g,BTN_OK,400);assert(victory_replay&&r1311_finishing&&g.screen==ABOMINATION_AFTER);game_render_frame(&g,pixels,W,404);assert(rift_asset_errors==0);g=before;
 for(int e=0;e<ENEMY_COUNT;e++){g.enemies[0].art=e;output(&g,0);}
 for(int c=0;c<3;c++)for(int act=1;act<=3;act++){
  game_new(&g,182);game_choose_character(&g,c);g.act=act;game_start_battle(&g,NODE_FIGHT);g.enemy_count=2;
  init_enemy(&g,&g.enemies[0],enemy_normal[act-1][c?c:0]);init_enemy(&g,&g.enemies[1],enemy_elite[act-1][c]);
  g.selection=g.hand_count;snprintf(name,sizeof(name),"shadow-%d-act-%d",c,act);output(&g,name);
  r182_shadows=0;snprintf(name,sizeof(name),"no-shadow-%d-act-%d",c,act);output(&g,name);r182_shadows=1;
 }
 for(int act=1;act<=3;act++)for(int k=0;k<18;k++){
  game_new(&g,182);g.act=act;game_start_battle(&g,k>=13?NODE_ELITE:NODE_FIGHT);g.enemy_count=1;
  int e=k<13?enemy_normal[act-1][k]:enemy_elite[act-1][k-13];init_enemy(&g,&g.enemies[0],e);g.selection=g.hand_count;snprintf(name,sizeof(name),"enemy-%02d-act-%d",e,act);output(&g,name);
 }
 for(int id=0;id<EVENT_COUNT;id++)for(int choice=0;choice<4;choice++){
  game_new(&g,182);game_begin_event(&g,id);g.gold=200;g.hp=40;if(choice>=game_event_choice_count(&g))continue;g.selection=choice;
  char title[100],detail[512];game_event_option(&g,choice,title,sizeof(title),detail,sizeof(detail));assert(r134_lines(detail,215,&r134_f9,rows)<=10);
  snprintf(name,sizeof(name),"event-%02d-choice-%d",id,choice);output(&g,name);
 }
 for(int c=0;c<3;c++){game_new(&g,182);g.screen=CHARACTER_SELECT;g.selection=c;snprintf(name,sizeof(name),"character-%d",c);output(&g,name);}
 game_new(&g,182);g.screen=ASCENT_SLOTS_VIEW;output(&g,"slots");
 for(int pick=0;pick<3;pick++){
  game_new(&g,182);game_begin_event(&g,18);g.gold=200;g.selection=2;game_action(&g,BTN_OK);g.selection=pick;snprintf(name,sizeof(name),"event-picker-%d",pick);output(&g,name);
  g.screen=DECK_VIEW;g.prior_screen=REST;snprintf(name,sizeof(name),"upgrade-picker-%d",pick);output(&g,name);
  g.screen=REWARD;g.battle_type=NODE_FIGHT;fill_rewards(&g);snprintf(name,sizeof(name),"reward-picker-%d",pick);output(&g,name);
  g.screen=CARD_CHOICE;g.next_screen=17;g.hand_count=3;g.hand[0]=0;g.hand[1]=1;g.hand[2]=2;snprintf(name,sizeof(name),"combat-picker-%d",pick);output(&g,name);
  expedition_mystery(&g);g.screen=MYSTERY_CARDS;g.selection=pick;snprintf(name,sizeof(name),"exchange-picker-%d",pick);output(&g,name);
 }
 Game map=g;map.screen=MAP;map.floor=0;map.selection=0;Game fight=map;game_start_battle(&fight,NODE_FIGHT);
 game_render_reset_fx();game_render_feedback(&map,&fight,BTN_OK,1000);
 assert(r161_transition_length==64);for(int t=0;t<=68;t+=4){assert(game_render_transition_silent(&fight,1000+t)==(t<48));game_render_frame(&fight,pixels,W,1000+t);assert(!rift_asset_errors);}
 game_render_reset_fx();fight.battle_type=NODE_BOSS;game_render_feedback(&map,&fight,BTN_OK,2000);assert(!game_render_transition_silent(&fight,2000));

 /* Every result path uses the same bounded event-art lookup as its choice. */
 for(int id=0;id<EVENT_COUNT;id++)for(int choice=0;choice<4;choice++){
  game_new(&g,183+id);game_begin_event(&g,id);g.gold=999;g.hp=40;g.potion_count=1;g.potions[0]=0;
  if(choice>=game_event_choice_count(&g))continue;g.selection=choice;game_action(&g,BTN_OK);
  if(g.screen==EVENT_CARD)game_action(&g,BTN_OK);
  snprintf(name,sizeof(name),"result-%02d-choice-%d",id,choice);output(&g,id>=17?name:0);
  g.screen=EVENT_RESULT;g.reward[0]=-1;g.effect_turn=id;output(&g,0);
 }
 for(int tab=0;tab<5;tab++){
  game_new(&g,183);g.screen=ASCENT_COLLECTION;g.scroll=tab;g.selection=0;snprintf(name,sizeof(name),"unlocks-%d",tab);output(&g,name);
  for(int ix=game_collection_unlock_count(tab);ix<game_collection_count(tab);ix++){
   g.selection=ix;assert(game_validate(&g));snprintf(name,sizeof(name),"base-%d",tab);output(&g,ix==game_collection_unlock_count(tab)?name:0);
  }
 }
 game_new(&g,183);game_start_battle(&g,NODE_ELITE);g.enemy_count=1;init_enemy(&g,&g.enemies[0],6);g.selection=g.hand_count;
 for(int intent=0;intent<INTENT_COUNT;intent++){
  g.enemies[0].intent=intent;g.enemies[0].intent_value=12;memset(pixels,0,sizeof(pixels));pix=pixels;sw=W;
  rift116_draw_enemy_intent(&g,0,335);int changed=0;for(int i=0;i<W*H;i++)changed+=pixels[i]!=0;assert(changed>30);
  snprintf(name,sizeof(name),"intent-%02d",intent);output(&g,intent>=18?name:0);
 }
 game_new(&g,183);game_start_battle(&g,NODE_FIGHT);g.enemy_count=2;init_enemy(&g,&g.enemies[0],2);init_enemy(&g,&g.enemies[1],47);g.selection=g.hand_count;output(&g,"mist-gazer-target");
 before=g;before.screen=DECK_VIEW;before.prior_screen=COMBAT;game_render_reset_fx();game_render_feedback(&before,&g,BTN_BACK,3000);assert(!r161_transition_active&&!fx_transition);
 puts("PASS all 42 event result art lookups, every selectable base item, all 25 overhead intents and instant combat deck return");
 puts("PASS new enemy sprites on act backgrounds, subtle shadow on/off comparison, all mystery choices, selected cards, menu panels and transition/music boundaries");

 puts("PASS all 180 new dialogue pages fit; 60 gifts; 20 relics; five collection tabs; all 59 enemy sprites; no asset errors");return 0;}
