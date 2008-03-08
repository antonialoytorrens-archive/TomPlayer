
ifndef INC_KERNEL
	INC_KERNEL=../../../../golinux/include
endif

CC=arm-linux-gcc
STRIP=arm-linux-strip
CFLAGS+=-DUSE_MINIMIG -Wall -I$(INC_KERNEL) 
INSTALL_DIR=../distrib/tomplayer/


SRC = config.c engine.c gui.c widescreen.c resume.c pwm.c

OBJ = $(subst .c,.o,$(SRC))

all : tomplayer refresh_wdg

tomplayer: $(OBJ)
	$(CC) $(LDFLAGS) -o tomplayer $^ -lminigui -lpthread -lc -lm $(ADD_LIBS)
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

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f $(INSTALL_DIR)/tomplayer
	rm -f *.o
