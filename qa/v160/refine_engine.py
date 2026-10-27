from pathlib import Path
q=Path(__file__).parent/'source'
p=q/'regent_engine.inc';s=p.read_text()
s=s.replace('g->regent.void_used<2','g->regent.void_used<2*RP(g,R_VOID_FORM)')
s=s.replace('g->regent.void_used++;','')
s=s.replace('regent_take(g->hand,&g->hand_count,index);g->energy-=cost;', 'regent_take(g->hand,&g->hand_count,index);g->regent.void_used++;g->energy-=cost;')
s=s.replace('if(CARD_ID(raw)<REGENT_FIRST||CARD_INSTANCE(raw))return raw;', 'if((g->character!=CHAR_REGENT&&CARD_ID(raw)<REGENT_FIRST)||CARD_INSTANCE(raw))return raw;')
s=s.replace('g->regent.choice_kind=t.n;', 'if(!ninja_has_living(g))return;\n g->regent.choice_kind=t.n;',1)
s=s.replace('RC_SEEK_ATTACK,RC_STRATAGEM','RC_SEEK_ATTACK,RC_STRATAGEM,RC_SEEK_THREE')
s=s.replace('case RC_GENERATE:return 3;', 'case RC_GENERATE:case RC_SEEK_THREE:return 3;')
s=s.replace('case RC_GENERATE:return "CHOOSE A CREATED CARD";', 'case RC_GENERATE:return "CHOOSE A CREATED CARD";case RC_SEEK_THREE:return "SEEKER STRIKE / CHOOSE A DRAW CARD";')
s=s.replace('case RC_GENERATE:regent_create(g,raw);break;', 'case RC_GENERATE:regent_create(g,raw);break;\n  case RC_SEEK_THREE:for(int j=0;j<g->draw_count;j++)if(g->draw[j]==raw){regent_to_hand(g,regent_take(g->draw,&g->draw_count,j));break;}break;')
s=s.replace('default:regent_take(p,n,index);regent_to_hand(g,raw);break;', 'default:if(g->hand_count<MAX_HAND){regent_take(p,n,index);regent_to_hand(g,raw);}break;')
s=s.replace('(kind!=RC_SEEK||g->hand_count<MAX_HAND)', '((kind!=RC_SEEK&&kind!=RC_SEEK_SKILL&&kind!=RC_SEEK_ATTACK&&kind!=RC_STRATAGEM)||g->hand_count<MAX_HAND)')
# Colorless shuffling is visible and never creates a card; snapshot the offered
# instance identities so saving mid-choice preserves the exact same three.
s=s.replace('#include "regent_effects.inc"','''static void regent_seek_three(Game*g,int raw,int e,int depth){
 shuffle(g,g->draw,g->draw_count);g->regent.option_count=MIN(3,g->draw_count);
 for(int i=0;i<g->regent.option_count;i++)g->regent.options[i]=g->draw[g->draw_count-1-i];
 regent_push(g,RT_CHOOSE,raw,e,RC_SEEK_THREE,1,depth);
}
#include "regent_effects.inc"''')
p.write_text(s)
p=q/'regent_effects.inc';s=p.read_text().replace('case C_SEEKER_STRIKE:deal(g,e,damage);CHOOSE(RC_SEEK,1);break;', 'case C_SEEKER_STRIKE:deal(g,e,damage);regent_seek_three(g,raw,e,depth);break;');p.write_text(s)
p=q/'game.c';s=p.read_text().replace('g->regent.void_used<2)return 0;', 'g->regent.void_used<2*g->regent.powers[R_VOID_FORM-REGENT_FIRST])return 0;')
s=s.replace('choice_kind>RC_STRATAGEM', 'choice_kind>RC_SEEK_THREE')
s=s.replace('else if(g->discard_count<MAX_PILE)\n   g->discard[g->discard_count++]=raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL);\n regent_created(g);', 'else if(g->discard_count<MAX_PILE)\n   g->discard[g->discard_count++]=raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL);\n else return;\n regent_created(g);')
s=s.replace('if(g->character==CHAR_NIGHTSHADE&&g->act>1){', 'if(g->character==CHAR_REGENT&&g->act>1){g->reward[0]=1000+R_METEOR_SHOWER;g->reward[1]=1000+R_THE_SEALED_THRONE;g->reward[2]=37;g->screen=BLESSING;g->selection=g->scroll=0;msg(g,"Choose an ancient gift.");return;}\n if(g->character==CHAR_NIGHTSHADE&&g->act>1){',1)
s=s.replace('if(card==N_SUPPRESS)for', 'if(card==R_METEOR_SHOWER)for(int i=0;i<g->deck_count;i++)if(CARD_ID(g->deck[i])==R_FALLING_STAR){g->deck[i]=(g->deck[i]&~511)|R_METEOR_SHOWER;replaced=1;break;}if(card==N_SUPPRESS)for')
s=s.replace('(g->character==CHAR_NIGHTSHADE&&(g->reward[i]==NR_DRAKE', '(g->character==CHAR_REGENT&&(g->reward[i]==1000+R_METEOR_SHOWER||g->reward[i]==1000+R_THE_SEALED_THRONE))||(g->character==CHAR_NIGHTSHADE&&(g->reward[i]==NR_DRAKE')
s=s.replace('STATUS(g->strength,"Strength: %d damage per hit");', '''if(g->character==CHAR_REGENT){STATUS(g->regent.stars,"Stars: %d available; saved between turns");STATUS(g->regent.forge,"Forge: +%d Sovereign Blade damage");STATUS(g->regent.created,"Cards created this combat: %d");STATUS(g->regent.vigor,"Vigor: +%d damage on next Attack");STATUS(g->regent.next_stars,"Gain %d Stars next turn");STATUS(g->regent.next_energy,"Gain %d Energy next turn");STATUS(g->regent.next_draw,"Draw %d extra cards next turn");STATUS_FLAG(g->regent.reflect,"Reflect: return blocked attack damage");STATUS(g->regent.panic,"Panic: no card Block for %d turn starts");}
 STATUS(g->strength,"Strength: %d damage per hit");''')
p.write_text(s)
p=q/'psp_start.c';s=p.read_text().replace('128*1024,PSP_THREAD_ATTR_USER','192*1024,PSP_THREAD_ATTR_USER');p.write_text(s)
