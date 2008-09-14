#ifndef __INCLUDE_BARCELONA_SND_H
#define __INCLUDE_BARCELONA_SND_H

#include <linux/ioctl.h>

#define PMXF_MIXIN_FIRST	1

typedef void (*cs_primixer_t)(void *destination, void *source, size_t samples, unsigned int flags);

struct fragment_descr {
	unsigned int count;
	size_t size;
};

enum   cs_buffer_state {
	CS_BUFFER_FREE,
	CS_BUFFER_INITIALIZING,
	CS_BUFFER_STOPPED,
	CS_BUFFER_RUNNING,
	CS_BUFFER_STOP_PENDING,
};

enum   cs_buffer_type {
	CSB_TYPE_DUMMY,
	CSB_TYPE_PLAYBACK,
	CSB_TYPE_RECORD,
};

struct cs_buffer_description {
	unsigned int	frequency;
	unsigned int	channels;
	unsigned int	bits_per_sample;
	unsigned int	samples;
	enum cs_buffer_type  type;
};

struct cs_primary_buffer_description {
	unsigned int	frequency;
	unsigned int	channels;
	unsigned int	bits_per_sample;
	unsigned int    chunksize;
	unsigned int	chunks;
};

struct cs_buffer {
	int no;
	
	struct cs_buffer_description desc;
	enum   cs_buffer_state state;
		
	cs_primixer_t	primixer;
	
	void *buffer;
	void *phys;
	size_t size, pointer, spos, samples;
	size_t cbfreq;

	void *owner;
	int mapcount;
	
	unsigned int bytes_per_sample;
};

struct cs_ossemu_private {
	int buffno_read;
	int pointer_read;
	int start_read;
	int buffno_write;
	int pointer_write;
	int start_write;
};

struct cb_chpos {
	int buffer;
	int position;
};

struct cb_cbfreq {
	int buffer;
	int frequency;
};

struct cb_qstop {
	int buffer;
	int stop_position;
};

struct cb_in_out { 
	int in;
	int out;
};

#define COOLSOUND_HEADPHONE (1 << 0)
#define COOLSOUND_PDK (1 << 1)

//#define TDEV
#ifdef TDEV
#define DMA_SIZE (16*1*128) 
#define DMA_CHUNKS 4
#define WRITE_BYTES (8*1*128)
#define WRITE_CHUNKS 128 
#define PRE_BUFFER (500 *32)
struct tdev_link {
	int active;
	void* data;
	int offset;
	struct tdev_link* next;
};
#endif
//#define SECONDARY_CHANNEL_SHIFT 1
#define SECONDARY_CHANNEL_SHIFT 0

#define INITIAL_VOLUME 117
//#define INITIAL_VOLUME 119

#define INITIAL_SAMPLERATE 16000
//#define INITIAL_SAMPLERATE 22050

#define COOLSOUND_DRIVER_MAGIC 'S'

#define COOLSOUND_SET_REG						_IOW(COOLSOUND_DRIVER_MAGIC, 1, unsigned int)
#define COOLSOUND_DEBUG							_IOW(COOLSOUND_DRIVER_MAGIC, 2, unsigned int)
#define COOLSOUND_MUTE_INTERNAL					_IO(COOLSOUND_DRIVER_MAGIC,  3)
#define COOLSOUND_UNMUTE_INTERNAL				_IO(COOLSOUND_DRIVER_MAGIC,  4)
#define COOLSOUND_SET_VOLUME					_IOW(COOLSOUND_DRIVER_MAGIC, 5, unsigned int)
#define COOLSOUND_SET_VOLUME_INTERNAL			_IOW(COOLSOUND_DRIVER_MAGIC, 6, unsigned int)
#define COOLSOUND_SET_VOLUME_EXTERNAL			_IOW(COOLSOUND_DRIVER_MAGIC, 7, unsigned int)
#define COOLSOUND_MUTE_EXTERNAL					_IO(COOLSOUND_DRIVER_MAGIC,  8)
#define COOLSOUND_UNMUTE_EXTERNAL				_IO(COOLSOUND_DRIVER_MAGIC,  9)
#define COOLSOUND_SET_TREBLE					_IOW(COOLSOUND_DRIVER_MAGIC, 10, unsigned int)
#define COOLSOUND_SET_BASS						_IOW(COOLSOUND_DRIVER_MAGIC, 11, unsigned int)
#define COOLSOUND_GET_HEADPHONE_CONNECTED  		_IOR(COOLSOUND_DRIVER_MAGIC, 12, unsigned int)
#define COOLSOUND_SET_VOLUME_EXTERNAL_LEFT  	_IOW(COOLSOUND_DRIVER_MAGIC, 13, unsigned int)
#define COOLSOUND_SET_VOLUME_EXTERNAL_RIGHT 	_IOW(COOLSOUND_DRIVER_MAGIC, 14, unsigned int)
#define COOLSOUND_SWITCH_LINE_IN 				_IOW(COOLSOUND_DRIVER_MAGIC, 15, unsigned int)
#define COOLSOUND_GET_EXTMIC_CONNECTED			_IOR(COOLSOUND_DRIVER_MAGIC, 16, unsigned int)
#define COOLSOUND_SET_EXTMIC_CONNECTION			_IOW(COOLSOUND_DRIVER_MAGIC, 17, unsigned int)

#define COOLSOUND_GET_BUFSIZE					_IOW(COOLSOUND_DRIVER_MAGIC, 19, int)
#define COOLSOUND_GET_BUFFER					_IOW(COOLSOUND_DRIVER_MAGIC, 20, struct cs_buffer_description)
#define COOLSOUND_PUT_BUFFER					_IOW(COOLSOUND_DRIVER_MAGIC, 21, int)
#define COOLSOUND_SEL_BUFFER					_IOW(COOLSOUND_DRIVER_MAGIC, 22, int)

#define COOLSOUND_RUN_BUFFER					_IOW(COOLSOUND_DRIVER_MAGIC, 23, int)
#define COOLSOUND_STOP_BUFFER					_IOW(COOLSOUND_DRIVER_MAGIC, 24, int)
#define COOLSOUND_QUEUE_STOP					_IOW(COOLSOUND_DRIVER_MAGIC, 25, struct cb_qstop)

#define COOLSOUND_WAIT_STOP						_IOW(COOLSOUND_DRIVER_MAGIC, 32, int)

#define COOLSOUND_GETPOS						_IOW(COOLSOUND_DRIVER_MAGIC, 26, int)
#define COOLSOUND_SETPOS						_IOW(COOLSOUND_DRIVER_MAGIC, 27, struct cb_chpos)
#define COOLSOUND_SETCBFREQ						_IOW(COOLSOUND_DRIVER_MAGIC, 28, struct cb_cbfreq)

#define COOLSOUND_SETUP_PRIMARY					_IOW(COOLSOUND_DRIVER_MAGIC, 29, struct cs_primary_buffer_description)
#define COOLSOUND_SETFILTER						_IOW(COOLSOUND_DRIVER_MAGIC, 30, int)

#define COOLSOUND_RUN_IN_OUT					_IOW(COOLSOUND_DRIVER_MAGIC, 31, struct cb_in_out)

#define COOLSOUND_MUTE_NAVIGATION				_IO(COOLSOUND_DRIVER_MAGIC,  32)
#define COOLSOUND_UNMUTE_NAVIGATION				_IO(COOLSOUND_DRIVER_MAGIC,  33)

#define COOLSOUND_MUTE_TELEPHONE				_IO(COOLSOUND_DRIVER_MAGIC,  34)
#define COOLSOUND_UNMUTE_TELEPHONE				_IO(COOLSOUND_DRIVER_MAGIC,  35)


#if 0
#define BARC_SND_SET_REG	COOLSOUND_SET_REG
#define BARC_SND_DEBUG		COOLSOUND_DEBUG
#define BARC_SND_SET_VOLUME	COOLSOUND_SET_VOLUME
#define COOLSOUND_MUTE	COOLSOUND_MUTE_INTERNAL
#define COOLSOUND_UNMUTE	COOLSOUND_UNMUTE_INTERNAL
#endif

#ifdef __KERNEL__
/* Definition of platform data struct for Barcelona sound driver	*/

struct barcelona_sound_info {
	unsigned int divider;	/* Divider to use to generate CODEC clk from iis when in INTERNAL_MASTER mode	*/
};



#endif /* __KERNEL__	*/

#endif
