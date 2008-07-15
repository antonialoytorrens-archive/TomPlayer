
ifndef INC_KERNEL
	INC_KERNEL=../../../../golinux/include
endif

CC=arm-linux-gcc
STRIP=arm-linux-strip
CFLAGS+= -Wall -I$(INC_KERNEL) 
INSTALL_DIR=../distrib/tomplayer/

ifdef DEBUG
CFLAGS+=-DDEBUG -g 
endif

SRC = window_context.c window.c gui.c gui_control.c callbacks.c list.c file_list.c config.c engine.c gui.c widescreen.c resume.c pwm.c playlist.c sound.c zip_skin.c power.c

OBJ = $(subst .c,.o,$(SRC))

all : tomplayer refresh_wdg

tomplayer: $(OBJ)
	$(CC) $(LDFLAGS) -o tomplayer $^  -liniparser -ldirectfb -lfusion -ldirect -lpthread -lz -lzip  -lm -lpthread -ljpeg -lILU  -lIL -lpng $(ADD_LIBS)
	$(STRIP) tomplayer
	mv tomplayer $(INSTALL_DIR)

refresh_wdg : watchdog.o
	$(CC) $(LDFLAGS) -o refresh_wdg $^
	$(STRIP) refresh_wdg
	mv refresh_wdg $(INSTALL_DIR)

#Explicit non trivial dependencies
callbacks.o: callbacks.c callbacks.h gui.h window_context.h debug.h list.h file_list.h engine.h resume.h playlist.h zip_skin.h config.h
config.o: config.c debug.h config.h zip_skin.h engine.h widescreen.h
engine.o: engine.c config.h engine.h resume.h widescreen.h pwm.h sound.h power.h debug.h zip_skin.h gui_control.h window_context.h file_list.h
file_list.o: file_list.c file_list.h debug.h engine.h
gui.o: gui.c gui.h window.h gui_control.h callbacks.h debug.h window_context.h engine.h pwm.h power.h
gui_control.o: gui_control.c gui.h gui_control.h window_context.h debug.h file_list.h version.h engine.h
list.o: list.c list.h debug.h
playlist.o: playlist.c config.h engine.h
power.o: power.c power.h engine.h
pwm.o: pwm.c pwm.h
resume.o: resume.c resume.h debug.h
sound.o: sound.c sound.h
watchdog.o: watchdog.c
widescreen.o: widescreen.c widescreen.h config.h
window.o: window.c gui.h window.h gui_control.h callbacks.h window_context.h debug.h
window_context.o: window_context.c window_context.h file_list.h debug.h
zip_skin.o: zip_skin.c zip_skin.h widescreen.h engine.h debug.h


%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f $(INSTALL_DIR)/tomplayer
	rm -f *.o


