/*
 * Copyright 2022 Young Mei
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include "muzic/uzlib.h"
#include "zlib.h"

/**
 * Write variable length of bits to the output buffer
 * @param out         Pointer to the uzlib_comp structure
 * @param bits        Literal of the bits to be written
 * @param nbits       Length of the bits to be written
 */
void outbits(struct uzlib_comp* out, unsigned long bits, int nbits);

/**
 * Write 4 bytes to the output buffer
 * @param out  Pointer to the uzlib_comp structure
 * @param st   The first byte to be written
 * @param nd   The second byte to be written
 * @param nd   The third byte to be written
 * @param nd   The fourth byte to be written
 */
static void out4bytes(struct uzlib_comp* out,
                      unsigned char st,
                      unsigned char nd,
                      unsigned char rd,
                      unsigned char th) {
  int newlen = out->outlen + 4;
  if (newlen > out->outsize) {
    out->outsize = newlen;
    out->outbuf = (unsigned char*)realloc(out->outbuf, newlen);
  }
  out->outbuf[out->outlen++] = st;
  out->outbuf[out->outlen++] = nd;
  out->outbuf[out->outlen++] = rd;
  out->outbuf[out->outlen++] = th;
}

/*
 * Adler-32 algorithm taken from the zlib source, which is
 * Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler
 */

#define A32_BASE 65521
#define A32_NMAX 5552

uint32_t muzic_adler32(const void* data, unsigned int length, uint32_t prev_sum) {
  const unsigned char* buf = (const unsigned char*)data;

  unsigned int s1 = prev_sum & 0xffff;
  unsigned int s2 = prev_sum >> 16;

  while (length > 0) {
    int k = length < A32_NMAX ? length : A32_NMAX;
    int i;

    for (i = k / 16; i; --i, buf += 16) {
      s1 += buf[0];
      s2 += s1;
      s1 += buf[1];
      s2 += s1;
      s1 += buf[2];
      s2 += s1;
      s1 += buf[3];
      s2 += s1;
      s1 += buf[4];
      s2 += s1;
      s1 += buf[5];
      s2 += s1;
      s1 += buf[6];
      s2 += s1;
      s1 += buf[7];
      s2 += s1;

      s1 += buf[8];
      s2 += s1;
      s1 += buf[9];
      s2 += s1;
      s1 += buf[10];
      s2 += s1;
      s1 += buf[11];
      s2 += s1;
      s1 += buf[12];
      s2 += s1;
      s1 += buf[13];
      s2 += s1;
      s1 += buf[14];
      s2 += s1;
      s1 += buf[15];
      s2 += s1;
    }

    for (i = k % 16; i; --i) {
      s1 += *buf++;
      s2 += s1;
    }

    s1 %= A32_BASE;
    s2 %= A32_BASE;

    length -= k;
  }

  return ((uint32_t)s2 << 16) | s1;
}

int deflateInit(z_stream* strm, int level) {
  if (strm == Z_NULL)
    return Z_STREAM_ERROR;
  strm->msg = Z_NULL;
  struct uzlib_comp* ustate = (struct uzlib_comp*)malloc(sizeof(struct uzlib_comp));
  if (ustate == Z_NULL)
    return Z_MEM_ERROR;
  strm->adler = 1;
  /* uzlib allocates memory for output on demand                            */
  /* so no worry about outbuf; just remember to deallocate it after use     */
  ustate->dict_size = 32768;
  ustate->hash_bits = 12;
  size_t hash_size = sizeof(uzlib_hash_entry_t) * (1 << ustate->hash_bits);
  /* this space is reused in every call to deflate() until deflateEnd()     */
  /* 0-initialization is thereby put in deflate()                           */
  ustate->hash_table = malloc(hash_size);
  if (ustate->hash_table == Z_NULL)
    return Z_MEM_ERROR;
  ustate->outbuf = NULL;
  /* Use this field to signal the start of a stream                         */
  ustate->comp_disabled = 1;
  strm->state.defl_state = ustate;
  return Z_OK;
}

int deflate(z_stream* strm, int flush) {
  struct uzlib_comp* ustate = strm->state.defl_state;

  /* We still have some pending data in the output buffer */
  if (ustate->outbuf != NULL) {
    if (strm->avail_out < ustate->outlen) {
      memcpy(strm->next_out, ustate->outbuf, strm->avail_out);
      ustate->outbuf += strm->avail_out;
      ustate->outlen -= strm->avail_out;
      strm->next_out += strm->avail_out;
      strm->total_out += strm->avail_out;
      strm->avail_out = 0;
      return Z_OK;
    }

    memcpy(strm->next_out, ustate->outbuf, ustate->outlen);

    ustate->outbuf -= strm->total_out;

    strm->avail_out -= ustate->outlen;
    strm->next_out += ustate->outlen;
    strm->total_out += ustate->outlen;

    free((void*)ustate->outbuf);
    ustate->outbuf = NULL;
    if (strm->avail_in == 0)
      return Z_OK;
  }

#ifdef MZ_NO_EMPTY_BLOCK
  if (strm->avail_in == 0 && flush != Z_FINISH)
    return Z_OK;
#endif

  ustate->outlen = 0;
  ustate->outsize = 0;
  ustate->outbits = 0;
  ustate->noutbits = 0;
  size_t hash_size = sizeof(uzlib_hash_entry_t) * (1 << ustate->hash_bits);
  memset(ustate->hash_table, 0, hash_size);

#ifdef MZ_ZLIB_HEADER
  /* When flush mode is Z_FINISH, always start a new stream */
  /* Otherwise only add the header to the start of a stream */
  if (flush == Z_FINISH || ustate->comp_disabled == 1) {
    outbits(ustate, 0x78, 8);
    outbits(ustate, 0x01, 8);
  }
#endif
  ustate->comp_disabled = 0;

#ifdef MZ_ZLIB_CHECKSUM
  uint32_t adler_v = muzic_adler32(strm->next_in, strm->avail_in, strm->adler);
#endif

  /* When flush mode is not Z_FINISH, don't mark the first block as the final */
  if (flush != Z_FINISH) {
    outbits(ustate, 0, 1); /* Not the final block */
    outbits(ustate, 1, 2); /* Static huffman block */
  } else {
    zlib_start_block(ustate);
  }

  uzlib_compress(ustate, strm->next_in, strm->avail_in);

  /* When flush mode is not Z_FINISH, put an empty block at the end of a chunk */
  if (flush != Z_FINISH) {
    outbits(ustate, 0, 7); /* close block */
    outbits(ustate, 0, 3); /* header of stored block */
    outbits(ustate, 0, 7); /* flush all bits */
    out4bytes(ustate, 0x00, 0x00, 0xFF, 0xFF);

#ifdef MZ_ZLIB_CHECKSUM
    strm->adler = adler_v;
#endif
  } else {
    zlib_finish_block(ustate);
#ifdef MZ_ZLIB_CHECKSUM
    out4bytes(ustate, (adler_v >> 24) & 0xFF, (adler_v >> 16) & 0xFF, (adler_v >> 8) & 0xFF,
              adler_v & 0xFF);
#endif
    /* Reset this field to signal a new stream will be started */
    ustate->comp_disabled = 1;
  }

  if (flush == Z_FINISH) {
    strm->total_in = strm->avail_in;
  } else {
    strm->total_in += strm->avail_in;
  }
  /* always deflate all available data                        */
  /* set avail_in to 0 and return Z_STREAM_END to signal that */
  strm->next_in += strm->avail_in;
  strm->avail_in = 0;

  /* if output buffer doesn't have enough space               */
  if (strm->avail_out < ustate->outlen) {
    if (flush == Z_FINISH) {
      free((void*)ustate->outbuf);
      return Z_BUF_ERROR;
    }
    memcpy(strm->next_out, ustate->outbuf, strm->avail_out);
    ustate->outbuf += strm->avail_out;
    ustate->outlen -= strm->avail_out;
    strm->next_out += strm->avail_out;
    strm->total_out = strm->avail_out;
    strm->avail_out = 0;
    return Z_OK;
  }

  memcpy(strm->next_out, ustate->outbuf, ustate->outlen);

  if (flush == Z_FINISH) {
    strm->total_out = ustate->outlen;
  } else {
    strm->total_out += ustate->outlen;
  }
  strm->avail_out -= ustate->outlen;
  strm->next_out += ustate->outlen;

  free((void*)ustate->outbuf);
  ustate->outbuf = NULL;
  return flush == Z_FINISH ? Z_STREAM_END : Z_OK;
}

int deflateEnd(z_stream* strm) {
  free(strm->state.defl_state->hash_table);
  free(strm->state.defl_state);
  return Z_OK;
}
