from pathlib import Path
q=Path(__file__).parent/'source'
p=q/'game.c';s=p.read_text().replace('#include "save_legacy.inc"','#include "save_legacy.inc"\n#include "save_v22.inc"')
s=s.replace('else if(version==21){','else if(version==22){if(n!=offsetof(Game,regent))return 0;memcpy(&temp,raw,n);temp.version=GAME_VERSION;}\n else if(version==21){')
s=s.replace('if(temp.screen==POTION_VIEW', 'if(version<23&&!migrate_raw22(&temp))return 0;\n if(temp.screen==POTION_VIEW',1)
s=s.replace('static int paused_from_screen=MAP;', 'static int resume_available;\nvoid game_set_resume_available(int available){resume_available=!!available;}\nint game_resume_available(void){return resume_available;}\nstatic int paused_from_screen=MAP;')
start=s.index(' case TITLE:\n',s.index('void game_action'))
end=s.index(' case CHARACTER_SELECT:',start)
s=s[:start]+''' case TITLE:
  if(b==BTN_UP)g->selection=(g->selection+3)%4;
  else if(b==BTN_DOWN)g->selection=(g->selection+1)%4;
  else if(b==BTN_OK){
   if(g->selection==0){if(!resume_available)msg(g,"No saved run.");}
   else if(g->selection<3){g->endless=g->selection==2;g->screen=CHARACTER_SELECT;g->selection=g->character;}
   else {g->screen=HELP;g->prior_screen=TITLE;}
  }else if(b==BTN_ALT){g->screen=HELP;g->prior_screen=TITLE;}break;
'''+s[end:]
s=s.replace('g->selection=g->endless?1:0;','g->selection=g->endless?2:1;')
s=s.replace('if(g->screen!=COMBAT||g->ic[38]||g->ninja.task_count)return;', 'if(g->screen!=COMBAT||g->ic[38]||g->ninja.task_count||g->regent.task_count)return;')
s=s.replace('if(g->screen!=COMBAT||g->ninja.task_count)return;', 'if(g->screen!=COMBAT||g->ninja.task_count||g->regent.task_count)return;')
s=s.replace('g->block-=blocked;n-=blocked;', '''g->block-=blocked;n-=blocked;
 if(g->regent.reflect&&blocked>0&&combat_fx_source>=0&&combat_fx_source<g->enemy_count){Enemy*e=&g->enemies[combat_fx_source];int b=MIN(e->block,blocked);e->block-=b;e->hp-=blocked-b;record_combat_fx(GFX_PROC,-1,combat_fx_source,blocked,b);}
 if(g->regent.gambit&&n>0)g->hp=0;''',1)
s=s.replace('if(g&&id<CARD_COUNT&&g->ic[IC_CORRUPTION]', 'if(g&&g->character==CHAR_REGENT&&g->regent.powers[R_VOID_FORM-REGENT_FIRST]&&g->regent.void_used<2)return 0;\n if(g&&id<CARD_COUNT&&g->ic[IC_CORRUPTION]',1)
p.write_text(s)
p=q/'psp_start.c';s=p.read_text()
s=s.replace('boot_trace("save load result",game_load(&game,"ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT"));', '''{Game saved;int loaded=game_load(&saved,"ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT");
    game_set_resume_available(loaded&&saved.screen!=TITLE&&saved.screen!=VICTORY&&saved.screen!=DEFEAT&&saved.screen!=CHARACTER_SELECT);
    game.selection=game_resume_available()?0:1;boot_trace("resume available",game_resume_available());}''')
s=s.replace('sceKernelExitGame();\n                return 0;', '''game_new(&game,0x93ab6711U ^ sceKernelGetSystemTimeLow());
                game_set_resume_available(1);game.selection=0;key=0;
                inspect_open=0;pending_card_ticks=enemy_turn_delay=plating_delay=0;
                game_render_reset_fx();''',1)
# The renderer currently has no reset API; provide one with the new source.
needle='        music_paused=game_render_music_paused(frame*4u);'
pos=s.index(needle,s.index('if(game.screen==PAUSE_MENU'))
s=s[:pos]+'''        if(game.screen==TITLE&&game.selection==0&&key==BTN_OK){
            if(game_resume_available()){
                Game saved;if(game_load(&saved,"ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT")&&saved.screen!=TITLE&&saved.screen!=DEFEAT&&saved.screen!=VICTORY){game=saved;game_render_reset_fx();inspect_open=0;}
                else{game_set_resume_available(0);snprintf(game.message,sizeof(game.message),"Saved run could not be loaded.");}
            }
            key=0;
        }
'''+s[pos:]
s=s.replace('game.screen!=COMBAT && game.screen!=DEFEAT)', 'game.screen!=COMBAT && game.screen!=DEFEAT && game.screen!=PAUSE_MENU && game.screen!=CHARACTER_SELECT)')
p.write_text(s)
p=q/'game.h';s=p.read_text().replace('void game_render_frame(', 'void game_render_reset_fx(void);\nvoid game_render_frame(');p.write_text(s)
