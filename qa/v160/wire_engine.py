from pathlib import Path
q=Path(__file__).parent/'source'
p=q/'game.c';s=p.read_text()
s=s.replace('static int regent_flags(int raw);','static void exhaust_card(Game*g,int raw);\nstatic void resolve_card(Game*g,int index,int enemy,int replay_raw);\nstatic int regent_flags(int raw);')
s=s.replace('if(g->character==CHAR_REGENT)g->regent.stars=3;','')
s=s.replace('if(index<0)return;if(index<g->hand_count&&CARD_ID(g->hand[index])>=NINJA_FIRST){','if(index<0)return;if(index<g->hand_count&&CARD_ID(g->hand[index])>=REGENT_FIRST){regent_play(g,index,enemy);return;}if(index<g->hand_count&&CARD_ID(g->hand[index])>=NINJA_FIRST){')
s=s.replace('return g&&g->character==CHAR_NIGHTSHADE?"Nightshade":"Warden";','return g&&g->character==CHAR_REGENT?"Regent":g&&g->character==CHAR_NIGHTSHADE?"Nightshade":"Warden";')
s=s.replace('if(!g||character<0||character>1)return;','if(!g||character<0||character>2)return;')
s=s.replace('g->character=character;g->deck_count=', 'if(character==CHAR_REGENT){g->character=CHAR_REGENT;g->deck_count=g->relic_count=g->potion_count=0;g->max_energy=3;g->maxhp=g->hp=75;g->gold=99;for(int i=0;i<4;i++){add_card(g,R_STRIKE_REGENT);add_card(g,R_DEFEND_REGENT);}add_card(g,R_FALLING_STAR);add_card(g,R_VENERATE);add_relic(g,63);msg(g,"The Regent claims the ruined tower.");return;}\n g->character=character;g->deck_count=')
s=s.replace('{"Prism Bone","Play an Attack, Skill and Power in one combat: gain 2 Energy and draw 2. Once per combat."},','{"Prism Bone","Play an Attack, Skill and Power in one combat: gain 2 Energy and draw 2. Once per combat."},\n {"Divine Right","At the start of each combat, gain 3 Stars. Stars remain between turns."},')
s=s.replace('void game_update(Game*g){','void game_update(Game*g){if(g->screen==COMBAT&&g->regent.task_count)regent_process(g);')
s=s.replace('screen==COMBAT||screen==CARD_CHOICE||screen==NINJA_CHOICE', 'screen==COMBAT||screen==CARD_CHOICE||screen==NINJA_CHOICE||screen==REGENT_CHOICE')
s=s.replace('g->selection=1-g->selection;\n  else if(b==BTN_OK){game_choose_character', 'g->selection=(g->selection+((b==BTN_LEFT||b==BTN_UP)?2:1))%3;\n  else if(b==BTN_OK){game_choose_character')
s=s.replace('case NINJA_CHOICE:{int n=', '''case REGENT_CHOICE:{int n=game_regent_choice_count(g);
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,n);
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<n)g->selection++;
  else if(b==BTN_OK)regent_choose(g,g->selection,0);
  else if(b==BTN_BACK)regent_choose(g,0,1);break;}
 case NINJA_CHOICE:{int n=''')
# Shared outside-COMBAT presentation stats are kept read-only.
s=s.replace('if(g->screen==NINJA_CHOICE&&', 'if(g->screen==REGENT_CHOICE&&(g->regent.choice_kind<1||g->regent.choice_kind>RC_STRATAGEM||g->regent.choice_remaining<1||g->selection<0||g->selection>=game_regent_choice_count(g)))return 0;\n if(g->regent.stars<0||g->regent.stars>999||g->regent.task_count<0||g->regent.task_count>REGENT_TASK_MAX)return 0;\n if(g->screen==NINJA_CHOICE&&')
# Damage reflection uses the amount actually absorbed, after block modifiers.
s=s.replace('int n=game_card_cost(raw);', 'int n=game_card_cost(raw);')
p.write_text(s)
for p in [q/'regent_engine.inc',q/'regent_effects.inc']:
 s=p.read_text().replace('R_I_AM_INVINCIBLE','R_IAM_INVINCIBLE');p.write_text(s)
