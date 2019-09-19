#ifndef zip__h
#define zip__h

#include "bytearray.h"
#include "files.h"
#include "zlib.h"
extern int little_endian;

typedef struct ZipHeader{
	unsigned int sig;//must be 0x04034b50
	unsigned short verNeed;
	unsigned short flag;
	unsigned short method;
	unsigned short modifytime;
	unsigned short modifydate;
	unsigned int crc32;
	unsigned int size;
	unsigned int unCompressedSize;
	unsigned short filenameLen;
	unsigned short extraFieldLen;
	char * fileName;
	char * extraField;
	char * data;
}ZipHeader;
typedef struct ZipFile{
	ZipHeader * header;
	ByteArray * bytearray;
}ZipFile;

void ZipFile_free(ZipFile * file);
ZipFile * ZipFile_unzipAll(ByteArray * bytearray,char * path);
#endif
