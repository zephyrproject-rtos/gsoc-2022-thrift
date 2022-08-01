/*
 * Copyright 2022 Young Mei
 *
 * SPDX-License-Identifier: Zlib
 */

#ifndef MZ_CONF_INCLUDED
#define MZ_CONF_INCLUDED

/* Don't insert empty block in the middle of a stream given empty inputs */
#define MZ_NO_EMPTY_BLOCK

/* Zlib pads raw deflate stream with a 2-byte header by default */
/* Enable this macro to emulate this behavior */
#define MZ_ZLIB_HEADER

/* Zlib ends raw deflate stream with a 4-byte checksum by default */
/* Enable this macro to emulate this behavior */
#define MZ_ZLIB_CHECKSUM

#ifndef UZLIB_CONF_DEBUG_LOG
/* Debug logging level 0, 1, 2, etc. */
#define UZLIB_CONF_DEBUG_LOG 0
#endif

#ifndef UZLIB_CONF_USE_MEMCPY
/* Use memcpy() for copying data out of LZ window or uncompressed blocks,
   instead of doing this byte by byte. For well-compressed data, this
   may noticeably increase decompression speed. But for less compressed,
   it can actually deteriorate it (due to the fact that many memcpy()
   implementations are optimized for large blocks of data, and have
   too much overhead for short strings of just a few bytes). */
#define UZLIB_CONF_USE_MEMCPY 0
#endif


#endif /* MZ_CONF_INCLUDED */
