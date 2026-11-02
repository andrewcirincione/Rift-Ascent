static void regent_description_now(const Game*g,int raw,int target,const CardPreview*p,char*out,int cap){
 int id=CARD_ID(raw),up=CARD_UP(raw),seen=0,low=0,high=0;
 for(int e=0;e<g->enemy_count;e++)if(p->hits[e]){if(!seen||p->low[e]<low)low=p->low[e];if(!seen||p->high[e]>high)high=p->high[e];seen=1;}
 if(!seen&&cards[id].type==0){int save=r150_card_raw,attack=r140_attack_context;r150_card_raw=raw;r140_attack_context=1;low=high=attack_amount(g,target,regent_base_damage(g,raw)+g->regent.vigor);r150_card_raw=save;r140_attack_context=attack;}
 char damage[32],block[32];preview_range(damage,sizeof(damage),low,high);preview_range(block,sizeof(block),p->block_low,p->block_high);
 if(p->uncertain)snprintf(damage,sizeof(damage),"variable");
 if(cards[id].type==0)ninja_replace_value(out,cap,"Deal ",damage);
 if(p->block_count)ninja_replace_value(out,cap,"Gain ",block);
 switch(id){
 case R_SOVEREIGN_BLADE:snprintf(out,cap,"Retain. Deal %s damage%s. Forge bonus: %d.%s",damage,g->regent.powers[R_SEEKING_EDGE-REGENT_FIRST]?" to ALL enemies":"",g->regent.forge,g->regent.powers[R_PARRY-REGENT_FIRST]?" Also gains Block from Parry.":"");if(p->block_count){int n=strlen(out);snprintf(out+n,cap-n," Gain %s Block.",block);}break;
 case R_CRESCENT_SPEAR:snprintf(out,cap,"Deal %s damage after modifiers. Base: 8 + %d per Star-cost card you own in combat.",damage,2+up);break;
 case R_SUPERMASSIVE:snprintf(out,cap,"Deal %s damage after modifiers. Base: 5 + %d per card created this combat (%d now).",damage,3+up,g->regent.created);break;
 case R_RADIATE:snprintf(out,cap,"Deal %s damage to ALL enemies after modifiers. Base: %d per Star gained this turn (%d now).",damage,3+up,g->regent.stars_gained);break;
 case R_LUNAR_BLAST:snprintf(out,cap,"Deal %s damage after modifiers. Base: %d per Skill already played this turn (%d now).",damage,4+up,g->regent.skills);break;
 case R_STARDUST:snprintf(out,cap,"Spend all Stars. Deal %s damage to a random enemy %d times (one hit per Star).",damage,g->regent.stars);break;
 case R_HEAVENLY_DRILL:snprintf(out,cap,"Deal %s damage %d times. X uses your Energy; hits double when X is 4 or more.",damage,g->energy>=4?2*g->energy:g->energy);break;
 case C_VOLLEY:snprintf(out,cap,"Deal %s damage to a random enemy %d times (X Energy).",damage,g->energy);break;
 case C_GOLD_AXE:snprintf(out,cap,"%sDeal %s damage after modifiers. Base: %d cards already played this combat.",up?"Retain. ":"",damage,g->regent.plays_combat);break;
 case C_MIND_BLAST:snprintf(out,cap,"Innate. Deal %s damage after modifiers. Base: %d cards in your Draw Pile.",damage,g->draw_count);break;
 case C_REND:snprintf(out,cap,"Deal %s damage after modifiers. Base: %d + %d per unique enemy debuff.",damage,15+3*up,5+3*up);break;
 case C_FISTICUFFS:snprintf(out,cap,"Deal %s damage. Gain %s Block, based on damage dealt and Block modifiers.",damage,block);break;
 case C_PANIC_BUTTON:snprintf(out,cap,"Gain %s Block. You cannot gain Block from cards for 2 turns. Exhaust.",block);break;
 case R_GLITTERSTREAM:{Game copy=*g;copy.ic[IC_UNMOVABLE_USED]=!g->ic[IC_UNMOVABLE_USED];snprintf(out,cap,"Gain %s Block. Next turn, gain %d Block.",block,regent_delayed_block(&copy,raw,5+2*up));break;}
 }
 int repeats=p->repeated;if(repeats){int n=strlen(out);snprintf(out+n,cap-n,repeats==1?" Plays twice.":" Plays %d times.",repeats+1);}
 append_mod_rules(raw,out,cap,1);
}
