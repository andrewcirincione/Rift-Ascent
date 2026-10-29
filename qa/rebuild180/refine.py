from pathlib import Path
r=Path(__file__).parent/'source'
p=r/'game.c';s=p.read_text()
s=s.replace('if(g&&id>=NINJA_FIRST&&id<NINJA_LAST){','if(g&&((id>=NINJA_FIRST&&id<NINJA_LAST)||(id>=354&&id<374))){')
s=s.replace(' int original_raw=raw;regent_foreign_begin(g,raw);',' int original_raw=raw;regent_foreign_begin(g,raw);\n int ascent_custom=id>=ASCENT_CARD_FIRST,ascent_after=ascent_custom?g->ninja.v[NS_AFTERIMAGE]:0,ascent_serpent=ascent_custom?g->ninja.v[NS_SERPENT]:0;')
s=s.replace(' if(enemy<0||enemy>=g->enemy_count||g->enemies[enemy].hp<=0){enemy=0;', ' if(ascent_custom&&cards[id].type==1){\n  g->ninja.v[NS_POUNCE]=0;\n  if(!(raw&CARD_EPHEMERAL)&&g->ninja.v[NS_BURST]){g->ninja.v[NS_BURST]--;repeat_this_attack++;}\n }\n if(enemy<0||enemy>=g->enemy_count||g->enemies[enemy].hp<=0){enemy=0;')
s=s.replace(' if(must_exhaust&&!(raw&CARD_EPHEMERAL))',' if(ascent_custom&&cards[id].type==1&&g->ninja.v[NS_PLANNER])raw|=CARD_SLY;\n if(must_exhaust&&!(raw&CARD_EPHEMERAL))')
s=s.replace(' r151_played(g,cards[id].type);regent_foreign_finish(g,raw);',' r151_played(g,cards[id].type);regent_foreign_finish(g,raw);\n if(ascent_custom){\n  if(cards[id].type==1)g->ninja.v[NS_SKILLS_TURN]++;\n  if(ascent_after)relic_block(g,ascent_after);\n  if(ascent_serpent)ninja_proc(g,random_living_foe(g),ascent_serpent,0);\n  for(int foe=0;foe<g->enemy_count;foe++)if(g->ninja.v[NS_STRANGLE0+foe])ninja_proc(g,foe,g->ninja.v[NS_STRANGLE0+foe],1);\n }')
s=s.replace(' if(g->act<1||(!g->endless&&g->act>3)', ' if(g->ascent.id){\n  if(g->ascent.slot>=4||g->ascent.cards>ASCENT_ALL_CARDS||g->ascent.potions>0xfffffu||g->ascent.relics>1023||g->ascent.secret>1||g->ascent.won>1||g->ascent.ascended>1)return 0;\n  for(int i=0;i<2;i++){int a=g->ascent.unlocked_cards[i],b=g->ascent.unlocked_potions[i];if(a!=-1&&(a<334+20*g->character||a>=354+20*g->character))return 0;if(b!=-1&&(b<6||b>=26))return 0;}\n  if(g->ascent.unlocked_relic!=-1&&(g->ascent.unlocked_relic<64||g->ascent.unlocked_relic>=74))return 0;\n }\n if(g->act<1||g->act>100000||(!g->endless&&g->act>3)')
p.write_text(s)
for name,case in [('ninja_engine.inc','NT_PLAY'),('regent_engine.inc','RT_PLAY')]:
 p=r/name;s=p.read_text().replace('  case '+case+':{','  case '+case+':{\n   if(id>=ASCENT_CARD_FIRST){resolve_card(g,-1,t.target,t.raw|CARD_FREE);break;}');p.write_text(s)
p=r/'ascent_storage.inc';s=p.read_text().replace(' for(int i=0;i<ASCENT_SLOTS;i++){Game saved;ascent_resume(&saved,i);}\n return 1;', ''' for(int i=0;i<ASCENT_SLOTS;i++){Game saved;ascent_resume(&saved,i);}
 /* Import the stable single-save prefix once. The old SAVE.DAT is untouched. */
 if(!va&&!vb&&!ascent_slots[0].present){
  char path[320];Game oldrun;snprintf(path,sizeof(path),"%s/SAVE.DAT",ascent_root);
  if(game_load(&oldrun,path)&&oldrun.screen!=TITLE&&oldrun.screen!=DEFEAT){
   oldrun.ascent.id=ascent_account.next_run++;oldrun.ascent.slot=0;
   for(int i=0;i<2;i++)oldrun.ascent.unlocked_cards[i]=oldrun.ascent.unlocked_potions[i]=-1;
   oldrun.ascent.unlocked_relic=-1;
   if(ascent_commit_account())ascent_checkpoint(&oldrun);
  }
 }
 return 1;''');p.write_text(s)
p=r/'render.c';s=p.read_text().replace('before->screen==STORY_BOSS||after->screen==STORY_ACT||after->screen==STORY_BOSS))','before->screen==STORY_BOSS||after->screen==STORY_ACT||after->screen==STORY_BOSS||ascent_story_screen(before->screen)||ascent_story_screen(after->screen)))');p.write_text(s)
