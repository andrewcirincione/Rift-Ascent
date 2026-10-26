from pathlib import Path
q=Path(__file__).parent/'source'
p=q/'game.h';s=p.read_text().replace('#define GAME_VERSION 18','#define GAME_VERSION 19').replace('BLESSING,REWARD_PREVIEW};','BLESSING,REWARD_PREVIEW,ACT_INTRO};').replace('short hp,maxhp,block,strength,weak,vulnerable,poison;short intent,intent_value,phase;short art;','int hp,maxhp,block,strength,weak,vulnerable,poison;int intent,intent_value,phase;int art;').replace('int relic_pending,relic_resume,deck_readonly,shop_selection;','int relic_pending,relic_resume,deck_readonly,shop_selection;\n int endless,combat_music;').replace('void game_end_turn(Game *g);','void game_end_turn(Game *g);\nvoid game_prepare_end_turn(Game *g);\nint game_music_track(const Game *g);')
p.write_text(s)
p=q/'game.c';s=p.read_text()
s=s.replace('Potions: +5 healing, Block or damage; +1 Strength, Energy or card drawn.','Each potion is stronger: Healing restores 17 HP; Strength grants 3; Block grants 19; Fire has 25 base damage; Energy grants 3; Insight draws 4.')
s=s.replace('{"Quiet Mercy","After every won combat, heal 3 additional HP."}','{"Quiet Mercy","After every won combat, heal 3 additional HP."},\n {"Enduring Might","Gain 2 permanent Strength for every future combat. Can be chosen again."},\n {"Enduring Vitality","Gain 10 maximum HP and heal to full. Can be chosen again."},\n {"Enduring Fortune","Gain 150 Gold. Can be chosen again."}')
s=s.replace('(g->act-1)*25','MIN(4,g->act-1)*25')
s=s.replace('static void add_relic(Game*g,int id){','static void add_relic(Game*g,int id){if(id>=37){if(id==37)g->permanent_strength+=2;else if(id==38){g->maxhp+=10;g->hp=g->maxhp;}else if(id==39)g->gold+=150;return;}')
s=s.replace('int pool[9],n=0;for(int id=28;id<37;id++)if(!relic(g,id))pool[n++]=id;','int pool[12],n=0;for(int id=28;id<37;id++)if(!relic(g,id))pool[n++]=id;\n if(n<3)for(int id=37;id<40;id++)pool[n++]=id;')
s=s.replace('int game_moon_heal_now(', 'static void begin_act(Game*g){g->screen=ACT_INTRO;g->scroll=0;g->selection=0;}\nint game_moon_heal_now(')
s=s.replace('g->kill_count+=g->enemy_count;','g->kill_count+=g->enemy_count;\n if(g->battle_type==NODE_BOSS&&g->act>=3&&!g->endless){\n  g->screen=VICTORY;g->win_screen=0;g->relic_pending=0;g->selection=0;\n  msg(g,"THE SPIRE HAS FALLEN");return;\n }',1)
s=s.replace('(g->act==3?2:1)','(!g->endless&&g->act>=3?2:1)')
s=s.replace('int percent=g->act==1?82:g->act==2?105:138;','long long percent=g->act==1?82:g->act==2?105:138LL+33LL*(g->act-3);')
s=s.replace('e->intent_value=MAX(1,(d->attack*percent+50)/100+g->floor/7+(e->phase?2:0));','e->intent_value=(int)CLAMP((d->attack*percent+50)/100+g->floor/7+(e->phase?2:0),1,100000000);')
s=s.replace('e->hp*2<=e->maxhp','e->hp<=e->maxhp/2')
s=s.replace('int percent=g->act==1?94:g->act==2?116:152;','long long percent=g->act==1?94:g->act==2?116:152LL+36LL*(g->act-3);').replace('percent=g->act==3?146:100+g->act*8','percent=g->act>=3?146LL+30LL*(g->act-3):100+g->act*8').replace('int hp=(d->hp*percent+50)/100;','long long hp=(d->hp*percent+50)/100;').replace('e->maxhp=(short)MIN(999,hp);','e->maxhp=(int)MIN(1000000000LL,hp);').replace('e->art=(short)id;','e->art=id;')
s=s.replace('[g->act-1]','[MIN(2,g->act-1)]').replace('(i+g->floor+g->act)%8','(i+g->floor+g->act%8)%8')
s=s.replace('g->screen=COMBAT;g->battle_type=mode;','g->screen=COMBAT;g->battle_type=mode;\n /* Music selection uses a separate seed hash, preserving gameplay RNG. */\n g->combat_music=(route_hash(g->seed^(unsigned)g->act*17041u^(unsigned)g->floor*6781u)&1u)?2:4;')
s=s.replace('void game_end_turn(Game*g){\n if(g->screen!=COMBAT)return;','void game_prepare_end_turn(Game*g){\n if(g->screen!=COMBAT||g->ic[38])return;\n g->ic[38]=1;')
s=s.replace(' if(g->ic[IC_TEMP_STRENGTH]>0){\n  g->strength-=g->ic[IC_TEMP_STRENGTH];',' check_win(g);\n}\nvoid game_end_turn(Game*g){\n if(g->screen!=COMBAT)return;\n game_prepare_end_turn(g);\n if(g->screen!=COMBAT)return;\n g->ic[38]=0;\n if(g->ic[IC_TEMP_STRENGTH]>0){\n  g->strength-=g->ic[IC_TEMP_STRENGTH];',1)
s=s.replace('void game_update(Game*g){(void)g;}','void game_update(Game*g){if(g->screen==ACT_INTRO&&++g->scroll>=120)offer_blessing(g);}\nint game_music_track(const Game*g){\n int screen=g->screen==PAUSE_MENU?game_pause_source():g->screen;\n if(screen==TITLE||screen==HELP)return 0;\n if(screen==COMBAT||screen==CARD_CHOICE||(screen==DECK_VIEW&&g->prior_screen==COMBAT)){\n  if(g->battle_type==NODE_ELITE)return 3;\n  if(g->battle_type==NODE_BOSS)return 0;\n  return g->combat_music==4?4:2;\n }\n return 1;\n}')
s=s.replace('if(b==BTN_UP||b==BTN_DOWN)g->selection=1-g->selection;\n  else if(b==BTN_OK){\n   if(g->selection==0){offer_blessing(g);}','if(b==BTN_UP)g->selection=(g->selection+2)%3;\n  else if(b==BTN_DOWN)g->selection=(g->selection+1)%3;\n  else if(b==BTN_OK){\n   if(g->selection<2){g->endless=g->selection==1;begin_act(g);}')
s=s.replace('map_generate(g);offer_blessing(g);','map_generate(g);begin_act(g);')
s=s.replace('g->act<1||g->act>3','g->act<1||(!g->endless&&g->act>3)').replace('g->screen>REWARD_PREVIEW','g->screen>ACT_INTRO').replace('g->reward[i]>=37','g->reward[i]>=40')
s=s.replace('if(!g||g->magic','if(!g||g->endless<0||g->endless>1||g->magic')
# Old snapshots are parsed in their original ABI, then converted field by field.
start=s.index(' else if(version==17||version==16){');end=s.index(' }else return 0;',start)
s=s[:start]+''' else if(version==18||version==17||version==16){
  unsigned char old[3144];memset(old,0,sizeof(old));
  if(version==18){if(n!=3144)return 0;memcpy(old,raw,n);}
  else{
   size_t prefix=offsetof(Game,map),oldtail=offsetof(Game,enemy_count)-32;
   size_t oldsize=3144-16-32;
   if(n!=(version==17?oldsize:oldsize-sizeof(temp.ic)))return 0;
   memcpy(old,raw,prefix);
   memcpy(old+offsetof(Game,enemy_count),raw+oldtail,n-oldtail);
  }
  size_t enemy_offset=offsetof(Game,enemies);
  memcpy(&temp,old,enemy_offset);
  for(int i=0;i<MAX_ENEMIES;i++){
   const unsigned char*e=old+enemy_offset+i*46;
   memcpy(temp.enemies[i].name,e,24);
   int*values=&temp.enemies[i].hp;
   for(int j=0;j<11;j++){short value;memcpy(&value,e+24+j*2,2);values[j]=value;}
  }
  memcpy((unsigned char*)&temp+offsetof(Game,next_screen),old+2728,416);
  temp.version=GAME_VERSION;temp.endless=0;temp.combat_music=2;
  if(version<18){
   int saved_map[12][3],saved_selected[12];size_t prefix=offsetof(Game,map);
   memcpy(saved_map,raw+prefix,sizeof(saved_map));
   memcpy(saved_selected,raw+prefix+sizeof(saved_map),sizeof(saved_selected));
   map_generate(&temp);
   for(int r=0;r<temp.floor&&r<12;r++){
    memcpy(temp.map[r],saved_map[r],sizeof(saved_map[r]));temp.map_selected[r]=saved_selected[r];
   }
   temp.relic_pending=0;temp.relic_resume=temp.win_screen?REWARD:MAP;
   if(temp.screen==MAP)map_select_reachable(&temp);
  }
''' +s[end:]
s=s.replace('p->wins+p->losses==p->runs&&\n  p->furthest_floor<=3*(MAP_ROWS+1);','p->wins+p->losses==p->runs;').replace('(unsigned)((g->act-1)*(MAP_ROWS+1)+g->floor)','(unsigned)MIN(4294967295ULL,(unsigned long long)(g->act-1)*(MAP_ROWS+1)+g->floor)')
p.write_text(s)
print('Rules, endless progression, phased Plating, save migration ready')
