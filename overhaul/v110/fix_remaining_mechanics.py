#!/usr/bin/env python3
"""Correct v1.1.0 mechanics, selection UI, and redundant cards without changing Game ABI.
Must run after audit_cards_patch.py and patch_render.py.
Existing Game fields store pending actions and per-combat powers; save size stays v16.
"""
from pathlib import Path
import sys
root=Path(sys.argv[1])
p=root/"game.c";h=root/"game.h";r=root/"render.c"
s=p.read_text();hs=h.read_text();rs=r.read_text()
def fix(a,b):
 global s
 if a not in s:raise RuntimeError("mechanic anchor missing "+a[:100])
 s=s.replace(a,b,1)
def head(a,b):
 global hs
 if a not in hs:raise RuntimeError("header anchor missing "+a[:100])
 hs=hs.replace(a,b,1)
def draw(a,b):
 global rs
 if a not in rs:raise RuntimeError("render anchor missing "+a[:100])
 rs=rs.replace(a,b,1)
head("PAUSE_MENU,EVENT_RESULT,RELIC_RESULT};","PAUSE_MENU,EVENT_RESULT,RELIC_RESULT,CARD_CHOICE};")
for a,b in [
 ('{"Stand Fast","Gain 4 block; draw 1",1,1,1}', '{"True Grit","Gain 7 Block. Exhaust a random card.",1,1,1}'),
 ('{"Last Stand","Gain 22 block; exhaust",2,2,1}', '{"Flame Barrier","Gain 12 Block. Reflect 4 per enemy hit this turn.",2,1,1}'),
 ('{"Quick Guard","Gain 5 block; draw 1",1,1,1}', '{"Rage","Each Attack this turn grants 3 Block.",0,1,1}'),
 ('{"Brawling Wave","Gain 5 Block. Deal 5 damage.",1,1,0}', '{"Rupture","Whenever you lose HP on your turn, gain 1 Strength.",1,1,2}'),
 ('{"Conflagration","Hit all enemies. Scales with attacks.",1,2,0}', '{"Conflagration","Deal 2 damage to ALL enemies 4 times.",1,2,0}'),
 ('"Gain 7 block; draw 1"', '"Gain 9 Block. Choose one card to Exhaust."'),
 ('"Gain 28 block; exhaust"', '"Gain 16 Block. Reflect 6 per enemy hit this turn."'),
 ('"Gain 8 block; draw 1"', '"Each Attack this turn grants 5 Block."'),
 ('"Gain 8 Block. Deal 8 damage."', '"When you lose HP on your turn, gain 2 Strength."'),
 ('"Hit all for 11; scales with attacks."', '"Deal 2 damage to ALL enemies 5 times."')
]:fix(a,b)
# Save-compatible persistent combat counters:
# shop_relic: attacks this turn, shop_cards[0]: Rupture stacks,
# shop_cards[1]: Flame Barrier reflection, reward_potion bit0: no draw,
# reward_potion high byte: Rage Block/Attack.
fix("g->reward_potion=0;g->turn=0;",
    "g->reward_potion=0;g->shop_relic=0;g->shop_cards[0]=g->shop_cards[1]=0;g->turn=0;")
fix("g->turn_played=0;g->reward_potion=0;\n if(relic(g,14)",
    "g->turn_played=0;g->reward_potion=0;g->shop_relic=0;g->shop_cards[1]=0;\n if(relic(g,14)")
fix("if(g->screen==COMBAT&&g->reward_potion)return;",
    "if((g->screen==COMBAT||g->screen==CARD_CHOICE)&&(g->reward_potion&1))return;")
fix("case 36:base=8+3*up+2*g->turn_played;break;",
    "case 36:base=2;break;")
fix("case 41:base=5+3*up;break;",
    "case 41:base=0;break;")
fix("if(id==27||id==22)dmg*= (id==27?2:4+up);",
    "if(id==27||id==22||id==36)dmg*= (id==27?2:4+up);")
fix("n=MAX(n,0);g->hp-=n;\n if(n>0&&g->screen==COMBAT",
    "n=MAX(n,0);g->hp-=n;\n if(n>0&&g->screen==COMBAT&&g->shop_cards[0]>0)g->strength+=g->shop_cards[0];\n if(n>0&&g->screen==COMBAT")
fix("void game_end_turn(Game*g){",'''static void enemy_hit(Game*g,Enemy*e,int raw){
 if(e->hp<=0)return;
 damage_player(g,raw);
 if(g->shop_cards[1]>0){
   int n=g->shop_cards[1],blocked=MIN(e->block,n);
   e->block-=blocked;e->hp-=n-blocked;
 }
}
void game_end_turn(Game*g){''')
pos=s.index("void game_end_turn(Game*g){")
end=s.index("static void all_damage",pos)
piece=s[pos:end].replace("damage_player(g,enemy_attack_value(e))","enemy_hit(g,e,enemy_attack_value(e))")
piece=piece.replace("damage_player(g,dmg);break;","enemy_hit(g,e,dmg);break;")
s=s[:pos]+piece+s[end:]
fix("if(CARD_ID(g->hand[i])==50)lose_hp(g,2);",
    "if(CARD_ID(g->hand[i])==50){g->hp-=2;player_death(g);}")
fix("case 17:gain_block(g,4+n);draw_card(g);break;",'''case 17:{
 gain_block(g,7+2*up);
 if(g->hand_count>0){
  if(up){g->next_screen=17;g->scroll=up;g->selection=0;
   g->prior_screen=COMBAT;g->screen=CARD_CHOICE;}
  else {int ix=rnd(g,g->hand_count),c=g->hand[ix];
   for(int j=ix;j+1<g->hand_count;j++)g->hand[j]=g->hand[j+1];
   g->hand_count--;if(g->exhaust_count<MAX_PILE)g->exhaust[g->exhaust_count++]=c;}
 }break;
}''')
fix("case 18:lose_hp(g,1);g->strength+=1+up;if(g->hand_count>0){int c=g->hand[--g->hand_count];if(g->exhaust_count<MAX_PILE)g->exhaust[g->exhaust_count++]=c;}break;",'''case 18:
 lose_hp(g,1);
 if(g->screen!=DEFEAT&&g->hand_count>0){
   g->next_screen=18;g->scroll=up;g->selection=0;
   g->prior_screen=COMBAT;g->screen=CARD_CHOICE;
 }else if(g->screen==COMBAT)g->strength+=1+up;
 break;''')
fix("case 23:gain_block(g,5);for(int z=0;z<g->hand_count;z++)if(!CARD_UP(g->hand[z])&&CARD_ID(g->hand[z])<50){g->hand[z]|=UPBIT;if(!up)break;}break;",'''case 23:
 gain_block(g,5);
 if(up){
  for(int z=0;z<g->hand_count;z++)
   if(!CARD_UP(g->hand[z])&&CARD_ID(g->hand[z])<50)g->hand[z]|=UPBIT;
 }else{
  for(int z=0;z<g->hand_count;z++)
   if(!CARD_UP(g->hand[z])&&CARD_ID(g->hand[z])<50){
    g->next_screen=23;g->scroll=0;g->selection=z;
    g->prior_screen=COMBAT;g->screen=CARD_CHOICE;break;
   }
 }break;''')
fix("case 24:gain_block(g,22+2*n);break;",
    "case 24:gain_block(g,12+4*up);g->shop_cards[1]+=4+2*up;break;")
fix("case 33:gain_block(g,5+n);draw_card(g);break;",
    "case 33:g->reward_potion+=((3+2*up)<<8);break;")
fix("case 36:all_damage(g,8+3*up+2*g->turn_played);break;",
    "case 36:for(int hit=0;hit<4+up;hit++)all_damage(g,2);break;")
fix("case 41:gain_block(g,5+n);deal(g,enemy,5+n);break;",
    "case 41:g->shop_cards[0]+=1+up;break;")
fix("case 43:if(g->hand_count>0){int c=g->hand[--g->hand_count];if(g->exhaust_count<MAX_PILE)g->exhaust[g->exhaust_count++]=c;}for(int j=0;j<2+up;j++)draw_card(g);break;",'''case 43:
 if(g->hand_count>0){g->next_screen=43;g->scroll=up;g->selection=0;
  g->prior_screen=COMBAT;g->screen=CARD_CHOICE;}
 else for(int j=0;j<2+up;j++)draw_card(g);
 break;''')
fix("case 26:for(int j=0;j<3+up;j++)draw_card(g);if(g->hand_count>0){int x=g->hand[--g->hand_count];if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=x;}break;",'''case 26:
 for(int j=0;j<3+up;j++)draw_card(g);
 if(g->hand_count>0){g->next_screen=26;g->scroll=up;g->selection=0;
  g->prior_screen=COMBAT;g->screen=CARD_CHOICE;}
 break;''')
fix("g->turn_played++;\n if(g->hp<=0",'''g->turn_played++;
 if(cards[id].type==0){
  g->shop_relic++;
  int rage=(g->reward_potion>>8)&255;
  if(rage)g->block=MIN(999,g->block+rage);
 }
 if(g->hp<=0''')
fix("if(id==8||id==9||id==34||id==20||id==24||id==28||id==38||id==42||id==45||id==47||id==48)",
    "if(id==8||id==9||id==34||id==20||id==24||id==28||id==38||id==41||id==42||id==45||id==47||id==48)")
fix("const char *node_name(int type){",'''static void settle_card_choice(Game*g,int idx){
 if(g->screen!=CARD_CHOICE||idx<0||idx>=g->hand_count)return;
 int pending=g->next_screen,up=g->scroll,c=g->hand[idx];
 if(pending==23){
  if(CARD_UP(c)||CARD_ID(c)>=50){msg(g,"Choose an unupgraded card.");return;}
  g->hand[idx]|=UPBIT;
 }else if(pending==18||pending==43||pending==17||pending==26){
  for(int i=idx;i+1<g->hand_count;i++)g->hand[i]=g->hand[i+1];
  g->hand_count--;
  if(pending==26){if(g->draw_count<MAX_PILE)g->draw[g->draw_count++]=c;}
  else if(g->exhaust_count<MAX_PILE)g->exhaust[g->exhaust_count++]=c;
 }
 g->screen=COMBAT;g->prior_screen=COMBAT;g->scroll=0;g->next_screen=0;
 g->selection=MIN(idx,g->hand_count);
 if(pending==18)g->strength+=1+up;
 if(pending==43)for(int i=0;i<2+up;i++)draw_card(g);
 if(g->screen==COMBAT)check_win(g);
}
const char *node_name(int type){''')
fix(" case COMBAT:\n  if(b==BTN_LEFT)",''' case CARD_CHOICE:
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN&&g->selection+3<g->hand_count)g->selection+=3;
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<g->hand_count)g->selection++;
  else if(b==BTN_OK)settle_card_choice(g,g->selection);
  else if(b==BTN_BACK)msg(g,"Choose a card to finish the effect.");
  break;
 case COMBAT:
  if(b==BTN_LEFT)''')
fix("g->screen<TITLE||g->screen>RELIC_RESULT","g->screen<TITLE||g->screen>CARD_CHOICE")
fix("if(g->screen==MAP&&(g->selection<0",'''if(g->screen==CARD_CHOICE&&
 (g->hand_count<1||g->selection<0||g->selection>=g->hand_count||
 (g->next_screen!=17&&g->next_screen!=18&&g->next_screen!=23&&g->next_screen!=26&&g->next_screen!=43)))return 0;
 if(g->screen==MAP&&(g->selection<0''')
draw("static void deck_screen(const Game*g){",'''static void card_choice_screen(const Game*g){
 header(g,"CHOOSE A CARD / RESOLVE THE EFFECT");
 int pending=g->next_screen;
 center(239,48,pending==23?"ARMAMENTS - UPGRADE ONE":
 pending==18?"BRAND - EXHAUST A CARD":
 pending==43?"BURNING PACT - EXHAUST":
 pending==17?"TRUE GRIT+ - EXHAUST": "PLACE ON TOP OF DRAW",GOLD,1);
 int page=(g->selection/6)*6;
 for(int i=page;i<g->hand_count&&i<page+6;i++){
  int off=i-page,x=17+(off%3)*108,y=69+(off/3)*90;
  card_render(x,y,99,84,g->hand[i],i==g->selection,1);
 }
 gilded_panel(346,67,126,172,0);
 if(g->hand_count){
  int c=g->hand[g->selection],id=CARD_ID(c);
  fittext(350,78,cards[id].name,16,GOLD);
  wrapped_text(350,101,card_description(c),16,6,WHITE);
  if(pending==23){
   text(350,174,"UPGRADE PREVIEW",GOLD,1);
   if(CARD_UP(c)||id>=50)text(350,191,"MAXED / UNABLE",RED,1);
   else wrapped_text(350,190,card_description(c|UPBIT),16,4,GREEN);
  }else text(350,186,pending==26?"TO DRAW PILE":"SEND TO EXHAUST",GOLD,1);
 }
 if(page+6<g->hand_count)text(348,220,"MORE CARDS BELOW",MUTED,1);
 footer("D-PAD CHOOSE  X CONFIRM  SELECT SAVE & QUIT");
}
static void deck_screen(const Game*g){''')
draw(" case DECK_VIEW:deck_screen(g);break;",
     " case DECK_VIEW:deck_screen(g);break;\n case CARD_CHOICE:card_choice_screen(g);break;")
p.write_text(s);h.write_text(hs);r.write_text(rs)
print("PASS: choice UI, True Grit/Flame Barrier/Rage/Rupture, current Conflagration 2x4/5, saved ABI.")
