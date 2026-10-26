#include "game.h"
#include "sound.h"
#include "render.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
extern void game_render_set_overlay(int,int,int);
extern void game_render_feedback(const Game*,const Game*,int,unsigned);
extern void game_render_set_status_scroll(int);
static void shot(Game*g,const char*name){
 static uint32_t pixels[W*H];char path[250];
 hand_exchange=0;fx_transition=0;victory_replay=0;fx_player_damage=0;fx_heal=fx_block=0;fx_enemy_damage[0]=fx_enemy_damage[1]=0;effect_kind=0;rift120_played_card=rift120_auto_card=-1;
 game_render_frame(g,pixels,W,300);
 snprintf(path,sizeof(path),"../evidence136/native-%s.ppm",name);
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n480 272\n255\n");
 for(int i=0;i<W*H;i++){fputc(pixels[i]&255,f);fputc((pixels[i]>>8)&255,f);fputc((pixels[i]>>16)&255,f);}fclose(f);
 snprintf(path,sizeof(path),"../evidence136/state-%s.dat",name);assert(game_save(g,path));
}
static void battle(Game*g){game_new(g,133);game_start_battle(g,NODE_FIGHT);g->hand_count=5;g->hand[0]=0;g->hand[1]=1;g->hand[2]=25;g->hand[3]=54;g->hand[4]=28;g->energy=5;g->block=12;g->enemy_count=2;
 for(int i=0;i<2;i++){g->enemies[i]=g->enemies[0];g->enemies[i].art=i?19:16;snprintf(g->enemies[i].name,24,"%s",game_enemy_name(g->enemies[i].art));g->enemies[i].block=8;}}
static void tests(void){Game g,t;
 for(unsigned seed=1;seed<2001;seed++){
  game_new(&g,seed);assert(MAP_ROWS+1==15);
  for(int r=0;r<MAP_ROWS-1;r++)for(int a=0;a<3;a++){
   unsigned ma=game_route_mask(&g,r,a);assert(ma&(1u<<a));assert(ma<8);
   for(int b=a+1;b<3;b++)for(int da=0;da<3;da++)for(int db=0;db<3;db++)
    assert(!(ma&(1u<<da))||!(game_route_mask(&g,r,b)&(1u<<db))||da<=db);
  }
  for(int c=0;c<3;c++)for(int r=1;r<MAP_ROWS;r++){
   int a=g.map[r-1][c],b=g.map[r][c];
   int utility_a=a==NODE_EVENT||a==NODE_SHOP||a==NODE_REST;
   int utility_b=b==NODE_EVENT||b==NODE_SHOP||b==NODE_REST;
   assert(!(utility_a&&utility_b));
  }
  for(int c=0;c<3;c++){int shop=0,rest=0;for(int r=0;r<MAP_ROWS;r++){shop+=g.map[r][c]==NODE_SHOP;rest+=g.map[r][c]==NODE_REST;}assert(shop==1&&rest==2);}
 }
 battle(&g);g.screen=SHOP;g.selection=2;g.gold=200;int count=g.deck_count;
 game_action(&g,BTN_ALT);assert(g.screen==DECK_VIEW&&g.deck_readonly);game_action(&g,BTN_OK);assert(g.deck_count==count&&g.gold==200);game_action(&g,BTN_BACK);assert(g.screen==SHOP&&g.selection==2);
 game_action(&g,BTN_POTION);assert(g.screen==POTION_VIEW);game_action(&g,BTN_OK);assert(g.potion_count==2);game_action(&g,BTN_BACK);assert(g.screen==SHOP&&g.selection==2);
 g.selection=4;game_action(&g,BTN_OK);assert(g.screen==DECK_VIEW&&!g.deck_readonly);game_action(&g,BTN_OK);assert(g.deck_count==count-1&&g.gold==140);
 for(int skip=0;skip<2;skip++){
  game_new(&g,133);g.screen=MAP;g.map[0][1]=NODE_TREASURE;int old=g.relic_count;game_choose_node(&g,1);assert(g.screen==RELIC_RESULT&&g.relic_count==old);g.selection=skip;game_action(&g,BTN_OK);assert(g.relic_count==old+!skip&&g.screen==MAP);
 }
 for(int mode=NODE_ELITE;mode<=NODE_BOSS;mode+=NODE_BOSS-NODE_ELITE){
  game_new(&g,133);game_start_battle(&g,mode);int old=g.relic_count;for(int i=0;i<g.enemy_count;i++)g.enemies[i].hp=0;game_end_turn(&g);
  assert(g.screen==RELIC_RESULT&&g.relic_count==old);game_action(&g,BTN_BACK);assert(g.screen==REWARD&&g.relic_count==old);g.selection=3;game_action(&g,BTN_OK);assert(g.screen==MAP);
 }
 /* A declined cursed relic must not change max HP or Energy. */
 for(int skip=0;skip<2;skip++){
  game_new(&g,133);g.screen=RELIC_RESULT;g.reward[0]=11;g.relic_pending=1;g.relic_resume=MAP;g.selection=skip;
  int hp=g.maxhp,energy=g.max_energy,count=g.relic_count;game_action(&g,BTN_OK);
  assert(g.maxhp==hp-(skip?0:12)&&g.max_energy==energy+!skip&&g.relic_count==count+!skip);
 }
 /* Traverse all 45 floors with deterministic won combats. */
 game_new(&g,133);g.screen=MAP;
 for(int act=1;act<=3;act++){
  assert(g.act==act);
  for(int floor=0;floor<MAP_ROWS;floor++){
   assert(g.floor==floor);g.screen=MAP;g.selection=g.lane;
   g.map[floor][g.lane]=NODE_FIGHT;game_choose_node(&g,g.lane);
   assert(g.screen==COMBAT);for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=0;
   game_end_turn(&g);assert(g.screen==REWARD);g.selection=3;game_action(&g,BTN_OK);
  }
  assert(g.floor==MAP_ROWS&&g.screen==MAP);game_action(&g,BTN_OK);
  assert(g.screen==COMBAT&&g.battle_type==NODE_BOSS);
  for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=0;
  game_end_turn(&g);assert(g.screen==RELIC_RESULT);game_action(&g,BTN_BACK);
  assert(g.screen==REWARD);g.selection=3;game_action(&g,BTN_OK);
 }
 assert(g.screen==VICTORY);RiftProfile profile;profile_init(&profile);assert(profile_record(&profile,&g));assert(profile.furthest_floor==45);
 battle(&g);assert(strstr(card_description(25),"discarded card")&&strstr(card_description(25|UPBIT),"draw pile"));
 g.ic[IC_BARRICADE]=1;g.ic[IC_PYRE]=2;g.ic[IC_DARK_EMBRACE]=1;assert(game_status_count(&g)>=3);
 assert(game_save(&g,"../evidence136/test-save.dat"));assert(game_load(&t,"../evidence136/test-save.dat"));assert(!memcmp(&g,&t,sizeof(g)));
 RiftSound sound;int16_t pcm[2048];rift_sound_reset(&sound);rift_sound_effect(4);rift_audio_generate(&sound,pcm,1024,0,0);long power=0;for(int i=0;i<2048;i++)power+=abs(pcm[i]);assert(power>500000);rift_sound_effect(6);rift_audio_generate(&sound,pcm,512,0,0);assert(sound.fx_kind==4);
 puts("PASS: 2,000 planar maps, utility spacing, shop browsing/removal, potion viewing, relic take/skip, elite/boss progression, 45-floor campaign, cursed-relic side effects, descriptions, powers, save roundtrip and damage audio");
}
#include "math_tests.h"
#include "visual_tests.h"
int main(void){math_tests();dynamic_visual_tests();tests();
 int minheight=1000,maxheight=0;
 for(int raw=0;raw<2*CARD_COUNT;raw++){
  int id=raw%CARD_COUNT,c=id|(raw>=CARD_COUNT?UPBIT:0);char lines[24][192];
  R135CardLayout l=r135_card_layout(c,126);
  assert(l.height>=100&&l.height<=166);
  assert(l.body_y+l.rows*l.leading<=l.height-18);
  assert(l.rows==r134_lines(rift131_clean_rules(card_description(c)),92,l.font,lines));
  assert(l.title_rows<=2);assert(l.art_h==29);
  if(l.height<minheight)minheight=l.height;if(l.height>maxheight)maxheight=l.height;
  assert(r134_lines(cards[id].name,50,&r134_f8,lines)<=2);
 }
 printf("PASS: all 184 base/upgraded previews retain complete text and 29px art; heights %d-%d (formerly 166)\n",minheight,maxheight);
Game g;char name[70];game_new(&g,133);shot(&g,"title");
 for(int i=0;i<5;i++){battle(&g);g.selection=i;snprintf(name,70,"hand-%d",i);shot(&g,name);}
 battle(&g);g.energy=0;shot(&g,"empty-energy");g.selection=g.hand_count;shot(&g,"end");
 battle(&g);g.hand_count=10;for(int i=5;i<10;i++)g.hand[i]=i;g.selection=9;shot(&g,"hand-10");
 for(int i=0;i<36;i++){battle(&g);g.selection=g.hand_count;g.enemy_count=1;g.enemies[0].art=i;snprintf(name,70,"enemy-%02d",i);shot(&g,name);}
 game_new(&g,133);g.screen=MAP;shot(&g,"map-start");g.floor=7;shot(&g,"map-middle");g.floor=13;shot(&g,"map-end");
 game_new(&g,133);g.screen=SHOP;g.shop_cards[0]=25;g.shop_cards[1]=54;g.shop_cards[2]=28;g.shop_relic=3;shot(&g,"shop");game_action(&g,BTN_ALT);shot(&g,"shop-deck");game_action(&g,BTN_BACK);game_action(&g,BTN_POTION);shot(&g,"shop-potions");
 game_new(&g,133);g.screen=MAP;g.map[0][1]=NODE_TREASURE;game_choose_node(&g,1);shot(&g,"relic");
 battle(&g);for(int i=0;i<33;i++)g.ic[i]=1;g.used_potion=(1<<16)|6;g.shop_cards[0]=2;g.shop_cards[1]=4;g.shop_cards[2]=3;g.heal_after_battle=3;g.weak=2;g.vulnerable=1;g.poison=2;
 game_render_set_overlay(1,2,0);for(int i=0;i<game_status_count(&g);i+=7){game_render_set_status_scroll(i);snprintf(name,70,"status-%02d",i);shot(&g,name);}game_render_set_overlay(0,0,0);
 game_new(&g,133);g.screen=EVENT;g.effect_turn=6;shot(&g,"armory");
 battle(&g);Game before=g;game_end_turn(&g);game_render_feedback(&before,&g,BTN_BACK,295);shot(&g,"discard-animation");

 game_new(&g,134);
 int screens[]={HELP,REST,REWARD,BOSS_REWARD,RELIC_VIEW,DEFEAT,VICTORY,EVENT_RESULT};
 for(int j=0;j<8;j++){g.screen=screens[j];g.selection=0;g.reward[0]=25;g.reward[1]=54;g.reward[2]=28;if(g.screen==BOSS_REWARD){g.reward[0]=1;g.reward[1]=2;g.reward[2]=3;}g.relic_count=10;for(int k=0;k<10;k++)g.relics[k]=k;snprintf(name,70,"screen-%02d",g.screen);shot(&g,name);}
 for(int j=0;j<12;j++){g.screen=EVENT;g.effect_turn=j;snprintf(name,70,"event-%02d",j);shot(&g,name);}
 battle(&g);for(int j=0;j<CARD_COUNT;j++)if(!strcmp(cards[j].name,"Setup Strike"))g.hand[4]=j;g.selection=g.hand_count;shot(&g,"setup-thumbnail");g.selection=4;shot(&g,"setup-preview");
 for(int j=0;j<CARD_COUNT;j++){battle(&g);g.hand[2]=j;g.selection=2;snprintf(name,70,"card-%02d",j);shot(&g,name);}
 battle(&g);g.selection=2;g.strength=3;g.dexterity=2;g.enemies[0].intent=0;g.enemies[0].intent_value=12;g.enemies[1].intent=7;g.enemies[1].intent_value=8;g.weak=2;g.vulnerable=1;g.poison=3;
 for(int e=0;e<2;e++){g.enemies[e].strength=2+e;g.enemies[e].weak=1+e;g.enemies[e].vulnerable=2;g.enemies[e].poison=3+e;}
 shot(&g,"combat-buffs");
 g.strength=-2;g.dexterity=-1;g.energy_penalty=1;shot(&g,"combat-negative");
 int values[6];assert(r135_status_values(&g,-1,values)==6);assert(values[0]==-2&&values[1]==-1&&values[2]==2&&values[3]==1&&values[4]==3&&values[5]==1);
 for(int e=0;e<2;e++){assert(r135_status_values(&g,e,values)==4);assert(values[2]==1+e&&values[3]==2&&values[4]==3+e);}
 battle(&g);g.selection=g.hand_count;g.strength=g.dexterity=g.weak=g.vulnerable=g.poison=g.energy_penalty=0;assert(r135_status_values(&g,-1,values)==0);shot(&g,"combat-no-status");
 puts("PASS: all player/enemy core status icons, turn/stack values, negative stats, energy drain and zero-state suppression");
 battle(&g);g.selection=2;g.hand[2]=54|UPBIT;shot(&g,"combat-long-description");
 battle(&g);g.selection=0;shot(&g,"combat-short-description");
 battle(&g);g.selection=4;game_render_preplay(&g,1000);
 static uint32_t pixels[W*H];int ages[]={0,6,12,18,25,30,35,37,38};
 for(int j=0;j<9;j++){game_render_frame(&g,pixels,W,1000+ages[j]);char path[150];snprintf(path,150,"../evidence136/flight-%02d.ppm",ages[j]);FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n480 272\n255\n");for(int k=0;k<W*H;k++){fputc(pixels[k]&255,f);fputc(pixels[k]>>8&255,f);fputc(pixels[k]>>16&255,f);}fclose(f);}
 RiftSound audio;int16_t pcm[22050*2];rift_sound_reset(&audio);rift_sound_effect(1);rift_sound_effect(8);rift_audio_generate(&audio,pcm,22050,0,0);assert(audio.fx_kind==1&&audio.block_age==10000);long energy=0;for(int j=6000*2;j<10000*2;j++)energy+=abs(pcm[j]);assert(energy>3000000);FILE*af=fopen("../evidence136/block-with-attack.s16le","wb");assert(af);fwrite(pcm,sizeof(pcm),1,af);fclose(af);
 puts("PASS: block mixes with attack and persists independently; all menus, 12 event illustrations, all card previews and nine animation frames rendered");
}
