from pathlib import Path
q=Path(__file__).parent/'source';p=q/'game.c';s=p.read_text().replace('Cost 1. Gain 1 Energy per Attack in Hand. Lock Energy.','Gain 1 Energy per Attack in your hand. Then you cannot gain Energy this turn.');p.write_text(s)
p=q/'render.c';s=p.read_text().replace('#include "rift138_art.h"','#include "rift138_art.h"\n#include "rift139_art.h"')
s=s.replace('r134_blit(x,y,20,22,r134_map_icons[7],36,36,255);','r134_blit(x-4,y-4,28,30,r139_shield,28,30,255);')
s=s.replace(' rect(x+4,y+5,w-8,h-10,face[type]);',' int grid=small&&w>=98;\n if(grid&&selected)y-=5;\n rect(x+4,y+5,w-8,h-10,grid&&selected?RGB(74,20,30):face[type]);')
s=s.replace(' if(rows>0)r134_wrap(x+inset,by,inner,rows,lead,rules,WHITE,body,small);',' if(rows>0)r134_wrap(x+inset,by,inner,rows,lead,rules,WHITE,body,small);\n if(grid){if(selected)r134_blit(x+w/2-20,y-9,40,23,r138_choice,42,28,255);else dark_rect(x+3,y+4,w-6,h-8,70);}')
s=s.replace(' button(416,215,62,29,"END",g->selection>=g->hand_count);',' if(g->selection>=g->hand_count){r134_blit(411,213,66,31,r139_end,66,31,255);r134_center(444,224,"END",RGB(40,13,12),&r134_f10b);}else button(416,215,62,29,"END",0);')
s=s.replace('"Restore up to 20 HP. Pay up to 40 gold; partial payment allowed."','"Pay 2 Gold per HP restored, up to 20 HP. UP/DOWN adjusts the amount."')
s=s.replace(' r134_wrap(137,145,294,5,12,rift113_event_details[id],GOLD,&r134_f9,0);',' if(id==1){char quote[140];int amount=game_moon_heal_now(g);snprintf(quote,sizeof(quote),"Heal %d HP for %d Gold.\\nUP/DOWN adjusts (2 Gold per HP, up to 20 HP).",amount,amount*2);r134_wrap(137,145,294,5,12,quote,GOLD,&r134_f9,0);}\n else r134_wrap(137,145,294,5,12,rift113_event_details[id],GOLD,&r134_f9,0);')
s=s.replace('if(g->relics[i]==6)discount=15;','if(g->relics[i]==6)discount=30;')
s=s.replace('if(chosen)gilded_panel(23,y-1,208,26,1);','if(chosen)r134_blit(20,y-2,214,29,r139_select,208,26,255);')
s=s.replace('r134_text(36,y+8,label,chosen?GOLD:WHITE,&r134_f9);','r134_wrap(chosen?56:36,y+8,chosen?175:195,1,10,label,chosen?WHITE:MUTED,&r134_f9,0);')
s=s.replace('footer("LEFT/RIGHT SELECT    X TAKE CARD OR SKIP");','footer("LEFT/RIGHT CHOOSE  X TAKE  TRI VIEW  SQ DECK");')
a=s.index('static void relic_screen(');b=s.index('static void battle_piles_screen',a)
s=s[:a]+'''static void relic_screen(const Game*g){
 header(g,"RELICS / ITEMS DISCOVERED");gilded_panel(16,43,225,207,0);gilded_panel(244,43,220,207,1);
 int first=CLAMP(g->selection-2,0,g->relic_count>6?g->relic_count-6:0);
 for(int i=first;i<g->relic_count&&i<first+6;i++){int y=56+(i-first)*30,chosen=i==g->selection;
  if(chosen)r134_blit(23,y-4,208,26,r139_select,208,26,255);
  r134_text(chosen?59:36,y+5,relic_defs[g->relics[i]].name,chosen?WHITE:MUTED,&r134_f9);
 }
 if(g->relic_count){int id=g->relics[g->selection];r134_blit(332,59,40,40,r134_utility_icons[1],40,40,255);r134_wrap(264,111,178,2,13,relic_defs[id].name,GOLD,&r138_title10,0);r134_wrap(264,144,178,7,12,relic_defs[id].desc,WHITE,&r134_f9,0);}
 footer("UP/DOWN BROWSE   X/CIRCLE RETURN");
}
static void blessing_screen(const Game*g){
 char title[60];snprintf(title,sizeof(title),"ACT %d / CHOOSE YOUR BLESSING",g->act);header(g,title);
 for(int i=0;i<3;i++){int x=22+i*149,id=g->reward[i],chosen=i==g->selection;
  gilded_panel(x,48,140,196,chosen);r134_blit(x+49,69,42,42,r134_utility_icons[1],40,40,255);
  r134_wrap(x+13,119,114,2,13,relic_defs[id].name,GOLD,&r138_title9,0);
  r134_wrap(x+13,154,114,7,11,relic_defs[id].desc,WHITE,&r134_f9,0);
  if(chosen)r134_blit(x+49,39,42,28,r138_choice,42,28,255);else dark_rect(x+7,55,126,182,70);
 }
 footer("LEFT/RIGHT CHOOSE   X ACCEPT BLESSING");
}
static void reward_preview_screen(const Game*g){
 header(g,"CARD REWARD / INSPECT");int raw=g->reward[g->selection];
 R135CardLayout layout=r135_card_layout(raw,174);card_render(153,55,174,MAX(174,layout.height),raw,1,1);
 const char*rarity=cards[CARD_ID(raw)].rarity==2?"RARE":cards[CARD_ID(raw)].rarity==1?"UNCOMMON":"COMMON";
 r134_center(240,237,rarity,GOLD,&r134_f9b);footer("LEFT/RIGHT BROWSE   O RETURN");
}
'''+s[b:]
s=s.replace('case REWARD:victory_replay=0;reward_screen(g);break;','case REWARD:victory_replay=0;reward_screen(g);break;\n case BLESSING:blessing_screen(g);break;\n case REWARD_PREVIEW:reward_preview_screen(g);break;')
p.write_text(s)
