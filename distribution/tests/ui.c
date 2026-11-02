#include <assert.h>
#include <stdio.h>
#include "../../qa/v186/source/render.c"
void rift_sound_effect(int kind){(void)kind;}
void rift_sound_impact(int amount,int blocked){(void)amount;(void)blocked;}
static uint32_t pixels[480*272];
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
 return 0;
}
