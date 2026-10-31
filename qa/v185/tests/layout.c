#include <assert.h>
#include <stddef.h>
#include <sys/stat.h>
#include "../source/game.c"
static void save(const char*name,const void*data,size_t size){if(size==sizeof(Game)&&!game_validate(data)){const Game*g=data;fprintf(stderr,"Invalid fixture %s: screen %d character %d act %d floor %d\n",name,g->screen,g->character,g->act,g->floor);abort();}char path[240];snprintf(path,sizeof(path),"qa/v185/evidence/%s.bin",name);FILE*f=fopen(path,"wb");assert(f&&fwrite(data,size,1,f)==1);fclose(f);}
int main(void){char temp[]="/tmp/rift-r4-fixtures-XXXXXX";assert(mkdtemp(temp)&&ascent_open(temp));ascent_account.debug=1;Game g,h;char name[80];
 FILE*expected=fopen("qa/v185/evidence/expected-turns.json","w");assert(expected);fputs("{",expected);
 const int arts[]={6,14,20,43,35,0,8,0};const int acts[]={1,1,2,2,3,1,3,1};const int intents[]={24,27,28,29,30,0,10,0};
 for(int k=0;k<8;k++){
  assert(ascent_begin(&g,0,0,18440));g.act=acts[k];g.floor=5;game_start_battle(&g,k==6?NODE_BOSS:k==2||k==5?NODE_FIGHT:NODE_ELITE);g.relic_count=0;g.enemy_count=1;
  g.turn=0;init_enemy(&g,&g.enemies[0],arts[k]);g.turn=1;g.enemies[0].intent=intents[k];g.hp=g.maxhp=80;g.block=k==2?5:0;g.vulnerable=g.poison=0;
  g.hand_count=g.discard_count=0;g.draw_count=30;for(int j=0;j<30;j++)g.draw[j]=1;g.selection=0;
  if(k==7){g.enemy_count=2;init_enemy(&g,&g.enemies[0],0);init_enemy(&g,&g.enemies[1],3);g.enemies[0].intent=g.enemies[1].intent=0;}
  if(k==0)sprintf(name,"fixture-stone");else sprintf(name,"fixture-combat-%d",k);save(name,&g,sizeof(g));
  if(k)fputs(",",expected);fprintf(expected,"\"%s\":[",name+8);h=g;
  for(int turn=0;turn<(k==0?4:1);turn++){
   int shown=game_enemy_intent_damage(&h,0),intent=h.enemies[0].intent;game_end_turn(&h);
   if(turn)fputs(",",expected);fprintf(expected,"{\"intent\":%d,\"shown\":%d,\"hp\":%d,\"energy\":%d,\"enemy_block\":%d,\"vulnerable\":%d,\"poison\":%d}",intent,shown,h.hp,h.energy,h.enemies[0].block,h.vulnerable,h.poison);
  }fputs("]",expected);
 }
 fputs("}\n",expected);fclose(expected);save("fixture-account",&ascent_account,sizeof(ascent_account));
 assert(ascent_begin(&g,0,2,18449));g.screen=REWARD;g.battle_type=NODE_BOSS;g.win_screen=1;g.reward[0]=R_DEBRIS;g.reward[1]=R_COMET;g.reward[2]=R_BIG_BANG;g.v151.card_done=0;assert(game_validate(&g));
 unsigned head[5]={0x52413846u,1,100,sizeof(g),hash_bytes((void*)&g,sizeof(g))};FILE*legacy=fopen("qa/v185/evidence/legacy-debris-slot.bin","wb");assert(legacy);fwrite(head,sizeof(head),1,legacy);fwrite(&g,sizeof(g),1,legacy);fclose(legacy);
 assert(ascent_begin(&g,0,2,18444));game_start_battle(&g,NODE_FIGHT);g.enemy_count=1;g.enemies[0].hp=g.enemies[0].maxhp=999;g.relic_count=0;g.hp=g.maxhp=75;
 g.hand_count=2;g.hand[0]=R_HEGEMONY|UPBIT;g.hand[1]=R_REFINE_BLADE|UPBIT;g.selection=0;g.draw_count=20;g.discard_count=0;
 for(int i=0;i<20;i++)g.draw[i]=R_DEFEND_REGENT;save("fixture-proxy-energy",&g,sizeof(g));
 FILE*f=fopen("qa/v185/evidence/state-layout.json","w");assert(f);fprintf(f,"{\"size\":%zu,\"account_size\":%zu",sizeof(Game),sizeof(AscentAccount));
#define OFF(x) fprintf(f,",\"" #x "\":%zu",offsetof(Game,x))
 OFF(master_index);OFF(subtitle);OFF(screen);OFF(selection);OFF(scroll);OFF(act);OFF(character);OFF(hp);OFF(maxhp);OFF(ascent);OFF(enemy_count);OFF(enemies);OFF(floor);OFF(battle_type);OFF(combat_music);OFF(potion_count);OFF(potions);OFF(hand_count);OFF(hand);OFF(reward);OFF(prior_screen);OFF(energy);OFF(ic);OFF(expedition);OFF(deck_count);OFF(relic_count);OFF(v151);OFF(revision182);OFF(gold);OFF(strength);OFF(block);OFF(weak);OFF(vulnerable);OFF(poison);OFF(deck);OFF(message);OFF(revision184);OFF(regent);OFF(ninja.v);
 fprintf(f,",\"enemy_size\":%zu,\"enemy_hp\":%zu,\"enemy_art\":%zu,\"account_cards\":%zu,\"account_wins\":%zu,\"account_ascended\":%zu,\"account_debug\":%zu,\"energy_lock\":%d,\"potion_offer\":%zu,\"screens\":{",sizeof(Enemy),offsetof(Enemy,hp),offsetof(Enemy,art),offsetof(AscentAccount,cards),offsetof(AscentAccount,wins),offsetof(AscentAccount,ascended),offsetof(AscentAccount,debug),IC_ENERGY_LOCK,offsetof(Game,v151.potion_offer));
#define SCREEN(x) fprintf(f,"\"" #x "\":%d,",x)
 SCREEN(BETA_FORGE);SCREEN(BETA_FORGE_CARD);SCREEN(ASCENT_BOSS);SCREEN(CUSTOM_CARDS);SCREEN(CUSTOM_ITEMS);SCREEN(CUSTOM_QUANTITY);SCREEN(CUSTOM_CONFIRM);SCREEN(ASCENT_ENDING);SCREEN(DECK_VIEW);SCREEN(PAUSE_MENU);SCREEN(EVENT);SCREEN(EVENT_CARD);SCREEN(EVENT_RESULT);SCREEN(TITLE);SCREEN(MAP);SCREEN(COMBAT);SCREEN(VICTORY);SCREEN(ASCENT_SLOTS_VIEW);SCREEN(ASCENT_OPTIONS);SCREEN(ASCENT_COLLECTION);SCREEN(CHARACTER_SELECT);SCREEN(ACT_INTRO);SCREEN(BLESSING);SCREEN(STORY_ACT_BEFORE_BLESSING);SCREEN(EXPEDITION_STORY);SCREEN(EXPEDITION_GIFT);SCREEN(ABOMINATION_BEFORE);SCREEN(ABOMINATION_AFTER);SCREEN(MYSTERY_STORY);SCREEN(MYSTERY_CARDS);SCREEN(REWARD);SCREEN(RELIC_RESULT);fprintf(f,"\"DEFEAT\":%d}}\n",DEFEAT);fclose(f);return 0;
}
