/** Screen shot server
 * 
 */


#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MYPORT 2007  
#define MAXBUFLEN 100

static unsigned short buffer_screen[480*272];

int dump_screen(unsigned char * buffer, int * size, int max_size){	
  static char * fb_mmap;
  static struct fb_var_screeninfo screeninfo;
  char * fb_dev;
  int fb_fd; 
  
  if (fb_mmap == NULL){
	  fb_dev = getenv("FRAMEBUFFER");
	  if (fb_dev == NULL){
	    fb_dev = "/dev/fb";
	  } 	
	  fb_fd = open(fb_dev, O_RDONLY);
	  if (fb_fd < 0){
	    fprintf(stderr,"Error while trying to open frame buffer device %s \n", fb_dev);
	    return -1;    
	  } else {
	    if (ioctl (fb_fd, FBIOGET_VSCREENINFO, &screeninfo) != 0){
	      fprintf(stderr,"Error while trying to get info on frame buffer device %s \n", fb_dev);
	      return -1;
	    }
	  }    
 	  fb_mmap = mmap(NULL, 2* screeninfo.xres * screeninfo.yres, PROT_READ,MAP_SHARED, fb_fd, 0);
 	  if (fb_mmap == MAP_FAILED ){
 		 fprintf(stderr,"Error while trying to mmap frame buffer device %s \n", fb_dev);
 		 perror("mmap");
 		 return -1;
 	  }
  }
  
  *size = screeninfo.xres * screeninfo.yres * 2;
  printf("OK mmap @%x - size : %i\n", fb_mmap, *size);
  if (max_size < *size){
	  return -1;
  }  
  memcpy(buffer, fb_mmap, *size);
  return 0;    
}

int main(){
	int sockfd;
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t addr_len;
    int numbytes;
    char buf[MAXBUFLEN];
    int screen_len;
    int i;
    int yes = 1;
    int new_fd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
           perror("setsockopt");
           exit(1);
    }

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1) {
        perror("bind");
        exit(1);
    }
    
    if (listen(sockfd, 10) == -1) {
            perror("listen");
            exit(1);
    }

    
    while (1){
    	addr_len = sizeof their_addr;
    	
    	if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, \
    	                &addr_len)) == -1) {
    	           perror("accept");
    	           continue;
    	}

        
    	if (dump_screen(buffer_screen,&screen_len, sizeof(buffer_screen)) != 0){
    		fprintf(stderr,"Dump screen error\n");
    		exit(1);
    	}
    	
    		
    		if ((numbytes = send(new_fd, buffer_screen, screen_len, 0) ) == -1) {
    		    			perror("send");
    		    			exit(1);
    		}    		
    		printf("sent : %i\n", numbytes);
    		sleep(2);
    		close(new_fd);
    	
    	printf("got packet from %s and replied \n",inet_ntoa(their_addr.sin_addr));    	    	
    }
    
    close(sockfd);
    return 0;	
}
