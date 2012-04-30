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

/* FIXME Do not post in FIFO */
//#define NOPOST
/* Debug Logs */
//#define DEBUG_LOG
#undef DEBUG_LOG

#ifdef NATIVE
#define INIT_DEV 1
#else
#define INIT_DEV 0
#endif 
  
int main(int argc, char ** argv ){
  char input_dev[256];
  int fd_event;
  int fifo_fd = -1;
  struct input_event ev;
  int nb;
  DFBInputDeviceKeyIdentifier dfb_key;
  FILE *log_file = NULL;
  int current_dev = INIT_DEV;
  
#ifdef DEBUG_LOG 
  log_file = fopen("/media/sdcard/tomplayer/evts2.txt", "a+");
#endif
  
  unlink(KEY_INPUT_FIFO);
  mkfifo(KEY_INPUT_FIFO, 0777);
#ifndef NOPOST
  fifo_fd=open(KEY_INPUT_FIFO, O_WRONLY);
  if (fifo_fd < 0){
    printf("Error while opening " KEY_INPUT_FIFO "\n");
    exit(1);
  }  
#endif
  if (log_file != NULL)
    fprintf(log_file,"Starting ...\n");
  
  signal(SIGPIPE, SIG_IGN);
  fd_event = -1;
  while(1) {
     while (fd_event < 0) {
        sprintf(input_dev, "/dev/input/event%i", current_dev);
        fd_event = open(input_dev, O_RDONLY);
        if (fd_event < 0){
            if (log_file != NULL) {
                fprintf(log_file,"Error while opening %s\n", input_dev);
                fflush(log_file);
            }
            printf("Error while opening %s\n", input_dev);
            sleep(1);
            current_dev++;
            if (current_dev > 9)
                current_dev = INIT_DEV;
        } else {
		fprintf(log_file,"Opening %s OK\n", input_dev);
                fflush(log_file);
	}
    }
    dfb_key = 0;
    nb = read(fd_event, &ev, sizeof(struct input_event));  
    /*printf ("Raw evt : type 0x%x code 0x%x value 0x%x\n", ev.type, ev.code, ev.value);*/
    if (log_file != NULL){
      fprintf(log_file,"Raw evt : type 0x%x code 0x%x value 0x%x\n", ev.type, ev.code, ev.value);
      fflush(log_file);
    }
    if (nb > 0) {
    if (ev.value == 1) {
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
/*            printf ("Key read from remote : 0x%x\n",dfb_key);     */
            if (log_file != NULL){
              fprintf(log_file, "Key read from remote : 0x%x\n",dfb_key);
              fflush(log_file);
            }
            if ((dfb_key) && (fifo_fd>0)) {
                write(fifo_fd, &dfb_key, sizeof(dfb_key));
            }
            
        }
        } else {
            //printf("Read error : %i\n",nb);
            if (log_file != NULL){
                fprintf(log_file,"Read error : %i\n",nb);
                fflush(log_file);
            }
            close(fd_event);
	    fd_event = -1;
        }
    }
    return 0;
}

