from pathlib import Path
import json,re
q=Path(__file__).parent;s=q/'source'
data=json.loads((q/'reference/silent-cards.json').read_text())
ids={x['id']:96+i for i,x in enumerate(data)}
(q/'reference/card-ids.json').write_text(json.dumps(ids,indent=2)+'\n')
h=s/'game.h';t=h.read_text().replace('#define MAX_RELICS 40','#define MAX_RELICS 64').replace('#define GAME_VERSION 20','#define GAME_VERSION 21')
t=t.replace('#define CARD_FREE 0x100','#define CARD_FREE 0x200').replace('#define CARD_FORCE_EXHAUST 0x200','#define CARD_FORCE_EXHAUST 0x400').replace('#define CARD_EPHEMERAL 0x400','#define CARD_EPHEMERAL 0x800').replace('#define CARD_MOD_SHIFT 11','#define CARD_MOD_SHIFT 12').replace('(7 << 11)','(7 << 12)').replace('>> 11','>> 12').replace('#define UPBIT 0x80','#define UPBIT 0x100').replace('((x)&0x7f)','((x)&0xff)')
t=t.replace('ACT_INTRO,EVENT_CARD','ACT_INTRO,EVENT_CARD,CHARACTER_SELECT,NINJA_CHOICE')
constants='''
#define CARD_SLY (1<<15)
#define CARD_SLY_TURN (1<<16)
#define CARD_INKY (1<<17)
#define CARD_RETAIN_TURN (1<<18)
#define CARD_COUNTER_SHIFT 19
#define CARD_COUNTER_MASK (31<<CARD_COUNTER_SHIFT)
#define CARD_COUNTER(raw) (((raw)>>CARD_COUNTER_SHIFT)&31)
#define CARD_RAW_MASK ((1<<24)-1)
#define NINJA_FIRST 96
#define NINJA_COUNT 89
#define NINJA_LAST (NINJA_FIRST+NINJA_COUNT)
enum {CHAR_WARDEN,CHAR_NIGHTSHADE};
enum {NR_SNAKE=44,NR_SKULL,NR_TINGSHA,NR_BANDAGES,NR_FUNNEL,NR_SCROLL,NR_KRANE,NR_DART,NR_DRAKE};
enum {NS_ACCURACY,NS_AFTERIMAGE,NS_THORNS,NS_ACCELERANT,NS_ENVENOM,NS_FAN,NS_PHANTOM,NS_BLADES,NS_FUMES,NS_OUTBREAK,NS_OUTBREAK_COUNT,NS_PLANNER,NS_SERPENT,NS_SPEEDSTER,NS_TOOLS,NS_PLANS,NS_TRACKING,NS_WRAITH,NS_INTANGIBLE,NS_TEMP_DEX,NS_BLUR,NS_NEXT_BLOCK,NS_NEXT_DRAW,NS_SHADOW_NEXT,NS_SHADOW_NOW,NS_SHADOWMELD,NS_BURST,NS_POUNCE,NS_CORROSIVE,NS_DRAWS_COMBAT,NS_DISCARDS_TURN,NS_SKILLS_TURN,NS_SHIVS_TURN,NS_STRANGLE0,NS_STRANGLE1,NS_BONUS_REWARDS,NS_SNEAKY,NS_RETAIN_READY,NS_OPENING_DRAW,NS_LAST_DRAW,NS_RESOLVING,NS_COUNT};
#define NINJA_TASK_MAX 128
typedef struct {int kind,raw,target,n,aux,depth;} NinjaTask;
typedef struct {
 int v[NS_COUNT],powers[NINJA_COUNT];
 NinjaTask tasks[NINJA_TASK_MAX];int task_count;
 int choice_kind,choice_remaining,choice_count,choice_raws[MAX_HAND],choice_source,choice_target,choice_depth;
 int nightmare_raw[MAX_HAND],nightmare_n[MAX_HAND],nightmare_count;
} NinjaState;
'''
constants+='enum {\n'+',\n'.join(' N_'+k+'='+str(v) for k,v in ids.items())+'\n};\n'
t=t.replace('typedef struct {char name[24]',constants+'\ntypedef struct {char name[24]')
t=t.replace(' int endless,combat_music;',' int endless,combat_music;\n int character;NinjaState ninja;')
t=t.replace('void game_new(Game *g,uint32_t seed);','void game_new(Game *g,uint32_t seed);\nvoid game_choose_character(Game *g,int character);\nconst char *game_character_name(const Game *g);\nint game_card_is_sly(const Game *g,int raw);\nint game_card_is_retained(const Game *g,int raw);\nint game_ninja_choice_eligible(const Game *g,int index);\nconst char *game_ninja_choice_title(const Game *g);')
h.write_text(t)
# Exact old structures, with fixed capacities and maps; fields are migrated by name.
oldh=Path('qa/v150r2/source/game.h').read_text();start=oldh.index('typedef struct {\n uint32_t magic,version,seed,rng;');end=oldh.index('} Game;',start)+len('} Game;')
legacy=oldh[start:end].replace('} Game;','} LegacyGame20;').replace('relics[MAX_RELICS]','relics[40]')
(s/'save_v20.inc').write_text('/* Frozen 1.5 layout. */\n'+legacy+'\ntypedef char legacy_v20_size[(sizeof(LegacyGame20)==3260)?1:-1];\n')
p=s/'save_legacy.inc';t=p.read_text().replace('relics[MAX_RELICS]','relics[40]');p.write_text(t)
# Record functional metadata; descriptions are regenerated separately for the PSP.
def plain(t):
 t=re.sub(r'\[energy:(\d+)\]',lambda m:m[1]+' Energy',t or '')
 t=re.sub(r'\[[^]]+\]','',t).replace('\n',' ')
 return t
# Concise rule translations, keeping every effect/keyword.
def desc(c,up):
 text=plain((c.get('upgrade_description') if up else None) or c['description'])
 text=text.replace('into your Hand','to your hand').replace('your Hand','your hand').replace('your Draw Pile','your draw pile').replace('your Discard Pile','your discard pile').replace('your Exhaust Pile','your exhaust pile')
 text=text.replace('Whenever you ','When you ').replace('At the start of your turn,','Turn start:').replace('At the end of your turn,','Turn end:').replace('additional','extra')
 kw=list(c.get('keywords') or []);u=c.get('upgrade') or {}
 if up:
  for k in ['Exhaust','Retain','Innate','Sly']:
   if u.get('add_'+k.lower()) and k not in kw:kw.append(k)
   if u.get('remove_'+k.lower()) and k in kw:kw.remove(k)
 for k in kw:
  if not re.search(r'\b'+k+r'\b',text):text+=' '+k+'.'
 if c.get('multiplayer_only') or c['id'] in ['FLANKING','SNEAKY']:text='Multiplayer only. '+text
 return text,kw
rows=[];ups=[];meta=[]
for c in data:
 base,flags=desc(c,False);upgrade,uflags=desc(c,True)
 cost=-1 if c['is_x_cost'] else c['cost'];rar={'Basic':4,'Common':0,'Uncommon':1,'Rare':2,'Ancient':3,'Token':5}[c['rarity']];typ={'Attack':0,'Skill':1,'Power':2}[c['type']]
 rows.append('{'+','.join([json.dumps(c['name']),json.dumps(base),str(cost),str(rar),str(typ)])+'},')
 ups.append(json.dumps(upgrade)+',')
 mask=lambda fs:sum({'Exhaust':1,'Retain':2,'Innate':4,'Sly':8}.get(x,0) for x in fs)
 upcost=(c.get('upgrade') or {}).get('cost',cost)
 damage=c.get('damage') or 0;block=c.get('block') or 0
 u=c.get('upgrade') or {};du=damage+int(u.get('damage','+0'));bu=block+int(u.get('block','+0'))
 meta.append('{'+','.join(map(str,[upcost,mask(flags),mask(uflags),damage,du,block,bu,{'Self':0,'AnyEnemy':1,'AllEnemies':2,'RandomEnemy':3,'None':0,'AnyPlayer':0,'AnyAlly':0}.get(c['target'],0)]))+'},')
(s/'ninja_cards.inc').write_text('\n'.join(rows)+'\n');(s/'ninja_upgrades.inc').write_text('\n'.join(ups)+'\n')
(s/'ninja_meta.inc').write_text('typedef struct {int upcost,flags,upflags,damage,updamage,block,upblock,target;} NinjaCardDef;\nstatic const NinjaCardDef ninja_cards[NINJA_COUNT]={\n'+'\n'.join(meta)+'\n};\n')
p=s/'game.c';t=p.read_text().replace('/* Completely original names, text, effects, and art. */','/* Original Rift Ascent art; card mechanics adapted from the reference catalog. */')
t=t.replace('};\nconst int CARD_COUNT','#include "ninja_cards.inc"\n};\nconst int CARD_COUNT',1)
t=t.replace('"At turn end, deal 6 damage to all enemies per unspent Energy (up to 2). Ignores damage modifiers."','"At turn end, deal 6 damage to all enemies per unspent Energy (up to 2). Ignores damage modifiers.",\n#include "ninja_upgrades.inc"')
t=t.replace('static int card_cost_base(int raw){','#include "ninja_meta.inc"\nstatic int card_cost_base(int raw){').replace(' if(id==21&&CARD_UP(raw))return 0;',' if(id>=NINJA_FIRST)return CARD_UP(raw)?ninja_cards[id-NINJA_FIRST].upcost:cards[id].cost;\n if(id==21&&CARD_UP(raw))return 0;',1)
p.write_text(t)
