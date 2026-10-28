#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../source/game.c"
#include "../source/render.c"
static uint32_t pixels[W*H];
static void output(Game*g,const char*name){
 game_render_reset_fx();game_render_frame(g,pixels,W,300);assert(rift_asset_errors==0);
 if(!name)return;char p[320];snprintf(p,sizeof(p),"qa/rebuild180/evidence/native-%s.ppm",name);FILE*f=fopen(p,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){uint32_t v=pixels[i];unsigned char b[3]={v,v>>8,v>>16};fwrite(b,3,1,f);}fclose(f);
}
int main(void){
 rift_assets_set_file("qa/rebuild180/evidence/EBOOT.PBP");assert(rift_assets_open());char root[]="/tmp/rift-render-XXXXXX";assert(mkdtemp(root));assert(ascent_open(root));Game g;char name[80];
 game_new(&g,1);g.selection=1;output(&g,"title");
 for(int c=0;c<3;c++){assert(ascent_begin(&g,c,c,100+c));g.screen=CHARACTER_SELECT;g.selection=c;output(&g,c==0?"characters":0);}
 g.screen=ASCENT_SLOTS_VIEW;g.selection=1;g.scroll=0;output(&g,"slots");g.screen=ASCENT_OPTIONS;output(&g,"options");
 for(int c=0;c<3;c++)for(int k=0;k<4;k++)for(int page=0;page<3;page++){
 g.character=c;g.screen=(int[]){ASCENT_ENDING,ASCENT_REVELATION,ASCENT_BOSS,ASCENT_TRUE_END}[k];g.selection=page;
 snprintf(name,sizeof(name),"story-%d-%d-%d",c,k,page);output(&g,name);
 }
 for(int c=0;c<3;c++)ascent_account.cards[c]=ASCENT_ALL_CARDS;
 ascent_account.potions=0xfffff;ascent_account.relics=1023;ascent_account.ascended=7;
 for(int tab=0;tab<5;tab++)for(int n=0;n<(tab==4?10:20);n++){g.screen=ASCENT_COLLECTION;g.scroll=tab;g.selection=n;snprintf(name,sizeof(name),"collection-%d-%d",tab,n);output(&g,n%6==0?name:0);}
 for(int id=334;id<394;id++)for(int up=0;up<2;up++){
  game_new(&g,id);game_choose_character(&g,(id-334)/20);g.ascent.cards=ASCENT_ALL_CARDS;game_start_battle(&g,NODE_FIGHT);g.hand[0]=id|(up?UPBIT:0);g.selection=0;g.energy=10;g.regent.stars=10;
  snprintf(name,sizeof(name),"card-%d-%d",id,up);output(&g,id%6==0&&!up?name:0);
 }
 for(int e=0;e<ENEMY_COUNT;e++){g.enemies[0].art=e;g.enemy_count=1;snprintf(name,sizeof(name),"enemy-%02d",e);output(&g,name);}
 for(int hp=80;hp<=1000000;hp*=10){g.hp=g.maxhp=hp;g.enemies[0].hp=g.enemies[0].maxhp=hp;output(&g,hp==800?"large-health":0);}
 for(int c=0;c<3;c++){g.character=c;g.ascent.unlocked_cards[0]=334+20*c;g.ascent.unlocked_cards[1]=352+20*c;g.screen=ASCENT_CARDS;snprintf(name,sizeof(name),"unlocked-cards-%d",c);output(&g,name);}
 g.ascent.unlocked_potions[0]=6;g.ascent.unlocked_potions[1]=8;g.screen=ASCENT_POTIONS;output(&g,"unlocked-potions");g.ascent.unlocked_relic=64;g.screen=ASCENT_RELIC;output(&g,"unlocked-relic");
 for(int id=0;id<RELIC_COUNT;id++){g.screen=RELIC_RESULT;g.reward[0]=id;g.relic_pending=1;g.selection=0;snprintf(name,sizeof(name),"relic-%d",id);output(&g,id<40?name:0);}
 for(int id=0;id<26;id+=3){g.screen=POTION_VIEW;g.prior_screen=MAP;g.selection=0;g.potion_count=MIN(3,26-id);for(int i=0;i<g.potion_count;i++)g.potions[i]=id+i;g.message[0]=0;snprintf(name,sizeof(name),"potions-%d",id);output(&g,name);}
 puts("PASS native renderer: all new cards, collections, 36 dialogue pages, 45 enemy sprites, large HP; no streaming errors");return 0;
}
