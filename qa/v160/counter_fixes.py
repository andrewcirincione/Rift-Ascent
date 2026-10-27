from pathlib import Path
q=Path(__file__).parent/'source'
def edit(n,a,b):
 p=q/n;s=p.read_text();assert a in s,(n,a[:80]);p.write_text(s.replace(a,b))
# Append new state: retain existing field offsets for fixtures and current audit scripts.
edit('game.h','int task_count,resolving;RegentTask tasks[REGENT_TASK_MAX];','int task_count,resolving;RegentTask tasks[REGENT_TASK_MAX];\n int automation_phase[10],panache_phase[5];')
edit('regent_engine.inc','RT_AUTOPLAY,RT_RETURN};','RT_AUTOPLAY,RT_RETURN,RT_SEEK_THREE};')
edit('regent_engine.inc','if(++g->regent.draws%10==0&&RP(g,C_AUTOMATION))gain_energy(g,RP(g,C_AUTOMATION));', 'g->regent.draws=(g->regent.draws+1)%10;gain_energy(g,g->regent.automation_phase[g->regent.draws]);')
edit('regent_engine.inc','if(!ninja_has_living(g))return;\n g->regent.choice_kind=', 'if(!ninja_has_living(g))return;\n if(g->hand_count>=MAX_HAND&&(t.n==RC_SEEK||t.n==RC_SEEK_SKILL||t.n==RC_SEEK_ATTACK||t.n==RC_STRATAGEM||t.n==RC_SEEK_THREE))return;\n g->regent.choice_kind=')
edit('regent_engine.inc','''shuffle(g,g->draw,g->draw_count);regent_after_shuffle(g);g->regent.option_count=MIN(3,g->draw_count);
 for(int i=0;i<g->regent.option_count;i++)g->regent.options[i]=g->draw[g->draw_count-1-i];
 regent_push(g,RT_CHOOSE,raw,e,RC_SEEK_THREE,1,depth);''', '''shuffle(g,g->draw,g->draw_count);regent_after_shuffle(g);
 regent_push(g,RT_SEEK_THREE,raw,e,0,0,depth);''')
edit('regent_engine.inc','case RT_RETURN:', '''case RT_SEEK_THREE:
   g->regent.option_count=MIN(3,g->draw_count);for(int i=0;i<g->regent.option_count;i++)g->regent.options[i]=g->draw[g->draw_count-1-i];regent_push(g,RT_CHOOSE,t.raw,t.target,RC_SEEK_THREE,1,t.depth);break;
  case RT_RETURN:''')
edit('regent_engine.inc','if(RP(g,C_PANACHE)&&g->turn_played%5==0)ninja_all_proc(g,RP(g,C_PANACHE));', 'if(g->regent.panache_phase[g->turn_played%5])ninja_all_proc(g,g->regent.panache_phase[g->turn_played%5]);')
edit('regent_engine.inc','g->regent.attack_vigor=0;g->selection=', 'if(id==C_PANACHE)g->regent.panache_phase[g->turn_played%5]+=10+4*CARD_UP(raw);\n g->regent.attack_vigor=0;g->selection=')
edit('regent_engine.inc','memset(g->regent.hit_count,0,sizeof(g->regent.hit_count));', 'memset(g->regent.panache_phase,0,sizeof(g->regent.panache_phase));g->regent.panache_phase[0]=RP(g,C_PANACHE);\n memset(g->regent.hit_count,0,sizeof(g->regent.hit_count));')
edit('regent_engine.inc','case RC_ENTROPY:p[index]=regent_stamp(g,regent_colorless(g,0));regent_created(g);break;', '''case RC_ENTROPY:{int pool[128],count=0;for(int c=REGENT_FIRST;c<REGENT_LAST;c++)if(c!=CARD_ID(raw)&&c!=R_ROYALTIES&&solo_reward_eligible(c))pool[count++]=c;if(count){p[index]=regent_stamp(g,pool[rnd(g,count)]);regent_created(g);}break;}''')
edit('regent_effects.inc','case C_AUTOMATION:case C_CALAMITY:', 'case C_AUTOMATION:POWER(id,1);g->regent.automation_phase[g->regent.draws%10]++;break;\n case C_CALAMITY:')
edit('regent_effects.inc','for(int c=0;c<CARD_COUNT;c++)if(solo_reward_eligible(c)&&game_card_cost(c)==0)', 'for(int c=REGENT_FIRST;c<REGENT_LAST;c++)if(solo_reward_eligible(c)&&c!=R_ROYALTIES&&game_card_cost(c)==0)')
edit('game.c','t->kind>RT_RETURN','t->kind>RT_SEEK_THREE')
# Merge Bomb copies with identical countdowns; no arbitrary sixteen-copy limit.
edit('regent_effects.inc','case C_THE_BOMB:for(int i=0;i<16;i++)if(!g->regent.bomb_ticks[i]){g->regent.bomb_ticks[i]=3;g->regent.bomb_damage[i]=40+10*up;break;}break;', 'case C_THE_BOMB:{int slot=-1;for(int i=0;i<16;i++)if(g->regent.bomb_ticks[i]==3){slot=i;break;}if(slot<0)for(int i=0;i<16;i++)if(!g->regent.bomb_ticks[i]){slot=i;g->regent.bomb_damage[i]=0;break;}if(slot>=0){g->regent.bomb_ticks[slot]=3;g->regent.bomb_damage[slot]+=40+10*up;}break;}')
