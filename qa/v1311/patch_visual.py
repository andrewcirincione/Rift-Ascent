from pathlib import Path
q=Path(__file__).parent/'source'
p=q/'render.c';s=p.read_text().replace('#include "rift138_art.h"','#include "rift138_art.h"\n#include "rift1311_art.h"')
s=s.replace('after->screen==RELIC_RESULT||after->screen==DEFEAT','after->screen==RELIC_RESULT||after->screen==VICTORY||after->screen==DEFEAT').replace('g->screen==RELIC_RESULT||g->screen==DEFEAT','g->screen==RELIC_RESULT||g->screen==VICTORY||g->screen==DEFEAT')
s=s.replace('"SQUARE","CIRCLE","START","SQ","X","O"','"SQUARE","CIRCLE","START","SQ","X","O","TRIANGLE","TRI"').replace('ids[]={2,1,3,2,0,1}','ids[]={2,1,3,2,0,1,4,4}').replace('for(int i=0;i<6;i++){int n=(int)strlen(names[i])','for(int i=0;i<8;i++){int n=(int)strlen(names[i])').replace('r135_square,r135_start};','r135_square,r135_start,r1311_triangle};')
s=s.replace('int values[6]','int values[7]').replace('values[5]=g->energy_penalty;','values[5]=g->energy_penalty;values[6]=g->ic[IC_PLATING];').replace('values[5]=0;}','values[5]=values[6]=0;}')
a=s.index('static int r135_status_values');b=s.index('static void r134_card_frame',a);part=s[a:b].replace('i<6','i<7').replace('r135_energy_drain};','r135_energy_drain,r1311_plating};').replace('int step=count>4?19:23,y=count>4?173-count*step:80,slot=0;','int size=count>5?16:20,step=count>5?14:count>4?18:23,y=count>4?174-count*step:80,slot=0;').replace('r134_blit(x,yy,20,20','r134_blit(x,yy,size,size').replace('yy+10','yy+(count>5?5:10)');s=s[:a]+part+s[b:]
s=s.replace('if(chosen)r134_blit(20,y-2,214,29,r139_select,208,26,255);','if(chosen)r134_blit(27,y+1,204,22,r1311_shop,204,22,255);').replace('r134_wrap(chosen?56:36,y+8,chosen?175:195','r134_wrap(36,y+8,195')
s=s.replace('snprintf(b,sizeof(b),"ACT %d / 3",g->act);','snprintf(b,sizeof(b),g->endless?"ACT %d / ENDLESS":"ACT %d / 3",g->act);')
s=s.replace('for(int i=0;i<2;i++){\n  int y=146+i*42','for(int i=0;i<3;i++){\n  int y=132+i*35').replace('i?"HOW TO PLAY":"BEGIN ASCENT"','i==2?"HOW TO PLAY":i==1?"ENDLESS MODE":"BEGIN ASCENT"').replace('r134_center(367,230','r134_center(367,244').replace('"ONE WARRIOR / THREE ACTS"','g->selection==1?"UNLIMITED ACTS / RISING DIFFICULTY":"ONE WARRIOR / THREE ACTS"')
s=s.replace(' pix=pixels;sw=stride;background(g);',''' pix=pixels;sw=stride;
 if(g->screen==ACT_INTRO){
  rect(0,0,W,H,RGB(0,0,0));
  int t=g->scroll,alpha=t<10?0:t<40?(t-10)*255/30:t<85?255:t<115?(115-t)*255/30:0;
  r134_blit(0,0,480,272,r1311_act,480,272,alpha);
  char label[40];snprintf(label,sizeof(label),"ACT %d",g->act);
  r134_center(240,208,label,RGB(230,222,202),&r134_f16);
  if(g->endless)r134_center(240,235,"ENDLESS ASCENT",GOLD,&r138_title9);
  dark_rect(0,198,480,74,255-alpha);return;
 }
 background(g);''')
# Remove the obsolete MAP popup now that the act has a dedicated introduction.
a=s.index(' if(g->screen==MAP && fx_transition');b=s.index('\n}\n\n/* Existing callers',a);s=s[:a]+s[b:]
p.write_text(s)
p=q/'combat_fx.inc';s=p.read_text().replace('r137_last_screen==RELIC_RESULT||','r137_last_screen==RELIC_RESULT||r137_last_screen==VICTORY||').replace('after->screen==RELIC_RESULT)','after->screen==RELIC_RESULT||after->screen==VICTORY)')
s=s.replace('r1310_enemy_bounds','r1311_enemy_bounds').replace('y=174-h+wave(animation_tick/4+e*24u,96,1)','y=174-h')
# Ground reference applies to the feet, ignoring stray specks below a body.
s=s.replace('int sx=xx*cw/w;','int sx=xx*cw/w;')
p.write_text(s)
p=q/'sound.c';s=p.read_text().replace('s->block_age<10000','s->block_age<6500').replace('(10000-a)*4500/10000','(6500-a)*1800/6500').replace('(700-a)*5','(700-a)*2').replace('tier==3?15000:tier==2?12000:10000','tier==3?17000:tier==2?15000:13000').replace('s->impact_low[v])/5','s->impact_low[v])/3').replace('if(a<1800){int grit=(noise/4096)*4096;hit+=(grit*((1800-a)*env/1800)/4096)*(amp/2)/32768;}','if(a<3000){int grit=(noise/4096)*4096;hit+=(grit*((3000-a)*env/3000)/4096)*amp/32768;}').replace('int g=80+(int)s->impact_age[i]*80/4410','int g=48+(int)s->impact_age[i]*112/4410')
p.write_text(s)
p=q/'psp_start.c';s=p.read_text().replace('#define RIFT_TRACK_COUNT 3','#define RIFT_TRACK_COUNT 5').replace('#define RIFT_MUSIC_FOOTER 40','#define RIFT_MUSIC_FOOTER 56').replace("'R','I','F','T','M','1','1','9'","'R','I','F','T','M','3','1','1'").replace('read_u32(footer+32)','read_u32(footer+48)').replace('Rift 1.1.9 original tracks loaded','Rift 1.3.11 five music tracks loaded')
s=s.replace('static int enemy_turn_delay=0,action_cooldown=0;','static int enemy_turn_delay=0,action_cooldown=0;\nstatic int plating_delay=0;')
s=s.replace('    game_action(&game,key);','    if(key==1024)game_prepare_end_turn(&game);\n    else game_action(&game,key);',1)
s=s.replace('if(game_render_fx_busy(frame*4u)&&key!=BTN_PAUSE)key=0;','if((game_render_fx_busy(frame*4u)||plating_delay)&&key!=BTN_PAUSE)key=0;')
s=s.replace('!inspect_open && !enemy_turn_delay &&','!inspect_open && !enemy_turn_delay && !plating_delay &&')
s=s.replace('!inspect_open&&!enemy_turn_delay &&','!inspect_open&&!enemy_turn_delay&&!plating_delay &&')
s=s.replace('            enemy_turn_delay=6;','''            step_action(1024,frame*4u);
            if(game.screen==COMBAT)plating_delay=game.ic[IC_PLATING]>0?14:1;''')
s=s.replace('        if(enemy_turn_delay>0){','''        /* Resume a saved mid-turn Plating phase without granting it twice. */
        if(game.screen==COMBAT&&game.ic[38]&&!plating_delay&&!enemy_turn_delay)plating_delay=1;
        if(game.screen==COMBAT&&plating_delay>0){
            plating_delay--;
            if(!plating_delay&&!game_render_fx_busy(frame*4u))enemy_turn_delay=6;
            else if(!plating_delay)plating_delay=1;
            key=0;
        }
        if(game.screen==COMBAT&&enemy_turn_delay>0){''')
a=s.index('        music_battle=(game.screen==TITLE?0:');b=s.index('        if(frame==1)',a)
s=s[:a]+'''        /* Keep the battle track through the last contact, death and fade. */
        if(!game_render_fx_busy(frame*4u))music_battle=game_music_track(&game);
'''+s[b:]
p.write_text(s)
p=q/'game.c';s=p.read_text().replace('Healing restores 17 HP','Healing restores 23 HP').replace('Block grants 19','Guard grants 25 Block');p.write_text(s)
print('Visual, audio and PSP transition patches ready')
