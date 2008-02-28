CC=arm-linux-gcc
CFLAGS+=-DUSE_MINIMIG -Wall
INSTALL_DIR=../distrib/tomplayer/

SRC = config.c engine.c gui.c widescreen.c resume.c

OBJ = $(subst .c,.o,$(SRC))

all : tomplayer

tomplayer: $(OBJ)
	$(CC) $(LDFLAGS) -o tomplayer $^ -lminigui -lpthread -lc -lm $(ADD_LIBS)
	mv tomplayer $(INSTALL_DIR)


#Explicit non trivial dependencies
config.o: config.c config.h engine.h widescreen.h
engine.o: engine.c config.h engine.h resume.h
gui.o: gui.c config.h engine.h version.h
widescreen.o:  widescreen.c config.h widescreen.h

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f $(INSTALL_DIR)/tomplayer
	rm -f *.o
