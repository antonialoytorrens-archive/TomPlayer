#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char ** argv ){
	static unsigned short * fb_mmap;
	static struct fb_var_screeninfo screeninfo;
	unsigned short * buffer16;
	int buffer_size;
	int fb;
	int i,j, fd;
        char  filename[1024];
        int is_inverted;

        if (argc<2){
          printf("%s base_filename\n", argv[0]);
          return -1;
        } 

        fb = open( "/dev/fb", O_RDWR);
        if (fb < 0){  
          fb = open( "/dev/fb0", O_RDWR);
          if (fb < 0){  
            perror("unable to open fb ");
            return -1;
          }
        }
        ioctl (fb, FBIOGET_VSCREENINFO, &screeninfo);
        fb_mmap = mmap(NULL,  screeninfo.xres*screeninfo.yres*2 , PROT_READ|PROT_WRITE,MAP_SHARED, fb, 0);
        if (fb_mmap == MAP_FAILED){
          perror("unable to mmap fb ");
          fb_mmap = NULL;
          close(fb);
          return -1;
        }
        close(fb);

        if (screeninfo.yres > screeninfo.xres){
          is_inverted = 1;
        } else {
          is_inverted = 0;
        }

	/* Alloc buffer for RBG conversion */
	buffer_size =  screeninfo.xres*screeninfo.yres*2;
	buffer16 = malloc( buffer_size );
	if (buffer16 == NULL){
	    fprintf(stderr, "Allocation error\n");
	    return -1;
	}
        /*printf("probing X : %i , y: %i , inverted : %i\n",screeninfo.xres, screeninfo.yres,  is_inverted);*/
        sprintf(filename,"%s_%i_%i.bin", argv[1], is_inverted ? screeninfo.yres : screeninfo.xres, is_inverted ? screeninfo.xres : screeninfo.yres);
        fd = open(filename, O_RDONLY);
        if (fd < 0) {
            printf("Cannot open %s\n", filename);
             return -1;
        }
        read(fd, buffer16, buffer_size);
        close(fd);

        if (!is_inverted){
                memcpy(fb_mmap, buffer16, buffer_size);
        }else{
                /* Magic combination for inverted coordinates */
                for (i=0; i<screeninfo.xres; i++){
                        for(j=0; j<screeninfo.yres; j++){
                                fb_mmap[j*screeninfo.xres+i]=buffer16[(-i+screeninfo.xres-1)*screeninfo.yres+j];
                        }
                }
        }

	
	free( buffer16 );
  return 0;
}
