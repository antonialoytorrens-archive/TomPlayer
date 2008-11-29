#ifndef MP3_HDR_H
#define MP3_HDR_H

int mp_get_mp3_header(unsigned char* hbuf,int* chans, int* freq, int* spf, int* mpa_layer, int* br);

#define mp_decode_mp3_header(hbuf)  mp_get_mp3_header(hbuf,NULL,NULL,NULL,NULL,NULL)

static inline int mp_check_mp3_header(unsigned int head){
    if( (head & 0x0000e0ff) != 0x0000e0ff ||  
        (head & 0x00fc0000) == 0x00fc0000) return 0;
    if(mp_decode_mp3_header((unsigned char*)(&head))<=0) return 0;
    return 1;
}

#endif /* MP3_HDR_H */
