#
# Tomplayer Gnu Makefile
#

ifndef NATIVE
#Compilation for real Tomtom device
DEP_BUILD=./dependencies/build/usr/local/
DEP_LIB=./dependencies/libs/
INC_KERNEL=$(DEP_LIB)/golinux-tt2318/include
CC=arm-linux-gcc
STRIP=arm-linux-strip
CFLAGS+= -D_GNU_SOURCE -Wall -I$(INC_KERNEL)  -I$(DEP_BUILD)/include/ -I$(DEP_BUILD)/include/directfb
LDFLAGS+=-L$(DEP_BUILD)/lib/ -Wl,-rpath=/usr/local/lib
else
#Compilation for native host
DEP_LIB=./dependencies/libs/
INC_KERNEL=$(DEP_LIB)/golinux-tt2318/include
CC=gcc
STRIP=strip
CFLAGS+=   -DNATIVE -Wall -I$(INC_KERNEL)  -I./dependencies/build_native/include -I./dependencies/build_native/usr/local/include -I/usr/include/freetype2 -I./dependencies/build_native/usr/local/include/directfb
LDFLAGS+=  -L./dependencies/build_native/lib -L./dependencies/build_native/usr/local/lib -ljasper
endif

#installation dir
INSTALL_DIR=../distrib/tomplayer/
#If DEBUG env vraible is set, compile accordingly
ifdef DEBUG
CFLAGS+=-DDEBUG -g 
endif
ifndef VERBOSE
V:=@
endif

#Sources for the initial tomplayer interface 
TOM_SRC = file_selector.c window.c  screens.c gui.c list.c skin.c config.c widescreen.c  resume.c power.c file_list.c label.c viewmeter.c pwm.c  gps.c
#Sources for mplayer engine
ENG_SRC = start_engine.c  engine.c config.c widescreen.c resume.c pwm.c sound.c  power.c font.c fm.c file_list.c diapo.c event_inputs.c play_int.c gps.c draw.c
#Sources for remote inputs 
REM_INPUTS = remote_inputs.c
#All sources
SOURCES:=$(TOM_SRC) $(ENG_SRC) $(REM_INPUTS)

#Objects 
TOM_OBJ = $(subst .c,.o,$(TOM_SRC))
ENG_OBJ = $(subst .c,.o,$(ENG_SRC))
ENG_OBJ += skin_devil.o
REM_OBJ = $(subst .c,.o,$(REM_INPUTS))

TARGETS:=tomplayer refresh_wdg start_engine splash_screen remote_inputs
# carminat_inputs

.PHONY : clean clean_deps install_libdeps all

all : $(TARGETS)

# Final targets dependencies
remote_inputs: $(REM_OBJ)
carminat_inputs: carminat_remote.o
tomplayer: LDFLAGS+= -ldirectfb -lfusion -ldirect -lpthread -lz -lzip  -lm -lpthread -ljpeg -lpng -liniparser $(ADD_LIBS)
tomplayer: $(TOM_OBJ)
start_engine:  LDFLAGS+= -liniparser -lpthread -lz -lzip  -lm -lpthread -ljpeg -lILU  -lIL -lpng -lts -lrt -lfreetype $(ADD_LIBS)
start_engine: $(ENG_OBJ) 
refresh_wdg : watchdog.o
splash_screen : splash.o 


# Objects with specific flags or source to be built from
file_selector.o : CFLAGS+=-D_BSD_SOURCE
skin_devil.o : CFLAGS+=-DWITH_DEVIL
skin_devil.o : skin.c
	$(V)$(CC) $(CFLAGS) -c $< -o $@


#Generic rules
%.o : %.c
	@echo "[Compiling $<]"
	$(V)$(CC) $(CFLAGS) -c $< -o $@

%.d: %.c
	@echo "[Generating dependency for $<]"
	$(V)$(CC) -M $(CFLAGS) $< | sed 's/$*.o/& $@/g' > $@

% : 
	@echo "[Linking and installing $@]"
	$(V)$(CC) -o $@ $^  $(LDFLAGS)
	$(V)cp $@ $(INSTALL_DIR)	
	$(V)$(STRIP) $(INSTALL_DIR)/$@

#Other make targets
clean: 
	rm -f $(addprefix $(INSTALL_DIR)/,$(TARGETS))	
	rm -f $(TARGETS)
	rm -f *.o

clean_deps :
	rm -f *.d

install_libdeps :
	rm -rf $(INSTALL_DIR)/deps
	make -C dependencies/libs/
	./create_script.sh

#include auto dependencies 
NOINCLUDE=clean clean_deps
ifeq ($(filter $(NOINCLUDE),$(MAKECMDGOALS)),)
-include $(SOURCES:.c=.d)
endif
