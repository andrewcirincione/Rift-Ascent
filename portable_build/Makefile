TARGET = rift_ascent_crt_free
OBJS = psp_start.o boot_trace.o mini_libc.o user_syscalls.o file_syscalls.o callback_syscalls.o game.o render.o artwork.o motion_art.o enemy_art.o sound.o
PSP_FW_VERSION = 660
PSP_LARGE_MEMORY = 0
CFLAGS = -DRIFT_BOOT_TRACE -O2 -G0 -Wall -Wextra -std=gnu11 -ffreestanding -fno-builtin -fno-strict-aliasing -fno-stack-protector
LDFLAGS += -nostdlib -nostartfiles -Wl,-e,_start -Wl,-Map,game.map
LIBS = -lpspdisplay -lpspge -lpspctrl -lpspaudio -lgcc
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = RIFT ASCENT 1.6
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
render.o: rift160_art.h regent_ids.h
