/*
    main.c
    uzo-proc
  
    Created by Martin on 23/06/2020.
    Copyright © 2020 Martin Rizzo. All rights reserved.
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define INFLATER_IMPLEMENTATION
#include "inflater.h"

typedef struct FileHeader {
    int    compressionMethod;
    size_t compressedSize;
    size_t uncompressedSize;
    long   contentPosition;
    char*  name;
    char   fullPath[1];
} FileHeader;

#define CHUNK_SIZE 512

#define getWord(ptr)  ( (ptr)[0] | (ptr)[1]<<8 )
#define getDWord(ptr) ( (ptr)[0] | (ptr)[1]<<8 | (ptr)[2]<<16 | (ptr)[3]<<24 )

#define isSequenceEqual(ptr1, ptr2, numberOfBytes) ( (ptr1)[0]==(ptr2)[0] && 0==memcmp((ptr1),(ptr2),(numberOfBytes)) )

static long getFileSize(FILE* file) { fseek(file,0,SEEK_END);  return ftell(file); }

static long min(long a, long b) { return a<b ? a : b; }


/*=================================================================================================================*/
#pragma mark - > ACCESSING THE ZIP DIRECTORY

/**
 * Finds the position of a sequence of bytes in the provided file
 * @param file                   The file to search
 * @param numberOfBytesToSearch  The maximum number of bytes to search in the provided file,
 * @param sequenceToFind         Pointer to the sequence of bytes to find
 * @param sequenceSize           The number of bytes contained in 'sequenceToFind'
 *                               (-1 = search all the file content)
 * @returns
 *    The position in the file where the sequence was found or (-1) if it was not found
 */
long reverseFindInFile(FILE* file, size_t numberOfBytesToSearch, const char* sequenceToFind, size_t sequenceSize) {
    char buffer[CHUNK_SIZE]; const char* ptr;
    long filePosition, fileSizeLeft; int bytesToRead, bytesToMove;
    const long fileSize        = file!=NULL ? getFileSize(file) : 0;
    const long fileSizeMinimum = numberOfBytesToSearch>=0 && fileSize>numberOfBytesToSearch ? (fileSize-numberOfBytesToSearch) : 0;
    assert( file!=NULL && sequenceToFind!=NULL );
    assert( 1<=sequenceSize && sequenceSize<CHUNK_SIZE );
    
    bytesToMove=0;
    for ( fileSizeLeft=fileSize; fileSizeLeft>fileSizeMinimum; fileSizeLeft=filePosition) {
        /* read a chunk of data from file */
        bytesToRead  = (int)min( (CHUNK_SIZE-bytesToMove) , fileSizeLeft );
        filePosition = (fileSizeLeft - bytesToRead);
        if (bytesToMove>0) { memmove(&buffer[bytesToRead], &buffer[0], bytesToMove); }
        fseek(file, filePosition, SEEK_SET); fread(buffer, 1, bytesToRead, file);
        /* try to find the sequence in the current chunk of data */
        for ( ptr=&buffer[bytesToRead+bytesToMove-sequenceSize]; ptr>=buffer; --ptr) {
            if ( isSequenceEqual(ptr,sequenceToFind,sequenceSize) ) { return filePosition+(ptr-buffer); }
        }
        bytesToMove = (int)sequenceSize;
    }
    return -1;
}


long getFirstFileHeaderPosition(FILE* zipFile) {
    static const char eocdSignature[4] = { 0x50, 0x4B, 0x05, 0x06 }; /* 'end of central directory' signature      */
    static const char cdfhSignature[4] = { 0x50, 0x4B, 0x01, 0x02 }; /* 'central directory file header' signature */
    long eocdPosition, filePosition; unsigned char eocd[20], cdfh[46]; /* end of central directory */
    unsigned numberOfRecords, fileDisk; long offsetOfDirectory;

    /* load the 'end-of-central-directory' record */
    eocdPosition = reverseFindInFile(zipFile,65536, eocdSignature,sizeof(eocdSignature));
    if (eocdPosition<0) { return -1; }
    fseek(zipFile, eocdPosition, SEEK_SET);
    if ( sizeof(eocd)!=fread(eocd, 1, sizeof(eocd), zipFile) ) { return -1; }
    
    /* read */
    if ( !isSequenceEqual(eocd, eocdSignature, sizeof(eocdSignature)) ) { return -1; }
    numberOfRecords   = getWord (&eocd[ 8]);
    offsetOfDirectory = getDWord(&eocd[16]);
    printf("number of records  : %d\n",  numberOfRecords  );
    printf("offset of directory: %ld\n", offsetOfDirectory);
    
    /* load the 'central-cirectory-file-header' record */
    if (numberOfRecords<1) { return -1; }
    fseek(zipFile, offsetOfDirectory, SEEK_SET);
    if ( sizeof(cdfh)!=fread(cdfh, 1, sizeof(cdfh), zipFile) ) { return -1; }
    
    /* read */
    if ( !isSequenceEqual(cdfh, cdfhSignature, sizeof(cdfhSignature)) ) { return -1; }
    fileDisk     = getWord (&cdfh[34]);
    filePosition = getDWord(&cdfh[42]);
    
    return filePosition;
}

FileHeader* allocFirstFileHeader(FILE* zipFile) {
    static const char lfhSignature[4] = { 0x50, 0x4B, 0x03, 0x04 }; /* 'local file header' signature */
    unsigned char lfh[30];
    char* ptr;
    int fullPathLength, extraFieldLength;
    FileHeader* fileHeader = NULL;
    long fileHeaderPosition;
    assert( zipFile!=NULL );
    
    /* load the 'file-header' record */
    fileHeaderPosition = getFirstFileHeaderPosition(zipFile);
    if (fileHeaderPosition<0) { return NULL; }
    fseek(zipFile, fileHeaderPosition, SEEK_SET);
    if ( fread(lfh, sizeof(lfh), 1, zipFile)!=1 ) { return NULL; }
    
    /* read */
    if ( !isSequenceEqual(lfh, lfhSignature, sizeof(lfhSignature)) ) { return NULL; }
    fullPathLength   = getWord(&lfh[26]);
    extraFieldLength = getWord(&lfh[28]);
    fileHeader = malloc(sizeof(FileHeader)+fullPathLength+1);
    fileHeader->compressionMethod = getWord (&lfh[ 8]);
    fileHeader->compressedSize    = getDWord(&lfh[18]);
    fileHeader->uncompressedSize  = getDWord(&lfh[22]);
    fileHeader->contentPosition   = fileHeaderPosition+30+fullPathLength+extraFieldLength;
    if ( fread(fileHeader->fullPath, fullPathLength, 1,zipFile)!=1 ) { return NULL; }
    fileHeader->fullPath[fullPathLength]='\0';
    fileHeader->name = fileHeader->fullPath;
    for (ptr=fileHeader->fullPath; *ptr!='\0'; ++ptr) {
        if (*ptr=='/' || *ptr=='\\') { fileHeader->name = (ptr+1); }
    }
    return fileHeader;
}

/*=================================================================================================================*/
#pragma mark - > DECOMPRESSING

void inflateFile(FILE* zipFile, const char* fileName, long fileContentPosition) {
    size_t outputBufferSize = 65536;
    size_t inputBufferSize  = 65536;
    char *outputBuffer, *inputBuffer; FILE* outputFile;
    Inflater* inflater; InfAction action;
    assert( zipFile!=NULL && fileName!=NULL && fileContentPosition>0 );
    
    outputBuffer = malloc(outputBufferSize);
    inputBuffer  = malloc(inputBufferSize);
    outputFile   = fopen(fileName, "wb");
    inflater     = inflaterCreate(0,0);
    if (outputBuffer!=NULL && inputBuffer!=NULL && outputFile!=NULL && inflater!=NULL ) {
        
        fseek(zipFile, fileContentPosition, SEEK_SET );
        inputBufferSize = fread(inputBuffer, 1, inputBufferSize, zipFile);
        
        action = InfAction_Init;
        while ( action != InfAction_Finish ) {
            action = inflaterProcessChunk(inflater, outputBuffer, outputBufferSize, inputBuffer, inputBufferSize);
            if ( action == InfAction_FillInputBuffer ) {
                inputBufferSize = fread(inputBuffer, 1, inputBufferSize, zipFile);
            }
            if ( action == InfAction_UseOutputBufferContent ) {
                fwrite(outputBuffer, 1, inflater->outputBufferContentSize, outputFile);
            }
        }
    }
    inflaterDestroy(inflater);
    fclose(outputFile);
    free(inputBuffer);
    free(outputBuffer);
}

void unzipFirstFile(const char* zipFilePath) {
    FILE *zipFile; FileHeader* fileHeader;
    assert( zipFilePath!=NULL && zipFilePath[0]!='\0' );
    
    zipFile = fopen(zipFilePath, "rb");
    if (zipFile==NULL) { return; }
    
    fileHeader = allocFirstFileHeader(zipFile);
    if (fileHeader->compressionMethod==8) {
        printf("Inflating \"%s\"\n", fileHeader->name);
        inflateFile(zipFile, fileHeader->name, fileHeader->contentPosition);
    }
    free(fileHeader);
    fclose(zipFile);
}

/*=================================================================================================================*/
#pragma mark - > MAIN

#define VERSION   "0.1"
#define COPYRIGHT "Copyright (c) 2020 Martin Rizzo"
#define isOption(param,name1,name2) \
    (strcmp(param,name1)==0 || strcmp(param,name2)==0)

/**
 * Application starting point
 * @param argc  The number of elements in the 'argv' array
 * @param argv  An array containing each command-line parameter (starting at argv[1])
 */
int main(int argc, char *argv[]) {
    const char **filePaths; int numberOfFiles;
    const char *param; int i;
    int printHelpAndExit=0, printVersionAndExit=0;
    const char *help[] = {
        "USAGE: uzo-proc [options] file1.zip file2.zip ...","",
        "  OPTIONS:",
        "    -h, --help             display this help and exit",
        "    -v, --version          output version information and exit",
        NULL
    };
    

    /* process all flags/options and store all filenames */
    filePaths     = malloc(argc * sizeof(char*));
    numberOfFiles = 0;
    for (i=1; i<argc; ++i) { param=argv[i];
        if ( param[0]!='-' ) { filePaths[numberOfFiles++]=param; }
        else {
            if      ( isOption(param,"-h","--help")    ) { printHelpAndExit=1;    }
            else if ( isOption(param,"-v","--version") ) { printVersionAndExit=1; }
        }
    }
    
    /* print help or version if requested */
    if ( printHelpAndExit    ) { i=0; while (help[i]!=NULL) { printf("%s\n",help[i++]); } return 0; }
    if ( printVersionAndExit ) { printf("UZO (unzip one file) version %s\n%s\n", VERSION, COPYRIGHT);    return 0; }
    
    /* decompress the first file of each requested zip file */
    for (i=0; i<numberOfFiles; ++i) {
        unzipFirstFile(filePaths[i]);
    }
    
    free(filePaths);
    return 0;
}
