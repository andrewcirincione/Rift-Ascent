/* A modification lives in the card's raw value, beside its upgrade bit.
   Pile moves, copies and upgrades preserve it; no parallel index can drift. */
static const char *mod_names[MOD_COUNT]={"None","Echo","Bound","Keen","Ward","Fleeting","Kindled"};
static const char *mod_rules[MOD_COUNT]={"",
 "Echo: costs 1 more Energy; plays twice.",
 "Bound: Retain at turn end.",
 "Keen: +2 base damage per hit.",
 "Ward: +4 base Block per card effect.",
 "Fleeting: costs 1 less Energy. Exhaust.",
 "Kindled: draw 1 extra card. Exhaust."};
const char *game_mod_name(int mod){return mod>=0&&mod<MOD_COUNT?mod_names[mod]:"Invalid";}
const char *game_mod_description(int mod){return mod>=0&&mod<MOD_COUNT?mod_rules[mod]:"";}
int game_card_cost(int raw){
 int cost=card_cost_base(raw);if(cost<0)return cost;
 if(CARD_MOD(raw)==MOD_ECHO)cost++;
 if(CARD_MOD(raw)==MOD_FLEETING&&cost>0)cost--;
 return cost;
}
int game_can_modify(int raw,int mod){
 int id=CARD_ID(raw);
 if(id>=CARD_COUNT||id==63||id==85||id==CARD_GIANT_ROCK||
    (id>=CARD_STATUS_MIN&&id<=CARD_STATUS_MAX)||CARD_MOD(raw)||mod<=0||mod>=MOD_COUNT)return 0;
 int type=cards[id].type;
 switch(mod){
 case MOD_ECHO:return type==0&&card_cost_base(raw)>=0&&id!=25;
 case MOD_BOUND:return 1;
 case MOD_KEEN:return type==0;
 case MOD_WARD:switch(id){case 1:case 5:case 9:case 12:case 16:case 17:case 23:case 24:case 31:case 45:case 46:case 58:case 93:return 1;default:return 0;}
 case MOD_FLEETING:return type!=2&&card_cost_base(raw)>0&&id!=25;
 case MOD_KINDLED:return type!=2&&id!=17&&id!=18&&id!=23&&id!=25&&id!=43;
 default:return 0;
 }
}
static void append_mod_rules(int raw,char*out,int cap,int live){
 int mod=CARD_MOD(raw);if(!mod||mod>=MOD_COUNT||cap<=0)return;
 const char *rules=game_mod_description(mod);
 if(live&&mod==MOD_ECHO)rules="Echo: +1 Energy cost.";
 if(live&&mod==MOD_KEEN)rules="Keen: +2 base/hit included.";
 if(live&&mod==MOD_WARD)rules="Ward: +4 base Block included.";
 int used=(int)strlen(out);if(used<cap)snprintf(out+used,(size_t)(cap-used)," %s",rules);
}
const char *card_description(int raw){
 static char text[8][384];static unsigned slot;
 char*out=text[(slot++)%8];snprintf(out,384,"%s",card_description_base(raw));
 append_mod_rules(raw,out,384,0);return out;
}
