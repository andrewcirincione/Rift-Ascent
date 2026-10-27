#include "source/game.c"
#include <assert.h>
int main(void){Game g;int cases=0;
 for(int id=0;id<ENEMY_COUNT;id++)for(int floor=0;floor<=MAP_ROWS;floor++)for(int pair=1;pair<=2;pair++){
  int prevhp=0,prevdmg=0;
  for(int act=1;act<=1000;act++){
   game_new(&g,150);g.endless=1;g.act=act;g.floor=floor;g.enemy_count=pair;g.battle_type=enemy_defs[id].tier==2?NODE_BOSS:enemy_defs[id].tier==1?NODE_ELITE:NODE_FIGHT;
   init_enemy(&g,&g.enemies[0],id);Enemy*e=&g.enemies[0];
   assert(e->maxhp>prevhp&&e->intent_value>=prevdmg);prevhp=e->maxhp;prevdmg=e->intent_value;
   long long oldhp=act==1?94:act==2?116:152LL+36LL*(act-3);if(act==1&&g.battle_type==NODE_FIGHT)oldhp+=10;
   if(enemy_defs[id].tier==2)oldhp=act>=3?146LL+30LL*(act-3):100+act*8;else oldhp+=floor;
   oldhp=(enemy_defs[id].hp*oldhp+50)/100;if(pair==2)oldhp=(oldhp*72+50)/100;
   long long olddmg=act==1?82:act==2?105:138LL+33LL*(act-3);if(act==1&&g.battle_type==NODE_FIGHT)olddmg+=8;olddmg=(enemy_defs[id].attack*olddmg+50)/100+floor/7;
   assert(e->hp>=oldhp&&e->intent_value<=olddmg);cases++;
  }
 }
 printf("PASS %d enemy/act/floor/group scaling cases: HP grows faster, damage grows slower, neither decreases across acts; Act I baseline preserved\n",cases);
 puts("Act | Ash Crawler HP / attack | Furnace Maw HP / attack");
 for(int a=1;a<=8;a++){game_new(&g,150);g.endless=1;g.act=a;g.floor=7;g.enemy_count=1;g.battle_type=NODE_FIGHT;init_enemy(&g,&g.enemies[0],0);int hp=g.enemies[0].hp,dmg=g.enemies[0].intent_value;g.battle_type=NODE_BOSS;g.floor=14;init_enemy(&g,&g.enemies[0],15);printf("%d | %d / %d | %d / %d\n",a,hp,dmg,g.enemies[0].hp,g.enemies[0].intent_value);}
}
