#include <assert.h>
#include <stdio.h>
#include "../../qa/v186/source/render.c"
#include "../../qa/v186/source/story170_data.h"
#include "../../qa/v186/source/expedition_stories.h"
void rift_sound_effect(int kind){(void)kind;}
void rift_sound_impact(int amount,int blocked){(void)amount;(void)blocked;}
static uint32_t pixels[480*272];
static int story_checked;
static void story_fit(const char*s){
 if(!s)return;
 char rows[24][192];int n=r134_lines(s,336,&r134_f9,rows);
 if(n>4){fprintf(stderr,"Story overflow (%d rows): %s\n",n,s);abort();}
 story_checked++;
}
int main(void){
 rift_assets_set_file("distribution/evidence/EBOOT.PBP");assert(rift_assets_open());Game g;game_new(&g,701);game_start_battle(&g,NODE_FIGHT);int checked=0;
 for(int id=0;id<CARD_COUNT;id++)for(int up=0;up<2;up++)for(int mod=0;mod<MOD_COUNT;mod++){
  int raw=id|(up?UPBIT:0)|(mod<<CARD_MOD_SHIFT);if(mod&&!game_can_modify(id|(up?UPBIT:0),mod))continue;
  card_combat_context=0;R135CardLayout l=r150_hand_preview_layout(raw);if(l.height>166||l.title_rows>2){fprintf(stderr,"UI overflow %s up%d mod%d height%d width%d title%d\n",cards[id].name,up,mod,l.height,l.width,l.title_rows);return 1;}
  char lines[24][192];int n=r134_lines(card_description(raw),l.width-34,l.font,lines);assert(l.body_y+n*l.leading+18<=l.height);checked++;
 }
 for(int c=0;c<3;c++)for(int e=1;e<=3;e++){
  game_choose_character(&g,c);game_start_battle(&g,NODE_FIGHT);g.enemy_count=e;g.regent.stars=9;
  for(int i=0;i<e;i++){g.enemies[i]=g.enemies[0];g.enemies[i].hp=88;g.enemies[i].maxhp=111;}
  g.hp=g.maxhp=987;game_render_reset_fx();game_render_frame(&g,pixels,480,100);assert(!rift_asset_errors);
 }
 printf("PASS %d full card previews: complete names/rules across upgrades and all valid forge modifiers fit the combat HUD; all characters render with 1/2/3 enemies and large max HP.\n",checked);
 for(int c=0;c<3;c++){
  char root[]="/tmp/rift-story-fit-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));
  for(int a=0;a<4;a++)for(int n=0;n<10;n++)story_fit(r170_entry[c][a][n]);
  for(int a=0;a<3;a++)for(int n=0;n<5;n++)for(int p=0;p<3;p++)story_fit(r170_boss[c][a][n][p]);
  for(int kind=0;kind<4;kind++)for(int p=0;p<3;p++)story_fit(ascent_dialogue[c][kind][p]);
  for(int n=0;n<20;n++)for(int p=0;p<3;p++)story_fit(expedition_stories[c][n].lines[p]);
  for(int wins=0;wins<=5;wins++){
   assert(ascent_begin(&g,0,c,900+c*10+wins));
   for(int a=1;a<=4;a++)for(int n=0;n<10;n++)for(int p=0;p<3;p++){
    g.act=a;g.scroll=n;g.selection=p;g.screen=STORY_ACT;story_fit(game_loop_story(&g));
    g.screen=STORY_BOSS;story_fit(game_loop_story(&g));
   }
   for(int n=0;n<20;n++)for(int p=0;p<3;p++){
    g.screen=EXPEDITION_STORY;g.expedition.story_id=n;g.selection=p;story_fit(game_loop_event(&g));
   }
   for(int k=0;k<4;k++)for(int p=0;p<3;p++)story_fit(game_loop_ascent(&g,k,p));
   g.act=3;g.floor=MAP_ROWS;game_start_battle(&g,NODE_BOSS);g.energy=12;g.hand_count=2;g.relic_count=0;
   for(int i=0;i<2;i++)g.hand[i]=c==0?0:c==1?173:269;
   for(int e=0;e<g.enemy_count;e++){g.enemies[e].hp=1;g.enemies[e].block=0;}
   for(int e=0;e<g.enemy_count;e++)if(g.enemies[e].hp>0)game_play_card(&g,0,e);
   assert(g.screen==ASCENT_ENDING);
  }
 }
 for(int p=0;p<ASCENT_FINALE_PAGES;p++)story_fit(ascent_finale_dialogue[p]);
 printf("PASS %d story text samples fit the four-line PSP dialogue area, including return history and five-win revelations.\n",story_checked);
 return 0;
}
