TARGET = rift_ascent_crt_free
OBJS = psp_start.o boot_trace.o mini_libc.o user_syscalls.o file_syscalls.o callback_syscalls.o game.o render.o artwork.o motion_art.o enemy_art.o sound.o asset_stream.o
PSP_FW_VERSION = 660
PSP_LARGE_MEMORY = 0
CFLAGS = -DRIFT_BOOT_TRACE -DRIFT_STREAMED_ART -O2 -G0 -Wall -Wextra -std=gnu11 -ffreestanding -fno-builtin -fno-strict-aliasing -fno-stack-protector
LDFLAGS += -nostdlib -nostartfiles -Wl,-e,_start -Wl,-Map,game.map -Wl,-zmax-page-size=128
LIBS = -lpspdisplay -lpspge -lpspctrl -lpspaudio -lgcc
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = RIFT ASCENT 1.8 PRE-RELEASE
PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_PIC1 = PIC1.PNG
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
