from pathlib import Path
import re
s=Path(__file__).parent/'source';p=s/'game.c';t=p.read_text().replace('"Play an Attack and a Skill in the same turn: gain 1 Energy. Once per turn."}','"Play an Attack and a Skill in the same turn: gain 1 Energy. Once per turn."},')
# Ensure previous last row delimiter.
t=t.replace('\n {"Serpent Ring"',',\n {"Serpent Ring"') if '\n {"Serpent Ring"' in t and not t[t.index('\n {"Serpent Ring"')-1]==',' else t
p.write_text(t)
legacy=(s/'save_v20.inc').read_text();body=legacy[legacy.index('uint32_t magic'):legacy.index('} LegacyGame20')]
fields=[]
for declaration in body.split(';'):
 declaration=declaration.strip()
 if not declaration:continue
 declaration=re.sub(r'^(uint32_t|int|char|Enemy)\s+','',declaration)
 for part in declaration.split(','):
  name=re.match(r'\s*(\w+)',part)
  if name:fields.append(name[1])
copy='\n'.join(' memcpy(&out->%s,&old.%s,sizeof(old.%s));'%(n,n,n) for n in fields)
p=s/'save_legacy.inc';t=p.read_text();a=t.index(' memset(out,0,sizeof(*out));memcpy');b=t.index(' out->version=GAME_VERSION;',a)
t=t[:a]+' memset(out,0,sizeof(*out));\n'+copy+'\n'+t[b:];p.write_text(t)
p=s/'save_v20.inc';t=p.read_text();t+='''static int migrate_card20(int raw){return (raw&127)|((raw&~127)<<1);}
static int migrate_raw_cards(Game*g){
 if(g->deck_count<0||g->deck_count>MAX_DECK||g->hand_count<0||g->hand_count>MAX_HAND||g->draw_count<0||g->draw_count>MAX_PILE||g->discard_count<0||g->discard_count>MAX_PILE||g->exhaust_count<0||g->exhaust_count>MAX_PILE)return 0;
 for(int i=0;i<g->deck_count;i++)g->deck[i]=migrate_card20(g->deck[i]);
 for(int i=0;i<g->hand_count;i++)g->hand[i]=migrate_card20(g->hand[i]);
 for(int i=0;i<g->draw_count;i++)g->draw[i]=migrate_card20(g->draw[i]);
 for(int i=0;i<g->discard_count;i++)g->discard[i]=migrate_card20(g->discard[i]);
 for(int i=0;i<g->exhaust_count;i++)g->exhaust[i]=migrate_card20(g->exhaust[i]);
 int screen=g->screen;
 if(screen==REWARD||screen==REWARD_PREVIEW||(screen==DECK_VIEW&&g->prior_screen==REWARD))for(int i=0;i<3;i++)g->reward[i]=migrate_card20(g->reward[i]);
 if(screen==EVENT&&(g->effect_turn==2||g->effect_turn==6))for(int i=0;i<2;i++)g->reward[i]=migrate_card20(g->reward[i]);
 if(screen==EVENT_RESULT&&g->reward[0]>=0)g->reward[0]=migrate_card20(g->reward[0]);
 if(screen==SHOP||((screen==DECK_VIEW||screen==POTION_VIEW)&&g->prior_screen==SHOP))for(int i=0;i<3;i++)if(g->shop_cards[i]>=0)g->shop_cards[i]=migrate_card20(g->shop_cards[i]);
 if(screen==CARD_CHOICE&&g->next_screen==25)g->ic[IC_PENDING_CARD]=migrate_card20(g->ic[IC_PENDING_CARD]);
 if(g->ic[46]>0)g->ic[46]=migrate_card20(g->ic[46]-1)+1;
 return 1;
}
static int load_v20(Game*out,const unsigned char*raw,size_t n){
 LegacyGame20 old;if(n!=sizeof(old))return 0;memcpy(&old,raw,n);memset(out,0,sizeof(*out));
'''+copy+'''
 out->version=GAME_VERSION;return 1;
}
''';p.write_text(t)
p=s/'game.c';t=p.read_text().replace('#include "save_legacy.inc"','#include "save_v20.inc"\n#include "save_legacy.inc"')
t=t.replace(' else if(!load_legacy_run(&temp,raw,n,version))return 0;',' else {if(version==20){if(!load_v20(&temp,raw,n))return 0;}else if(!load_legacy_run(&temp,raw,n,version))return 0;if(!migrate_raw_cards(&temp))return 0;}')
p.write_text(t)
