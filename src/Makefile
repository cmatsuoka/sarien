
SRC_OBJS	= global.o agi_v2.o agi_v3.o agi_v4.o agi.o cli.o words.o \
		  objects.o picture.o view.o id.o logic.o op_test.o op_cmd.o \
		  op_dbg.o patches.o keyboard.o menu.o font.o lzw.o getopt.o \
		  getopt1.o savegame.o sound.o silent.o console.o cycle.o \
		  inv.o text.o graphics.o sprite.o checks.o motion.o rand.o \
		  picview.o fileglob.o

SRC_DFILES	= Makefile $(OBJS:.o=.c) main.c cli.c hirespic.c \
		  agi.h getopt.h keyboard.h logic.h lzw.h menu.h opcodes.h \
		  picture.h rand.h savegame.h sound.h sarien.h portdefs.h \
		  view.h win32.h console.h text.h graphics.h sprite.h list.h \
		  scale2x.h

SRC_PATH	= src

CLI_OBJS += $(addprefix $(SRC_PATH)/, main.o cli.o path.o sdl.o)

OBJS += $(addprefix $(SRC_PATH)/,$(SRC_OBJS))

default-src::
	$(MAKE) -C ..

dest-src::
	mkdir -p $(DIST)/$(SRC_PATH)
	cp -RPp $(addprefix $(SRC_PATH)/,$(SRC_DFILES)) $(DIST)/$(SRC_PATH)

