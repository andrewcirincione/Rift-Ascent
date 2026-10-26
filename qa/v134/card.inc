static void card_render(int x,int y,int w,int h,int raw,int selected,int active){
 int id=CARD_ID(raw);if(id<0||id>=CARD_COUNT)return;
 int small=h<105,type=CLAMP(cards[id].type,0,3),inset=small?8:14,inner=w-2*inset;
 const uint32_t face[4]={RGB(25,19,22),RGB(16,25,33),RGB(26,19,33),RGB(20,27,22)};
 rect(x+4,y+5,w-8,h-10,face[type]);
 char title[64];snprintf(title,sizeof(title),"%s%s",cards[id].name,CARD_UP(raw)?"+":"");
 const RiftFont *titlefont=small?&r134_f8:&r134_f9b;
 int ty=y+(small?7:12),titlelead=small?9:11,arty=y+(small?27:37),arth=small?17:CLAMP(h/4-3,25,39);
 r134_card_art(x+inset,arty,inner,arth,id);
 /* Frame is composited after the image. Its alpha is never overwritten by art. */
 r134_blit(x,y,w,h,r134_card_frames[type],126,166,255);
 r134_wrap(x+inset,ty,inner,2,titlelead,title,selected?RGB(255,230,180):WHITE,titlefont,0);
 char cost[12];int energy=card_combat_context?game_card_cost_now(card_combat_context,raw):game_card_cost(raw);
 if(energy==-1)snprintf(cost,sizeof(cost),"X");else if(energy==-2)snprintf(cost,sizeof(cost),"-");else snprintf(cost,sizeof(cost),"%d",energy);
 int orb=small?14:18;r134_blit(x+inset,arty,orb,orb,r134_energy,46,46,255);
 r134_center(x+inset+orb/2,arty+(small?2:4),cost,WHITE,&r134_f9b);
 if(card_combat_context&&type==0){int damage=rift132_preview_damage(card_combat_context,raw);if(damage>0){
  char value[16];snprintf(value,sizeof(value),"%d",damage);int width=r134_width(value,&r134_f9b);
  dark_rect(x+w-inset-width-4,arty,width+4,12,190);r134_text(x+w-inset-width-2,arty+1,value,GOLD,&r134_f9b);
 }}
 int by=arty+arth+(small?3:6),bottom=y+h-(small?8:13),lead=small?9:11;
 const RiftFont*body=small?&r134_f8:&r134_f9;const char*rules=rift131_clean_rules(card_description(raw));
 char check[24][192];int rows=(bottom-by)/lead;
 if(!small&&r134_lines(rules,inner,body,check)>rows){body=&r134_f8;lead=10;rows=(bottom-by)/lead;}
 if(rows>0)r134_wrap(x+inset,by,inner,rows,lead,rules,WHITE,body,small);
 /* Keep text legible on unaffordable cards; only dim their painting. */
 if(!active)dark_rect(x+inset,arty,inner,arth,96);
}
