#!/usr/bin/env python3
"""1.1.0 full Ironclad card catalog (main v0.107.1), legacy-save migration.

Retains existing IDs 0..49 and status IDs 50..53 so old decks do not
silently change. Adds missing Ironclad cards 54..90; Giant Rock token=91.
"""
from pathlib import Path
import sys, re
root=Path(sys.argv[1]);p=root/"game.c";h=root/"game.h";r=root/"render.c"
s=p.read_text();hs=h.read_text();rs=r.read_text()
def fix(a,b):
 global s
 if a not in s:raise RuntimeError("CATALOG game anchor missing "+a[:100])
 s=s.replace(a,b,1)
def head(a,b):
 global hs
 if a not in hs:raise RuntimeError("CATALOG header anchor missing "+a[:100])
 hs=hs.replace(a,b,1)
def draw(a,b):
 global rs
 if a not in rs:raise RuntimeError("CATALOG renderer anchor missing "+a[:100])
 rs=rs.replace(a,b,1)
# id, name, base description, cost, rarity (0 common, 1 uncommon, 2 rare, 3 ancient),
# type (0 Attack,1 Skill,2 Power), upgraded rules
extra=[
(54,"Aggression","At turn start: take +Upgrade random Discard Attack.",1,2,2,"Innate. At turn start: take +Upgrade random Discard Attack."),
(55,"Barricade","Block is retained between turns.",3,2,2,"Cost 2. Block is retained between turns."),
(56,"Break","Deal 20. Apply 5 Vulnerable.",1,3,0,"Deal 30. Apply 7 Vulnerable."),
(57,"Cascade","Play top X cards from Draw.",-1,2,1,"Play top X+1 cards from Draw."),
(58,"Colossus","Gain 5 Block. Half damage from Vulnerable foes.",1,1,1,"Gain 8 Block. Half damage from Vulnerable foes."),
(59,"Corruption","Skills cost 0 and Exhaust on play.",3,3,2,"Cost 2. Skills cost 0 and Exhaust."),
(60,"Crimson Mantle","Turn start: lose 1 HP; gain 8 Block.",1,2,2,"Turn start: lose 1 HP; gain 10 Block."),
(61,"Cruelty","Vulnerable foes take +25% damage.",1,2,2,"Vulnerable foes take +50% damage."),
(62,"Dark Embrace","When a card Exhausts, draw 1.",2,2,2,"Cost 1. When a card Exhausts, draw 1."),
(63,"Demonic Shield","MULTIPLAYER: lose 1 HP; give ally your Block.",0,1,1,"MULTIPLAYER: lose 1 HP; give ally Block. No Exhaust."),
(64,"Drum of Battle","Draw 2. On Exhaust: gain 2 Energy.",1,1,1,"Draw 2. On Exhaust: gain 3 Energy."),
(65,"Expect a Fight","Gain 1 Energy per Attack in Hand. No more Energy.",2,1,1,"Cost 1. Gain 1 Energy per Attack in Hand. Lock Energy."),
(66,"Fight Me!","Deal 5 twice. Gain 3 STR; enemy +1 STR.",2,1,0,"Deal 6 twice. Gain 4 STR; enemy +1 STR."),
(67,"Forgotten Ritual","If Exhausted this turn, gain 3 Energy. Exhaust.",1,1,1,"If Exhausted this turn, gain 4 Energy. Exhaust."),
(68,"Havoc","Play top Draw card; Exhaust it.",1,0,1,"Cost 0. Play top Draw card; Exhaust it."),
(69,"Hellraiser","Draw Strike: auto-play it on random enemy.",2,2,2,"Cost 1. Draw Strike: auto-play it."),
(70,"Howl from Beyond","Deal 16 ALL. End turn: plays from Exhaust.",3,1,0,"Deal 21 ALL. End turn: plays from Exhaust."),
(71,"Infernal Blade","Add random Attack to Hand; free this turn. Exhaust.",1,1,1,"Cost 0. Add random free Attack. Exhaust."),
(72,"Juggernaut","Gain Block: deal 6 to random foe.",2,2,2,"Gain Block: deal 8 to random foe."),
(73,"Juggling","On 3rd Attack each turn, copy it to Hand.",1,1,2,"Innate. On 3rd Attack each turn, copy to Hand."),
(74,"Mangle","Deal 15. Enemy -10 STR this turn.",3,2,0,"Deal 20. Enemy -15 STR this turn."),
(75,"Not Yet","Heal 10 HP. Exhaust.",2,2,1,"Heal 13 HP. Exhaust."),
(76,"One-Two Punch","Next Attack this turn plays twice.",1,2,1,"Next 2 Attacks this turn play twice."),
(77,"Primal Force","Transform hand Attacks into Giant Rocks.",0,2,1,"Transform hand Attacks into Giant Rock+."),
(78,"Pyre","Start turn: gain 1 Energy.",2,2,2,"Start turn: gain 2 Energy."),
(79,"Rampage","Deal 9. This Rampage +5 damage for combat.",1,1,0,"Deal 9. This Rampage +9 each use."),
(80,"Setup Strike","Deal 7. Gain 2 STR this turn.",1,0,0,"Deal 9. Gain 3 STR this turn."),
(81,"Spite","Deal 5. If lost HP this turn: hit twice.",0,1,0,"Deal 5. If lost HP this turn: hit 3 times."),
(82,"Stampede","End turn: auto-play random Hand Attack.",2,1,2,"Cost 1. End turn: auto-play random Attack."),
(83,"Stoke","Exhaust Hand. Add random card per exhausted.",1,2,1,"Exhaust Hand. Add upgraded card per exhausted."),
(84,"Stone Armor","Gain 4 Plating. End turn: gain Block.",1,1,2,"Gain 6 Plating. End turn: gain Block."),
(85,"Tank","MULTIPLAYER: double enemy damage to self, halve allies.",1,2,2,"MULTIPLAYER: cost 0. Protect allies."),
(86,"Tear Asunder","Deal 5. +1 hit per HP-loss event this combat.",2,2,0,"Deal 7. +1 hit per HP-loss event."),
(87,"Thrash","Deal 4 twice. Exhaust random Attack; gain its damage.",1,2,0,"Deal 6 twice. Exhaust Attack, add damage."),
(88,"Unmovable","First card Block gained per turn is doubled.",2,2,2,"Cost 1. First card Block per turn doubled."),
(89,"Unrelenting","Deal 14. Next Attack costs 0.",2,1,0,"Deal 20. Next Attack costs 0."),
(90,"Vicious","Applying Vulnerable draws 1 card.",1,1,2,"Applying Vulnerable draws 2 cards."),
(91,"Giant Rock","Deal 16 damage. (Generated token)",1,0,0,"Deal 20 damage. (Generated token)")
]
assert len(extra)==38 and [t[0] for t in extra]==list(range(54,92))
def cquote(x):return '"'+x.replace("\\","\\\\").replace('"','\\"')+'"'
lines=[]
for id,name,desc,cost,rarity,kind,_ in extra:
 lines.append(" {%s,%s,%d,%d,%d}, // %d" %(cquote(name),cquote(desc),cost,rarity,kind,id))
fix(' {"Dread","Unplayable; lose energy",-2,0,3}           //53\n};',
    ' {"Dread","Unplayable; lose energy",-2,0,3},          //53\n'+"\n".join(lines)+'\n};')
fix('static const char *upgrade_descriptions[50]={','static const char *upgrade_descriptions[92]={')
fix(' "At turn start: +4 Strength.","Deal 6, +3 per Strike you own."\n};',
 ' "At turn start: +4 Strength.","Deal 6, +3 per Strike you own.",\n NULL,NULL,NULL,NULL,\n'+
 ",\n".join(cquote(t[6]) for t in extra)+'\n};')
fix('if(CARD_UP(raw)&&id<50)return upgrade_descriptions[id];',
    'if(CARD_UP(raw)&&id<92&&upgrade_descriptions[id])return upgrade_descriptions[id];')
fix('if(id==37)return 0;\n return cards[id].cost;',
'''if(id==37)return 0;
 if(CARD_UP(raw)&&(id==55||id==59||id==62||id==65||id==68||
    id==69||id==71||id==82||id==85||id==88))return cards[id].cost-1;
 return cards[id].cost;''')
# Card ID space fits in seven bits (and uses bit7 for upgraded).
head('#define GAME_VERSION 16','#define GAME_VERSION 17\n#define CARD_STATUS_MIN 50\n#define CARD_STATUS_MAX 53\n#define CARD_PLAYABLE 87\n#define CARD_GIANT_ROCK 91\n#define CARD_FREE 0x100\n#define CARD_FORCE_EXHAUST 0x200\n#define CARD_EPHEMERAL 0x400')
head('char message[72], subtitle[72];','char message[72], subtitle[72];\n int ic[48]; /* v17 Ironclad state; appended to migrate v16 saves */')
head('const char *card_description(int raw);',
'''const char *card_description(int raw);
int game_card_is_playable(int raw);
int game_card_is_upgradeable(int raw);''')
# Explicit inclusion avoids accidentally rewarding the four status cards or Giant Rock.
fix('static int card_reward(Game*g){int r=rnd(g,100);int rarity=r<13?2:(r<60?1:0);int tries=0;for(;;){int id=rnd(g,CARD_COUNT-4);if(id>2 && (cards[id].rarity==rarity||tries++>100))return id;}}',
'''int game_card_is_playable(int raw){
 int id=CARD_ID(raw);
 return id>=0&&id<CARD_COUNT&&!(id>=CARD_STATUS_MIN&&id<=CARD_STATUS_MAX);
}
int game_card_is_upgradeable(int raw){
 return game_card_is_playable(raw)&&!CARD_UP(raw)&&CARD_ID(raw)!=CARD_GIANT_ROCK;
}
static int solo_reward_eligible(int id){
 return id>=3&&id<CARD_COUNT&&game_card_is_playable(id)&&
   id!=CARD_GIANT_ROCK&&id!=63&&id!=85&&id!=56&&id!=59;
}
static int card_reward(Game*g){
 int roll=rnd(g,100),rarity=roll<15?2:(roll<54?1:0);
 for(int tries=0;tries<300;tries++){
  int id=rnd(g,CARD_COUNT);
  if(solo_reward_eligible(id)&&cards[id].rarity==rarity)return id;
 }
 for(int id=3;id<CARD_COUNT;id++)
   if(solo_reward_eligible(id)&&cards[id].rarity==rarity)return id;
 return 5;
}''')
fix('if(!CARD_UP(g->deck[i])&&CARD_ID(g->deck[i])<50)count++;',
    'if(game_card_is_upgradeable(g->deck[i]))count++;')
fix('if(!CARD_UP(g->hand[z])&&CARD_ID(g->hand[z])<50)',
    'if(game_card_is_upgradeable(g->hand[z]))')
fix('if(CARD_UP(c)||CARD_ID(c)>=50){msg(g,"Choose an unupgraded card.");return;}',
    'if(!game_card_is_upgradeable(c)){msg(g,"Choose an unupgraded card.");return;}')
fix('if(CARD_UP(g->deck[idx])||CARD_ID(g->deck[idx])>=50){',
    'if(!game_card_is_upgradeable(g->deck[idx])){')
fix('if(CARD_ID(g->reward[i])>=CARD_COUNT-4)return 0;',
    'if(!solo_reward_eligible(CARD_ID(g->reward[i])))return 0;')
# The two Ancient powers can be discovered at the existing ancient lesson.
fix('case 2: if(g->deck_count<MAX_DECK)add_card(g,card_reward(g)|UPBIT);msg(g,"A forgotten lesson: gained an upgraded card.");break;',
'''case 2:{
   int card=rnd(g,6)==0?(rnd(g,2)?56:59):card_reward(g);
   if(g->deck_count<MAX_DECK)add_card(g,card|UPBIT);
   snprintf(g->message,sizeof(g->message),"Ancient lesson: %s+ gained.",cards[card].name);
   break;
 }''')
# All six-deck card and choice previews must work for ids above 53.
draw('if(CARD_UP(c)||id>=50)text(350,191,"MAXED / UNABLE",RED,1);',
     'if(!game_card_is_upgradeable(c))text(350,191,"MAXED / UNABLE",RED,1);')
draw('if(CARD_UP(raw)||id>=50)text(351,168,"MAX UPGRADED",RED,1);',
     'if(!game_card_is_upgradeable(raw))text(351,168,"MAX UPGRADED",RED,1);')
# Save format migration: legacy exact-byte snapshot and checksum read into the
# unchanged prefix. Reject corrupted snapshots rather than resetting progress.
a='int game_load(Game*g,const char*path){Game temp;uint32_t check;FILE*f=fopen(path,"rb");if(!f)return 0;int ok=fread(&check,4,1,f)==1&&fread(&temp,sizeof(temp),1,f)==1;fclose(f);if(!ok||check!=hash_bytes((const unsigned char*)&temp,sizeof(temp))||!game_validate(&temp))return 0;*g=temp;return 1;}'
b='''int game_load(Game*g,const char*path){
 Game temp;uint32_t check;FILE*f=fopen(path,"rb");if(!f)return 0;
 if(fseek(f,0,SEEK_END)){fclose(f);return 0;}
 long len=ftell(f);rewind(f);
 size_t legacy=offsetof(Game,ic),now=sizeof(Game),n;
 if(len==(long)(now+4))n=now;
 else if(len==(long)(legacy+4))n=legacy;
 else {fclose(f);return 0;}
 memset(&temp,0,sizeof(temp));
 int ok=fread(&check,4,1,f)==1&&fread(&temp,n,1,f)==1;
 fclose(f);
 if(!ok||check!=hash_bytes((const unsigned char*)&temp,n))return 0;
 if(n==legacy&&temp.version==16)temp.version=GAME_VERSION;
 if(!game_validate(&temp))return 0;
 *g=temp;return 1;
}'''
fix(a,b)
p.write_text(s);h.write_text(hs);r.write_text(rs)
print("PASS 87-card catalog: IDs 54-90, Giant Rock token, rewards, upgrades, v16->v17 save migration.")
