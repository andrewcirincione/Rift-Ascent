"""Apply audited rarity metadata; never change card effects or energy costs."""
from pathlib import Path
import json,re
q=Path(__file__).parent;p=q/'source/game.c';s=p.read_text();meta=json.loads((q/'rarity-reference.json').read_text());rarity={x['name']:x['rarity'] for x in meta['cards']}
pat=r'(\{"([^"]+)","[^"\n]*",-?\d+,)(\d+)(,\d+\})'
s=re.sub(pat,lambda m:m[1]+str(rarity.get(m[2],int(m[3])))+m[4],s)
s=s.replace('id!=CARD_GIANT_ROCK&&id!=63&&id!=85&&id!=56&&id!=59;', 'id!=CARD_GIANT_ROCK&&id!=63&&id!=85&&cards[id].rarity<=2;')
# Older boss-offer saves may contain cards formerly misclassified as Rare.
needle=' *g=temp;return 1;'
fix=''' if(temp.battle_type==NODE_BOSS&&temp.win_screen&&
    (temp.screen==REWARD||temp.screen==REWARD_PREVIEW||(temp.screen==DECK_VIEW&&temp.prior_screen==REWARD))){
  int invalid=0;for(int i=0;i<3;i++)invalid|=cards[CARD_ID(temp.reward[i])].rarity!=2;
  if(invalid)fill_rewards(&temp);
 }
'''
assert needle in s;s=s.replace(needle,fix+needle);p.write_text(s)
p=q/'source/render.c';s=p.read_text().replace('header(g,"SPOILS OF BATTLE / CHOOSE ONE CARD");','header(g,g->battle_type==NODE_BOSS?"BOSS SPOILS / CHOOSE ONE RARE CARD":"SPOILS OF BATTLE / CHOOSE ONE CARD");')
# The existing bottom text-safe margin can hold a seven-pixel rarity label on offers.
s=s.replace('card_render(x,y,126,166,g->reward[i],chosen,1);', '''card_render(x,y,126,166,g->reward[i],chosen,1);
  int rarity=cards[CARD_ID(g->reward[i])].rarity;
  r134_center(x+63,y+148,rarity==2?"RARE":rarity==1?"UNCOMMON":"COMMON",rarity==2?GOLD:rarity==1?BLUE:WHITE,&r138_title7);''')
p.write_text(s)
