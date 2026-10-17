TARGET = rift_hw_test
OBJS = main.o
CFLAGS = -O2 -G0 -Wall -Wextra -std=gnu11
LIBS = -lpspdebug -lpspdisplay -lpspctrl -lpspkernel
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = RIFT HARDWARE TEST (30s auto-exit)
PSP_LARGE_MEMORY = 0
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
