#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <linux/input.h>
#include <directfb.h>
#include <signal.h>


#include "config.h"

int main(int argc, char ** argv ){
  int fd_event, fifo_fd;
  struct input_event ev;
  int nb;
  DFBInputDeviceKeyIdentifier dfb_key;
/*
  FILE *log_file;
  
  log_file = fopen("/media/sdcard/log/evts2.txt", "w+");
  */
  unlink(KEY_INPUT_FIFO);
  mkfifo(KEY_INPUT_FIFO, 0777);
  fifo_fd=open(KEY_INPUT_FIFO, O_WRONLY);
  if (fifo_fd < 0){
    printf("Error while opening " KEY_INPUT_FIFO "\n");
    exit(1);
  }
    
  do {      
#ifdef NATIVE
    fd_event = open("/dev/input/event1", O_RDONLY);
#else
    fd_event = open("/dev/input/event0", O_RDONLY);
#endif    
    if (fd_event < 0){
        printf("Error while opening /dev/input/event0 \n");
        perror("evdev open");	
        sleep(2);
    }    
	} while (fd_event < 0);
  signal(SIGPIPE, SIG_IGN);
    
	while(1) {
        dfb_key = 0;
    nb = read(fd_event, &ev, sizeof(struct input_event));  
/*  printf ("Raw evt : type 0x%x code 0x%x value 0x%x\n", ev.type, ev.code, ev.value);
    if (log_file != NULL){
      fprintf(log_file,"Raw evt : type 0x%x code 0x%x value 0x%x\n", ev.type, ev.code, ev.value);
      fflush(log_file);
    }*/
    if (nb > 0) {
    if (ev.value == 1 ) {
    /* button released*/
    switch (ev.code){
                case KEY_UP :
                    dfb_key = DIKI_UP; 
                    break;
                case KEY_DOWN :
                    dfb_key = DIKI_DOWN;
                    break;
                case KEY_LEFT :
                    dfb_key = DIKI_LEFT;
                    break;
                case KEY_RIGHT :
                    dfb_key = DIKI_RIGHT;
                    break;
                case KEY_ENTER :
                    dfb_key = DIKI_ENTER; 
                    break;   
                case KEY_SPACE :
                case KEY_EQUAL : /* BT remote => Back */
                    dfb_key = DIKI_BACKSPACE; 
                    break;                    
                case KEY_KPMINUS :
                case KEY_ESC : /* BT remote => select left */
                    dfb_key = DIKI_KP_MINUS; 
                    break;                    
                case KEY_KPPLUS :
                case KEY_BACKSPACE : /* BT remote => Select Right */                    
                    dfb_key = DIKI_KP_PLUS; 
                    break;    
                case KEY_F9 :  /* BT Remote => map */          
                    dfb_key = DIKI_F9; 
                    break;
                case KEY_F10 : /* BT Remote => menu */          
                    dfb_key = DIKI_F10; 
                    break;    
                case KEY_F8 :   
                     dfb_key = DIKI_F8;
                     break;
                case KEY_F7 :   
                     dfb_key = DIKI_F7;
                     break;
                case KEY_F6 :   
                     dfb_key = DIKI_F6;
                     break;     
                case KEY_F5 :   
                     dfb_key = DIKI_F5;
                     break;     
                default :
                    /* Unhandled key */
                    break;
            }            
/*            printf ("Key read from remote : 0x%x\n",dfb_key);     
            if (log_file != NULL){
              fprintf(log_file,"Key read from remote : 0x%x\n",dfb_key );
              fflush(log_file);
            }*/            
            if (dfb_key){
                write(fifo_fd, &dfb_key, sizeof(dfb_key));
            }
            
		}
		} else {
		  printf("Read error : %i\n",nb);
		}
    }
	return 0;
}

