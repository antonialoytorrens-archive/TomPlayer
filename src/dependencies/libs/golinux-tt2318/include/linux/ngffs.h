/* include/linux/ngffs.h
 *
 * NGFFS, next generation flash file system
 * Public data structures and defines
 *
 * Copyright (C) 2004-2007 TomTom BV <http://www.tomtom.com/>
 * Author: Koen Martens <kmartens@sonologic.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_LINUX_NGFFS_H
#define __INCLUDE_LINUX_NGFFS_H

//
// Configuration
//

#define NGFFS_MAXCSIZE		512	// maximum chunk size
#define NGFFS_MAXFSIZE		(30*1024) // maximum file size
#define NGFFS_MAXDIRENT		32	// maximum entries per directory
#define NGFFS_MAX_NAME_LEN	256
					// (i) we want to make sure that
					// sizeof(struct dir_chunk)+
					//  (MAXDIRENT*sizeof(__u32))+
					//  maxnamelen
					// does not exceed
					// sizeof(struct data_chunk)+512 !!!

#define NGFFS_HAS_SYSFILE	1	// Set to true value to enable
					// sysfiles (define them in
					// sysfiles.h)


// Sanity check, ensure that pagesize (4kb) is a multiple of chunk size
#if (((PAGE_CACHE_SIZE / NGFFS_MAXCSIZE) * NGFFS_MAXCSIZE) != PAGE_CACHE_SIZE)
#error "NGFFS_MAXCSIZE badly chosen"
#endif

#define EMPTY_BYTES		10	// Length of 0-byte block lead-in

#define NGFFS_NUMCHUNKS(x)	((x/NGFFS_MAXCSIZE)+((x%NGFFS_MAXCSIZE)?1:0))

#define NGFFS_LARGESTDIR	(sizeof(struct dir_chunk)+NGFFS_MAX_NAME_LEN + \
					(sizeof(__u32)*NGFFS_MAXDIRENT))
#define NGFFS_LARGESTDATA	(sizeof(struct data_chunk)+NGFFS_MAXCSIZE)
#define NGFFS_LARGESTCHUNK	((NGFFS_LARGESTDIR>NGFFS_LARGESTDATA) ? \
					NGFFS_LARGESTDIR : NGFFS_LARGESTDATA )


// Chunk types:
//
#define NGFFS_HEADER_CHUNK	1
#define NGFFS_DIR_CHUNK		2
#define NGFFS_FILE_CHUNK	3
#define NGFFS_DATA_CHUNK	4
#define NGFFS_UNUSED		(-1)

// Values for chunk->valid field
#define NGFFS_CHUNK_VALID	0xff		// TODO: change name to
#define NGFFS_CHUNK_INVALID	0x00

#define NGFFS_IS_WRITTEN	0x0		// written out completely
#define NGFFS_NOT_WRITTEN	0xf		// write process interrupted

// these macro's do not update the valid field crc!
#define SET_VALID(chunk)	((chunk)->valid |= 0xff)
#define RESET_VALID(chunk)	((chunk)->valid &= ~(0xff))
#define SET_WRITTEN(chunk)	((chunk)->valid &= ~(0xf00))
#define RESET_WRITTEN(chunk)	((chunk)->valid |= 0xf00)

// Chunk structures
//

struct ngffs_metadata {
	__u32		link_count;
	umode_t		mode;
	uid_t		uid;
	gid_t		gid;
//	time_t		atime;	// last accessed not used, mtime returned instead
	time_t		mtime;
	time_t		ctime;
};

// TODO: fix field offsets
struct generic_chunk {
	__u32		type;
	__u32		valid;		// 0xff=valid
	__u32		len;
	__u32		extra;		// length of dirname
	__u32		modification;	// if applicable
	__u32		entry_id;	// if applicable
};

struct header_chunk {
	__u32		type;
	__u32		valid;		// 0xff=valid, 0xf0=written, 0x00=invalid
	__u32		len;
	__u32		usage;		// usage count
	char		activated;	// 0xff=not activated
	__u32		maxfsize;
	__u32		maxcsize;
	__u32		maxdirent;
};

struct file_chunk {
	__u32		type;		// ==NGFFS_FILE_CHUNK
	__u32		valid;		// 0xff=valid
	__u32		len;		// length of the file in bytes
	__u32		namelen;	// length of filename
	__u32		modification;	// number of modification (largest is last)
	__u32		entry_id;	// ino
	struct ngffs_metadata meta;
//	char		name[namelen]	// variable length filename
};

// see (i) in regard to size
struct dir_chunk {
	__u32		type;		// ==NGFFS_DIR_CHUNK
	__u32		valid;		// 0xff=valid
	__u32		len;		// number of entries
	__u32		namelen;	// length of dirname
	__u32		modification;	// number of modification (largest is last)
	__u32		entry_id;	// ino
	struct ngffs_metadata meta;
//	char		name[namelen]	// variable length filename
//	__u32		entries[len]	// variable length array of file/dir chunks
};

// see (i) in regard to size
struct data_chunk {
	__u32		type;		// ==NGFFS_DATA_CHUNK
	__u32		valid;		// 0xff=valid
	__u32		len;		// length of this chunk in bytes
	__u32		seq_id;		// which part of file it defines
	__u32		modification;	// number of modification (largest is last)
	__u32		entry_id;	// which inode it is associated with
};

// Easy casting between chunks
//
#define GENERIC_CHUNK(x)	((struct generic_chunk *)(x))
#define DIR_CHUNK(x)		((struct dir_chunk *)(x))
#define FILE_CHUNK(x)		((struct file_chunk *)(x))
#define DATA_CHUNK(x)		((struct data_chunk *)(x))
#define HEADER_CHUNK(x)		((struct header_chunk *)(x))

#endif /* __INCLUDE_LINUX_NGFFS_H */

/* EOF */
