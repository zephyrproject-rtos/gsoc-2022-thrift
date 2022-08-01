/**
 * @file       inflater.c
 * @date       Jun 2, 2020
 * @author     Martin Rizzo | <martinrizzo@gmail.com>
 * @copyright  Copyright (c) 2020 Martin Rizzo.
 *             This project is released under the MIT License.
 * -------------------------------------------------------------------------
 *  Inflater - One-header library to decode data compressed with the DEFLATE algorithm.
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
#include "inflater.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*====================================================================================================================*/
#pragma mark  -  CONSTANTS AND MACROS

static size_t min(size_t a, size_t b) { return a<b ? a : b; }

/** Macro to access to the full internal state */
#define inf (*(Inf_State*)infptr)

#define InfMinBufferSize    (32*1024)
#define InfHelperBufferSize (64*1024)

#define Inf_EndOfBlock           256
#define Inf_MaxValidLengthCode   285
#define Inf_MaxValidDistanceCode 29
#define Inf_LastValidLength      18
#define Inf_LastValidSymbol      290
#define Inf_CodeLengthMapSize    ((Inf_LastValidLength+1)+(Inf_LastValidSymbol+1))
#define Inf_NextIndexMask        0x03FF
#define Inf_MainTableSize        256      /**< 8bits                        */
#define Inf_HuffTableSize        (2*1024) /**< main-table + all sub-tables  */

#define Inf_LastValidLength 18
#define Inf_InvalidLength   24
#define Inf_SymlenSequenceSize 19
#define Inf_CodeLengthTableSize ((Inf_LastValidLength+1)+(Inf_LastValidCode+1))


/* Macros used for control flow inside the 'inflateProcessChunk(..) function */
#define inf__FILL_INPUT_BUFFER()                             inf.pub.action=InfAction_FillInputBuffer;            break;
#define inf__USE_OUTPUT_BUFFER_CONTENT()                     inf.pub.action=InfAction_UseOutputBufferContent;     break;
#define inf__FINISH()               step=InfStep_END;        inf.pub.action=InfAction_Finish;                     break;
#define inf__ERROR(err)             step=InfStep_FATAL_ERROR;inf.pub.action=InfAction_Finish; inf.pub.error=err;  break;
#define inf__goto(dest_step)        step=dest_step;                                                               break;
#define inf__fallthrough(next_step) step=next_step;




typedef enum InfMode {
    InfMode_Uninitialized,
    InfMode_Take,
    InfMode_Feed
} InflaterMode;

typedef enum InfFlags {
    InfFlags_FreeOutputBuffer,
    InfFlags_FreeInputBuffer,
    InfFlags_FreeItself
} InfFlags;

typedef enum InfStep {
    InfStep_START, InfStep_PROCESS_NEXT_BLOCK, InfStep_READ_BLOCK_HEADER,
    InfStep_PROCESS_UNCOMPRESSED_BLOCK, InfStep_OUTPUT_UNCOMPRESSED_BLOCK,
    InfStep_LOAD_FIXED_HUFFMAN_DECODERS, InfStep_LOAD_DYNAMIC_HUFFMAN_DECODERS,
    InfStep_LOAD_FRONT_DECODER, InfStep_LOAD_LITERAL_DECODER, InfStep_LOAD_DISTANCE_DECODER,
    InfStep_PROCESS_COMPRESSED_BLOCK,
    
    InfStep_Read_LiteralOrLength,
    InfStep_Read_LiteralOrLength2,
    InfStep_Read_LengthBits,
    InfStep_Read_Distance,
    InfStep_Read_DistanceBits,
    
    InfStep_OUTPUT_SEQUENCE, InfStep_FATAL_ERROR, InfStep_END
} InfStep;

static const unsigned char Inf_Reverse[256] = {
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

typedef int InfBool; /**< Boolean value */
#define Inf_FALSE 0
#define Inf_TRUE  1

typedef struct InfSymlen {
    unsigned  symbol;        /**< The decoded value attached to the huffman code */
    unsigned  huffmanLength; /**< The huffman code length (in number of bits)    */
    struct InfSymlen* next;
} InfSymlen;

typedef union Inf_Huff {
    struct value    { unsigned length:15, isvalid:1,  code:15; } value;
    struct subtable { unsigned   mask:15,   error:1, index:15; } subtable;
    unsigned raw;
} Inf_Huff;

#define Inf_HuffConst(code_,length_) { length_, 1, code_ }
#define Inf_Huff_SetValue(s, huff, hufflen, byte) s.value.code=byte; s.value.isvalid=1; s.value.length=hufflen
#define Inf_Huff_SetTableRef(s, index_,mask_) (s.subtable.index=(index_), s.subtable.error=0, s.subtable.mask=(mask_), s)
#define Inf_Huff_Decode(table, tmp, bitbuffer) \
    (tmp=table[bitbuffer & 0xFF], tmp.value.isvalid ? tmp : table[ tmp.subtable.index + ((bitbuffer>>8) & tmp.subtable.mask) ])


/*====================================================================================================================*/
#pragma mark  -  READING DATA FROM THE BITSTREAM

/** The bitstream */
typedef struct Inf_BS {
    unsigned    bits;     /**< The bit-buffer                                                          */
    unsigned    size;     /**< Number of bits currently loaded in the bit-buffer                       */
    const Byte* inputPtr; /**< Pointer to the read buffer location where next byte will be loaded from */
    const Byte* inputEnd; /**< Pointer to the end of the read buffer                                   */
} Inf_BS;

/** Loads the next byte (8 bits) from the input buffer to the bit-buffer */
static InfBool Inf_BS_LoadNextByte(Inf_BS* bitstream) {
    assert( bitstream!=NULL );
    if (bitstream->inputPtr==bitstream->inputEnd) { return Inf_FALSE; }
    bitstream->bits |= (*bitstream->inputPtr++ << bitstream->size);
    bitstream->size += 8;
    return Inf_TRUE;
}

/** Reads a sequence of bits from the bitstream. Returns FALSE if the bit-buffer doesn't have enought bits loaded. */
static InfBool Inf_BS_ReadBits(Inf_BS* bitstream, unsigned* dest, int numberOfBitsToRead) {
    assert( bitstream!=NULL && dest!=NULL && numberOfBitsToRead>=0 );
    while (bitstream->size<numberOfBitsToRead) {
        if (!Inf_BS_LoadNextByte(bitstream)) { return Inf_FALSE; }
    }
    (*dest) = bitstream->bits & ((1<<numberOfBitsToRead)-1);
    bitstream->bits >>= numberOfBitsToRead;
    bitstream->size  -= numberOfBitsToRead;
    return Inf_TRUE;
}

/** Reads a sequence of huffman encoded bits from the bitstream. Returns FALSE if bit-buffer doesn't have enought bits loaded. */
static InfBool Inf_BS_ReadEncodedBits(Inf_BS* bitstream, unsigned* dest, const Inf_Huff* decoder) {
    unsigned numberOfBitsToAdvance; Inf_Huff data, tmp;
    assert( bitstream!=NULL && dest!=NULL && decoder!=NULL );

    if (bitstream->size==0 && !Inf_BS_LoadNextByte(bitstream)) { return Inf_FALSE; }
    data=Inf_Huff_Decode(decoder, tmp, bitstream->bits); numberOfBitsToAdvance=data.value.length;
    
    while (bitstream->size<numberOfBitsToAdvance) {
        if (!Inf_BS_LoadNextByte(bitstream)) { return Inf_FALSE; }
        data=Inf_Huff_Decode(decoder, tmp, bitstream->bits); numberOfBitsToAdvance=data.value.length;
    }
    (*dest) = data.value.code;
    bitstream->bits >>= numberOfBitsToAdvance;
    bitstream->size  -= numberOfBitsToAdvance;
    return Inf_TRUE;
}

/** Read a DWORD (32 bits) from the bitstream. Returns FALSE if bit-buffer doesn't have enought bits loaded. */
static InfBool Inf_BS_ReadDWord(Inf_BS* bitstream, unsigned* dest) {
    static const unsigned numberOfBitsToRead = 32; /**< number of bits in a DWord */
    const unsigned        bitsToSkip         = (bitstream->size%8);
    assert( bitstream!=NULL && dest!=NULL && 8*sizeof(*dest)>=numberOfBitsToRead );
    /* skip any padding bits because bitbuffer must be byte aligned before reading a Word */
    bitstream->bits >>= bitsToSkip;
    bitstream->size  -= bitsToSkip;
    while ( bitstream->size<numberOfBitsToRead ) { if (!Inf_BS_LoadNextByte(bitstream)) { return Inf_FALSE; } }
    assert( bitstream->size==numberOfBitsToRead );
    (*dest) = bitstream->bits;
    bitstream->bits = bitstream->size = 0;
    return Inf_TRUE;
}

/** Reads a sequence of ALIGNED bytes from the bitstream (bit-buffer must be empty) */
static InfBool Inf_BS_ReadBytes(Inf_BS* bitstream, unsigned char* dest, size_t* inout_numberOfBytes) {
    size_t numberOfRequestedBytes, successfulBytes;
    assert( bitstream!=NULL && dest!=NULL && inout_numberOfBytes!=NULL && bitstream->size==0 );
    numberOfRequestedBytes = (*inout_numberOfBytes);
    successfulBytes        = min( numberOfRequestedBytes, (bitstream->inputEnd-bitstream->inputPtr) );
    memcpy(dest, bitstream->inputPtr, successfulBytes);
    bitstream->inputPtr += successfulBytes;
    (*inout_numberOfBytes) = successfulBytes;
    return (successfulBytes == numberOfRequestedBytes);
}


/*====================================================================================================================*/
#pragma mark  -  LOADING THE SYMBOL/LENGTH LIST

/** The symbol-length list reader */
typedef struct Inf_SLList {
    unsigned      command;             /**< current command, ex: InfCmd_CopyPreviousLength         */
    unsigned      symbol;              /**< current symbol value                                   */
    unsigned      huffmanLength;       /**< last huffman-length read                               */
    unsigned      repetitions;         /**< number of repetitions of the last huffman-length read  */
    unsigned char lengthsBySymbol[19]; /**< Array used internally to sort lengths by symbol number */
    /* The final list is sorted by the `length` value,             */
    /* it's the resulting of concatenating all these partial lists */
    InfSymlen* headPtr[Inf_LastValidLength+1];  /**< heads pointers, one by list (each list represents a length) */
    InfSymlen* tailPtr[Inf_LastValidLength+1];  /**< tails pointers, one by list (each list represents a length) */
    InfSymlen  elements[Inf_LastValidSymbol+1]; /**< Free elements to be added to the list */
    int        elementIndex;                    /**< Index to the next free element that is ready to be added */
} Inf_SLList;

/** Adds a range of symbol-length pairs to the list (inf.reader) */
#define Inf_SLList_AddRange(list, symbol, firstSymbol, lastSymbol, huffmanLength) \
    for (symbol=firstSymbol; symbol<lastSymbol; ++symbol) {                       \
        Inf_SLList_AddSymlen(list, symbol, huffmanLength);                        \
    }

/** Adds a symbol-length pair to the list (inf.sllist) */
static void Inf_SLList_AddSymlen(Inf_SLList* list, unsigned symbol, unsigned huffmanLength) {
    assert( 0<=symbol        &&        symbol<=Inf_LastValidSymbol );
    assert( 0<=huffmanLength && huffmanLength<=Inf_LastValidLength );
    if (huffmanLength>0) {
        InfSymlen* const newSymlen = &list->elements[ list->elementIndex++ ];
        InfSymlen* const last       = list->tailPtr[huffmanLength];
        if (last) { last->next = newSymlen; } else { list->headPtr[huffmanLength] = newSymlen; }
        newSymlen->symbol        = symbol;
        newSymlen->huffmanLength = huffmanLength;
        newSymlen->next          = NULL;
        list->tailPtr[huffmanLength] = newSymlen;
    }
}

/** Adds 3bit lengths attached to a predefined sequence of symbols: 16, 17, 18, 0, 8, 7, 9, ... */
static InfBool Inf_SLList_Add3BitSymlens(Inf_SLList* list, Inf_BS* bitstream, unsigned numberOfSymlens) {
    static const unsigned symbolOrder[Inf_SymlenSequenceSize] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
    const unsigned end = (numberOfSymlens<Inf_SymlenSequenceSize) ? numberOfSymlens : Inf_SymlenSequenceSize;
    unsigned symbol, length;
    assert( list!=NULL && bitstream!=NULL && numberOfSymlens>0 );
    
    while (list->symbol<end) {
        if ( !Inf_BS_ReadBits(bitstream,&length,3) ) { return Inf_FALSE; }
        list->lengthsBySymbol[ symbolOrder[list->symbol++] ] = (unsigned char)length;
    }
    while (list->symbol<Inf_SymlenSequenceSize) {
        list->lengthsBySymbol[ symbolOrder[list->symbol++] ] = 0;
    }
    for (symbol=0; symbol<Inf_SymlenSequenceSize; symbol++) {
        length = (unsigned)list->lengthsBySymbol[symbol];
        if (length>0) { Inf_SLList_AddSymlen(list, symbol, length); }
    }
    return Inf_TRUE;
}

/** Adds a sequence of symbol-length pairs reading and decimpressing data from the bitstream */
static InfBool Inf_SLList_AddEncodedSymlens(Inf_SLList* list, Inf_BS* bitstream, unsigned numberOfSymlens, const Inf_Huff* decoder) {
    enum InfCmd {
        InfCmd_CopyPreviousLength  = 16, /**< repeat the previous length            */
        InfCmd_RepeatZeroLength_3  = 17, /**< repeat zero length (3 or more times)  */
        InfCmd_RepeatZeroLength_11 = 18  /**< repeat zero length (11 or more times) */
    };
    unsigned value;
    assert( list!=NULL && numberOfSymlens>0 && decoder!=NULL );

    /* add (one by one) all symbols with their corresponding huffmanLengths taking into account the number of repetitions */
    while ( list->symbol<numberOfSymlens )
    {
        /* IMPORTANT: this loop even adds any repetition that is pending from a previous load, for example      */
        /* some repetitions from the literal-decoder definition may end up in the `distance-decoder` definition */
        while (list->repetitions>0 && list->symbol<numberOfSymlens) {
            Inf_SLList_AddSymlen(list, list->symbol, list->huffmanLength);
            ++list->symbol; --list->repetitions;
        }
        if (list->symbol<numberOfSymlens) {
            assert( list->repetitions==0 );
            
            /* read a new command */
            if ( !list->command ) {
                if ( !Inf_BS_ReadEncodedBits(bitstream, &list->command, decoder) ) { return Inf_FALSE; }
            }
            /* use the command to configure the next repetition loop */
            switch (list->command) {
                case InfCmd_CopyPreviousLength:
                    if ( !Inf_BS_ReadBits(bitstream,&value,2) ) { return Inf_FALSE; }
                    list->repetitions   = 3 + value;
                    break;
                case InfCmd_RepeatZeroLength_3:
                    if ( !Inf_BS_ReadBits(bitstream,&value,3) ) { return Inf_FALSE; }
                    list->repetitions   = 3 + value;
                    list->huffmanLength = 0;
                    break;
                case InfCmd_RepeatZeroLength_11:
                    if ( !Inf_BS_ReadBits(bitstream,&value,7) ) { return Inf_FALSE; }
                    list->repetitions   = 11 + value;
                    list->huffmanLength = 0;
                    break;
                default: /* length is the value stored in 'list->command' */
                    list->repetitions   = 1;
                    list->huffmanLength = list->command;
                    break;
            }
            /* mark current command as finished */
            list->command = 0;
        }
    }
    return Inf_TRUE;
}

/** Finishes the creation of a symbol-length list (inf.reader) */
static const InfSymlen* Inf_SLList_GetSorted(Inf_SLList* list, InfBool resetRepetitions) {
    int lastValidLength = 0;
    int currentLength   = 0;
    InfSymlen *firstElement, *currentHead;
    
    /* connect all lists creating a big one sorted by `length` */
    while (list->headPtr[currentLength]==NULL) { ++currentLength; }
    firstElement = list->headPtr[ lastValidLength = currentLength ];
    
    do {
        do { currentHead = list->headPtr[ ++currentLength ];
        } while ( currentLength<Inf_LastValidLength && currentHead==NULL );
        list->tailPtr[lastValidLength]->next = currentHead;
        lastValidLength = currentLength;
    } while ( currentLength<Inf_LastValidLength );
    
    /* reset */
    if (resetRepetitions) { list->command = list->huffmanLength = list->repetitions = 0; }
    list->elementIndex = list->symbol = 0;
    for (currentLength=0; currentLength<=Inf_LastValidLength; ++currentLength) {
        list->headPtr[currentLength] = list->tailPtr[currentLength] = NULL;
    }
    
    return firstElement;
}


/*====================================================================================================================*/
#pragma mark  -  CREATING HUFFMAN TABLE DECODERS

#define Inf_Huff_NextCanonical(huffman, currentLength, newLength) \
    huffman       = (huffman+1) << (newLength - currentLength);   \
    huffmanLength = newLength                                     \

/**
 * Fills a huffman table (or subtable) with all posible values that match the provided huffman code
 * @param table            Pointer to the huffman table (or sub-table) to fill
 * @param tableSize        Number of entries available in `table`
 * @param huffman          The huffman code
 * @param huffmanLength    The length of the huffman code in number of bits
 * @param byte             The decoded byte corresponding to the provided huffman code
 * @param discardedBits    Number of bits to discard (0=table, 8=sub-table)
 */
static void Inf_Huff_FillTable(Inf_Huff* table,
                               unsigned  tableSize,
                               unsigned  huffman,
                               unsigned  huffmanLength,
                               unsigned  byte,
                               unsigned  discardedBits)
{
    unsigned unknownBits, knownHuffman; Inf_Huff data;
    const unsigned unknownStep    = (1<<(huffmanLength-discardedBits));
    const unsigned reverseHuffman = ((Inf_Reverse[huffman&0xFF]<<8)|(Inf_Reverse[huffman>>8])) >> (16-huffmanLength);
    assert( table!=NULL && huffmanLength>discardedBits && reverseHuffman<(1<<huffmanLength) );
    
    Inf_Huff_SetValue(data, reverseHuffman, huffmanLength, byte);
    knownHuffman = reverseHuffman >> discardedBits;
    for (unknownBits=0; unknownBits<tableSize; unknownBits+=unknownStep) {
        table[ unknownBits | knownHuffman ] = data;
    }
}

/**
 * Makes a huffman table to be used in fast extraction from bitstream
 * @param table   Pointer to the huffman table to fill
 * @param symlen  The first element of a list containing `symbol-length` data sorted by length
 * @returns
 *      The same table pointer provided in the first parameter.
 */
static const Inf_Huff* Inf_Huff_MakeDecoder(Inf_Huff* table, const InfSymlen *symlen) {
    static const Inf_Huff invalid = Inf_HuffConst(0, Inf_InvalidLength);
    Inf_Huff data; int i, subtableIndex; const InfSymlen *symlen_end;
    unsigned subtableSize, huffman, huffmanLength;
    assert( table!=NULL && symlen!=NULL  );
    
    /* reset the main-table */
    for (i=0; i<Inf_MainTableSize; ++i) { table[i] = invalid; }
    
    /* init huffman canonical code */
    huffman       = 0;
    huffmanLength = symlen->huffmanLength;

    /* lengths from 1 to 8                              */
    /* unknown bits are filled with all possible values */
    while ( symlen!=NULL && huffmanLength<=8 ) {
        Inf_Huff_FillTable(table, 256, huffman, huffmanLength, symlen->symbol, 0);
        if ( (symlen=symlen->next)!=NULL ) { Inf_Huff_NextCanonical(huffman, huffmanLength, symlen->huffmanLength); }
    }
    /* lengths from 9         */
    /* subtables are created  */
    subtableIndex = Inf_MainTableSize;
    while ( symlen!=NULL )
    {
        const InfSymlen* const symlen_first  = symlen;
        const unsigned         huffman_first = huffman;
        const unsigned         index         = huffman >> (huffmanLength-8);
        
        /* calculate subtable size (and find first and last element) */
        do {
            subtableSize = huffmanLength;
            if ( (symlen=symlen->next)!=NULL ) { Inf_Huff_NextCanonical(huffman, huffmanLength, symlen->huffmanLength); }
        } while ( symlen!=NULL && (huffman>>(huffmanLength-8))==index );
        symlen_end   = symlen;
        subtableSize = (1<<(subtableSize-8));
        assert( subtableIndex+subtableSize <= Inf_HuffTableSize );
        
        /* create subtable */
        huffman       = huffman_first;
        huffmanLength = symlen_first->huffmanLength;
        table[Inf_Reverse[index]] = Inf_Huff_SetTableRef(data, subtableIndex, subtableSize-1);
        symlen = symlen_first; while ( symlen!=symlen_end ) {
            Inf_Huff_FillTable(&table[subtableIndex], subtableSize, huffman, huffmanLength, symlen->symbol, 8);
            if ( (symlen=symlen->next)!=NULL ) { Inf_Huff_NextCanonical(huffman, huffmanLength, symlen->huffmanLength); }
        }
        subtableIndex += subtableSize;
    }
    return table;
}

static const Inf_Huff* Inf_Huff_MakeFixedLiteralDecoder(Inf_SLList* tmplist) {
    static const Inf_Huff *cachedDecoder=NULL; static Inf_Huff table[Inf_HuffTableSize]; unsigned tmp;
    if (cachedDecoder) { return cachedDecoder; }
    Inf_SLList_AddRange(tmplist, tmp,   0,144, 8);
    Inf_SLList_AddRange(tmplist, tmp, 144,256, 9);
    Inf_SLList_AddRange(tmplist, tmp, 256,280, 7);
    Inf_SLList_AddRange(tmplist, tmp, 280,288, 8);
    return (cachedDecoder = Inf_Huff_MakeDecoder(table, Inf_SLList_GetSorted(tmplist, Inf_TRUE)));
}

static const Inf_Huff* Inf_Huff_MakeFixedDistanceDecoder(Inf_SLList* tmplist) {
    static const Inf_Huff *cachedDecoder=NULL; static Inf_Huff table[Inf_HuffTableSize]; unsigned tmp;
    if (cachedDecoder) { return cachedDecoder; }
    Inf_SLList_AddRange(tmplist, tmp,  0,32, 5);
    return (cachedDecoder = Inf_Huff_MakeDecoder(table, Inf_SLList_GetSorted(tmplist, Inf_TRUE)));
}


/*====================================================================================================================*/
#pragma mark  -  IMPLEMENTATION OF PUBLIC FUNCTIONS

/** The current state of the inflate process (all this info is hidden behind the `Inflater` pointer) */
typedef struct Inf_State {
    
    Inflater   pub;       /**< The public data exposed in the `Inflater` pointer */
    Inf_BS     bitstream; /**< The bitbuffer                                     */
    Inf_SLList sllist;    /**< The symbol-length list                            */
    
    /* Data used directly by the inflate process */
    InfStep         step;            /**< The current step in the inflate process, ex: InfStep_ReadBlockHeader */
    unsigned        isLastBlock;     /**< TRUE (1) when processing the last block of the data set              */
    unsigned        symcount;        /**< The number of symbols used in front,literal & distance decoders      */
    unsigned        literal;         /**< literal value to output                                              */
    unsigned        sequence_dist;   /**< distance of the sequence to output                                   */
    unsigned        sequence_len;    /**< length of the sequence to output                                     */
    const Inf_Huff* frontDecoder;    /**< The huffman decoder used to decode the next 2 huffman decoders (it's crazy!) */
    const Inf_Huff* literalDecoder;  /**< The literal+length huffman decoder                                   */
    const Inf_Huff* distanceDecoder; /**< The distance huffman decoder                                         */
    Inf_Huff        huffmanTableA[Inf_HuffTableSize]; /**< pri. buffer where huffman tables used by decoders are stored */
    Inf_Huff        huffmanTableB[Inf_HuffTableSize]; /**< sec. buffer where huffman tables used by decoders are stored */

} Inf_State;

Inflater* inflaterCreate(void* workingBuffer, size_t workingBufferSize) {
    unsigned length;
    Inf_State* infptr = (Inf_State*)malloc(sizeof(Inf_State));
    
    inf.pub.mode            = InfMode_Uninitialized;
    inf.pub.flags           = InfFlags_FreeItself;
    inf.pub.action          = InfAction_Init;
    inf.pub.error           = InfError_None;
    
    inf.pub.userPtr          = NULL;
    inf.pub.dataProviderFunc = NULL;
    inf.pub.dataReceiverFunc = NULL;
    
    inf.pub.inputChunkPtr   = NULL;
    inf.pub.inputChunkEnd   = NULL;
    inf.pub.outputChunk     = NULL;
    inf.pub.outputChunkSize = 0;
    inf.pub.outputBufferContentSize = 0;
    
    /*---- inflaterTake / inflaterFeed ----------- */
    inf.pub.helperOutput.buffer     = NULL;
    inf.pub.helperOutput.bufferSize = 0;
    inf.pub.helperInput.buffer      = NULL;
    inf.pub.helperInput.bufferSize  = 0;
    inf.pub.takeOutputPtr           = NULL;
    inf.pub.takeOutputRemaining     = 0;
    inf.pub.providedData.buffer     = NULL;
    inf.pub.providedData.bufferSize = 0;
    
    /*---- decompress ---------------------------- */
    inf.step        = 0;
    inf.bitstream.bits = 0;
    inf.bitstream.size = 0;
    
    /*---- symlen list --------------------------- */
    inf.sllist.command          = 0;
    inf.sllist.huffmanLength    = 0;
    inf.sllist.repetitions      = 0;
    inf.sllist.symbol           = 0;
    inf.sllist.elementIndex = 0;
    for (length=0; length<=Inf_LastValidLength; ++length) {
        inf.sllist.headPtr[length] = inf.sllist.tailPtr[length] = NULL;
    }

    
    if (workingBuffer!=NULL && workingBufferSize>=InfHelperBufferSize) {
        inf.pub.helperOutput.buffer     = workingBuffer;
        inf.pub.helperOutput.bufferSize = workingBufferSize;
    }
    assert( (Inflater*)infptr == &inf.pub );
    return (Inflater*)infptr;
}

/**
 * Decompresses a chunk of compressed data
 * @param infptr            Pointer to the `Inflater` object created with `inflaterCreate(..)`
 * @param outputBuffer      Pointer to the destination buffer where decompressed data will be stored
 * @param outputBufferSize  The available capacity of `destBuffer` in number of bytes
 * @param inputBuffer       Pointer to the source buffer from where compressed data is read
 * @param inputBufferSize   The length of `sourBuffer` in number of bytes
 * @returns
 *     The action required to execute to continue with the next chunk, ex: `InfAction_FillInputBuffer`
 */
InfAction inflaterProcessChunk(Inflater*         infptr,
                               void* const       outputBuffer,
                               size_t            outputBufferSize,
                               const void* const inputBuffer,
                               size_t            inputBufferSize)
{
    static const unsigned int lengthStarts[] = {
         3,  4,  5,  6,  7,  8,  9,  10,  11,  13,  15,  17,  19,  23,  27,  31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const unsigned int lengthExtraBits[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const unsigned int distanceStarts[] = {
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static const unsigned int distanceExtraBits[] = {
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};
    enum BlockType {
         BlockType_Uncompressed   = 0,
         BlockType_FixedHuffman   = 1,
         BlockType_DynamicHuffman = 2
    };
    InfBool canReadAll; InfStep step; size_t numberOfBytes; unsigned temp; unsigned char *sequencePtr, *writePtr;
    unsigned char* const writeEnd = ((Byte*)outputBuffer + outputBufferSize);
    Inf_SLList*    const sllist    = &inf.sllist;
    Inf_BS*        const bitstream = &inf.bitstream;
    
    assert( inputBuffer !=NULL && inputBufferSize >0 );
    assert( outputBuffer!=NULL && outputBufferSize>0 );

    switch ( inf.pub.action ) {
        case InfAction_Init:
            inf.pub.outputChunk   = (Byte      *)outputBuffer;
            inf.pub.inputChunkPtr = inf.bitstream.inputPtr = (const Byte*)inputBuffer;
            inf.pub.inputChunkEnd = inf.bitstream.inputEnd = inf.bitstream.inputPtr + inputBufferSize;
            break;
        case InfAction_FillInputBuffer:
            inf.pub.outputChunk  += inf.pub.outputChunkSize;
            inf.pub.inputChunkPtr = inf.bitstream.inputPtr = (const Byte*)inputBuffer;
            inf.pub.inputChunkEnd = inf.bitstream.inputEnd = inf.bitstream.inputPtr + inputBufferSize;
            break;
        case InfAction_UseOutputBufferContent:
            inf.pub.outputChunk = (Byte*)outputBuffer;
            inf.pub.outputBufferContentSize = 0;
            break;
        case InfAction_ProcessNextChunk:
            inf.pub.outputChunk += inf.pub.outputChunkSize;
            break;
        default: /* InfAction_Finish */
            inf.pub.outputChunkSize = 0;
            return inf.pub.action;
    }

    assert( outputBuffer!=NULL && outputBufferSize>=(32*1024) );
    assert( inf.pub.outputChunk!=NULL && inf.pub.outputChunk<writeEnd );
    assert( inf.bitstream.inputPtr!=NULL && inf.bitstream.inputPtr<=inf.bitstream.inputEnd );
    
    step           = (InfStep)inf.step;
    writePtr       = inf.pub.outputChunk;
    inf.pub.action = InfAction_ProcessNextChunk;
    while ( inf.pub.action==InfAction_ProcessNextChunk ) {
        switch (step) {
                
            case InfStep_START:
                inf.isLastBlock = 0;
                inf__fallthrough(InfStep_PROCESS_NEXT_BLOCK);
                
            /*-------------------------------------------------------------------------------------
             * infstep: PROCESS_NEXT_BLOCK
             */
            case InfStep_PROCESS_NEXT_BLOCK:
                if (inf.isLastBlock) {
                    if (writePtr > inf.pub.outputChunk) {
                        printf(" > END (use remaining output buffer)\n");
                        inf__USE_OUTPUT_BUFFER_CONTENT();
                    }
                    printf(" > END OF STREAM\n\n");
                    inf__FINISH();
                }
                inf__fallthrough(InfStep_READ_BLOCK_HEADER);
                
            /*-------------------------------------------------------------------------------------
             * infstep: READ_BLOCK_HEADER
             */
            case InfStep_READ_BLOCK_HEADER:
                if ( !Inf_BS_ReadBits(bitstream,&temp,3) ) { inf__FILL_INPUT_BUFFER(); }
                inf.isLastBlock = (temp&0x01);
                switch (temp>>1) {
                    case BlockType_Uncompressed:   inf__goto(InfStep_PROCESS_UNCOMPRESSED_BLOCK);
                    case BlockType_FixedHuffman:   inf__goto(InfStep_LOAD_FIXED_HUFFMAN_DECODERS);
                    case BlockType_DynamicHuffman: inf__goto(InfStep_LOAD_DYNAMIC_HUFFMAN_DECODERS);
                    default: inf__ERROR(InfError_BadBlockType);
                }
                break;
                
            /*-------------------------------------------------------------------------------------
             * infstep: PROCESS_UNCOMPRESSED_BLOCK
             */
            case InfStep_PROCESS_UNCOMPRESSED_BLOCK:
                if ( !Inf_BS_ReadDWord(bitstream,&temp) ) { inf__FILL_INPUT_BUFFER(); }
                inf.sequence_len = (temp & 0xFFFF);
                if ( inf.sequence_len != ((~temp)>>16) ) { inf__ERROR(InfError_BadBlockLength); }
                inf__fallthrough(InfStep_OUTPUT_UNCOMPRESSED_BLOCK);
                
            case InfStep_OUTPUT_UNCOMPRESSED_BLOCK:
                numberOfBytes = min( inf.sequence_len, (writeEnd-writePtr) );
                canReadAll    = Inf_BS_ReadBytes(bitstream, writePtr, &numberOfBytes);
                inf.sequence_len -= numberOfBytes;
                writePtr         += numberOfBytes;
                if      ( !canReadAll        ) { inf__FILL_INPUT_BUFFER();         }
                else if ( inf.sequence_len>0 ) { inf__USE_OUTPUT_BUFFER_CONTENT(); }
                assert( inf.sequence_len==0 );
                inf__goto(InfStep_PROCESS_NEXT_BLOCK);

            /*-------------------------------------------------------------------------------------
             * infstep: LOAD_FIXED_HUFFMAN_DECODERS
             */
            case InfStep_LOAD_FIXED_HUFFMAN_DECODERS:
                inf.literalDecoder  = Inf_Huff_MakeFixedLiteralDecoder (&inf.sllist);
                inf.distanceDecoder = Inf_Huff_MakeFixedDistanceDecoder(&inf.sllist);
                inf__goto(InfStep_PROCESS_COMPRESSED_BLOCK);
                
            /*-------------------------------------------------------------------------------------
             * infstep: LOAD_DYNAMIC_HUFFMAN_DECODERS
             */
            case InfStep_LOAD_DYNAMIC_HUFFMAN_DECODERS:
                if ( !Inf_BS_ReadBits(bitstream,&inf.symcount,5+5+4) ) { inf__FILL_INPUT_BUFFER(); }
                inf__fallthrough(InfStep_LOAD_FRONT_DECODER);

            case InfStep_LOAD_FRONT_DECODER:
                if ( !Inf_SLList_Add3BitSymlens(sllist,bitstream,((inf.symcount>>10)&0x0F)+4) ) { inf__FILL_INPUT_BUFFER(); }
                inf.frontDecoder = Inf_Huff_MakeDecoder(inf.huffmanTableA, Inf_SLList_GetSorted(sllist,Inf_TRUE));
                inf__fallthrough(InfStep_LOAD_LITERAL_DECODER);
                
            case InfStep_LOAD_LITERAL_DECODER:
                if ( !Inf_SLList_AddEncodedSymlens(sllist,bitstream,(inf.symcount&0x1F)+257,inf.frontDecoder) ) { inf__FILL_INPUT_BUFFER(); }
                inf.literalDecoder = Inf_Huff_MakeDecoder(inf.huffmanTableB, Inf_SLList_GetSorted(sllist,Inf_FALSE));
                inf__fallthrough(InfStep_LOAD_DISTANCE_DECODER);
                
            case InfStep_LOAD_DISTANCE_DECODER:
                if ( !Inf_SLList_AddEncodedSymlens(sllist,bitstream,((inf.symcount>>5)&0x1F)+1,inf.frontDecoder) ) { inf__FILL_INPUT_BUFFER(); }
                inf.distanceDecoder = Inf_Huff_MakeDecoder(inf.huffmanTableA, Inf_SLList_GetSorted(sllist,Inf_TRUE));
                inf__fallthrough(InfStep_PROCESS_COMPRESSED_BLOCK);
                
            /*-------------------------------------------------------------------------------------
             * infstep: PROCESS_COMPRESSED_BLOCK
             */
            case InfStep_PROCESS_COMPRESSED_BLOCK:
            case InfStep_Read_LiteralOrLength:
                if ( !Inf_BS_ReadEncodedBits(bitstream,&inf.literal,inf.literalDecoder) ) { inf__FILL_INPUT_BUFFER(); }
                inf__fallthrough(InfStep_Read_LiteralOrLength2);
                
            case InfStep_Read_LiteralOrLength2:
                if (inf.literal <Inf_EndOfBlock) {
                    if (writePtr==writeEnd) { inf__USE_OUTPUT_BUFFER_CONTENT(); }
                    *writePtr++ = inf.literal;
                    inf__goto(InfStep_Read_LiteralOrLength);
                }
                else if (inf.literal==Inf_EndOfBlock) {
                    printf(" > EndOfBlock\n");
                    inf__goto(InfStep_PROCESS_NEXT_BLOCK);
                }
                else if (inf.literal>Inf_MaxValidLengthCode) { inf__ERROR(InfError_BadBlockContent); }
                inf.literal -= 257;
                inf__fallthrough(InfStep_Read_LengthBits);
                
            case InfStep_Read_LengthBits:
                if ( !Inf_BS_ReadBits(bitstream, &temp, lengthExtraBits[inf.literal]) ) { inf__FILL_INPUT_BUFFER(); }
                inf.sequence_len = temp + lengthStarts[inf.literal];
                assert( inf.sequence_len<(32*1024) );
                inf__fallthrough(InfStep_Read_Distance);
                
            case InfStep_Read_Distance:
                if ( !Inf_BS_ReadEncodedBits(bitstream,&inf.literal,inf.distanceDecoder) ) { inf__FILL_INPUT_BUFFER(); }
                if (inf.literal>Inf_MaxValidDistanceCode) { inf__ERROR(InfError_BadBlockContent); }
                inf__fallthrough(InfStep_Read_DistanceBits);
                
            case InfStep_Read_DistanceBits:
                if ( !Inf_BS_ReadBits(bitstream, &temp, distanceExtraBits[inf.literal]) ) { inf__FILL_INPUT_BUFFER(); }
                inf.sequence_dist =  temp + distanceStarts[inf.literal];
                assert( inf.sequence_dist<(32*1024) );
                inf__fallthrough(InfStep_OUTPUT_SEQUENCE);

            /*-------------------------------------------------------------------------------------
             * infstep: OUTPUT_SEQUENCE
             *
             *         outputBegin     writePtr                      writeEnd
             *              V             V                             V
             * <... ghost > # [[ straight * overlapped  ...... ghost ]] #
             */
            case InfStep_OUTPUT_SEQUENCE:
                sequencePtr = writePtr - inf.sequence_dist;
                if (sequencePtr<(Byte*)outputBuffer) {
                    sequencePtr += outputBufferSize;
                    /*-- copy bytes ---*/
                    inf.sequence_len -= numberOfBytes = min( inf.sequence_len, (writeEnd-sequencePtr) );
                    memmove( writePtr, sequencePtr, numberOfBytes ); writePtr+=numberOfBytes;
                    /*-----------------*/
                    if ( inf.sequence_len==0 ) { inf__goto(InfStep_Read_LiteralOrLength); }
                    if ( writePtr==writeEnd  ) { inf__USE_OUTPUT_BUFFER_CONTENT();        }
                    sequencePtr = outputBuffer;
                }
                /*-- copy bytes ---*/
                inf.sequence_len -= numberOfBytes = min( inf.sequence_len, (writeEnd-writePtr) );
                while ( numberOfBytes-->0 ) { *writePtr++ = *sequencePtr++; }
                /*-----------------*/
                if ( inf.sequence_len==0 ) { inf__goto(InfStep_Read_LiteralOrLength); }
                if ( writePtr==writeEnd  ) { inf__USE_OUTPUT_BUFFER_CONTENT();        }
                inf__ERROR(InfError_BadBlockContent);
                
            case InfStep_FATAL_ERROR:
            case InfStep_END:
                break;
        }
    }
    
    inf.step                         = (unsigned)step;
    inf.pub.inputChunkPtr            = inf.bitstream.inputPtr;
    inf.pub.outputChunkSize          = (writePtr - inf.pub.outputChunk);
    inf.pub.outputBufferContentSize += inf.pub.outputChunkSize;
    printf(" # inf.action = %d\n", inf.pub.action);
    return inf.pub.action;
}


/**
 * Decompresses a chunk of data
 * @param infptr                Pointer to the `Inflater` object created with `inflaterCreate(..)`
 * @param decompressedData      Pointer to the destination buffer where decompressed data will be stored
 * @param decompressedDataSize  The available capacity of `destBuffer` in number of bytes
 */
size_t inflaterTake(Inflater* infptr, void* decompressedData, size_t decompressedDataSize) {
    /* InfAction action; */
    size_t sourAvailableBytes,destBytesStillNeeded;
    Byte* dest; const Byte* sour;
    Byte* outputBuffer; size_t outputBufferSize;
    int fillInputBuffer;

    /* initialization */
    assert( (inf.pub.mode==InfMode_Uninitialized) || (inf.pub.mode==InfMode_Take) );
    if (inf.pub.mode==InfMode_Uninitialized) {
        inf.pub.mode= InfMode_Take;
        if (inf.pub.helperOutput.buffer==NULL) {
            inf.pub.helperOutput.buffer = malloc( inf.pub.helperOutput.bufferSize=InfHelperBufferSize );
            inf.pub.flags |= InfFlags_FreeOutputBuffer;
        }
        if (inf.pub.helperInput.buffer==NULL) {
            inf.pub.helperInput.buffer  = malloc( inf.pub.helperInput.bufferSize=InfHelperBufferSize );
            inf.pub.flags |= InfFlags_FreeInputBuffer;
        }
        inf.pub.takeOutputPtr           = (Byte*)inf.pub.helperOutput.buffer;
        inf.pub.takeOutputRemaining     = 0;
        inf.pub.providedData.buffer     = NULL;
        inf.pub.providedData.bufferSize = 0;
    }

    outputBuffer         = (Byte*)inf.pub.helperOutput.buffer;
    outputBufferSize     =        inf.pub.helperOutput.bufferSize;
    sour                 =        inf.pub.takeOutputPtr;
    sourAvailableBytes   =        inf.pub.takeOutputRemaining;
    dest                 = (Byte*)decompressedData;
    destBytesStillNeeded =        decompressedDataSize;
    
    
    if ( inf.pub.action==InfAction_Finish ) { return 0; }

    while ( destBytesStillNeeded>0 && inf.pub.action!=InfAction_Finish )
    {
        fillInputBuffer = (inf.pub.action==InfAction_Init || inf.pub.action==InfAction_FillInputBuffer);
        
        if (inf.pub.action==InfAction_UseOutputBufferContent && sourAvailableBytes>0 ) {
            /* take available bytes */
            const size_t size = sourAvailableBytes<destBytesStillNeeded ? sourAvailableBytes : destBytesStillNeeded;
            memcpy(dest,sour,size); dest+=size; destBytesStillNeeded-=size; sour+=size; sourAvailableBytes-=size;
            if (sourAvailableBytes==0) { fillInputBuffer=1; sour=outputBuffer; }
        }
        if ( fillInputBuffer ) {
            /* get compressed data from the data provider (when required) */
            if (inf.pub.action==InfAction_Init || inf.pub.action==InfAction_FillInputBuffer) {
                inf.pub.providedData=inf.pub.helperInput;
                inf.pub.dataProviderFunc(&inf.pub, &inf.pub.providedData );
                if ( inf.pub.providedData.bufferSize==0 ) { break; }
            }
            /* decompress the provided data and generate more sourAvailableBytes */
            do {
                inflaterProcessChunk(infptr, outputBuffer, outputBufferSize, inf.pub.providedData.buffer, inf.pub.providedData.bufferSize);
                sourAvailableBytes += inf.pub.outputChunkSize;
            } while ( inf.pub.action==InfAction_ProcessNextChunk );
        }
    }
    inf.pub.takeOutputPtr=sour; inf.pub.takeOutputRemaining=sourAvailableBytes;
    return (decompressedDataSize-destBytesStillNeeded);
}

/**
 * Decompresses a chunk of data
 * @param infptr              Pointer to the `Inflater` object created with `infalterCreate(..)`
 * @param compressedData      Pointer to the source buffer from where compressed data is read
 * @param compressedDataSize  The length of `compressedData` in number of bytes
 */
size_t inflaterFeed(Inflater* infptr, const void* compressedData, size_t compressedDataSize) {
    InfAction action; /* size_t numberOfConsumedBytes=0; */
    Byte* outputBuffer; size_t outputBufferSize;
    assert( (inf.pub.mode==InfMode_Uninitialized) || (inf.pub.mode=InfMode_Feed) );
    assert( inf.pub.dataReceiverFunc!=NULL );

    /* initialization */
    if (inf.pub.mode==InfMode_Uninitialized) {
        inf.pub.mode= InfMode_Feed;
        if (inf.pub.helperOutput.buffer==NULL) {
            inf.pub.helperOutput.buffer = (Byte*)malloc( inf.pub.helperOutput.bufferSize=InfHelperBufferSize );
            inf.pub.flags = InfFlags_FreeOutputBuffer;
        }
    }
    
    /* decompression */
    outputBuffer     = (Byte*)inf.pub.helperOutput.buffer;
    outputBufferSize =        inf.pub.helperOutput.bufferSize;
    do {
        action = inflaterProcessChunk(infptr, outputBuffer, outputBufferSize, compressedData, compressedDataSize);
        if (action==InfAction_UseOutputBufferContent) {
            inf.pub.dataReceiverFunc(&inf.pub, outputBuffer, inf.pub.outputBufferContentSize);
        }
        /* numberOfConsumedBytes += inf.inputChunkSize; */
    } while (action==InfAction_ProcessNextChunk || action==InfAction_UseOutputBufferContent);
    
/*  return numberOfConsumedBytes;  */
    return compressedDataSize;
}

/* receiverFunc     The function that will be called to store the resulting decompressed data */


void inflaterDestroy(Inflater* infptr) {
    if (infptr) {
        /* delete inf.obj; */
        if (0!=(inf.pub.flags & InfFlags_FreeInputBuffer  )) { free((void*)inf.pub.helperInput.buffer);  }
        if (0!=(inf.pub.flags & InfFlags_FreeOutputBuffer )) { free((void*)inf.pub.helperOutput.buffer); }
        if (0!=(inf.pub.flags & InfFlags_FreeItself       )) { free((void*)infptr);                      }
    }
}


#undef inf__FILL_INPUT_BUFFER
#undef inf__USE_OUTPUT_BUFFER_CONTENT
#undef inf__FINISH
#undef inf__ERROR
#undef inf__goto
#undef inf__fallthrough

#undef inf
