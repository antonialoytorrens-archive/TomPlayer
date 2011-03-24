/* Retrieves geodetic informations from GPS SiRF data
*
*/


#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define SIRF_SYNC 0xA0A2
#define SIRF_POST 0xB3B0
#define SIRF_FRAME_LEN_MIN 8
#define SIRF_FRAME_FOOTER_LEN 4
#define SIRF_FRAME_HEADER_LEN 4
#define SIRF_FRAME_CHCKSUM_LEN 2
#define SIRF_PAYLOAD_MAX 1023
#define SIRF_CKSUM_MASK 0x7FFF;

struct gps_data{
    unsigned short int lat_deg;
    unsigned short int lat_mins;
    unsigned short int long_deg;
    unsigned short int long_mins;
    unsigned int alt_cm;
    unsigned int sat_id_list;
    unsigned int sat_nb;
    unsigned short int speed_kmh;
    struct tm time;
};

#define SIRF_GEODETIC_MSGID 0x29
#define SIRF_GEODETIC_MSG_LEN 91

#pragma pack(1)
struct geodetic_nav_data{
    uint8_t  msg_id;
    uint16_t nav_valid;
    uint16_t nav_type;
    uint16_t week_nb;
    uint32_t TOW;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minuts;
    uint16_t seconds;
    uint32_t sat_id_list;
    uint32_t latitude;
    uint32_t longitude;
    uint32_t alt_ellips;
    uint32_t altitude;
    uint8_t map_daturn;
    uint16_t speed;
    uint8_t dummy[46];
    uint8_t sv_nb;
    uint8_t dummy2[2];
};


struct sirf_header{
    uint16_t sync;
    uint16_t len;
};

struct sirf_footer{
    uint16_t chk;
    uint16_t post_sync;
};

//#pragma pack() 

static int gpsfd = -1;
static struct gps_data data;


static void dump(unsigned char * buffer, int len ){
    int i;
    for(i = 0; i < len; i++){
            printf("0.2X ", buffer[i]);
            if ((i % 16) == 0){
                printf ("\n");
            }
    }
}

static inline uint16_t endian16_swap(uint16_t val){    
    uint16_t temp;
    temp = val & 0xFF;
    temp = (val >> 8) | (temp << 8);
    return temp;
}


static void handle_msg(unsigned char * buffer, int len ){
    struct geodetic_nav_data * msg = (struct geodetic_nav_data *)buffer;
    if ((len != SIRF_GEODETIC_MSG_LEN) || (msg->msg_id != SIRF_GEODETIC_MSGID)){
        return;
    }
    /* Extract values from geodetic msg */
    
    data.lat_deg = msg->latitude / 10000000;
    data.lat_mins = ((msg->latitude * 60) / 10000000 ) % 60;     
    data.long_deg = msg->longitude / 10000000;    
    data.long_mins = ((msg->longitude * 60) / 10000000 ) % 60;     
    data.alt_cm = msg->altitude;
    data.sat_id_list = msg->sat_id_list;
    data.sat_nb = msg->sv_nb;
    data.speed_kmh = (msg->speed * 36) / 1000;
    data.time.tm_sec = msg->seconds;
    data.time.tm_min = msg->minuts;
    data.time.tm_hour = msg->hour;
    data.time.tm_mday = msg->day;
    data.time.tm_mon = msg->month;
    data.time.tm_year = msg->year - 1900;
    data.time.tm_isdst = -1;
    mktime(&data.time);
};

static uint16_t sirf_chksum(unsigned char * buffer, int len ){
    int i;
    uint16_t cksum = 0;
    
    for (i = 0; i < len; i++){
        cksum = (cksum + buffer[i]) & SIRF_CKSUM_MASK;
    }
    return cksum ;
}

/**
\retval >0 Valid frame, returns its length (full frame len including header and footer)
\retval  0 Incomplete frame
\retval -1 Invalid frame
*/
static check_frame(unsigned char * buffer, int len ){
    uint16_t payload_len;
    struct sirf_footer * footer;
    struct sirf_header * header = (struct sirf_header *)buffer;
    if (len <= SIRF_FRAME_LEN_MIN){
        return 0;
    }
    payload_len = endian16_swap(header->len);
    if (payload_len >= SIRF_PAYLOAD_MAX){
        return -1;
    } else {
        if ((payload_len + SIRF_FRAME_HEADER_LEN + SIRF_FRAME_FOOTER_LEN) > len){
            return 0;
        } else {
            footer = ((struct sirf_footer * )&buffer[payload_len  + SIRF_FRAME_HEADER_LEN]);
            
            /* We have enought bytes - Perform sanity check */
            if (footer->post_sync != SIRF_POST){
                return -1;
            }
            if (sirf_chksum(&buffer[SIRF_FRAME_HEADER_LEN], payload_len) != footer->chk){
                return -1;
            }
            /* Frame is valid ! */ 
            handle_msg(&buffer[SIRF_FRAME_HEADER_LEN], payload_len);
            return (payload_len + SIRF_FRAME_HEADER_LEN + SIRF_FRAME_FOOTER_LEN);      
        }
    }
}


int gps_init (void){      
    gpsfd = open("/dev/gpsdata", O_RDONLY|O_NONBLOCK);
    if (gpsfd < 0)
    {    
        gpsfd = -1;
        return -1;        
    }       
    return 0;
}


int gps_read(){
    #define BUFFER_LEN 1034 /* More than max SIRF frame */
    static unsigned char buffer[BUFFER_LEN];    
    static int curr_idx = 0; 
    struct sirf_header * header;
    int frame_state;
    int read_len;
    int len;
    int i;
       
    read_len = read(gpsfd, &buffer[curr_idx], sizeof (buffer)-curr_idx);
    if (read_len <= 0)
        return;
    /* Search Prelude Sync            
        We look for a 16 bits synchro word */
    len = curr_idx + read_len ;        
    i = 0;
    while (i < (len -1)){
        header = (struct sirf_header *)buffer;
        if (header->sync == SIRF_SYNC){
            printf("Sync found at %d\n", i);
            frame_state = check_frame(&buffer[i], len - i);
            if (frame_state > 0){
                /* valid frame */                
                i += frame_state;
            } else {
                if (frame_state == 0) {
                    /* Incomplete frame*/                        
                    break;
                } else {
                    /* Invalid frame */
                    i++;
                }
            }
        } else {
            i++;
        }
    }
    if (len > i) {
        memmove(&buffer[0] , &buffer[i], len - i);
        curr_idx = len - i;
    } 

}


int main(){
    gps_init();
    gps_read();
}
