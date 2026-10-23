#!/usr/bin/env python3
"""1.0.2 item 06: identify and fully describe EVERY merchant item."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
start=s.index("static void shop_screen(const Game*g){")
end=s.index("static void relic_screen(",start)
replacement='''static void shop_screen(const Game*g){
 header(g,"BLACK MARKET / INSPECT BEFORE YOU BUY");
 int discount=0;
 for(int i=0;i<g->relic_count;i++)if(g->relics[i]==6)discount=15;
 char label[90],caption[140],title[52],cost[24];
 int selected=g->selection,price=0;
 gilded_panel(246,51,222,190,1);
 for(int i=0;i<8;i++){
   int y=57+i*23,chosen=(i==selected);
   rect(12,y,224,21,chosen?RGB(93,69,47):RGB(27,36,49));
   border(12,y,224,21,chosen?GOLD:RGB(53,67,79));
   label[0]=0;
   if(i<3){
     if(g->shop_cards[i]>=0)
       snprintf(label,sizeof(label),"CARD  %s",cards[g->shop_cards[i]].name);
     else snprintf(label,sizeof(label),"CARD  SOLD OUT");
   }else if(i==3)snprintf(label,sizeof(label),"RELIC %s",
        g->shop_relic>=0?relic_defs[g->shop_relic].name:"SOLD OUT");
   else if(i==4)snprintf(label,sizeof(label),"SERVICE HEAL 18 HP");
   else if(i==5)snprintf(label,sizeof(label),"SERVICE REMOVE CARD");
   else if(i==6)snprintf(label,sizeof(label),"LEAVE THE STORE");
   else snprintf(label,sizeof(label),"POTION RANDOM");
   fittext(19,y+5,label,30,chosen?GOLD:WHITE);
 }
 const char *kind="SERVICE";caption[0]=0;title[0]=0;
 if(selected<3){
    kind="CARD";
    int id=g->shop_cards[selected];
    if(id<0){snprintf(title,sizeof(title),"SOLD OUT");snprintf(caption,sizeof(caption),"This card has already been purchased.");}
    else{
      snprintf(title,sizeof(title),"%s",cards[id].name);
      snprintf(caption,sizeof(caption),"%s",cards[id].desc);
      price=50+25*cards[id].rarity-discount;
    }
 }else if(selected==3){
    kind="RELIC";
    if(g->shop_relic<0){snprintf(title,sizeof(title),"SOLD OUT");snprintf(caption,sizeof(caption),"The relic has been purchased.");}
    else{
      snprintf(title,sizeof(title),"%s",relic_defs[g->shop_relic].name);
      snprintf(caption,sizeof(caption),"%s",relic_defs[g->shop_relic].desc);
      price=155-discount;
    }
 }else if(selected==4){
    snprintf(title,sizeof(title),"HEALING SERVICE");
    snprintf(caption,sizeof(caption),"Restore 18 HP, up to your maximum health. One purchase per selection.");
    price=45;
 }else if(selected==5){
    snprintf(title,sizeof(title),"CARD REMOVAL");
    snprintf(caption,sizeof(caption),"Choose one card from your deck and remove it permanently.");
    price=60;
 }else if(selected==6){
    snprintf(title,sizeof(title),"LEAVE SHOP");
    snprintf(caption,sizeof(caption),"Keep your purchases and continue along the connected route.");
 }else{
    kind="POTION";
    snprintf(title,sizeof(title),"RANDOM POTION");
    snprintf(caption,sizeof(caption),"Receive one random potion if your three-slot belt has space.");
    price=50;
 }
 text(257,62,kind,GOLD,1);
 fittext(257,81,title,29,WHITE);
 rect(255,106,203,1,GOLD);
 wrapped_text(257,117,caption,27,7,WHITE);
 if(price){
   snprintf(cost,sizeof(cost),"PRICE %d GOLD",price);
   text(257,207,cost,g->gold>=price?GREEN:RED,1);
 }
 footer("UP/DOWN SELECT   X PURCHASE   CIRCLE LEAVE");
}'''
s=s[:start]+replacement+"\n"+s[end:]
p.write_text(s)
print("PASS 06: merchant entries show CARD/RELIC/POTION/SERVICE, full selected description, price.")
