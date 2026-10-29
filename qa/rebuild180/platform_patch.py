from pathlib import Path
S=Path(__file__).parent/'source'
p=S/'psp_start.c';s=p.read_text().replace('#include "boot_trace.h"','#include "boot_trace.h"\n#include "ascent_paths.h"')
s=s.replace('sceIoOpen("ms0:/PSP/GAME/RIFTASCENT/EBOOT.PBP",','sceIoOpen(ascent_eboot_path(),')
s=s.replace('    (void)args;(void)argp;\n    boot_trace_reset();','    ascent_launch_path(args,argp);\n    boot_trace_reset();')
s=s.replace('    (void)bytes;(void)argp;\n    boot_trace("worker entered",1);','    (void)bytes;(void)argp;\n    ascent_open(ascent_directory());\n    boot_trace("worker entered",1);')
a=s.index('    {Game saved;int loaded=game_load');b=s.index('    profile_init',a)
s=s[:a]+'''    {int found=0;for(int i=0;i<ASCENT_SLOTS;i++)found|=ascent_slot(i)->present;
     game_set_resume_available(found);game.selection=found?0:1;}
'''+s[b:]
a=s.index('        if(game.screen==TITLE&&game.selection==0&&key==BTN_OK){');b=s.index('        music_paused=',a);s=s[:a]+s[b:]
s=s.replace('game_save(&snapshot,"ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT")','ascent_checkpoint(&snapshot)').replace('game_save(&game,"ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT")','ascent_checkpoint(&game)')
# Direction-only browsing no longer writes save files every press.
s=s.replace('if(key && game.screen!=TITLE','if((key==BTN_OK||key==BTN_BACK||key==BTN_DEBUG_KILL||key==BTN_DEBUG_ACT) && game.screen!=TITLE')
s=s.replace('        key=game_render_story_input(&game,key,frame*4u);','''        if(ascent_debug()&&!inspect_open){
         if(key==BTN_TAB_LEFT)key=BTN_DEBUG_KILL;
         if(key==BTN_TAB_RIGHT)key=BTN_DEBUG_ACT;
        }
        key=game_render_story_input(&game,key,frame*4u);''')
s=s.replace('game_set_resume_available(0);','game_set_resume_available(1);')
p.write_text(s)
p=S/'asset_stream.c';s=p.read_text().replace('#include <pspiofilemgr.h>','#include <pspiofilemgr.h>\n#include "ascent_paths.h"').replace('sceIoOpen("ms0:/PSP/GAME/RIFTASCENT/EBOOT.PBP",','sceIoOpen(ascent_eboot_path(),');p.write_text(s)
p=S/'Makefile';s=p.read_text().replace('OBJS = ','OBJS = ascent_paths.o ').replace('PSP_EBOOT_TITLE = RIFT ASCENT 1.71','PSP_EBOOT_TITLE = RIFT ASCENT 1.8').replace('-Wl,-Map,game.map','-Wl,-Map,game.map -Wl,-zmax-page-size=128');p.write_text(s)
