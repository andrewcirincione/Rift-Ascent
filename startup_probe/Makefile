# Experimental PSP diagnostic: no C runtime, no CRT0, no libcglue.
# This does not establish physical-hardware compatibility.
TARGET = rift_bare_start
OBJS = bare_start.o
PSP_FW_VERSION = 660
PSP_LARGE_MEMORY = 0
CFLAGS = -O2 -G0 -Wall -Wextra -ffreestanding -fno-builtin -fno-stack-protector
LDFLAGS += -nostdlib -nostartfiles -Wl,-e,_start -Wl,-Map,bareboot.map
LIBS = -lpspdisplay -lpspge -lgcc
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Rift Bare Startup - Not For Hardware Testing
PSPSDK := $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
