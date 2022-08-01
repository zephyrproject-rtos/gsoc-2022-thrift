/*
 * Copyright 2022 Young Mei
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MZ_ZLIB_COMP_INCLUDED
#define MZ_ZLIB_COMP_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define Z_NO_FLUSH 0
#define Z_PARTIAL_FLUSH 1
#define Z_SYNC_FLUSH 2
#define Z_FULL_FLUSH 3
#define Z_FINISH 4
#define Z_BLOCK 5
#define Z_TREES 6

#define Z_OK 0
#define Z_STREAM_END 1
#define Z_NEED_DICT 2
#define Z_ERRNO (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR (-3)
#define Z_MEM_ERROR (-4)
#define Z_BUF_ERROR (-5)

#define Z_NO_COMPRESSION 0
#define Z_BEST_SPEED 1
#define Z_BEST_COMPRESSION 9
#define Z_DEFAULT_COMPRESSION (-1)

#define Z_NULL 0

struct Inf_State;

typedef struct z_stream_s {
  unsigned char* next_in; /* next input byte */
  unsigned int avail_in;  /* number of bytes available at next_in */
  unsigned long total_in; /* total number of input bytes read so far */

  unsigned char* next_out; /* next output byte will go here */
  unsigned int avail_out;  /* remaining free space at next_out */
  unsigned long total_out; /* total number of bytes output so far */

  char* msg; /* last error message, NULL if no error */
  union {
    struct Inf_State* infl_state;
    struct uzlib_comp* defl_state;
  } state;

  void* zalloc; /* used to allocate the internal state */
  void* zfree;  /* used to free the internal state */
  void* opaque; /* private data object passed to zalloc and zfree */

  int data_type;          /* best guess about the data type: binary or text
                                 for deflate, or the decoding state for inflate */
  unsigned long adler;    /* Adler-32 or CRC-32 value of the uncompressed data */
  unsigned long reserved; /* reserved for future use */
} z_stream;

typedef z_stream* z_streamp;

int inflateInit(z_stream* strm);
int inflate(z_stream* strm, int flush);
int inflateEnd(z_stream* strm);

int deflateInit(z_stream* strm, int level);
int deflate(z_stream* strm, int flush);
int deflateEnd(z_stream* strm);

#ifdef __cplusplus
}
#endif

#endif // MZ_ZLIB_COMP_INCLUDED
