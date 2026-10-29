from pathlib import Path
r=Path(__file__).resolve().parent/'source'
p=r/'game.c';s=p.read_text()
s=s.replace('"Scorch Witch",50,12,0,{5,0,6,1}','"Brine Witch",50,12,0,{6,0,2,1}').replace('"Ashborne Monk"','"Drowned Penitent"')
oldnew={6:('1,10,0,4','1,10,21,4'),7:('7,3,4,0','1,18,10,22'),13:('5,6,9,10','5,19,9,18'),14:('11,7,4,3','20,7,18,3'),22:('10,7,6,1','1,7,23,10'),23:('13,10,4,0','13,16,18,21'),33:('4,10,6,0','4,19,10,21'),34:('11,3,7,13','20,3,7,22'),35:('10,6,4,7','1,10,23,7'),42:('14,15,1,16','14,15,18,16'),43:('17,15,16,4','17,23,16,21'),15:('6,10,4,7','1,19,10,4'),24:('3,10,11,4','3,18,11,21'),8:('4,10,11,7','4,10,22,20')}
a=s.index('static const EnemyDef enemy_defs');b=s.index('/* Normal / elite',a);block=s[a:b];lines=block.splitlines()
for id,(old,new) in oldnew.items():
 line=lines[id+1];assert '{'+old+'}' in line,(id,line);lines[id+1]=line.replace('{'+old+'}','{'+new+'}')
block='\n'.join(lines)+'\n';block=block.replace('\n};','\n {"Crown Eater",174,19,1,{22,7,21,18}},\n};');s=s[:a]+block+s[b:]
s=s.replace('{33,34,35,43}','{33,34,35,46}').replace('mode==NODE_ELITE?enemy_elite[act][rnd(g,4)]','mode==NODE_ELITE?enemy_elite[g->act>=4?rnd(g,3):act][rnd(g,4)]')
s=s.replace('"PIERCE","SUNDER","FLURRY","ENTOMB"','"PIERCE","SUNDER","FLURRY","ENTOMB","FRACTURE","SEAR","RALLY","PURGE","GLOOM","BLIGHT"')
s=s.replace('"Adds one unplayable Injury to discard for this combat and gains Block."','''"Adds one unplayable Injury to discard for this combat and gains Block.",
 "A half-strength hit; applies 2 Weak and 2 Vulnerable.",
 "Two small attacks, then adds a Burn to your discard pile.",
 "All living enemies gain 1 Strength and 8 Block.",
 "Removes enemy Weak and Vulnerable, halves Poison, then gains Block.",
 "Applies 2 Weak and reduces your next turn Energy by 1. No direct hit.",
 "Adds 2 Poison and one Daze to your discard pile. No direct hit."''')
s=s.replace('case 5:case 6:case 8:return MAX(0,v/2+e->strength);','case 5:case 6:case 8:case 18:return MAX(0,v/2+e->strength);')
s=s.replace('case 16:return MAX(0,v/3+e->strength);','case 16:case 19:return MAX(0,v/3+e->strength);')
s=s.replace('(e->intent==7?2:e->intent==16?3:1)','(e->intent==7||e->intent==19?2:e->intent==16?3:1)')
s=s.replace('case 17:snprintf(out,cap,"Add Injury to discard; gain %d Block",e->intent_value/2);break;','''case 17:snprintf(out,cap,"Add Injury to discard; gain %d Block",e->intent_value/2);break;
 case 18:snprintf(out,cap,"Deal %d DMG; apply 2 Weak and Vulnerable",total);break;
 case 19:snprintf(out,cap,"Deal %d DMG twice; add Burn",hits?total/hits:0);break;
 case 20:snprintf(out,cap,"All foes gain 1 Strength and 8 Block");break;
 case 21:snprintf(out,cap,"Cleanse Weak/Vulnerable, halve Poison; %d Block",e->intent_value/2);break;
 case 22:snprintf(out,cap,"Apply 2 Weak; drain 1 Energy next turn");break;
 case 23:snprintf(out,cap,"Apply 2 Poison; add Daze to discard");break;''')
s=s.replace('case 17:if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=52;e->block+=e->intent_value/2;break;','''case 17:if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=52;e->block+=e->intent_value/2;break;
   case 18:enemy_hit(g,e,enemy_attack_value(e));g->weak=MAX(g->weak,2);g->vulnerable=MAX(g->vulnerable,2);break;
   case 19:for(int hit=0;hit<2&&g->screen==COMBAT&&e->hp>0;hit++)enemy_hit(g,e,enemy_attack_value(e));if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=50;break;
   case 20:for(int z=0;z<g->enemy_count;z++)if(g->enemies[z].hp>0){g->enemies[z].strength++;g->enemies[z].block+=8;}break;
   case 21:e->weak=e->vulnerable=0;e->poison/=2;e->block+=e->intent_value/2;break;
   case 22:g->weak=MAX(g->weak,2);g->energy_penalty=MIN(2,g->energy_penalty+1);break;
   case 23:g->poison=MIN(12,g->poison+2);if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=51;break;''')
# Clear all old foe Block first so an ally's Rally does not vanish before acting.
s=s.replace(' for(int k=0;k<g->enemy_count;k++){\n  Enemy *e=&g->enemies[k];if(e->hp<=0)continue;\n  e->block=0;', ' for(int k=0;k<g->enemy_count;k++)g->enemies[k].block=0;\n for(int k=0;k<g->enemy_count;k++){\n  Enemy *e=&g->enemies[k];if(e->hp<=0)continue;')
p.write_text(s)
p=r/'render.c';s=p.read_text();old='''    before->turn!=after->turn){
   char turn[64];''';new='''    before->turn!=after->turn){
   for(int i=0;i<before->enemy_count;i++)if(before->enemies[i].hp>0&&before->enemies[i].intent>=18){
    char action[72];snprintf(action,sizeof(action),"%s: %s",before->enemies[i].name,game_intent_title(before->enemies[i].intent));rift120_log_add(action);
    game_enemy_intent_text(before,i,action,sizeof(action));rift120_log_add(action);
   }
   char turn[64];''';assert old in s;s=s.replace(old,new);p.write_text(s)
# Every revised sprite uses a uniform streamed alpha descriptor, before old tables.
p=r/'combat_fx.inc';s=p.read_text();s=s.replace('static const uint32_t*ascent_enemy_art(int art){','static const uint32_t*revision182_enemy_art(int art){return art>=0&&art<ENEMY_COUNT?aesthetic_enemies[art]:0;}\nstatic const uint32_t*ascent_enemy_art(int art){')
s=s.replace('if(ascent_enemy_art(art)){iw=144;', 'if(revision182_enemy_art(art)){iw=192;ih=224;rgba=revision182_enemy_art(art);}\n else if(ascent_enemy_art(art)){iw=144;')
s=s.replace('int w=ascent_enemy_art(art)?','int w=revision182_enemy_art(art)?192*h/224:ascent_enemy_art(art)?')
s=s.replace('!ascent_enemy_art(art)&&boss<0','!revision182_enemy_art(art)&&!ascent_enemy_art(art)&&boss<0')
s=s.replace('if(ascent_enemy_art(art))r170_rgba_actor','if(revision182_enemy_art(art))r170_rgba_actor(ex-w/2,y,w,h,revision182_enemy_art(art),192,224,tilt,opacity,flash);\n  else if(ascent_enemy_art(art))r170_rgba_actor')
s=s.replace('r134_blit(ex-44,149,88,25,r134_target,88,25,255);','r134_blit(ex-29,160,58,13,aesthetic_target,108,24,255);')
p.write_text(s)
print('Applied 47 act-specific enemies, unique elite pools, 6 elite/boss intents and target marker')
