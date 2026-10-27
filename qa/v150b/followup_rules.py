from pathlib import Path
s=Path(__file__).parent/'source'
p=s/'game.h';t=p.read_text().replace('#define CARD_COUNTER_MASK (31<<CARD_COUNTER_SHIFT)','#define CARD_COUNTER_MASK (3<<CARD_COUNTER_SHIFT)').replace('>>CARD_COUNTER_SHIFT)&31','>>CARD_COUNTER_SHIFT)&3').replace('#define CARD_RAW_MASK ((1<<24)-1)','#define CARD_INSTANCE_SHIFT 21\n#define CARD_INSTANCE_MASK (1023<<CARD_INSTANCE_SHIFT)\n#define CARD_INSTANCE(raw) (((raw)>>CARD_INSTANCE_SHIFT)&1023)\n#define CARD_RAW_MASK 0x7fffffff')
t=t.replace(' int v[NS_COUNT],powers[NINJA_COUNT];',' int v[NS_COUNT],powers[NINJA_COUNT];int next_instance;')
p.write_text(t)
p=s/'game.c';t=p.read_text().replace('static int ninja_flags(int raw);','static int ninja_flags(int raw);\nstatic int ninja_stamp(Game*g,int raw);')
t=t.replace('static void add_generated(Game*g,int raw){','static void add_generated(Game*g,int raw){\n raw=ninja_stamp(g,raw);')
t=t.replace('memcpy(g->draw,g->deck,(size_t)g->deck_count*sizeof(int));shuffle','memcpy(g->draw,g->deck,(size_t)g->deck_count*sizeof(int));for(int i=0;i<g->draw_count;i++)g->draw[i]=ninja_stamp(g,g->draw[i]);shuffle')
t=t.replace('static void offer_blessing(Game*g){','static void offer_blessing(Game*g){\n if(g->character==CHAR_NIGHTSHADE&&g->act>1){g->reward[0]=1000+N_SUPPRESS;g->reward[1]=1000+N_WRAITH_FORM;g->reward[2]=relic(g,NR_DRAKE)?37:NR_DRAKE;g->screen=BLESSING;g->selection=g->scroll=0;msg(g,"Choose an ancient gift.");return;}')
t=t.replace('else if(b==BTN_OK){add_relic(g,g->reward[g->selection]);enter_map(g);}','''else if(b==BTN_OK){int gift=g->reward[g->selection];if(gift>=1000){int card=gift-1000,replaced=0;if(card==N_SUPPRESS)for(int i=0;i<g->deck_count;i++)if(CARD_ID(g->deck[i])==N_NEUTRALIZE){g->deck[i]=(g->deck[i]&~255)|N_SUPPRESS;replaced=1;break;}if(!replaced){if(g->deck_count>=MAX_DECK){msg(g,"Deck full. Choose another blessing.");break;}add_card(g,card);}}else add_relic(g,gift);enter_map(g);}''')
t=t.replace('if(g->reward[i]<28||g->reward[i]>=40)return 0;','if(!((g->reward[i]>=28&&g->reward[i]<40)||(g->character==CHAR_NIGHTSHADE&&(g->reward[i]==NR_DRAKE||g->reward[i]==1000+N_SUPPRESS||g->reward[i]==1000+N_WRAITH_FORM))))return 0;')
# Ongoing work gets validated before load, so malicious/corrupt fields cannot index arrays.
t=t.replace(' if(g->screen==CHARACTER_SELECT&&',''' if(g->ninja.nightmare_count<0||g->ninja.nightmare_count>MAX_HAND)return 0;
 for(int i=0;i<g->ninja.nightmare_count;i++)if(!valid_card_raw(g->ninja.nightmare_raw[i])||g->ninja.nightmare_n[i]<1||g->ninja.nightmare_n[i]>MAX_HAND)return 0;
 for(int i=0;i<g->ninja.task_count;i++){const NinjaTask*t=&g->ninja.tasks[i];if(t->kind<NT_PLAY||t->kind>NT_ENDTURN||!valid_card_raw(t->raw)||t->target<0||t->target>=MAX_ENEMIES||t->n<0||t->n>1000000||t->aux<0||t->aux>1000000||t->depth<0||t->depth>3)return 0;}
 for(int i=0;i<g->ninja.choice_count;i++)if(i>=MAX_HAND||!valid_card_raw(g->ninja.choice_raws[i]))return 0;
 if(g->screen==CHARACTER_SELECT&&''',1)
# New status lines reflect actual accumulated values; powers appear once.
t=t.replace(' STATUS(g->ic[IC_NEXT_ATTACK_REPEAT],"One-Two Punch: repeat next %d Attack(s)");',''' STATUS(g->ic[IC_NEXT_ATTACK_REPEAT],"One-Two Punch: repeat next %d Attack(s)");
 STATUS(g->ninja.v[NS_INTANGIBLE],"Intangible: damage capped at 1 for %d turns");
 STATUS(g->ninja.v[NS_TEMP_DEX],"Temporary Dexterity: %d expires this turn");
 STATUS(g->ninja.v[NS_BLUR],"Blur: keep Block for %d turn starts");
 STATUS(g->ninja.v[NS_BURST],"Burst: next %d Skills play twice");
 STATUS_FLAG(g->ninja.v[NS_POUNCE],"Pounce: next Skill costs 0");
 STATUS_FLAG(g->ninja.v[NS_SHADOWMELD],"Shadowmeld: double Block gained this turn");
 STATUS_FLAG(g->ninja.v[NS_SHADOW_NOW],"Shadow Step: double Attack damage this turn");
 STATUS_FLAG(g->ninja.v[NS_SHADOW_NEXT],"Shadow Step: double Attack damage next turn");
 STATUS(g->ninja.v[NS_CORROSIVE],"Corrosive Wave: draw applies %d Poison to all");
 STATUS(g->ninja.v[NS_NEXT_DRAW],"Predator: draw %d extra cards next turn");
 STATUS(g->ninja.v[NS_NEXT_BLOCK],"Dodge and Roll: %d base Block next turn");''')
p.write_text(t)
p=s/'ninja_engine.inc';t=p.read_text();a=t.index('static int ninja_flags')
t=t[:a]+'''/* Per-combat instance tags let a replay update the same physical card's
   permanent cost reduction, while Nightmare copies remain independent. */
static int ninja_instance_used(const Game*g,int serial){
 for(int i=0;i<g->draw_count;i++)if(CARD_INSTANCE(g->draw[i])==serial)return 1;
 for(int i=0;i<g->hand_count;i++)if(CARD_INSTANCE(g->hand[i])==serial)return 1;
 for(int i=0;i<g->discard_count;i++)if(CARD_INSTANCE(g->discard[i])==serial)return 1;
 for(int i=0;i<g->exhaust_count;i++)if(CARD_INSTANCE(g->exhaust[i])==serial)return 1;
 for(int i=0;i<g->ninja.task_count;i++)if(CARD_INSTANCE(g->ninja.tasks[i].raw)==serial)return 1;
 return 0;
}
static int ninja_stamp(Game*g,int raw){
 if(CARD_ID(raw)<NINJA_FIRST||CARD_INSTANCE(raw))return raw;
 for(int i=0;i<1023;i++){int serial=g->ninja.next_instance%1023+1;g->ninja.next_instance=serial;if(!ninja_instance_used(g,serial))return raw|(serial<<CARD_INSTANCE_SHIFT);}return raw;
}
static void ninja_reduce_instance(Game*g,int raw){
 int serial=CARD_INSTANCE(raw);if(!serial)return;
 int *piles[]={g->hand,g->draw,g->discard,g->exhaust};int count[]={g->hand_count,g->draw_count,g->discard_count,g->exhaust_count};
 for(int p=0;p<4;p++)for(int i=0;i<count[p];i++)if(CARD_INSTANCE(piles[p][i])==serial){int c=piles[p][i],n=MIN(3,CARD_COUNTER(c)+1);piles[p][i]=(c&~CARD_COUNTER_MASK)|(n<<CARD_COUNTER_SHIFT);return;}
}
'''+t[a:]
t=t.replace('if(id==N_UP_MY_SLEEVE){int count=MIN(31,CARD_COUNTER(raw)+1);raw=(raw&~CARD_COUNTER_MASK)|(count<<CARD_COUNTER_SHIFT);}','if(id==N_UP_MY_SLEEVE){if(raw&CARD_EPHEMERAL)ninja_reduce_instance(g,raw);else{int count=MIN(3,CARD_COUNTER(raw)+1);raw=(raw&~CARD_COUNTER_MASK)|(count<<CARD_COUNTER_SHIFT);}}')
t=t.replace('int raw=g->hand[index],id=CARD_ID(raw),cost=', 'int raw=ninja_stamp(g,g->hand[index]),id=CARD_ID(raw),cost=')
t=t.replace('ninja_clean(raw);g->ninja.nightmare_n[n]=3','ninja_clean(raw)&~CARD_INSTANCE_MASK;g->ninja.nightmare_n[n]=3')
t=t.replace('add_generated(g,g->ninja.nightmare_raw[i])','add_generated(g,g->ninja.nightmare_raw[i]&~CARD_INSTANCE_MASK)')
# Replay damage whose choices are unresolved is explicitly uncertain.
t=t.replace('static int ninja_can_play', 'static int ninja_can_play')
p.write_text(t)
