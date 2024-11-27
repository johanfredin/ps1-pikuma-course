.PHONY: all cleansub

all:
	mkpsxiso -y ./tools/isoconfig.xml
	rm -f wipefaux wipefaux.bin
	rm -f *.map
	rm -f *.o 
	rm -f *.dep

cleansub:
	$(MAKE) clean
	rm -f wipefaux.cue 
	rm -f wipefaux.bin 
	rm -f GAME.iso 

TARGET = wipefaux
TYPE = ps-exe

SRCS = \
third_party/nugget/common/crt0/crt0.s \
main.c display.c globals.c joypad.c camera.c utils.c, object.c texture.c lzss.c track.c

CPPFLAGS += -Ithird_party/psyq-iwyu/include
LDFLAGS += -Lthird_party/psyq/lib
LDFLAGS += -Wl,--start-group
LDFLAGS += -lapi
LDFLAGS += -lc
LDFLAGS += -lc2
LDFLAGS += -lcard
LDFLAGS += -lcd
LDFLAGS += -lcomb
LDFLAGS += -lds
LDFLAGS += -letc
LDFLAGS += -lgpu
LDFLAGS += -lgs
LDFLAGS += -lgte
LDFLAGS += -lgun
LDFLAGS += -lhmd
LDFLAGS += -lmath
LDFLAGS += -lmcrd
LDFLAGS += -lmcx
LDFLAGS += -lpad
LDFLAGS += -lpress
LDFLAGS += -lsio
LDFLAGS += -lsnd
LDFLAGS += -lspu
LDFLAGS += -ltap
LDFLAGS += -Wl,--end-group

include third_party/nugget/common.mk
