TARGET = rift_ascent_crt_free
OBJS = ascent_paths.o psp_start.o boot_trace.o mini_libc.o user_syscalls.o file_syscalls.o callback_syscalls.o game.o render.o artwork.o motion_art.o enemy_art.o sound.o asset_stream.o
PSP_FW_VERSION = 660
PSP_LARGE_MEMORY = 0
CFLAGS = -DRIFT_BOOT_TRACE -DRIFT_STREAMED_ART -O2 -G0 -Wall -Wextra -std=gnu11 -ffreestanding -fno-builtin -fno-strict-aliasing -fno-stack-protector
LDFLAGS += -nostdlib -nostartfiles -Wl,-e,_start -Wl,-Map,game.map -Wl,-zmax-page-size=128
LIBS = -lpspdisplay -lpspge -lpspctrl -lpspaudio -lgcc
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = RIFT ASCENT 1.8.4 BETA 1
PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_ICON1 = ICON1.PMF
PSP_EBOOT_PIC1 = PIC1.PNG
PSP_EBOOT_SND0 = SND0.AT3
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

render.o: rift151_art.h rift150b_art.h rift150_art.h rift140_art.h rift1311_art.h combat_fx.inc rift1310_art.h rift139_art.h rift138_art.h rift137_art.h game.h sound.h
game.o: inventory151.inc relics151.inc ninja_engine.inc card_preview.inc card_mods.inc events150.inc save_legacy.inc game.h
sound.o psp_start.o: sound.h game.h

game.o: regent_ids.h regent_cards.inc regent_upgrades.inc regent_meta.inc regent_engine.inc regent_effects.inc regent_preview.inc save_v22.inc
render.o: rift160_art.h rift160_star_contrast.h regent_ids.h
asset_stream.o render.o psp_start.o: asset_stream.h

render.o: rift161_art.h

render.o: rift162_art.h

render.o: rift170_art.h

game.o: story170_data.h

render.o: ascent_art.h ascent_ui.inc ascent_dialogue.h
game.o: ascent_storage.inc ascent_flow.inc ascent_progress.h new_cards.inc new_effects.inc new_potions.h new_potion_effects.inc new_relics.inc new_card_flags.h

game.o: expedition.h expedition_stories.h expedition_flow.inc expedition_relics.inc
render.o: aesthetic_art.h expedition.h expedition_stories.h expedition_art.h expedition_ui.inc

game.o: custom_run.inc
render.o: custom_ui.inc

game.o: beta_game.inc
render.o: beta_ui.inc
