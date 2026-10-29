/* Damage/Block values come from the real resolver. Only the rule's relevant
   numeric field is replaced; costs, draw counts and Poison remain distinct. */
static void ninja_replace_value(char*out,int cap,const char*prefix,const char*value){
 char temp[384];snprintf(temp,sizeof(temp),"%s",out);char*p=temp;size_t len=strlen(prefix);for(;*p;p++){size_t n=0;while(prefix[n]&&p[n]&&prefix[n]==p[n])n++;if(!prefix[n])break;}if(!*p)return;p+=len;
 if(*p<'0'||*p>'9')return;char*end=p;while(*end>='0'&&*end<='9')end++;
 int at=(int)(p-temp);if(at>=cap)return;memcpy(out,temp,(size_t)at);snprintf(out+at,(size_t)(cap-at),"%s%s",value,end);
}
static void ninja_description_now(const Game*g,int raw,int target,const CardPreview*p,char*out,int cap){
 int id=CARD_ID(raw),up=CARD_UP(raw),seen=0,low=0,high=0;
 int all=p->repeated||ninja_cards[id-NINJA_FIRST].target>=2||(id==N_SHIV&&g->ninja.v[NS_FAN]);
 for(int e=0;e<g->enemy_count;e++)if((all||e==target)&&p->hits[e]){if(!seen||p->low[e]<low)low=p->low[e];if(!seen||p->high[e]>high)high=p->high[e];seen=1;}
 char damage[32],block[32];preview_range(damage,sizeof(damage),low,high);preview_range(block,sizeof(block),p->block_low,p->block_high);
 if(cards[id].type==0){
  if(!seen){int saved_raw=r150_card_raw,saved_attack=r140_attack_context;r150_card_raw=raw;r140_attack_context=1;int n=attack_amount(g,target,ninja_base_damage(g,raw,1));r150_card_raw=saved_raw;r140_attack_context=saved_attack;snprintf(damage,sizeof(damage),"%d",n);}
  if(p->uncertain)snprintf(damage,sizeof(damage),"variable");
  ninja_replace_value(out,cap,"Deal ",damage);
  if(id==N_MEMENTO_MORI)snprintf(out,cap,"Deal %s damage after modifiers. Base: %d + %d per card discarded this turn (%d now).",damage,9+2*up,4+up,g->ninja.v[NS_DISCARDS_TURN]);
  if(id==N_MURDER)snprintf(out,cap,"Deal %s damage after modifiers. Base: 1 + cards drawn this combat (%d now).",damage,g->ninja.v[NS_DRAWS_COMBAT]);
  if(id==N_PRECISE_CUT)snprintf(out,cap,"Deal %s damage after modifiers. Base: %d minus 2 per other card in hand.",damage,13+3*up);
  if(id==N_FINISHER)snprintf(out,cap,"Deal %s damage per hit. Hit once per prior Attack this turn (%d now).",damage,g->ic[IC_ATTACKS_THIS_TURN]);
  if(id==N_FLECHETTES){int n=0;for(int i=0;i<g->hand_count;i++)n+=cards[CARD_ID(g->hand[i])].type==1;snprintf(out,cap,"Deal %s damage %d times, once per Skill in your hand.",damage,n);}
  if(id==N_SKEWER)snprintf(out,cap,"Deal %s damage %d times (X).",damage,g->energy);
 }
 if(p->block_count){
  if(id==N_MIRAGE)snprintf(out,cap,"Gain %s Block after modifiers, based on Poison on all enemies. Exhaust.",block);
  else ninja_replace_value(out,cap,"Gain ",block);
 }
 if(id==N_ESCAPE_PLAN){int amount=MAX(0,3+2*up+g->dexterity+(relic(g,15)?2:0)+(CARD_MOD(raw)==MOD_WARD?4:0));for(int i=0;i<MIN(20,g->ninja.v[NS_SHADOWMELD])&&amount<999;i++)amount*=2;snprintf(out,cap,"Draw 1 card. If it is a Skill, gain %d Block.",MIN(999-g->block,amount));}
 if(id==N_SHIV&&g->ninja.v[NS_FAN])snprintf(out,cap,"Deal %s damage to ALL enemies. Exhaust.",damage);
 if(id==N_MALAISE)snprintf(out,cap,"Enemy loses %d Strength. Apply %d Weak. Exhaust.",g->energy+up,g->energy+up);
 if(relic(g,NR_SKULL)){
  int poison=id==N_DEADLY_POISON?5+2*up:id==N_POISONED_STAB?3+up:id==N_BOUNCING_FLASK?3:id==N_BUBBLE_BUBBLE?9+3*up:id==N_HAZE?4+2*up:id==N_SNAKEBITE?7+3*up:0;
  if(poison){char value[24];snprintf(value,sizeof(value),"%d",poison+1);ninja_replace_value(out,cap,"Apply ",value);ninja_replace_value(out,cap,"apply ",value);}
 }
 if(raw&CARD_INKY){int n=(int)strlen(out);snprintf(out+n,cap-n," Inky: unblocked hits apply 1 Weak.");}
 if((raw&(CARD_SLY|CARD_SLY_TURN))&&!(ninja_flags(raw)&8)){int n=(int)strlen(out);snprintf(out+n,cap-n,(raw&CARD_SLY_TURN)?" Sly this turn.":" Sly.");}
 if(id==N_SHIV&&g->ninja.v[NS_PHANTOM]){int n=(int)strlen(out);snprintf(out+n,cap-n," Retain.");}
 int repeats=cards[id].type==0?((CARD_MOD(raw)==MOD_ECHO)+(g->ic[IC_NEXT_ATTACK_REPEAT]>0)):cards[id].type==1?(g->ninja.v[NS_BURST]>0):0;
 if(repeats){int n=(int)strlen(out);snprintf(out+n,cap-n,repeats==1?" Plays twice.":" Plays %d times.",1+repeats);}
 append_mod_rules(raw,out,cap,1);
}
