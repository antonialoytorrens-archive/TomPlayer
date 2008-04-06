
ifndef INC_KERNEL
	INC_KERNEL=../../../../golinux/include
endif

CC=arm-linux-gcc
STRIP=arm-linux-strip
CFLAGS+=-DUSE_MINIMIG -Wall -I$(INC_KERNEL) 
INSTALL_DIR=../distrib/tomplayer/

ifdef DEBUG
CFLAGS+=-DDEBUG
endif

SRC = config.c engine.c gui.c widescreen.c resume.c pwm.c playlist.c sound.c zip_skin.c power.c

OBJ = $(subst .c,.o,$(SRC))

all : tomplayer refresh_wdg

tomplayer: $(OBJ)
	$(CC) $(LDFLAGS) -o tomplayer $^ -lc -lm -lpthread -lIL -lILU -lpng -lz -lminigui -ljpeg -lzip $(ADD_LIBS)
	$(STRIP) tomplayer
	mv tomplayer $(INSTALL_DIR)

refresh_wdg : watchdog.o
	$(CC) $(LDFLAGS) -o refresh_wdg $^
	$(STRIP) refresh_wdg
	mv refresh_wdg $(INSTALL_DIR)
	

#Explicit non trivial dependencies
config.o: config.c config.h engine.h widescreen.h
engine.o: engine.c config.h engine.h resume.h widescreen.h
gui.o: gui.c config.h engine.h version.h resume.h 
widescreen.o:  widescreen.c config.h widescreen.h
playlist.o: playlist.c playlist.h config.h

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f $(INSTALL_DIR)/tomplayer
	rm -f *.o


