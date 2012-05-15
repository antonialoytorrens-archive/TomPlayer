/** 
 * \file gps.c
 * \author Stephan Rafin
 *
 * This module handles GPS to extract useful information from SiRF geodetic message
 *
 * $URL$
 * $Rev$
 * $Author$
 * $Date$
 *
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "gps.h"

#ifdef DEBUG
#define PRINTDF(s, ...) fprintf (stderr, (s), ##__VA_ARGS__)
#else
#define PRINTDF(s, ...)
#endif


/* Internal SiRF constants and format description */
#define SIRF_SYNC 0xA2A0
#define SIRF_POST 0xB3B0
#define SIRF_FRAME_LEN_MIN 8
#define SIRF_FRAME_FOOTER_LEN 4
#define SIRF_FRAME_HEADER_LEN 4
#define SIRF_FRAME_CHCKSUM_LEN 2
#define SIRF_PAYLOAD_MAX 1023
#define SIRF_CKSUM_MASK 0x7FFF;
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
    uint16_t msecs;
    uint32_t sat_id_list;
    int32_t latitude;
    int32_t longitude;
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

#pragma pack() 

/* GPS module status */
static struct {
    int gpsfd;
    struct gps_data data;
    pthread_mutex_t data_mutex;    
}gps_state = {
    .gpsfd = -1,
    .data_mutex = PTHREAD_MUTEX_INITIALIZER
};


/** Stupid dump buffer debug function */
#if DEBUG
static void dump(unsigned char * buffer, int len ){
    int i;
    for(i = 0; i < len; i++){
        if ((i % 16) == 0){
            PRINTDF ("\n");
        }
        PRINTDF("%0.2X ", buffer[i]);
          
    }
    PRINTDF("\n");
}
#else 
#define dump(x,y) do{}while(0)
#endif

/** 16 bits endianess hleper */
static inline uint16_t endian16_swap(uint16_t val){    
    uint16_t temp;
    temp = val & 0xFF;
    temp = (val >> 8) | (temp << 8);    
    return temp;
}

/** 32 bits endianess hleper */
static inline uint32_t endian32_swap(uint32_t val){    
    uint32_t temp;
    temp = ( val >> 24) | ((val & 0x00FF0000) >> 8) |  ((val & 0x0000FF00) << 8) | ((val & 0x000000FF) << 24); 
    return temp;
}


/** Handle Output SiRF message 
 *
 * \note For now we are only interested in Geodetic message
 */
static void handle_msg(unsigned char * buffer, int len ){
    time_t curr_time, gps_time;
    struct timeval new_time;     
    char * saved_tz = NULL;    
    struct geodetic_nav_data * msg = (struct geodetic_nav_data *)buffer;
    
    if ((len != SIRF_GEODETIC_MSG_LEN) || (msg->msg_id != SIRF_GEODETIC_MSGID)){
        return;
    }
    /* Extract values from geodetic msg */
    pthread_mutex_lock(&gps_state.data_mutex);
    gps_state.data.seq += 1;
    gps_state.data.lat_deg = ((int32_t)endian32_swap(msg->latitude)) / 10000000;
    gps_state.data.lat_mins = ((endian32_swap(msg->latitude) * 60) / 10000000 ) % 60;     
    gps_state.data.long_deg =((int32_t)endian32_swap(msg->longitude)) / 10000000;    
    gps_state.data.long_mins = ((endian32_swap(msg->longitude) * 60) / 10000000 ) % 60;     
    gps_state.data.alt_cm = endian32_swap(msg->altitude);
    gps_state.data.sat_id_list = endian32_swap(msg->sat_id_list);
    gps_state.data.sat_nb = msg->sv_nb;
    gps_state.data.speed_kmh = (endian16_swap(msg->speed) * 36) / 1000;
    gps_state.data.time.tm_sec = endian16_swap(msg->msecs)/1000;    
    gps_state.data.time.tm_min = msg->minuts;    
    gps_state.data.time.tm_hour = msg->hour;    
    gps_state.data.time.tm_mday = msg->day;
    gps_state.data.time.tm_mon = msg->month - 1;
    gps_state.data.time.tm_year = endian16_swap(msg->year) - 1900;
    gps_state.data.time.tm_isdst = -1;
    time(&curr_time);  
    if (getenv("TZ") != NULL){
        saved_tz = strdup(getenv("TZ"));    
    } else{
        saved_tz = NULL;
    }
    unsetenv("TZ");
    gps_time = mktime(&gps_state.data.time);
    if (abs(gps_time - curr_time) > 10){
        PRINTDF("Syncing clock needed ! system : %d - GPS : %d\n", curr_time, gps_time);
        new_time.tv_sec = gps_time;
        new_time.tv_usec = 0;        
        settimeofday(&new_time, NULL);
    }
    if (saved_tz != NULL){
        setenv("TZ", saved_tz, 1);
        free(saved_tz);
    }    
    pthread_mutex_unlock(&gps_state.data_mutex);
    PRINTDF("Geodetic OK !\n");
};

/** Compute SiRF checksum 
 * \return The computed checksum
 */
static uint16_t sirf_chksum(unsigned char * buffer, int len ){
    int i;
    uint16_t cksum = 0;
    
    for (i = 0; i < len; i++){
        cksum = (cksum + buffer[i]) & SIRF_CKSUM_MASK;
    }
    return cksum ;
}

/** Check SiRF frame validity 
 *
 * 
 * \retval >0 Valid frame, returns its length (full frame len including header and footer)
 * \retval  0 Incomplete frame
 * \retval -1 Invalid frame
 */
static int check_frame(unsigned char * buffer, int len ){
    uint16_t payload_len;
    struct sirf_footer * footer;
    struct sirf_header * header = (struct sirf_header *)buffer;
    if (len <= SIRF_FRAME_LEN_MIN){
        return 0;
    }
    payload_len = endian16_swap(header->len);
    if (payload_len >= SIRF_PAYLOAD_MAX){      
        PRINTDF("Error payload length : %d\n", payload_len);
        return -1;
    } else {
        if ((payload_len + SIRF_FRAME_HEADER_LEN + SIRF_FRAME_FOOTER_LEN) > len){
            PRINTDF("Not enough data \n");
            return 0;
        } else {
            footer = ((struct sirf_footer * )&buffer[payload_len  + SIRF_FRAME_HEADER_LEN]);
            
            /* We have enought bytes - Perform sanity check */
            if (footer->post_sync != SIRF_POST){     
                PRINTDF("Post sync not found \n");
                return -1;
            }
            if (sirf_chksum(&buffer[SIRF_FRAME_HEADER_LEN], payload_len) != endian16_swap(footer->chk)){
                PRINTDF("Bad cksum !\n");
                return -1;
            }
            /* Frame is valid ! */ 
            handle_msg(&buffer[SIRF_FRAME_HEADER_LEN], payload_len);
            return (payload_len + SIRF_FRAME_HEADER_LEN + SIRF_FRAME_FOOTER_LEN);      
        }
    }
}


/** Initialize the GPS module */
int gps_init (void){      
    gps_state.gpsfd = open("/dev/gpsdata", O_RDONLY|O_NONBLOCK);
    if (gps_state.gpsfd < 0)
    {    
        gps_state.gpsfd = -1;
        return -1;        
    }       
    return 0;
}


/** Read and Parse available GPS data
 *  This function has to be called regulary to refresh the data provided by gps_get_data()
 *
 * \retval -1 No data was available
 * \retval  0 Data has been handled
 */
int gps_update(void){
    #define BUFFER_LEN 1034 /* More than max SIRF frame */
    static unsigned char buffer[BUFFER_LEN];    
    static int curr_idx = 0; 
    struct sirf_header * header;
    int frame_state;
    int read_len;
    int len;
    int i;
    
    if (gps_state.gpsfd == -1){
        return -1;
    }
    read_len = read(gps_state.gpsfd, &buffer[curr_idx], sizeof (buffer)-curr_idx);
    if (read_len <= 0)
        return -1;

    /* Search Prelude Sync            
        We look for a 16 bits synchro word */
    len = curr_idx + read_len ;        
    PRINTDF("Buffer : curr idx : %d - %d \n", curr_idx, read_len );
    dump(buffer, len);
    i = 0;
    while (i < (len -1)){        
        header = (struct sirf_header *)&buffer[i];
        if (header->sync == SIRF_SYNC){
            PRINTDF("Sync found at %d\n", i);
            frame_state = check_frame(&buffer[i], len - i);
            PRINTDF("frame state : %d\n", frame_state);
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
    if (len >= i) {
        memmove(&buffer[0] , &buffer[i], len - i);
        curr_idx = len - i;
        PRINTDF("i : %d - cur idx = %i \n", i,  len - i);
    } else {
        PRINTDF("Error  we should never be there\n");
    }
    return 0;
}

/** Retrieve GPS data 
  * \param[out] data the retrieved GPS data
  *
  * \retval  0 OK
  * \retval -1 KO
  * \note this function can be called by another thread than the one which calls gps_read()
  */
int gps_get_data(struct gps_data *data){
    if (gps_state.gpsfd == -1) 
        return -1;
    pthread_mutex_lock(&gps_state.data_mutex);
    *data = gps_state.data;
    pthread_mutex_unlock(&gps_state.data_mutex);
    return 0;
}

#if 0
/* To test as a standalone executable */
int main(){
    struct gps_data info;
    int disp_seq = 0;
    
    gps_init();
    while (1){
        if (gps_update() == -1){
            gps_get_data(&info);
            if (info.seq != disp_seq){
                time_t curr_time;
                struct tm * ptm;   
                disp_seq = info.seq;
                printf("Lat  : %i°%i'\n", info.lat_deg, info.lat_mins);
                printf("Long : %i°%i'\n", info.long_deg, info.long_mins);  
                printf("Alt  : %i,%im\n", info.alt_cm / 100, info.alt_cm % 100);
                printf("Sats : %i\n",    info.sat_nb );
                printf("vit  : %ikm/h\n", info.speed_kmh);
                printf("TimeG: %s",asctime(&info.time));
                time(&curr_time);
                ptm = localtime(&curr_time);
                printf("TimeS: %02d : %02d\n",ptm->tm_hour, ptm->tm_min );  
            }
            usleep(100000);
        }
    }
}
#endif
