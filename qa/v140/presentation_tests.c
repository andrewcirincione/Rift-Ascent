#include "game.h"
#include "render.c"
#include <assert.h>
#include <stdio.h>
static uint32_t pixels[480*272];
static void shot(Game*g,unsigned tick,const char*name){
 game_render_frame(g,pixels,480,tick);char p[256];snprintf(p,sizeof(p),"qa/v140/evidence/native-%s.ppm",name);FILE*f=fopen(p,"wb");assert(f);fprintf(f,"P6\n480 272\n255\n");
 for(int i=0;i<480*272;i++){fputc(pixels[i]&255,f);fputc(pixels[i]>>8&255,f);fputc(pixels[i]>>16&255,f);}fclose(f);
 snprintf(p,sizeof(p),"qa/v140/evidence/state-%s.dat",name);assert(game_save(g,p));
}
static void battle(Game*g){game_new(g,140);game_start_battle(g,NODE_FIGHT);g->enemy_count=2;g->enemies[1]=g->enemies[0];g->enemies[0].art=16;g->enemies[1].art=19;
 for(int i=0;i<2;i++){snprintf(g->enemies[i].name,24,"%s",game_enemy_name(g->enemies[i].art));g->enemies[i].hp=g->enemies[i].maxhp=120;}
 g->hand_count=10;for(int i=0;i<10;i++)g->hand[i]=i;g->hand[2]=54|UPBIT;g->energy=5;g->selection=0;
 r137_count=r1311_finishing=victory_replay=hand_exchange=fx_transition=0;rift120_played_card=rift120_auto_card=-1;
}
int main(void){Game g,b;char name[80];
 battle(&g);g.selection=2;shot(&g,300,"combat");g.selection=5;shot(&g,300,"hand-middle");g.selection=9;shot(&g,300,"hand-left");
 for(int art=0;art<36;art++){
  battle(&g);g.selection=g.hand_count;g.enemy_count=1;g.enemies[0].art=art;
  snprintf(g.enemies[0].name,24,"%s",game_enemy_name(art));
  int seen=0;for(int tick=0;tick<600;tick+=4)seen|=1<<r140_idle_breath(art,tick);assert(seen==7);
  uint32_t previous[480*272];int changed=0;
  for(int f=0;f<2;f++){
   int tick=0;while(r140_idle_breath(art,tick)!=f*2)tick+=4;
   snprintf(name,sizeof(name),"idle-%02d-%d",art,f);shot(&g,tick,name);
   if(!f)memcpy(previous,pixels,sizeof(pixels));
   else for(int y=85;y<174;y++)for(int x=265;x<440;x++)changed+=pixels[y*480+x]!=previous[y*480+x];
  }
  assert(changed>100);
 }
 game_new(&g,140);g.screen=REST;shot(&g,300,"camp-rest");g.selection=1;shot(&g,300,"camp-upgrade");
 g.screen=POTION_VIEW;g.selection=1;g.potion_count=3;g.potions[2]=5;g.relics[g.relic_count++]=17;shot(&g,300,"potions");
 battle(&g);g.relic_count=8;int ids[]={7,17,3,29,36,28,18,35};memcpy(g.relics,ids,sizeof(ids));g.potion_count=3;g.potions[0]=0;g.potions[1]=1;g.potions[2]=5;
 game_render_set_overlay(1,3,1);shot(&g,300,"inventory-potions");game_render_set_overlay(1,4,0);game_render_set_status_scroll(1);shot(&g,300,"inventory-relics");game_render_set_overlay(0,0,0);
 g.screen=RELIC_VIEW;g.selection=1;shot(&g,300,"relic-orb");
 for(int id=0;id<6;id++){game_new(&g,140);g.screen=REWARD;g.reward[0]=25;g.reward[1]=54;g.reward[2]=28;g.ic[37]=id+1;snprintf(name,sizeof(name),"potion-reward-%d",id);shot(&g,300,name);}
 game_new(&g,140);g.screen=SHOP;g.shop_cards[0]=25;g.shop_cards[1]=54;g.shop_cards[2]=28;g.shop_relic=17;g.selection=3;shot(&g,300,"shop-orb");
 g.selection=5;g.gold=500;g.potion_count=0;shot(&g,300,"shop-purchase-ready");
 g.selection=5;g.gold=200;g.potion_count=1;b=g;game_action(&g,BTN_OK);game_render_feedback(&b,&g,BTN_OK,300);shot(&g,304,"shop-gained");
 battle(&g);g.hand_count=1;g.hand[0]=8;g.selection=0;shot(&g,300,"power-ready");b=g;game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&b,&g,BTN_OK,1000);assert(r137_duration==16);
 for(int f=0;f<=16;f+=4){snprintf(name,sizeof(name),"power-%02d",f);shot(&g,1000+f,name);}assert(r140_power_pose(16)==-1);
 battle(&g);g.enemy_count=1;g.enemies[0].hp=1;g.hand_count=1;g.hand[0]=0;g.selection=0;shot(&g,300,"finisher");b=g;game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&b,&g,BTN_OK,1000);
 RiftSound sound;static int16_t pcm[44100*2];rift_sound_reset(&sound);unsigned start=sound.victory_seen;
 game_render_update_fx(1000+r1311_banner_at()-1);rift_audio_generate(&sound,pcm,1,0,0);assert(sound.victory_seen==start);
 game_render_update_fx(1000+r1311_banner_at());rift_audio_generate(&sound,pcm,44100,0,0);assert(sound.victory_seen==start+1);
 long long energy=0;int peak=0;for(int i=0;i<28665;i++){int v=abs(pcm[i*2]);if(v>peak)peak=v;energy+=(long long)v*v;}assert(peak>2000&&peak<32767&&energy>100000000);
 for(int i=28665;i<44100;i++)assert(pcm[i*2]==0);game_render_update_fx(1400);rift_audio_generate(&sound,pcm,1,0,0);assert(sound.victory_seen==start+1);
 FILE*f=fopen("qa/v140/evidence/victory-chime.s16le","wb");fwrite(pcm+2,sizeof(int16_t),28664*2,f);fclose(f);
 shot(&g,1000+r1311_banner_at()+8,"victory");
 game_new(&g,140);g.screen=MAP;for(int r=0;r<MAP_ROWS;r++)for(int c=0;c<3;c++)g.map[r][c]=NODE_TREASURE;shot(&g,300,"map-chest");

 game_new(&g,140);g.screen=MAP;g.hp=25;g.potion_count=2;g.potions[0]=0;g.potions[1]=1;g.relics[g.relic_count++]=17;shot(&g,300,"map-potions-ready");

 battle(&g);g.act=3;g.enemy_count=1;g.enemies[0].hp=1;g.battle_type=NODE_BOSS;g.hand_count=1;g.hand[0]=0;g.selection=0;shot(&g,300,"final-boss-ready");
 game_play_card(&g,0,0);assert(g.screen==VICTORY);r137_count=r1311_finishing=victory_replay=0;shot(&g,300,"endless-choice");g.selection=1;shot(&g,300,"menu-choice");
 game_new(&g,140);g.screen=REWARD;g.battle_type=NODE_BOSS;g.win_screen=1;g.reward[0]=48;g.reward[1]=54;g.reward[2]=55;shot(&g,300,"boss-rewards");
 for(int id=0;id<CARD_COUNT;id++){battle(&g);g.hand[0]=id;g.selection=0;snprintf(name,sizeof(name),"card-top-%02d",id);shot(&g,300,name);}
 battle(&g);g.hand[2]=35|UPBIT;g.selection=2;g.strength=5;g.weak=1;g.enemies[0].vulnerable=3;shot(&g,300,"bully-preview");
 game_new(&g,140);g.screen=DECK_VIEW;g.prior_screen=REST;g.deck_count=10;g.deck[9]=35;g.selection=8;shot(&g,300,"upgrade-short-row");g.prior_screen=SHOP;g.gold=200;shot(&g,300,"remove-short-row");
 battle(&g);g.screen=CARD_CHOICE;g.next_screen=25;g.discard_count=7;for(int i=0;i<7;i++)g.discard[i]=i;g.selection=5;shot(&g,300,"discard-short-row");
 battle(&g);g.hand[2]=13|UPBIT;g.selection=2;g.exhaust_count=5;for(int i=0;i<5;i++)g.exhaust[i]=1;g.strength=3;g.weak=1;g.enemies[0].vulnerable=2;shot(&g,300,"ashen-preview");
 battle(&g);g.energy=0;g.hand[2]=2;g.hand[4]=35;g.selection=2;shot(&g,300,"no-energy");g.energy=1;g.hand[2]=7;shot(&g,300,"low-energy");
 battle(&g);g.hand_count=1;g.hand[0]=7;g.selection=0;g.enemies[0].hp=g.enemies[0].maxhp=1000;shot(&g,300,"heavy-ready");
 game_render_preplay(&g,900);shot(&g,900,"attack-pre-0");shot(&g,912,"attack-pre-1");b=g;game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&b,&g,BTN_OK,1000);
 for(int f=0;f<=24;f+=4){snprintf(name,sizeof(name),"attack-new-%02d",f);shot(&g,1000+f,name);}
 for(int seed=1;seed<=6;seed++){game_new(&g,seed);g.screen=MAP;for(int scroll=0;scroll<=12;scroll+=6){g.scroll=scroll;g.selection=0;unsigned mask=game_route_visible_mask(&g,scroll);while(!(mask&(1u<<g.selection)))g.selection++;snprintf(name,sizeof(name),"map-routes-%d-%d",seed,scroll);shot(&g,300,name);}}
 for(int n=0;n<36;n++){assert(!(r140_chest[n]>>24));assert(!(r140_chest[35*36+n]>>24));assert(!(r140_chest[n*36]>>24));assert(!(r140_chest[n*36+35]>>24));}
 printf("PASS 36 enemies breathe at fixed foot anchors; 16-tick three-pose Power; once-only 650ms victory chime peak %d; full chest margins; all requested screens/fixtures rendered\n",peak);
}
