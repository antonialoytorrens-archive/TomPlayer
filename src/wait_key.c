#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <directfb.h>
#include <time.h>
#include "config.h"

#if 0
   case CARM_KEY_TOP_LEFT:
        key = DIKI_KP_MINUS; 
    case CARM_KEY_TOP_RIGHT:
        key = DIKI_KP_PLUS;
    case CARM_KEY_MAP:
        key = DIKI_F9;
    case CARM_KEY_MENU:
        key = DIKI_F10;
    case CARM_KEY_BACK:
        key = DIKI_BACKSPACE;;
    /* The following keys are not on the remote */
    case CARM_KEY_INFO:    
        key = DIKI_F8;
    case CARM_KEY_LIGHT:
        key = DIKI_F7;
    case CARM_KEY_REPEAT:
        key = DIKI_F6;
    case CARM_KEY_DEST:
        key = DIKI_F5;
#endif
        

int main(int argc, char **argv) {
  DFBInputDeviceKeyIdentifier key; 
  int input_fd = -1;
  int counter = 0;
  time_t first_ts;
  time_t current_ts;
  int found = 0;
  
  /* Dont want to be killed by SIGPIPE */
  signal (SIGPIPE, SIG_IGN);   

  do {
  printf("opening FIFO\n");
  input_fd = open(KEY_INPUT_FIFO, O_RDONLY);  
  if (input_fd < 0){
    fprintf(stdout, "Error while opening FIFO");
    exit(-1);
  } 
  printf("FIFO opened\n");
  /* Read events  */
  while (read(input_fd, &key, sizeof(key)) > 0) {
      if (key == DIKI_BACKSPACE) {
          time(&current_ts);
          counter++;
          if (counter == 1)
              first_ts = current_ts;
          if (counter >= 6) {
            if ((current_ts - first_ts) < 5) {
                found = 1;
                break;
            }
            else {
                counter = 1;
                first_ts = current_ts;
            }
          }
      } else {
          counter = 0;
      }          
  }  
  close(input_fd);
  sleep(2);
  } while (found != 1);
  return 0;  
}
