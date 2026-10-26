from pathlib import Path
q=Path(__file__).parent/'source'
p=q/'game.c';s=p.read_text()
def change(old,new):
 global s
 assert old in s,old[:100]
 s=s.replace(old,new)
change('static int preview_depth,preview_phase,preview_choice=-1,preview_nested;', 'static int preview_depth,preview_phase,preview_choice=-1,preview_nested;\n/* Transient call-stack value: a repeated X Attack retains its original X. */\nstatic int repeated_x=-1;')
change('if(cost==-1 || (id==10 && g->ic[IC_CHAIN_DEPTH]==0))cost=g->energy;', '''int x_value=game_card_cost(raw)==-1?g->energy:0;
 if(id==10&&(raw&CARD_EPHEMERAL)&&repeated_x>=0)x_value=repeated_x;
 if(cost==-1)cost=g->energy;''')
change('case 10:for(int j=0;j<cost;j++)all_damage(g,5+3*up);break;', 'case 10:for(int j=0;j<x_value;j++)all_damage(g,5+3*up);break;')
change('autoplay_from_draw(g,MIN(30,cost+up),0);','autoplay_from_draw(g,MIN(30,x_value+up),0);')
change('autoplay_card(g,original_raw|CARD_EPHEMERAL,0);','int saved_x=repeated_x;repeated_x=id==10?x_value:-1;\n   autoplay_card(g,original_raw|CARD_EPHEMERAL,0);repeated_x=saved_x;')
change('if(g->vulnerable)n=n*3/2;\n if(g->ic[IC_COLOSSUS]&&g->enemies[index].vulnerable)n=(n+1)/2;\n if(g->ic[IC_TANK])n*=2;', 'if(g->ic[IC_COLOSSUS]&&g->enemies[index].vulnerable)n=(n+1)/2;\n if(g->ic[IC_TANK])n*=2;\n if(g->vulnerable)n=n*3/2;')
# Displayed intentions are derived from the same damage helper used by the icon.
needle='static void enemy_hit(Game*g,Enemy*e,int raw){'
insert='''void game_enemy_intent_text(const Game*g,int index,char*out,int cap){
 if(!out||cap<=0)return;out[0]=0;
 if(!g||index<0||index>=g->enemy_count)return;
 const Enemy*e=&g->enemies[index];
 if(e->hp<=0){snprintf(out,cap,"Defeated");return;}
 int total=game_enemy_intent_damage(g,index),hits=game_enemy_intent_hits(g,index);
 switch(e->intent){
 case 0:snprintf(out,cap,"Deal %d DMG",total);break;
 case 1:snprintf(out,cap,"Gain %d Block",e->intent_value);break;
 case 2:snprintf(out,cap,"Deal %d DMG; apply 2 Weak",total);break;
 case 3:snprintf(out,cap,"Deal %d DMG; apply 2 Vulnerable",total);break;
 case 4:snprintf(out,cap,"Gain 2 Strength and 8 Block");break;
 case 5:snprintf(out,cap,"Deal %d DMG; add Burn to discard",total);break;
 case 6:snprintf(out,cap,"Deal %d DMG; apply 2 Poison",total);break;
 case 7:snprintf(out,cap,"Deal %d DMG twice",hits?total/hits:0);break;
 case 8:snprintf(out,cap,"Deal %d DMG; drain 1 Energy next turn",total);break;
 case 9:snprintf(out,cap,"Heal %d HP; gain %d Block",MIN(e->maxhp-e->hp,e->maxhp/9+3),e->intent_value/2);break;
 case 10:snprintf(out,cap,"Deal %d DMG; add Daze to discard",total);break;
 case 11:snprintf(out,cap,"Add Dread to discard; gain %d Block",e->intent_value/2);break;
 case 12:snprintf(out,cap,"Deal %d DMG; heal for HP lost",total);break;
 case 13:snprintf(out,cap,"Gain %d Block and 1 Strength",e->intent_value);break;
 }
}
'''
change(needle,insert+needle)
change('"Healing","Might","Guard","Fire","Energy","Insight"','"Healing","Strength","Guard","Fire","Energy","Insight"')
# Notice stored in previously unused ic slot 37; survives reward/relic inspection and save/resume.
change('g->kill_count+=g->enemy_count;', 'g->ic[37]=0;\n g->kill_count+=g->enemy_count;')
change('fill_rewards(g);if(g->potion_count<MAX_POTIONS&&rnd(g,100)<22)g->potions[g->potion_count++]=rnd(g,6);g->screen=REWARD;g->selection=0;', 'fill_rewards(g);if(g->potion_count<MAX_POTIONS&&rnd(g,100)<22){int potion=rnd(g,6);g->potions[g->potion_count++]=potion;g->ic[37]=potion+1;}g->screen=REWARD;g->selection=0;')
change('g->gold-=50;g->potions[g->potion_count++]=rnd(g,6);\n   msg(g,"Purchased a potion.");', 'g->gold-=50;int potion=rnd(g,6);g->potions[g->potion_count++]=potion;\n   snprintf(g->message,sizeof(g->message),"Potion gained: %s",game_potion_name(potion));')
change('''if(g->deck_count<MAX_DECK)add_card(g,card|UPBIT);
   snprintf(g->message,sizeof(g->message),"Ancient lesson: %s+ gained.",cards[card].name);
   break;''','''if(g->deck_count<MAX_DECK){
    add_card(g,card|UPBIT);g->reward[0]=card|UPBIT;g->reward[1]=-1;
    g->screen=EVENT_RESULT;g->selection=0;
    snprintf(g->message,sizeof(g->message),"Obtained %s+",cards[card].name);
   }else msg(g,"Your deck is full.");
   break;''')
change('int id=g->potions[index];int bonus=relic(g,17)?5:0;', 'int id=g->potions[index];int bonus=relic(g,17)?5:0;\n int old_hp=g->hp,old_strength=g->strength,old_block=g->block,old_energy=g->energy,old_hand=g->hand_count;')
change('msg(g,"Potion used.");check_win(g);','''switch(id){
 case 0:snprintf(g->message,sizeof(g->message),"Healing potion: +%d HP",g->hp-old_hp);break;
 case 1:snprintf(g->message,sizeof(g->message),"Strength +%d (now %d)",g->strength-old_strength,g->strength);break;
 case 2:snprintf(g->message,sizeof(g->message),"Guard potion: +%d Block",g->block-old_block);break;
 case 3:snprintf(g->message,sizeof(g->message),"Fire potion used");break;
 case 4:snprintf(g->message,sizeof(g->message),"Energy potion: +%d Energy",g->energy-old_energy);break;
 case 5:snprintf(g->message,sizeof(g->message),"Insight potion: drew %d",MAX(0,g->hand_count-old_hand));break;
 }
 check_win(g);''')
p.write_text(s)
p=q/'game.h';s=p.read_text().replace('int game_enemy_intent_hits(const Game *g,int index);','int game_enemy_intent_hits(const Game *g,int index);\nvoid game_enemy_intent_text(const Game*g,int index,char*out,int cap);');p.write_text(s)
p=q/'psp_start.c';s=p.read_text().replace('if(game.potion_count==0)inspect_open=0;','inspect_open=0; /* Show the applied status and explicit potion result. */');p.write_text(s)
