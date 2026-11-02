/* A modification lives in the card's raw value, beside its upgrade bit.
   Pile moves, copies and upgrades preserve it; no parallel index can drift. */
static const char *mod_names[MOD_COUNT]={"None","Twinwoven","Vowbound","Embercut","Rivetward","Ashspent","Loreburn","Dawnset"};
static const char *mod_rules[MOD_COUNT]={"",
 "Twinwoven: costs 1 more Energy; plays twice.",
 "Vowbound: Retain at turn end.",
 "Embercut: +2 base damage per hit.",
 "Rivetward: +4 base Guard per card effect.",
 "Ashspent: costs 1 less Energy. Exile.",
 "Loreburn: draw 1 extra card. Exile.",
 "Dawnset: starts in your opening hand, within the hand limit."};
const char *game_mod_name(int mod){return mod>=0&&mod<MOD_COUNT?mod_names[mod]:"Invalid";}
const char *game_mod_description(int mod){return mod>=0&&mod<MOD_COUNT?mod_rules[mod]:"";}
int game_card_cost(int raw){
 int cost=card_cost_base(raw);if(cost<0)return cost;
 if(CARD_MOD(raw)==MOD_ECHO)cost++;
 if(CARD_MOD(raw)==MOD_FLEETING&&cost>0)cost--;
 return cost;
}
int game_can_modify(int raw,int mod){
 int id=CARD_ID(raw);if(id>=CARD_COUNT||!game_card_is_playable(raw)||CARD_MOD(raw)||mod<=0||mod>=MOD_COUNT)return 0;
 int type=cards[id].type;
 switch(mod){
 case MOD_ECHO:return type<2&&card_cost_base(raw)>=1&&!dist_has_op(raw,DC_HEAL);
 case MOD_BOUND:case MOD_PRIMED:return 1;
 case MOD_KEEN:return type==0;
 case MOD_WARD:return dist_has_op(raw,DC_GUARD)||dist_has_op(raw,DC_VENT_GUARD)||dist_has_op(raw,DC_THREAT_GUARD)||dist_has_op(raw,DC_LOW_GUARD)||dist_has_op(raw,DC_RESERVE_GUARD)||dist_has_op(raw,DC_FOLD);
 case MOD_FLEETING:return type!=2&&card_cost_base(raw)>0;
 case MOD_KINDLED:return type!=2;
 default:return 0;
 }
}
static void append_mod_rules(int raw,char*out,int cap,int live){
 int mod=CARD_MOD(raw);if(!mod||mod>=MOD_COUNT||cap<=0)return;
 const char *rules=game_mod_description(mod);
 if(live&&mod==MOD_ECHO)rules="Twinwoven: +1 Energy cost.";
 if(live&&mod==MOD_KEEN)rules="Embercut: +2 base/hit included.";
 if(live&&mod==MOD_WARD)rules="Rivetward: +4 base Guard included.";
 int used=(int)strlen(out);if(used<cap)snprintf(out+used,(size_t)(cap-used),"%s %s",used&&out[used-1]!='.'?".":"",rules);
}
const char *card_description(int raw){
 static char text[8][384];static unsigned slot;
 char*out=text[(slot++)%8];snprintf(out,384,"%s",card_description_base(raw));
 append_mod_rules(raw,out,384,0);return out;
}
