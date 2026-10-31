/* 1.3.6: observe the real card resolver on an isolated copy. The observer
 * records direct card hits before enemy Block, and actual Block gained after
 * the 999 cap. Reactive powers are not silently folded into attack damage.
 * Random-target and random-exhaust cards enumerate their possible choices.
 * Hidden autoplay/draw chains are explicitly unresolved, never predicted by
 * looking ahead in the live RNG. */
static void preview_once(const Game*g,int raw,int target,int choice,CardPreview*out){
 Game copy=*g;int index=-1;
 memset(out,0,sizeof(*out));
 if(copy.selection>=0&&copy.selection<copy.hand_count&&copy.hand[copy.selection]==raw)index=copy.selection;
 for(int i=0;index<0&&i<copy.hand_count;i++)if(copy.hand[i]==raw)index=i;
 if(index<0)return;
 copy.screen=COMBAT;
 int cost=game_card_cost_now(&copy,raw);
 /* Insufficient Energy should not hide damage / Block modifiers. X retains
    the actual available Energy; it is never replaced by an invented value. */
 if(cost>copy.energy)copy.energy=cost;
 int stars=game_card_star_cost(&copy,raw);if(stars>copy.regent.stars)copy.regent.stars=stars;
 preview_trace=out;preview_depth=preview_phase=preview_nested=0;preview_choice=choice;
 game_play_card(&copy,index,target);
 if(copy.screen==NINJA_CHOICE){for(int i=0;i<copy.ninja.task_count;i++)if(copy.ninja.tasks[i].kind==NT_PLAY&&copy.ninja.tasks[i].depth==2)out->uncertain=1;}
 preview_trace=0;preview_depth=preview_phase=preview_nested=0;preview_choice=-1;
}
void game_card_preview(const Game*g,int raw,int target,CardPreview*out){
 memset(out,0,sizeof(*out));
 int id=CARD_ID(raw);
 if(!g||id>=CARD_COUNT||g->screen!=COMBAT)return;
 int choices=1;
 if(id==22||id==N_RICOCHET||id==R_STARDUST||id==C_VOLLEY){choices=0;for(int e=0;e<g->enemy_count;e++)if(g->enemies[e].hp>0)choices++;}
 if(id==87){choices=0;int skipped=0;
  for(int i=0;i<g->hand_count;i++){
   if(!skipped&&g->hand[i]==raw){skipped=1;continue;}
   if(cards[CARD_ID(g->hand[i])].type==0)choices++;
  }
 }
 if(choices<1)choices=1;
 for(int c=0;c<choices;c++){
  CardPreview p;preview_once(g,raw,target,(id==22||id==87||id==N_RICOCHET||id==R_STARDUST||id==C_VOLLEY)?c:-1,&p);
  if(!c){*out=p;continue;}
  for(int e=0;e<MAX_ENEMIES;e++)if(p.hits[e]){
   if(!out->hits[e]||p.low[e]<out->low[e])out->low[e]=p.low[e];
   if(!out->hits[e]||p.high[e]>out->high[e])out->high[e]=p.high[e];
   out->hits[e]=MAX(out->hits[e],p.hits[e]);
   out->total[e]=MAX(out->total[e],p.total[e]);
  }
  if(p.block_count){
   if(!out->block_count||p.block_low<out->block_low)out->block_low=p.block_low;
   out->block_high=MAX(out->block_high,p.block_high);
   out->block_count=MAX(out->block_count,p.block_count);
  }
  out->uncertain|=p.uncertain;out->repeated=MAX(out->repeated,p.repeated);
 }
 out->random=(id==22||id==87||id==N_RICOCHET||id==R_STARDUST||id==C_VOLLEY)&&choices>1;
}
static void preview_range(char*out,int cap,int low,int high){
 if(low==high)snprintf(out,(size_t)cap,"%d",low);
 else snprintf(out,(size_t)cap,"%d-%d",low,high);
}
static void preview_replace_number(char*out,int cap,const char*source,int ordinal,const char*value){
 int at=0,start=-1,end=-1;
 for(int i=0;source[i];i++)if(source[i]>='0'&&source[i]<='9'){
  int first=i;while(source[i+1]>='0'&&source[i+1]<='9')i++;
  if(at++==ordinal){start=first;end=i+1;break;}
 }
 if(start<0){snprintf(out,(size_t)cap,"%s",source);return;}
 int n=MIN(start,cap-1);memcpy(out,source,(size_t)n);out[n]=0;
 snprintf(out+n,(size_t)(cap-n),"%s%s",value,source+end);
}
#include "ninja_preview.inc"
#include "regent_preview.inc"
void game_card_description_now(const Game*g,int raw,int target,char*out,int cap){
 if(!out||cap<=0)return;
 const char*base=card_description_base(raw);int id=CARD_ID(raw),up=CARD_UP(raw);
 snprintf(out,(size_t)cap,"%s",card_description(raw));
 if(!g||g->screen!=COMBAT||id>=CARD_COUNT)return;
 int in_hand=0;for(int i=0;i<g->hand_count;i++)if(g->hand[i]==raw)in_hand=1;
 if(!in_hand)return;
 snprintf(out,(size_t)cap,"%s",base);
 if(target<0||target>=g->enemy_count||g->enemies[target].hp<=0){
  target=0;while(target+1<g->enemy_count&&g->enemies[target].hp<=0)target++;
 }
 CardPreview p;game_card_preview(g,raw,target,&p);
 if(id>=ASCENT_CARD_FIRST){append_mod_rules(raw,out,cap,0);return;}
 if(id>=REGENT_FIRST){regent_description_now(g,raw,target,&p,out,cap);return;}
 if(id>=NINJA_FIRST&&id<NINJA_LAST){ninja_description_now(g,raw,target,&p,out,cap);return;}
 char damage[32],block[32],temp[320];int low=0,high=0,seen=0;
 int all=id==3||id==10||id==14||id==19||id==22||id==36||id==37||id==70||p.repeated;
 for(int e=0;e<g->enemy_count;e++)if((all||e==target)&&p.hits[e]){
  if(!seen||p.low[e]<low)low=p.low[e];if(!seen||p.high[e]>high)high=p.high[e];seen=1;
 }
 preview_range(damage,sizeof(damage),low,high);
 preview_range(block,sizeof(block),p.block_low,p.block_high);
 if(p.uncertain){snprintf(damage,sizeof(damage),"variable");snprintf(block,sizeof(block),"variable");}
 if(cards[id].type==0){
  switch(id){
   case 92:snprintf(out,(size_t)cap,"Remove %d enemy Block. Deal %s damage.",6+3*up,damage);break;
   case 12:snprintf(out,(size_t)cap,"Gain %s Block. Deal %s damage.",block,damage);break;
   case 13:snprintf(out,(size_t)cap,"Deal %s damage after modifiers. Base: 6 + %d per card in your Exhaust pile (%d now).",damage,3+up,g->exhaust_count);break;
   case 21:snprintf(out,(size_t)cap,"Deal %s damage, based on your Block.",damage);break;
   case 35:snprintf(out,(size_t)cap,"Deal %s damage after modifiers. Base: 4 + %d per turn of Vulnerable on the target (%d now).",damage,2+up,g->enemies[target].vulnerable);break;
   case 42:{int count=MAX(0,g->hand_count-1);snprintf(out,(size_t)cap,"Exhaust hand. Deal %s damage %d times. Exhaust.",damage,count);break;}
   case 49:snprintf(out,(size_t)cap,"Deal %s damage (includes %d owned Strikes).",damage,strike_count(g));break;
   case 79:snprintf(out,(size_t)cap,"Deal %s damage. Rampage gains %d base damage this combat.",damage,5+4*up);break;
   case 81:snprintf(out,(size_t)cap,"Deal %s damage %d time%s. Extra hits if HP lost this turn.",damage,g->ic[IC_HPLOSS_TURN]?2+up:1,g->ic[IC_HPLOSS_TURN]?"s":"");break;
   case 86:snprintf(out,(size_t)cap,"Deal %s damage %d times (1 + HP-loss events, max 20).",damage,MIN(20,1+g->ic[IC_HPLOSS_COMBAT]));break;
   case 87:snprintf(out,(size_t)cap,"Exhaust a random Attack; add its base damage. Deal %s twice.",damage);break;
   default:preview_replace_number(out,cap,base,(id==14||id==29||id==37)?1:0,damage);break;
  }
  if(id==10){snprintf(temp,sizeof(temp),"Deal %s damage to all enemies %d times (X).",damage,g->energy);snprintf(out,(size_t)cap,"%s",temp);}
  if(p.repeated){int n=(int)strlen(out);snprintf(out+n,(size_t)(cap-n),p.repeated==1?" Plays twice.":" Plays %d times.",1+p.repeated);}
 }else if(p.block_count){
  if(id==93){snprintf(out,(size_t)cap,"Gain %s Block. Includes +4 if any enemy intends to attack.",block);}
  else if(id==16){char second[20];snprintf(second,sizeof(second),"%d",p.block_count>1?p.block_second:MAX(0,8+3*up+g->dexterity+(relic(g,15)?2:0)+(CARD_MOD(raw)==MOD_WARD?4:0)));
   snprintf(out,(size_t)cap,"Gain %d Block. If a card exhausted this turn, gain %s more Block.",p.block_first,second);
  }else if(id==45)snprintf(out,(size_t)cap,"Exhaust all non-Attacks. Gain %s Block each (%d now).",block,p.block_total);
  else preview_replace_number(out,cap,base,id==46?1:0,block);
 }else if(id==33){
  int n=MAX(0,3+2*up+g->dexterity+(relic(g,15)?2:0));
  snprintf(out,(size_t)cap,"Each Attack this turn grants %d Block.%s",n,
   g->ic[IC_UNMOVABLE]&&!g->ic[IC_UNMOVABLE_USED]?" Unmovable can double it.":"");
 }
 append_mod_rules(raw,out,cap,1);
}
static int enemy_status_entries(const Game*g,int enemy,int wanted,char*out,int cap){
 if(!g||enemy<0||enemy>=g->enemy_count)return 0;
 const Enemy*e=&g->enemies[enemy];int n=0;
#define FOE_STATUS(value,format) do{int v=(value);if(v){if(n==wanted&&out&&cap>0)snprintf(out,(size_t)cap,format,v);n++;}}while(0)
 FOE_STATUS(e->strength,"Strength: %d");
 if(relic(g,NR_KRANE)){FOE_STATUS(e->weak,"Weak: %d turns (-40%% damage)");}else{FOE_STATUS(e->weak,"Weak: %d turns (-25%% damage)");}
 FOE_STATUS(e->vulnerable,"Vulnerable: %d turns (+50%% taken)");
 FOE_STATUS(e->poison,"Poison: %d HP on next trigger");
 FOE_STATUS((*STRANGLE_SLOT(g,enemy)),"Strangle: lose %d HP per card this turn");
 FOE_STATUS((*MANGLE_SLOT(g,enemy)),"Mangle: restore %d STR next turn");
#undef FOE_STATUS
 return n;
}
int game_enemy_status_count(const Game*g,int e){return enemy_status_entries(g,e,-1,0,0);}
void game_enemy_status_line(const Game*g,int e,int index,char*out,int cap){
 if(out&&cap>0)out[0]=0;enemy_status_entries(g,e,index,out,cap);
}
