from pathlib import Path
p=Path('GAME/src/render.c');s=p.read_text()
def function(name,new,nextname):
 global s
 a=s.index('static void '+name+'(');b=s.index('static void '+nextname+'(',a);s=s[:a]+new+'\n'+s[b:]
function('boss_reward_screen',r'''static void boss_reward_screen(const Game*g){
 header(g,"GUARDIAN DEFEATED / SELECT A RELIC");
 for(int i=0;i<3;i++){
  int x=18+i*150,y=47,selected=g->selection==i;
  gilded_panel(x,y,144,178,selected);
  r134_blit(x+47,y+12,50,50,r134_utility_icons[1],40,40,selected?255:180);
  if(g->reward[i]>=0){
   r134_wrap(x+13,y+69,118,2,12,relic_defs[g->reward[i]].name,GOLD,&r134_f10,0);
   r134_wrap(x+13,y+98,118,5,12,relic_defs[g->reward[i]].desc,WHITE,&r134_f9,0);
  }
 }
 button(195,229,90,25,"SKIP",g->selection==3);
 footer("LEFT/RIGHT SELECT  X CONFIRM  O SKIP");
}''','rest_screen')
a=s.index('static void rest_screen');b=s.index('static const char*event_titles',a)
s=s[:a]+r'''static void rest_screen(const Game*g){
 header(g,"REST SITE / A MOMENT OF PEACE");
 for(int i=0;i<2;i++){
  int x=30+i*230;gilded_panel(x,47,190,199,g->selection==i);
  r134_blit(x+55,69,80,80,i?r134_event_icons[10]:r134_map_icons[4],i?80:36,i?80:36,255);
  r134_center(x+95,158,i?"TEMPER YOUR STEEL":"THE EMBERS REMAIN",GOLD,&r134_f9b);
  button(x+12,194,166,35,i?"FORGE - UPGRADE":"REST - HEAL 30%",g->selection==i);
 }
 footer("LEFT/RIGHT SELECT    X CONFIRM");
}
'''+s[b:]
function('event_screen',r'''static void event_screen(const Game*g){
 header(g,"A CHOICE IN THE DARK");
 int id=CLAMP(g->effect_turn,0,11);gilded_panel(20,42,440,210,1);
 r134_center(240,57,event_titles[id],GOLD,&r134_f18b);
 r134_blit(38,91,80,80,r134_event_icons[id],80,80,255);
 r134_wrap(137,96,294,3,15,rift_event_lore[id],WHITE,&r134_f10,0);
 r134_wrap(137,145,294,5,12,rift113_event_details[id],GOLD,&r134_f9,0);
 button(51,214,174,29,"ACCEPT",g->selection==0);
 button(255,214,174,29,"LEAVE",g->selection==1);
 footer("LEFT/RIGHT CHOOSE    X CONFIRM");
}''','shop_screen')
s=s.replace('gilded_panel(246,51,222,190,1);','gilded_panel(10,44,230,204,0);\n gilded_panel(246,44,224,204,1);')
s=s.replace('card_render(291,60,132,143','card_render(295,51,126,166')
s=s.replace('text(257,207,cost,','text(270,227,cost,')
s=s.replace('r134_blit(400,72,36,36,','r134_blit(337,65,40,40,')
s=s.replace('text(257,62,kind,GOLD,1);\n   fittext(257,81,title,29,WHITE);','r134_center(358,112,title,GOLD,&r134_f10);')
s=s.replace('wrapped_text(257,117,caption,27,7,WHITE);','r134_wrap(265,134,186,6,13,caption,WHITE,&r134_f10,0);')
function('potion_screen',r'''static void potion_screen(const Game*g){
 header(g,"POTION BELT");
 for(int i=0;i<g->potion_count;i++){
  int y=53+i*61;gilded_panel(27,y,426,58,i==g->selection);
  r134_blit(42,y+8,36,36,r134_utility_icons[0],40,40,255);
  r134_text(89,y+10,game_potion_name(g->potions[i]),GOLD,&r134_f10);
  r134_wrap(89,y+27,341,2,12,game_potion_description(g->potions[i]),WHITE,&r134_f9,0);
 }
 if(!g->potion_count)center(240,110,"NO POTIONS",MUTED,1);
 footer("UP/DOWN BROWSE  O RETURN");
}''','relic_screen')
function('relic_screen',r'''static void relic_screen(const Game*g){
 header(g,"RELICS / ITEMS DISCOVERED");
 char b[112];snprintf(b,sizeof(b),"%d RELICS",g->relic_count);text(350,48,b,GOLD,1);
 int first=CLAMP(g->selection-3,0,g->relic_count>6?g->relic_count-6:0);
 for(int i=first;i<g->relic_count&&i<first+6;i++){
  int id=g->relics[i],y=68+(i-first)*29;
  if(i==g->selection)button(25,y-4,430,29,"",1);
  r134_blit(36,y,23,23,r134_utility_icons[1],40,40,255);
  r134_text(66,y,relic_defs[id].name,GOLD,&r134_f9b);
  r134_wrap(66,y+12,369,1,11,relic_defs[id].desc,WHITE,&r134_f8,0);
 }
 footer("UP/DOWN BROWSE   X/CIRCLE RETURN");
}''','battle_piles_screen')
# Full preview retains visual card identity and avoids truncated descriptions.
a=s.index(' gilded_panel(342,70,128,161,0);');b=s.index(' footer(',a)
s=s[:a]+''' if(count)card_render(341,72,126,166,pile[g->selection],1,1);
 else center(402,137,"EMPTY PILE",MUTED,1);
'''+s[b:]
a=s.index(' gilded_panel(344,55,126,188,0);');b=s.index(' footer(',a)
s=s[:a]+r''' if(g->deck_count>0){
   int raw=g->deck[g->selection];
   if(g->prior_screen==REST||g->prior_screen==EVENT){
     gilded_panel(339,48,133,197,1);
     r134_text(352,61,"CURRENT",GOLD,&r134_f9b);
     r134_wrap(352,78,105,6,11,card_description(raw),WHITE,&r134_f9,0);
     r134_text(352,147,"UPGRADED",GOLD,&r134_f9b);
     r134_wrap(352,164,105,6,11,game_card_is_upgradeable(raw)?card_description(raw|UPBIT):"Already upgraded",GREEN,&r134_f9,0);
   }else card_render(341,62,126,166,raw,1,1);
   snprintf(b,sizeof(b),"%d / %d",g->selection+1,g->deck_count);r134_center(404,238,b,GOLD,&r134_f9);
 }
'''+s[b:]
# Use fully drawn card for the card inspector, descriptive text beside it.
a=s.index('   if(g->hand_count>0&&g->selection<g->hand_count){',s.index('static void inspector'))
b=s.index('   }else text(43,99',a)
s=s[:a]+r'''   if(g->hand_count>0&&g->selection<g->hand_count){
      int raw=g->hand[g->selection];card_combat_context=g;
      card_render(39,75,120,160,raw,1,1);card_combat_context=0;
      r134_wrap(181,91,250,5,15,card_description(raw),WHITE,&r134_f10,0);
      r134_text(181,185,"Strength adds damage to each hit.",MUTED,&r134_f9);
      r134_text(181,202,"Weak -25% / Vulnerable +50%",MUTED,&r134_f9);
'''+s[b:]
# True fade: preserve the backdrop and alpha blend the complete moving card.
s=s.replace('card_combat_context=g;card_render(x,y,w,h,shown,1,1);card_combat_context=0;\n     if(age>=26)dark_rect(x,y,w,h,(age-25)*19);',r'''static uint32_t backdrop[112*136];
     int opacity=age<26?255:(38-age)*255/12;
     if(opacity<255)for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++)backdrop[yy*w+xx]=pix[(y+yy)*sw+x+xx];
     card_combat_context=g;card_render(x,y,w,h,shown,1,1);card_combat_context=0;
     if(opacity<255)for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){uint32_t*d=&pix[(y+yy)*sw+x+xx];*d=r134_mix(backdrop[yy*w+xx],*d,opacity);}''')
s=s.replace('if(!selected||playing)card_render','if((!selected||playing)&&!(playing&&selected&&animation_tick-rift120_play_tick<18))card_render')
# Source images have been cropped, not distorted; normal rendering is unscaled at native resolution.
p.write_text(s)
