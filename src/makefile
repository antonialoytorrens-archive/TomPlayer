ifndef NATIVE
DEP_BUILD=./dependencies/build/usr/local/
DEP_LIB=./dependencies/libs/
INC_KERNEL=$(DEP_LIB)/golinux-tt2318/include
CC=arm-linux-gcc
STRIP=arm-linux-strip
CFLAGS+=    -Wall -I$(INC_KERNEL)  -I$(DEP_BUILD)/include/ -I$(DEP_BUILD)/include/directfb
LDFLAGS+=-L$(DEP_BUILD)/lib/ -Wl,-rpath=/usr/local/lib
else
DEP_LIB=./dependencies/libs/
INC_KERNEL=$(DEP_LIB)/golinux-tt2318/include
CC=gcc
STRIP=strip
CFLAGS+=   -DNATIVE -Wall -I$(INC_KERNEL)  -I./dependencies/build_native/include -I./dependencies/build_native/usr/local/include -I/usr/include/freetype2 -I./dependencies/build_native/usr/local/include/directfb
LDFLAGS+=  -L./dependencies/build_native/lib -L./dependencies/build_native/usr/local/lib 
endif


INSTALL_DIR=../distrib/tomplayer/

ifdef DEBUG
CFLAGS+=-DDEBUG -g 
endif

TOM_SRC = file_selector.c window.c  screens.c gui.c list.c zip_skin.c config.c widescreen.c  resume.c pwm.c power.c
ENG_SRC = start_engine.c  engine.c config.c widescreen.c resume.c pwm.c sound.c  power.c font.c

# callbacks.c file_list.c playlist.c

TOM_OBJ = $(subst .c,.o,$(TOM_SRC))
ENG_OBJ = $(subst .c,.o,$(ENG_SRC))
ENG_OBJ += zip_skin_devil.o

all : tomplayer refresh_wdg start_engine


tomplayer: $(TOM_OBJ)
	$(CC) $(LDFLAGS) -o tomplayer $^  -liniparser -ldirectfb -lfusion -ldirect -lpthread -lz -lzip  -lm -lpthread -ljpeg -lpng $(ADD_LIBS)
	cp tomplayer $(INSTALL_DIR)	
#	mv tomplayer $(INSTALL_DIR)				
	$(STRIP) $(INSTALL_DIR)/tomplayer

start_engine : $(ENG_OBJ) 
	$(CC) $(LDFLAGS) -o start_engine $^  -liniparser -lpthread -lz -lzip  -lm -lpthread -ljpeg -lILU  -lIL -lpng -lts -lfreetype $(ADD_LIBS)
	cp start_engine $(INSTALL_DIR)	
#	mv tomplayer $(INSTALL_DIR)				
	$(STRIP) $(INSTALL_DIR)/start_engine
	

refresh_wdg : watchdog.o
	$(CC) $(LDFLAGS) -o refresh_wdg $^
	$(STRIP) refresh_wdg
	mv refresh_wdg $(INSTALL_DIR)

install_deps :
	rm -rf $(INSTALL_DIR)/deps
	make -C dependencies/libs/
	./create_script.sh


file_selector.o : CFLAGS+=-D_BSD_SOURCE

zip_skin_devil.o : CFLAGS+=-DWITH_DEVIL
zip_skin_devil.o : zip_skin.c
	$(CC) $(CFLAGS) -c $< -o $@


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


