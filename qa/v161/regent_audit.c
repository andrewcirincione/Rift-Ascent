#define main smoke_main
#include "smoke.c"
#undef main
static int checks;
#define CHECK(x) do{checks++;if(!(x)){fprintf(stderr,"FAIL line%d: %s\n",__LINE__,#x);abort();}}while(0)
#include "../v160/reference/expected.inc"
static void play(Game*g,int raw){g->screen=COMBAT;g->selection=0;g->hand_count=1;g->hand[0]=regent_stamp(g,raw);game_play_card(g,0,0);choices(g);}
static int count_id(const Game*g,int id){int n=0;const int*p[]={g->hand,g->draw,g->discard,g->exhaust};int c[]={g->hand_count,g->draw_count,g->discard_count,g->exhaust_count};for(int a=0;a<4;a++)for(int i=0;i<c[a];i++)n+=CARD_ID(p[a][i])==id;return n;}
static void math(void){Game g;int cases=0;for(int id=REGENT_FIRST;id<EXTRA_LAST;id++)for(int up=0;up<2;up++){
 int raw=id|(up?UPBIT:0);CHECK(game_card_cost(raw)==expected[id-REGENT_FIRST][up][0]);CHECK(game_card_star_cost(0,raw)==expected[id-REGENT_FIRST][up][1]);
 if(cards[id].type!=0)continue;
 for(int s=-3;s<=21;s+=6)for(int w=0;w<2;w++)for(int v=0;v<2;v++){
 setup(&g,raw);g.energy=4;g.regent.stars=9;g.strength=s;g.weak=w;g.enemies[0].vulnerable=v;g.enemies[1].vulnerable=v;
 int base=expected[id-REGENT_FIRST][up][2];
 if(id==R_SOVEREIGN_BLADE)base=10;
 if(id==R_LUNAR_BLAST||id==C_GOLD_AXE)base=0;
 if(id==R_RADIATE)base=(3+up)*3;
 if(id==R_SUPERMASSIVE)base=5;
 if(id==R_CRESCENT_SPEAR)base=10+up;
 if(id==C_MIND_BLAST)base=4;
 if(id==C_REND)base+=v*(5+3*up);
 int want=MAX(0,base+s);if(w)want=want*3/4;if(v)want=want*3/2;
 CardPreview p;game_card_preview(&g,raw,0,&p);Game before=g;char desc[512];game_card_description_now(&g,raw,0,desc,sizeof(desc));CHECK(!memcmp(&before,&g,sizeof(g)));
 game_combat_fx_begin();game_play_card(&g,0,0);choices(&g);game_combat_fx_end();
 int found=0;for(int i=0;i<combat_fx_count;i++)if(combat_fx[i].kind==GFX_ATTACK){if(combat_fx[i].amount!=want){fprintf(stderr,"math %s up%d str%d weak%d vul%d actual%d expected%d\n",cards[id].name,up,s,w,v,combat_fx[i].amount,want);abort();}found=1;break;}CHECK(found);CHECK(game_validate(&g));cases++;
 }
 }
 printf("PASS %d attack math cases; independent wiki damage/cost fixtures; pure previews\n",cases);
}
static void resources(void){Game g;setup(&g,R_COMET);g.energy=0;g.regent.stars=4;int hp=g.enemies[0].hp;game_play_card(&g,0,0);CHECK(g.hand_count==4&&g.regent.stars==4&&g.enemies[0].hp==hp);g.regent.stars=5;game_play_card(&g,0,0);CHECK(g.regent.stars==0&&g.enemies[0].hp==hp-33);
 setup(&g,R_FALLING_STAR|(MOD_ECHO<<CARD_MOD_SHIFT));g.regent.stars=2;game_play_card(&g,0,0);CHECK(g.regent.stars==0);CHECK(g.enemies[0].hp==50000-8-12);CHECK(g.turn_played==2);
 setup(&g,R_CRESCENT_SPEAR|(MOD_ECHO<<CARD_MOD_SHIFT));game_play_card(&g,0,0);CHECK(g.enemies[0].hp==49980);
 setup(&g,R_STARDUST|(MOD_ECHO<<CARD_MOD_SHIFT));g.regent.stars=4;game_play_card(&g,0,0);CHECK(g.regent.stars==0);CHECK(100000-g.enemies[0].hp-g.enemies[1].hp==40);CHECK(count_id(&g,R_STARDUST)==1);
 setup(&g,R_STARDUST);g.regent.stars=4;g.regent.powers[R_VOID_FORM-REGENT_FIRST]=1;game_play_card(&g,0,0);CHECK(g.regent.stars==4);CHECK(100000-g.enemies[0].hp-g.enemies[1].hp==20);CHECK(g.regent.void_used==1);
 setup(&g,R_HEAVENLY_DRILL);g.energy=4;game_play_card(&g,0,0);CHECK(g.energy==0&&50000-g.enemies[0].hp==64);
 setup(&g,R_COMET);g.regent.powers[R_VOID_FORM-REGENT_FIRST]=2;g.energy=g.regent.stars=0;for(int i=0;i<4;i++){play(&g,R_COMET);CHECK(g.regent.void_used==i+1);}play(&g,R_COMET);CHECK(g.hand_count==1&&g.regent.void_used==4);
 setup(&g,0);g.regent.powers[R_VOID_FORM-REGENT_FIRST]=1;g.energy=0;play(&g,0);play(&g,N_STRIKE_SILENT);CHECK(g.regent.void_used==2);CHECK(g.turn_played==2);play(&g,R_STRIKE_REGENT);CHECK(g.hand_count==1);
 setup(&g,R_COMET|CARD_FREE);g.regent.powers[R_VOID_FORM-REGENT_FIRST]=1;g.energy=g.regent.stars=0;game_play_card(&g,0,0);CHECK(g.regent.void_used==1);
 setup(&g,R_COMET);g.regent.powers[R_BLACK_HOLE-REGENT_FIRST]=3;g.regent.powers[R_CHILD_OF_THE_STARS-REGENT_FIRST]=2;g.regent.stars=10;g.dexterity=8;game_play_card(&g,0,0);CHECK(g.block==10&&g.regent.stars==5);CHECK(g.enemies[1].hp==49997);regent_gain_stars(&g,9);CHECK(g.enemies[1].hp==49994);
 setup(&g,R_MONOLOGUE);play(&g,R_MONOLOGUE);CHECK(g.strength==0);play(&g,R_MONOLOGUE);CHECK(g.strength==1);play(&g,N_STRIKE_SILENT);CHECK(g.strength==3);play(&g,0);CHECK(g.strength==5);CHECK(g.regent.plays_combat==4);CHECK(g.ic[IC_TEMP_STRENGTH]==5);
 puts("PASS Star payments, X, Void Form stacks, free/manual/replay accounting, mixed-character triggers");
}
static void forge(void){Game g;setup(&g,R_BULWARK);play(&g,R_BULWARK);CHECK(g.regent.forge==10&&count_id(&g,R_SOVEREIGN_BLADE)==1&&g.block==12);regent_forge(&g,7);CHECK(g.regent.forge==17&&count_id(&g,R_SOVEREIGN_BLADE)==1);g.regent.powers[R_SWORD_SAGE-REGENT_FIRST]=2;g.regent.powers[R_PARRY-REGENT_FIRST]=10;g.regent.powers[R_SEEKING_EDGE-REGENT_FIRST]=1;g.regent.conqueror[0]=1;g.dexterity=2;g.regent.vigor=3;play(&g,R_SOVEREIGN_BLADE);CHECK(g.enemies[0].hp==50000-60-54-54);CHECK(g.enemies[1].hp==50000-30-27-27);CHECK(g.block==48);
 setup(&g,R_BEAT_INTO_SHAPE);g.regent.hit_count[0]=3;play(&g,R_BEAT_INTO_SHAPE);CHECK(g.regent.forge==20);
 setup(&g,R_SUMMON_FORTH);regent_forge(&g,1);int blade=0;for(int i=0;i<g.hand_count;i++)if(CARD_ID(g.hand[i])==R_SOVEREIGN_BLADE)blade=g.hand[i];g.exhaust_count=1;g.exhaust[0]=blade;g.hand_count=0;play(&g,R_SUMMON_FORTH);CHECK(count_id(&g,R_SOVEREIGN_BLADE)==1&&g.regent.forge==9);
 puts("PASS single Blade, Forge persistence, Vigor, Conqueror, Seeking Edge, Parry and Sword Sage stacking");}
static void selection_save(void){Game g,c;setup(&g,R_GLIMMER);game_play_card(&g,0,0);CHECK(g.screen==REGENT_CHOICE);int hand=g.hand_count;game_action(&g,BTN_BACK);CHECK(g.screen==REGENT_CHOICE&&g.hand_count==hand);CHECK(game_save(&g,"qa/v161/evidence/choice-roundtrip.dat"));CHECK(game_load(&c,"qa/v161/evidence/choice-roundtrip.dat"));CHECK(!memcmp(&g,&c,sizeof(g)));game_action(&g,BTN_PAUSE);CHECK(g.screen==PAUSE_MENU);game_action(&g,BTN_BACK);CHECK(g.screen==REGENT_CHOICE);regent_choose(&g,0,0);regent_choose(&c,0,0);CHECK(g.hand_count==hand-1&&g.draw[g.draw_count-1]==c.draw[c.draw_count-1]);
 setup(&g,R_GUARDS);game_play_card(&g,0,0);CHECK(g.screen==REGENT_CHOICE);regent_choose(&g,0,0);CHECK(CARD_ID(g.hand[0])==R_MINION_SACRIFICE);game_action(&g,BTN_BACK);CHECK(g.screen==COMBAT);
 setup(&g,R_CHARGE);game_play_card(&g,0,0);regent_choose(&g,0,0);CHECK(g.screen==REGENT_CHOICE&&!game_regent_choice_eligible(&g,0));regent_choose(&g,1,0);CHECK(g.screen==COMBAT);CHECK(count_id(&g,R_MINION_DIVE_BOMB)==2);
 setup(&g,C_SEEKER_STRIKE);game_play_card(&g,0,0);CHECK(g.screen==REGENT_CHOICE&&game_regent_choice_count(&g)==3);choices(&g);CHECK(g.hand_count==4);
 setup(&g,R_PROPHESIZE);g.draw_count=0;g.discard_count=8;for(int i=0;i<8;i++)g.discard[i]=regent_stamp(&g,R_DEFEND_REGENT);g.regent.powers[C_STRATAGEM-REGENT_FIRST]=1;game_play_card(&g,0,0);CHECK(g.screen==REGENT_CHOICE&&g.draw_count==8&&g.hand_count==3);choices(&g);CHECK(g.hand_count==10&&g.draw_count==1);CHECK(g.regent.task_count==0);
 for(int k=0;k<20;k++){setup(&g,C_SPLASH);game_play_card(&g,0,0);CHECK(g.screen==REGENT_CHOICE);int raw=game_regent_choice_raw(&g,0);CHECK(CARD_ID(raw)<NINJA_LAST&&cards[CARD_ID(raw)].type==0);choices(&g);g.regent.monologue=2;g.regent.powers[R_THE_SEALED_THRONE-REGENT_FIRST]=1;int stars=g.regent.stars;play(&g,raw);CHECK(g.regent.stars>=stars+1&&g.strength>=2);CHECK(game_validate(&g));}
 puts("PASS all mandatory/optional selections, pause/save/resume, shuffle ordering, mixed-card choices");}
static void powers_misc(void){Game g;
 setup(&g,R_GLITTERSTREAM);g.dexterity=4;play(&g,R_GLITTERSTREAM);CHECK(g.block==15&&g.regent.next_block==9);g.dexterity=0;g.block=0;regent_start_turn(&g);CHECK(g.block==9&&g.regent.next_block==0);
 setup(&g,C_FISTICUFFS);g.enemies[0].block=99;g.dexterity=3;play(&g,C_FISTICUFFS);CHECK(g.block==10&&g.enemies[0].hp==50000);
 setup(&g,C_OMNISLICE);g.enemies[0].block=3;g.enemies[1].vulnerable=1;play(&g,C_OMNISLICE);CHECK(g.enemies[1].hp==49995);
 setup(&g,R_PILLAR_OF_CREATION);g.regent.powers[R_PILLAR_OF_CREATION-REGENT_FIRST]=3;g.regent.powers[R_ARSENAL-REGENT_FIRST]=1;g.hand_count=MAX_HAND;g.discard_count=MAX_PILE;regent_create(&g,R_DEBRIS);CHECK(g.regent.created==0&&g.strength==0&&g.block==0);g.discard_count--;regent_create(&g,R_DEBRIS);CHECK(g.regent.created==1&&g.strength==1&&g.block==3);
 setup(&g,R_BOMBARDMENT);play(&g,R_BOMBARDMENT);CHECK(count_id(&g,R_BOMBARDMENT)==1&&g.exhaust_count==1);int hp=g.enemies[0].hp;regent_start_turn(&g);CHECK(g.enemies[0].hp==hp-18&&g.exhaust_count==1);
 setup(&g,R_PARTICLE_WALL);g.regent.stars=6;play(&g,R_PARTICLE_WALL);CHECK(g.hand_count==1&&g.block==9);game_play_card(&g,0,0);CHECK(g.hand_count==1&&g.block==18&&g.regent.stars==2);
 setup(&g,R_DYING_STAR);g.hand_count=1;game_end_turn(&g);choices(&g);CHECK(count_id(&g,R_DYING_STAR)==1&&g.exhaust_count==1);
 setup(&g,R_KINGLY_KICK);int k=CARD_INSTANCE(g.hand[0]);regent_on_draw(&g,g.hand[0]);CHECK(game_card_cost_now(&g,g.hand[0])==3&&g.regent.instance_cost[k]==1);
 setup(&g,R_KINGLY_PUNCH);regent_on_draw(&g,g.hand[0]);game_play_card(&g,0,0);CHECK(g.enemies[0].hp==49988);
 setup(&g,C_HIDDEN_GEM);g.draw_count=1;g.draw[0]=regent_stamp(&g,N_STRIKE_SILENT);play(&g,C_HIDDEN_GEM);CHECK(g.regent.instance_replay[CARD_INSTANCE(g.draw[0])]==2);int raw=g.draw[0];g.draw_count=0;play(&g,raw);CHECK(g.enemies[0].hp==49982&&g.regent.plays_combat==4);
 setup(&g,R_COMET);g.regent.option_count=MAX_PILE+1;CHECK(!game_validate(&g));setup(&g,R_COMET);g.regent.task_count=REGENT_TASK_MAX+1;CHECK(!game_validate(&g));
 setup(&g,C_AUTOMATION);g.energy=30;g.regent.draws=7;play(&g,C_AUTOMATION);int en=g.energy;for(int i=0;i<4;i++)regent_on_draw(&g,R_DEFEND_REGENT);play(&g,C_AUTOMATION);en=g.energy;for(int i=0;i<5;i++)regent_on_draw(&g,R_DEFEND_REGENT);CHECK(g.energy==en);regent_on_draw(&g,R_DEFEND_REGENT);CHECK(g.energy==en+1);for(int i=0;i<4;i++)regent_on_draw(&g,R_DEFEND_REGENT);CHECK(g.energy==en+2);
 setup(&g,C_PANACHE);g.turn_played=3;play(&g,C_PANACHE);hp=g.enemies[0].hp;for(int i=0;i<4;i++)play(&g,R_KNOW_THY_PLACE);CHECK(g.enemies[0].hp==hp);play(&g,C_PANACHE|UPBIT);CHECK(g.enemies[0].hp==hp-10);for(int i=0;i<4;i++)play(&g,R_KNOW_THY_PLACE);CHECK(g.enemies[0].hp==hp-10);play(&g,R_KNOW_THY_PLACE);CHECK(g.enemies[0].hp==hp-34);
 setup(&g,C_SEEKER_STRIKE);g.regent.powers[C_STRATAGEM-REGENT_FIRST]=1;game_play_card(&g,0,0);CHECK(g.regent.choice_kind==RC_STRATAGEM);regent_choose(&g,0,0);CHECK(g.screen==REGENT_CHOICE&&g.regent.choice_kind==RC_SEEK_THREE);regent_choose(&g,0,0);CHECK(g.screen==COMBAT&&g.hand_count==5);
 setup(&g,C_JACKPOT);g.hand_count=1;game_play_card(&g,0,0);CHECK(g.hand_count==3);for(int i=0;i<3;i++)CHECK(CARD_ID(g.hand[i])>=REGENT_FIRST&&CARD_ID(g.hand[i])<REGENT_LAST&&game_card_cost(g.hand[i])==0);
 puts("PASS delayed Block/Dexterity, full-hit Block, unpowered splash, creation capacity, exhaust/autoplay, persistent draw scaling and foreign Replay");}
int main(void){math();resources();forge();selection_save();powers_misc();printf("PASS Regent audit: %d assertions\n",checks);return 0;}
