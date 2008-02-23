CC=arm-linux-gcc
CFLAGS+=-DUSE_MINIMIG -Wall


gmplayer: config.o engine.o gui.o widescreen.o
	$(CC) $(LDFLAGS) -o gmplayer widescreen.o config.o engine.o gui.o  -lminigui -lpthread -lc -lm

config.o: config.c config.h engine.h widescreen.h
	$(CC) $(CFLAGS) -c config.c

engine.o: engine.c config.h engine.h
	$(CC) $(CFLAGS) -c engine.c
    
gui.o: gui.c config.h engine.h version.h
	$(CC) $(CFLAGS) -c gui.c

widescreen.o:  widescreen.c config.h widescreen.h
	$(CC) $(CFLAGS) -c widescreen.c
clean:
	rm -f gmplayer
	rm -f *.o
