from pathlib import Path
q=Path(__file__).parent/'source'
def edit(n,a,b):
 p=q/n;s=p.read_text();assert a in s,(n,a[:80]);p.write_text(s.replace(a,b))
edit('game.c','CARD_ID(raw)!=CARD_GIANT_ROCK;', 'CARD_ID(raw)!=CARD_GIANT_ROCK&&CARD_ID(raw)!=R_DEBRIS;')
edit('regent_effects.inc','case C_FISTICUFFS:{int hp=e>=0?g->enemies[e].hp:0;deal(g,e,damage);if(e>=0)card_block(g,MAX(0,hp-g->enemies[e].hp));break;}', 'case C_FISTICUFFS:{int dealt=e>=0?attack_amount(g,e,damage):0;deal(g,e,damage);card_block(g,dealt);break;}')
edit('regent_effects.inc','g->regent.next_block+=5+2*up;', 'g->regent.next_block+=regent_delayed_block(g,raw,5+2*up);')
edit('regent_engine.inc','#include "regent_effects.inc"', '''static int regent_delayed_block(const Game*g,int raw,int base){
 if(g->regent.panic)return 0;
 int n=MAX(0,base+g->dexterity+(relic(g,15)?2:0)+(CARD_MOD(raw)==MOD_WARD?4:0));
 /* Both halves of the same card share Unmovable's card multiplier. */
 if(g->ic[IC_UNMOVABLE]&&g->ic[IC_UNMOVABLE_USED]==1)n*=2;
 for(int i=0;i<MIN(20,g->ninja.v[NS_SHADOWMELD]);i++)n=(int)MIN(999LL,(long long)n*2);
 return MIN(999,n);
}
#include "regent_effects.inc"''')
# Capture Unmovable availability before immediate block sets the used bit.
edit('regent_effects.inc','case R_GLITTERSTREAM:card_block(g,block);g->regent.next_block+=regent_delayed_block(g,raw,5+2*up);break;', 'case R_GLITTERSTREAM:{int used=g->ic[IC_UNMOVABLE_USED];card_block(g,block);int saved=g->ic[IC_UNMOVABLE_USED];g->ic[IC_UNMOVABLE_USED]=!used;g->regent.next_block+=regent_delayed_block(g,raw,5+2*up);g->ic[IC_UNMOVABLE_USED]=saved;break;}')
edit('regent_preview.inc','block,5+2*up);break;', 'block,regent_delayed_block(g,raw,5+2*up));break;')
# regent_delayed_block expects used=1 for the current card; preview uses state before card.
edit('regent_preview.inc','case R_GLITTERSTREAM:if(p->block_count)snprintf(out,cap,"Gain %s Block. Next turn, gain %d Block.",block,regent_delayed_block(g,raw,5+2*up));break;', 'case R_GLITTERSTREAM:{Game copy=*g;copy.ic[IC_UNMOVABLE_USED]=!g->ic[IC_UNMOVABLE_USED];snprintf(out,cap,"Gain %s Block. Next turn, gain %d Block.",block,regent_delayed_block(&copy,raw,5+2*up));break;}')
edit('regent_engine.inc','id>=REGENT_FIRST&&id<REGENT_LAST&&solo_reward_eligible(id)', 'id>=REGENT_FIRST&&id<REGENT_LAST&&solo_reward_eligible(id)&&id!=R_ROYALTIES')
edit('regent_engine.inc','shuffle(g,g->draw,g->draw_count);g->regent.option_count', 'shuffle(g,g->draw,g->draw_count);regent_after_shuffle(g);g->regent.option_count')
# Useful concrete status descriptions for every new Power.
p=q/'game.c';s=p.read_text();a='if(id>=REGENT_FIRST){snprintf(out,(size_t)cap,"%s: %d played",cards[id].name,g->regent.played[id-REGENT_FIRST]);return;}'
b='''if(id>=REGENT_FIRST){int n=g->regent.powers[id-REGENT_FIRST];const char*rule="active";
 switch(id){
 case R_ARSENAL:rule="+%d Strength per created card";break;
 case R_BLACK_HOLE:rule="%d damage/all per Star gain or spend";break;
 case R_CHILD_OF_THE_STARS:rule="%d Block for each Star spent";break;
 case R_FURNACE:rule="Forge %d each turn start";break;
 case R_GENESIS:rule="+%d Stars each turn start";break;
 case R_HAMMER_TIME:rule="Forge gains %d additional copies";break;
 case R_MONARCHS_GAZE:rule="each hit: foe loses %d Strength this turn";break;
 case R_NEUTRON_AEGIS:case C_ETERNAL_ARMOR:rule="Plating granted; see current Plating";break;
 case R_ORBIT:rule="gain %d Energy per 4 Energy spent";break;
 case R_PALE_BLUE_DOT:rule="5+ plays: draw %d extra next turn";break;
 case R_PARRY:rule="Sovereign Blade gains %d Block";break;
 case R_PILLAR_OF_CREATION:rule="%d Block per created card";break;
 case R_ROYALTIES:rule="gain %d Gold at combat end";break;
 case R_SEEKING_EDGE:rule="Sovereign Blade hits ALL enemies";break;
 case R_SPECTRUM_SHIFT:rule="create %d Colorless cards/turn";break;
 case R_SWORD_SAGE:rule="Sovereign Blade replays %d times";break;
 case R_THE_SEALED_THRONE:rule="gain %d Stars per card played";break;
 case R_TYRANNY:rule="draw %d, then Exhaust that many/turn";break;
 case R_VOID_FORM:n*=2;rule="first %d manual plays/turn are free";break;
 case C_AUTOMATION:rule="gain %d Energy per 10 cards drawn";break;
 case C_CALAMITY:rule="create %d Attacks per Attack played";break;
 case C_ENTROPY:rule="Transform %d cards each turn start";break;
 case C_FASTEN:rule="Defend cards gain +%d Block";break;
 case C_MAYHEM:rule="autoplay %d top draw cards/turn";break;
 case C_NOSTALGIA:rule="first Attack/Skill goes atop Draw";break;
 case C_PANACHE:rule="%d damage/all per 5 cards this turn";break;
 case C_PREP_TIME:rule="gain %d Vigor each turn start";break;
 case C_PROWESS:rule="Strength and Dexterity granted";break;
 case C_ROLLING_BOULDER:rule="%d damage/all next turn; grows +5/copy";break;
 case C_STRATAGEM:rule="pick %d draw cards after a shuffle";break;
 }
 char detail[150];snprintf(detail,sizeof(detail),rule,n);snprintf(out,cap,"%s: %s",cards[id].name,detail);return;}'''
assert a in s;s=s.replace(a,b);p.write_text(s)
p=Path(__file__).parent/'catalog.py';s=p.read_text().replace("if 'multiplayer' in (c.get('sourceNotes') or '').lower():rarity=4", "# All five ally cards use explicit local solo adaptations and normal rarity.")
s=s.replace("return s\ndef val", "return s.replace('You draws','You draw').replace('You adds','You add').replace('You chooses','You choose').replace('gains 1 Energy, and gains','gain 1 Energy, and gain')\ndef val")
p.write_text(s)
