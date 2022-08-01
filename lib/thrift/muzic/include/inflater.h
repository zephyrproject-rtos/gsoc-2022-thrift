/* SPDX-License-Identifier: MIT */
/**
 * @file       inflater.h
 * @date       Jun 2, 2020
 * @author     Martin Rizzo | <martinrizzo@gmail.com>
 * @copyright  Copyright (c) 2020 Martin Rizzo.
 *             This project is released under the MIT License.
 * -------------------------------------------------------------------------
 *  Inflater - One-header library to decode data compressed with the Deflate algorithm.
 * -------------------------------------------------------------------------
 *  Copyright (c) 2020 Martin Rizzo
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * -------------------------------------------------------------------------
 */
#ifndef INFLATER_H_INCLUDED
#define INFLATER_H_INCLUDED
#include <stdlib.h>

#include "mz_config.h"
#include "zlib.h"

#undef  Byte
#define Byte  unsigned char

typedef enum InfError {
    InfError_BadBlockContent = -8,     /**< The content of block is invalid */
    InfError_BadBlockLength = -7,      /**< The length of block is invalid  */
    InfError_BadBlockType = -6,        /**< The type of block is invalid    */
    InfError_UnsupportedZLibHeader = -5,
    InfError_BadZLibHeader = -4,
    InfError_BadParameter = -3,
    InfError_Adler32Mismatch = -2,
    InfError_Failed = -1,
    InfError_None = 0
} InfError;

typedef enum InfAction {
    InfAction_Finish                 = 0,
    InfAction_FillInputBuffer        = 1,
    InfAction_UseOutputBufferContent = 2,
    InfAction_Feed2ndZlibHeaderByte  = 3,
    InfAction_ProcessNextChunk       = 256,
    InfAction_Init                   = 1024
} InfAction;

typedef struct InfData {
    const void* buffer;
    size_t      bufferSize;
} InfData;

#endif /* ifndef INFLATER_H_INCLUDED */
