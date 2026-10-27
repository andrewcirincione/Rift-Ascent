from pathlib import Path
q=Path(__file__).parent/'source'
def edit(name,old,new):
 p=q/name;s=p.read_text();assert old in s,(name,old[:80]);p.write_text(s.replace(old,new))
edit('game.h','int stars,forge,blade_created,created,stars_gained,skills,plays_combat;', 'int stars,forge,blade_created,created,stars_gained,skills,plays_combat;\n int shuffle_pending,shuffle_draws;')
edit('game.c','static int regent_processing;','static int regent_processing;\nstatic void regent_foreign_begin(Game*g,int raw);\nstatic void regent_foreign_finish(Game*g,int raw);')
edit('game.c','if(g->hand_count>=MAX_HAND)return;\n if(!g->draw_count)', 'if(g->hand_count>=MAX_HAND)return;\n if(g->regent.shuffle_pending){g->regent.shuffle_draws=MIN(MAX_PILE,g->regent.shuffle_draws+1);return;}\n if(!g->draw_count)')
edit('game.c','g->discard_count=0;shuffle(g,g->draw,g->draw_count);}', 'g->discard_count=0;shuffle(g,g->draw,g->draw_count);regent_after_shuffle(g);if(g->regent.shuffle_pending){g->regent.shuffle_draws++;return;}}')
edit('regent_engine.inc','static void regent_after_shuffle(Game*g){if(g->character==CHAR_REGENT&&RP(g,C_STRATAGEM))regent_push(g,RT_CHOOSE,C_STRATAGEM,g->target,RC_STRATAGEM,RP(g,C_STRATAGEM),0);}', 'static void regent_after_shuffle(Game*g){if(g->character==CHAR_REGENT&&RP(g,C_STRATAGEM))g->regent.shuffle_pending=1;}')
edit('regent_engine.inc','if(regent_processing)return;regent_processing=1;', 'if(regent_processing||g->ninja.task_count)return;regent_processing=1;')
edit('regent_engine.inc','for(int steps=0;steps<4096&&g->screen==COMBAT&&g->regent.task_count;steps++){', '''for(int steps=0;steps<4096&&g->screen==COMBAT&&!g->ninja.task_count&&(g->regent.task_count||g->regent.shuffle_pending);steps++){
  if(g->regent.shuffle_pending){int n=g->regent.shuffle_draws;g->regent.shuffle_pending=g->regent.shuffle_draws=0;regent_push(g,RT_DRAW,C_STRATAGEM,g->target,n,0,0);regent_push(g,RT_CHOOSE,C_STRATAGEM,g->target,RC_STRATAGEM,RP(g,C_STRATAGEM),0);}''')
edit('regent_engine.inc','case RT_DRAW:for(int i=0;i<t.n&&g->hand_count<MAX_HAND;i++)draw_card(g);break;', 'case RT_DRAW:for(int i=0;i<t.n&&g->hand_count<MAX_HAND;i++)draw_card(g);break;')
# One slot is necessarily vacant after popping ENDTURN: retained queue can be at most MAX-1.
edit('regent_engine.inc','game_card_star_cost(g,raw)==-1?g->regent.stars:0,0)', '(CARD_ID(raw)==R_STARDUST)?g->regent.stars:0,0)')
edit('regent_engine.inc','if(g->screen!=COMBAT||index<0||index>=g->hand_count||g->regent.task_count)return;', 'if(g->screen!=COMBAT||index<0||index>=g->hand_count||(g->regent.task_count&&!(g->hand[index]&CARD_FREE)))return;')
edit('regent_engine.inc','int x=game_card_cost(raw)==-1?g->energy:0,sx=(CARD_UP(raw)?regent_cards[CARD_ID(raw)-REGENT_FIRST].upstars:regent_cards[CARD_ID(raw)-REGENT_FIRST].stars)==-1?g->regent.stars:0;', '''int id=CARD_ID(raw);if(!game_card_is_playable(raw)||(id>=NINJA_FIRST&&id<NINJA_LAST&&!ninja_can_play(g,raw)))return;
 int x=game_card_cost(raw)==-1?g->energy:0,sx=id==R_STARDUST?g->regent.stars:0;''')
edit('regent_engine.inc','g->regent.void_used++;g->energy-=cost;', 'if(!(raw&CARD_FREE))g->regent.void_used++;g->energy-=cost;')
edit('game.c','if(index<0)return;if(index<g->hand_count&&CARD_ID(g->hand[index])>=REGENT_FIRST)', 'if(index<0)return;if(index<g->hand_count&&(g->character==CHAR_REGENT||CARD_ID(g->hand[index])>=REGENT_FIRST))')
edit('game.c','void game_update(Game*g){if(g->screen==COMBAT&&g->regent.task_count)regent_process(g);if(g->screen==COMBAT&&g->ninja.task_count)ninja_process(g);', 'void game_update(Game*g){if(g->screen==COMBAT&&g->ninja.task_count)ninja_process(g);if(g->screen==COMBAT&&(g->regent.task_count||g->regent.shuffle_pending))regent_process(g);')
# Native foreign-card hooks preserve their own choices/discards/relic counters.
edit('game.c','int original_raw=raw;','int original_raw=raw;regent_foreign_begin(g,raw);')
edit('game.c','int repeat_this_attack=(!(raw&CARD_EPHEMERAL)&&CARD_MOD(raw)==MOD_ECHO)?1:0;', 'int repeat_this_attack=(!(raw&CARD_EPHEMERAL)&&CARD_MOD(raw)==MOD_ECHO)?1:0;\n if(g->character==CHAR_REGENT&&!(raw&CARD_EPHEMERAL))repeat_this_attack+=g->regent.instance_replay[CARD_INSTANCE(raw)];')
edit('game.c','r151_played(g,cards[id].type);\n for(int repeat=', 'r151_played(g,cards[id].type);regent_foreign_finish(g,raw);\n for(int repeat=')
edit('ninja_engine.inc','r140_rust_targets=0;\n   int repeat=0;', 'r140_rust_targets=0;regent_foreign_begin(g,t.raw);\n   int repeat=0;')
edit('ninja_engine.inc','if(!(t.raw&CARD_EPHEMERAL)){\n    if(cards[id].type==0)', 'if(!(t.raw&CARD_EPHEMERAL)){\n    if(g->character==CHAR_REGENT)repeat+=g->regent.instance_replay[CARD_INSTANCE(t.raw)];\n    if(cards[id].type==0)')
edit('ninja_engine.inc','repeat=CARD_MOD(t.raw)==MOD_ECHO;', 'repeat+=CARD_MOD(t.raw)==MOD_ECHO;')
edit('ninja_engine.inc','r151_played(g,type);\n /* Only powers', 'r151_played(g,type);regent_foreign_finish(g,raw);\n /* Only powers')
p=q/'regent_engine.inc';s=p.read_text();ix=s.index('static void regent_process(Game*g){')
s=s[:ix]+'''static void regent_foreign_begin(Game*g,int raw){
 if(g->character!=CHAR_REGENT)return;
 if(cards[CARD_ID(raw)].type==0){g->regent.attack_vigor=g->regent.vigor;g->regent.vigor=0;}
}
static void regent_foreign_finish(Game*g,int raw){
 if(g->character!=CHAR_REGENT)return;int type=cards[CARD_ID(raw)].type;
 g->regent.plays_combat=MIN(1000000,g->regent.plays_combat+1);
 if(type==1){g->regent.skills++;if(g->regent.skills%3==0){int*p[]={g->draw,g->discard,g->exhaust};int*n[]={&g->draw_count,&g->discard_count,&g->exhaust_count};for(int z=0;z<3;z++)for(int i=*n[z]-1;i>=0&&g->hand_count<MAX_HAND;i--)if(CARD_ID(p[z][i])==R_MAKE_IT_SO)regent_to_hand(g,regent_take(p[z],n[z],i));}}
 if(type==0&&RP(g,C_CALAMITY)){int pool[256],n=0;for(int c=REGENT_FIRST;c<REGENT_LAST;c++)if(solo_reward_eligible(c)&&cards[c].type==0)pool[n++]=c;for(int i=0;i<MIN(MAX_HAND,RP(g,C_CALAMITY))&&n;i++)regent_create(g,pool[rnd(g,n)]);}
 if(!(raw&CARD_EPHEMERAL)&&RP(g,C_NOSTALGIA)&&!g->regent.nostalgia_used&&(type==0||type==1)&&g->draw_count<MAX_PILE){for(int i=g->discard_count-1;i>=0;i--)if(CARD_INSTANCE(g->discard[i])==CARD_INSTANCE(raw)){g->draw[g->draw_count++]=regent_take(g->discard,&g->discard_count,i);g->regent.nostalgia_used=1;break;}}
 if(g->regent.monologue){g->strength+=g->regent.monologue;g->ic[IC_TEMP_STRENGTH]+=g->regent.monologue;}
 regent_gain_stars(g,RP(g,R_THE_SEALED_THRONE));
 if(RP(g,C_PANACHE)&&g->turn_played%5==0)ninja_all_proc(g,RP(g,C_PANACHE));
 g->regent.attack_vigor=0;
}
''' +s[ix:];p.write_text(s)
# Freeze costs for paid cards, guard PSP anticipation on BOTH resources.
edit('psp_start.c','game_card_cost_now(&game,raw)==-1)) fx=', 'game_card_cost_now(&game,raw)==-1)&&game_card_star_cost(&game,raw)<=game.regent.stars) fx=')
p=q/'psp_start.c';s=p.read_text();print(s[s.index('int cost=game_card_cost_now(&game,raw);'):s.index('int cost=game_card_cost_now(&game,raw);')+350]);p.write_text(s)
# Save bounds must be checked before choice-count access.
p=q/'game.c';s=p.read_text();key=' if(g->screen==REGENT_CHOICE&&';idx=s.index(key,s.index('int game_validate'))
s=s[:idx]+''' if(g->regent.option_count<0||g->regent.option_count>MAX_PILE||g->regent.return_count<0||g->regent.return_count>MAX_PILE||g->regent.task_count<0||g->regent.task_count>REGENT_TASK_MAX||g->regent.shuffle_pending<0||g->regent.shuffle_pending>1||g->regent.shuffle_draws<0||g->regent.shuffle_draws>MAX_PILE)return 0;
 for(int i=0;i<g->regent.task_count;i++){const RegentTask*t=&g->regent.tasks[i];if(t->kind<RT_PLAY||t->kind>RT_RETURN||!valid_card_raw(t->raw)||t->target< -1||t->target>=MAX_ENEMIES||t->n<0||t->n>1000000||t->aux<0||t->aux>1000000||t->depth<0||t->depth>3)return 0;}
 for(int i=0;i<g->regent.option_count;i++)if(!valid_card_raw(g->regent.options[i]))return 0;
 for(int i=0;i<g->regent.return_count;i++)if(!valid_card_raw(g->regent.return_cards[i]))return 0;
 for(int i=0;i<512;i++)if(g->regent.instance_cost[i]<0||g->regent.instance_cost[i]>99||g->regent.instance_damage[i]<0||g->regent.instance_damage[i]>1000000||g->regent.instance_replay[i]<0||g->regent.instance_replay[i]>20)return 0;
 for(int i=0;i<EXTRA_LAST-REGENT_FIRST;i++)if(g->regent.powers[i]<0||g->regent.powers[i]>1000000||g->regent.played[i]<0||g->regent.played[i]>1000000)return 0;
''' +s[idx:];p.write_text(s)
