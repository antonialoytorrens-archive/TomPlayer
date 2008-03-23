
ifndef INC_KERNEL
	INC_KERNEL=../../../../../golinux/include
endif

CC=arm-linux-gcc
STRIP=arm-linux-strip
CFLAGS+=-DUSE_MINIMIG -Wall -I$(INC_KERNEL) 


TURN_SRC = turn_off.c 
TURN_OBJ = $(subst .c,.o,$(TURN_SRC))

CALIB_SRC = calib.c
CALIB_OBJ= $(subst .c,.o,$(CALIB_SRC))

all : turn_off calib

turn_off : $(TURN_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^  -lc 
	$(STRIP) $@

calib : $(CALIB_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^  -lc 
	$(STRIP) $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:	
	rm -f *.o
	rm calib turn_off

