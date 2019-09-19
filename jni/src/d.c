/**
 * @file doc.c
 gcc -Werror d.c -D test_doc -lm && ./a.out
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-10-22
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<ctype.h>

typedef struct ByteArray{
	char * data;
	int position;
	int length;
} ByteArray;

char ByteArray_readByte(ByteArray * bytearray);
void ByteArray_readBytes(ByteArray * bytearray,int size,char * bytes);
unsigned short int ByteArray_readInt16(ByteArray * bytearray);
unsigned int ByteArray_readInt32(ByteArray * bytearray);
int ByteArray_print16(ByteArray*bytearray,int n);
int ByteArray_prints(ByteArray*bytearray,int n);
void ByteArray_printx(ByteArray*bytearray,int num);


ByteArray * ByteArray_new(int size);
void ByteArray_free(ByteArray*bytearray);
ByteArray*ByteArray_resize(ByteArray*bytearray,int size);
ByteArray * ByteArray_writeBytes(ByteArray*bytearray,char*bytes,int length);
void ByteArray_rewind(ByteArray*bytearray);
char * ByteArray_readUtf16(ByteArray * bytearray,int n);//need free

extern int little_endian;

typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef short OFFSET;
typedef ULONG SECT;
typedef ULONG FSINDEX;
typedef USHORT FSOFFSET;
typedef ULONG DFSIGNATURE;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef WORD DFPROPTYPE;
typedef ULONG SID;
//typedef CLSID GUID;
typedef struct tagFILETIME 
{
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME, TIME_T;
typedef enum tagSTGTY
{
	STGTY_INVALID,
	STGTY_STORAGE,
	STGTY_STREAM,
	STGTY_LOCKBYTES,
	STGTY_PROPERTY,
	STGTY_ROOT,
} STGTY;

typedef struct StorageEntry
{
	char * name;//u16 string 64 bytes;
	unsigned short entryStrLen;//2 bytes
	char type;//1 byte; 0:empty,1:Storage,2:stream,3:lockbytes,4:property,t,root Storage
	char color;//1 byte; 0:red,1:black
	int leftSonId;//4 bytes; -1:nothing;
	int rightSonId;//4 bytes; -1:nothing
	int rootId;// 4 bytes;
	char *id;//16 bytes
	int uid;//4 bytes
	char * createTime;//8 bytes;from 1601.1.1 00:00:00, ms
	char * modifyTime;//8 bytes
	int sid0;//4 bytes
	int streamSize;//4 bytes
	short int noUse;//4 bytes

	int did;
	struct StorageEntry * root;
	struct StorageEntry * left;
	struct StorageEntry * right;
}StorageEntry;


typedef struct OleHeader
{
	ByteArray *bytearray;
	char *headstr;
	int little_endian;
	int sid0;
	unsigned int sectorSize;
	unsigned int miniSsize;
	unsigned int numS;
	unsigned int streamSize;
	int miniSid0;
	unsigned int numMiniS;
	int msid0;
	unsigned int numMiniMS;
	int MSectorIDs[109];


	int numStorages;
	StorageEntry *rootStorage;
	StorageEntry *storages[16];
}OleHeader;


unsigned int Ole_getEntryPostion(OleHeader * header,StorageEntry *entry);
StorageEntry * Ole_getEntryByName(OleHeader *header,char *name);
OleHeader * Ole_read(OleHeader* header,ByteArray * bytearray);
void Ole_free(OleHeader * header);
static int Ole_getSectorPos(OleHeader* header,int sid)
{
	return 0x200 + sid*header->sectorSize;
}
static int Ole_getShortSectorPos(OleHeader* header,int sid)
{
	return 0x200 + sid*header->miniSsize;
}


unsigned int Ole_getEntryPostion(OleHeader * header,StorageEntry *entry)
{
	unsigned int position;
	if(entry->streamSize >= header->streamSize){
		position = Ole_getSectorPos(header,entry->sid0);
	}else{
		position = Ole_getShortSectorPos(header,entry->sid0);
	}
	return position;
}


StorageEntry * Ole_getEntryByName(OleHeader *header,char *name)
{
	int i = 0;
	while(i< header->numStorages)
	{
		StorageEntry * entry = header->storages[i];
		if(entry)
		{
			//printf("%s,",entry->name);
			if(strcmp(entry->name,name)==0)
			{
				return entry;
			}
		}
		++i;
	}
	return NULL;
}

static StorageEntry* Ole_readStorageFromBytearray(ByteArray* bytearray)
{
	//int position = bytearray->position;
	StorageEntry* storage = (StorageEntry*)malloc(sizeof(StorageEntry));
	//ByteArray_print16(bytearray,0);
	memset(storage,0,sizeof(StorageEntry));
	storage->name = ByteArray_readUtf16(bytearray,32);
	//bytearray->position += 64;
	storage->entryStrLen = ByteArray_readInt16(bytearray);
	storage->type= ByteArray_readByte(bytearray);
	storage->color= ByteArray_readByte(bytearray);
	storage->leftSonId = ByteArray_readInt32(bytearray);
	storage->rightSonId = ByteArray_readInt32(bytearray);
	storage->rootId = ByteArray_readInt32(bytearray);
	storage->id= bytearray->data+bytearray->position;
	bytearray->position+=16;
	storage->uid= ByteArray_readInt32(bytearray);
	bytearray->position+=16;
	storage->sid0= ByteArray_readInt32(bytearray);
	storage->streamSize= ByteArray_readInt32(bytearray);
	bytearray->position+=4;

	printf("name: %s,",storage->name);
	//printf("%d,",bytearray->position-position);
	//printf("storage id:%d\n",storage->id);
	switch(storage->type)
	{
		case 0:
			printf("type:%s,","invalid");
			return storage;
			break;
		case 1:
			printf("type:%s,","storage");
			break;
		case 2:
			printf("type:%s,","stream");
			break;
		case 3:
			printf("type:%s,","lockbytes");
			break;
		case 4:
			printf("type:%s,","property");
			break;
		case 5:
			printf("type:%s,","root");
			break;
	}
	/*
	   if(storage->color)
	   printf("storage color:%s\n","black");
	   else
	   printf("storage color:%s\n","red");
	   */

	if(storage->leftSonId!=-1) printf("left :%d ;",storage->leftSonId);
	if(storage->rightSonId !=-1) printf("right :%d ;",storage->rightSonId);
	if(storage->rootId !=-1) printf("treeId :%d ;",storage->rootId);

	printf("streamSize:0x%x ;",storage->streamSize);
	printf("sectorId :%d\n",storage->sid0);
	//printf("-----------------------\n");
	return storage;
}

static StorageEntry * Ole_readEntrys(OleHeader * header,int entryId)
{
	if(entryId>=0)
	{
		ByteArray * bytearray = header->bytearray;
		bytearray->position = Ole_getSectorPos(header,header->sid0) + 128 * entryId;
		printf("\n=======<0x%x>(%d)",bytearray->position,entryId);
		StorageEntry * entry = Ole_readStorageFromBytearray(bytearray);
		header->storages[entryId] = entry;
		entry->did = entryId;
		if(header->numStorages -1< entryId)
		{
			header->numStorages = entryId + 1;
		}

		/*
		   if(entry->sid0>=0){
		   bytearray->position = Ole_getSectorPos(header->header,entry->sid0);
	//printf("\n_=[0x%x]=(%d): ",bytearray->position,entryId);
	short int sid = ByteArray_readInt16(bytearray);
	while(sid){
	printf("%d,",sid);
	if(sid==-2)
	break;
	sid = ByteArray_readInt16(bytearray);
	}
	}
	*/

		if(entry->leftSonId>0){
			entry->left = Ole_readEntrys(header,entry->leftSonId);
		}
		if(entry->rightSonId>0){
			entry->left = Ole_readEntrys(header,entry->rightSonId);
		}
		if(entry->rootId>0){
			entry->root = Ole_readEntrys(header,entry->rootId);
		}
		return entry;
	}
	return NULL;
}


OleHeader * Ole_read(OleHeader * header,ByteArray * bytearray)
{
	if(header)
		return header;

	bytearray->position = 0;
	int i=0;
	char _header[8];
	header = malloc(sizeof(OleHeader));
	memset(header,0,sizeof(OleHeader));
	header->bytearray = bytearray;
	bytearray->position = 0;
	ByteArray_readBytes(bytearray,8,_header);

	if(strncmp(_header,"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1",8)==0){
		//printf("headstr\n");
	}else if(strncmp(_header,"\x0e\x11\xfc\x0d\xd0\xcf\x11\xe0",8)==0){
		//printf("old headstr\n");
	}else{
		printf("not a doc file\n");
		free(header);
		return NULL;
	}

	bytearray->position = 0x1c;
	unsigned int byteOrder=(unsigned int)ByteArray_readInt16(bytearray);
	if(byteOrder==0xfffe)
	{
		little_endian= 1;
		printf("little_endian\n");
	}else if(byteOrder==0xfeff){
		little_endian= 0;
		printf("big_endian\n");
	}else{
		printf("unknow byteOrder\n");
		free(header);
		return NULL;
	}
	bytearray->position = 0x1e;
	header->sectorSize= pow(2,ByteArray_readInt16(bytearray));
	bytearray->position = 0x20;
	header->miniSsize= pow(2,ByteArray_readInt16(bytearray));
	bytearray->position = 0x2c;
	header->numS= ByteArray_readInt32(bytearray);
	bytearray->position = 0x30;
	header->sid0= (int)ByteArray_readInt32(bytearray);

	bytearray->position = 0x38;
	header->streamSize = ByteArray_readInt32(bytearray);//maximum size for mini-streams: typically 4096 bytes
	bytearray->position = 0x3c;
	header->miniSid0= (int)ByteArray_readInt32(bytearray);
	bytearray->position = Ole_getSectorPos(header,header->miniSid0);
	//ByteArray_print16(bytearray,0);
	bytearray->position = 0x40;
	header->numMiniS= ByteArray_readInt32(bytearray);
	bytearray->position = 0x44;
	header->msid0= ByteArray_readInt32(bytearray);
	bytearray->position = 0x48;
	header->numMiniMS= ByteArray_readInt32(bytearray);


	bytearray->position = 0x4c;
	i=0;
	while(i< header->numS){
		header->MSectorIDs[i]= ByteArray_readInt32(bytearray);
		printf("%d,",header->MSectorIDs[i]);
		if(header->MSectorIDs[i]== -2)
			break;
		++i;
	}
	i=0;
	while(i< header->numS){
		int sid= header->MSectorIDs[i];
		if(sid>0){
			bytearray->position = Ole_getSectorPos(header,sid);
			int _sid = ByteArray_readInt32(bytearray);
			printf("\narr[%d]%d:",i,sid);
			while(_sid>=-4){
				if(_sid==-2)
					break;
				printf("%d,",_sid);
				_sid = ByteArray_readInt32(bytearray);
			}
		}
		if(sid== -2)
			break;
		++i;
	}

	printf("\n");


	printf("sid0 : %d, " ,header->sid0);
	printf("sectorSize:%d, " ,header->sectorSize);
	printf("miniSsize :%d, " ,header->miniSsize);
	printf("numS :%d, " ,header->numS);
	printf("numMiniS :%d, " ,header->numMiniS);
	printf("numMiniMS:%d, " ,header->numMiniMS);
	printf("streamSize : %d, " ,header->streamSize);
	printf("msid0 : %d, " ,header->msid0);
	printf("miniSid0 : %d, " ,header->miniSid0);

	printf("==========\n");

	//return 0;
	header->rootStorage = Ole_readEntrys(header,0);
	if(header->rootStorage == NULL)
		return 0;


	return header;
}

void StorageEntry_free(StorageEntry*entry)
{
	if(entry)
	{
		if(entry->name)
			free(entry->name);
		free(entry);
	}
}
void Ole_free(OleHeader * header)
{
	if(header)
	{
		if(header->rootStorage)
		{
			int i=0;
			while(i<header->numStorages)
			{
				StorageEntry_free(header->storages[i]);
				++i;
			}
		}

		free(header);
	}
}
extern int little_endian;

typedef struct FibBaseFlag1
{
	unsigned short A:1;// - fDot (1 bit): Specifies whether this is a document template.
	unsigned short B:1;// - fGlsy (1 bit): Specifies whether this is a document that contains only AutoText items (see FibRgFcLcb97.fcSttbfGlsy, FibRgFcLcb97.fcPlcfGlsy and FibRgFcLcb97.fcSttbGlsyStyle).
	unsigned short C:1;// - fComplex (1 bit): Specifies that the last save operation that was performed on this document was an incremental save operation.
	unsigned short D:1;// - fHasPic (1 bit): When set to 0, there SHOULD<13> be no pictures in the document.  
	unsigned short E:4;// - cQuickSaves (4 bits): An unsigned integer. If nFib is less than 0x00D9, then cQuickSaves specifies the number of consecutive times this document was incrementally saved. If nFib is 0x00D9 or greater, then cQuickSaves MUST be 0xF.
	unsigned short F:1;// - fEncrypted (1 bit): Specifies whether the document is encrypted or obfuscated as specified in Encryption and Obfuscation.
	unsigned short G:1;// - fWhichTblStm (1 bit): Specifies the Table stream to which the FIB refers. When this value is set to 1, use 1Table; when this value is set to 0, use 0Table.
	unsigned short H:1;// - fReadOnlyRecommended (1 bit): Specifies whether the document author recommended that the document be opened in read-only mode.
	unsigned short I:1;// - fWriteReservation (1 bit): Specifies whether the document has a write-reservation password.
	unsigned short J:1;// - fExtChar (1 bit): This value MUST be 1.
	unsigned short K:1;// - fLoadOverride (1 bit): Specifies whether to override the language information and font that are specified in the paragraph style at istd 0 (the normal style) with the defaults that are appropriate for the installation language of the application.
	unsigned short L:1;// - fFarEast (1 bit): Specifies whether the installation language of the application that created the document was an East Asian language.
	unsigned short M:1;// - fObfuscated (1 bit): If fEncrypted is 1, this bit specifies whether the document is obfuscated by using XOR obfuscation (section 2.2.6.1); otherwise, this bit MUST be ignored.
}FibBaseFlag1;

typedef struct FibBaseFlag2
{
	unsigned char N:1;// - fMac (1 bit): This value MUST be 0, and MUST be ignored.
	unsigned char O:1;// - fEmptySpecial (1 bit): This value SHOULD<15> be 0 and SHOULD<16> be ignored.
	unsigned char P:1;// - fLoadOverridePage (1 bit): Specifies whether to override the section properties for page size, orientation, and margins with the defaults that are appropriate for the installation language of the application.
	unsigned char Q:1;// - reserved1 (1 bit): This value is undefined and MUST be ignored.
	unsigned char R:1;// - reserved2 (1 bit): This value is undefined and MUST be ignored.
	unsigned char S:3;// - fSpare0 (3 bits): This value is undefined and MUST be ignored.
}FibBaseFlag2;


typedef struct FibBase
{
	unsigned short wIdent;//wIdent (2 bytes): An unsigned integer that specifies that this is a Word Binary File. This value MUST be 0xA5EC.
	unsigned short nFib;// (2 bytes): An unsigned integer that specifies the version number of the file format used.  Superseded by FibRgCswNew.nFibNew if it is present. This value SHOULD<12> be 0x00C1.
	short unused;// (2 bytes): This value is undefined and MUST be ignored.
	short lid;// (2 bytes): A LID that specifies the install language of the application that is producing the document. If nFib is 0x00D9 or greater, then any East Asian install lid or any install lid with a base language of Spanish, German or French MUST be recorded as lidAmerican. If the nFib is 0x0101 or greater, then any install lid with a base language of Vietnamese, Thai, or Hindi MUST be recorded as lidAmerican.
	unsigned short pnNext;// (2 bytes): An unsigned integer that specifies the offset in the WordDocument stream of the FIB for the document which contains all the AutoText items. If this value is 0, there are no AutoText items attached. Otherwise the FIB is found at file location pnNext¡Á512. If fGlsy is 1 or fDot is 0, this value MUST be 0. If pnNext is not 0, each FIB MUST share the same values for
	FibBaseFlag1 flag;
	unsigned short nFibBack;// (2 bytes): This value SHOULD<14> be 0x00BF. This value MUST be 0x00BF or 0x00C1.
	int lKey;// (4 bytes);//: If fEncrypted is 1 and fObfuscation is 1, this value specifies the XOR obfuscation (section 2.2.6.1) password verifier. If fEncrypted is 1 and fObfuscation is 0, this value specifies the size of the EncryptionHeader that is stored at the beginning of the Table stream as described in Encryption and Obfuscation. Otherwise, this value MUST be 0.
	FibBaseFlag2 flag2;
	char envr;// (1 byte): This value MUST be 0, and MUST be ignored.
}FibBase;

typedef struct FibRgW97
{
	char reserved[26];
	short lidFE;//0x00C1 If FibBase.fFarEast is "true", this is the LID of the stored style names. Otherwise it MUST
	//0x00D9 0x0101 0x010C 0x0112 The LID of the stored style names (STD.xstzName be ignored.
}FibRgW97;

typedef struct FibRgLw97
{
	int cbMac;// (4 bytes): Specifies the count of bytes of those written to the WordDocument stream of the file that have any meaning. All bytes in the WordDocument stream at offset cbMac and greater MUST be ignored.
	int reserved1;// (4 bytes): This value is undefined and MUST be ignored.
	int reserved2;// (4 bytes): This value is undefined and MUST be ignored.
	int ccpText;// (4 bytes): A signed integer that specifies the count of CPs in the main document. This value MUST be zero, 1, or greater.
	int ccpFtn;// (4 bytes): A signed integer that specifies the count of CPs in the footnote subdocument. This value MUST be zero, 1, or greater.
	int ccpHdd;// (4 bytes): A signed integer that specifies the count of CPs in the header subdocument. This value MUST be zero, 1, or greater.
	int reserved3;// (4 bytes): This value MUST be zero and MUST be ignored.
	int ccpAtn;// (4 bytes): A signed integer that specifies the count of CPs in the comment subdocument. This value MUST be zero, 1, or greater.
	int ccpEdn ;//(4 bytes): A signed integer that specifies the count of CPs in the endnote subdocument. This value MUST be zero, 1, or greater.
	int ccpTxbx ;//(4 bytes): A signed integer that specifies the count of CPs in the textbox subdocument of the main document. This value MUST be zero, 1, or greater.
	int ccpHdrTxbx ;//(4 bytes): A signed integer that specifies the count of CPs in the textbox subdocument of the header. This value MUST be zero, 1, or greater.
	int reserved4 ;//(4 bytes): This value is undefined and MUST be ignored.
	int reserved5 ;//(4 bytes): This value is undefined and MUST be ignored.
	int reserved6 ;//(4 bytes): This value MUST be equal or less than the number of data elements in PlcBteChpx, as specified by FibRgFcLcb97.fcPlcfBteChpx and FibRgFcLcb97.lcbPlcfBteChpx. This value MUST be ignored.
	int reserved7 ;//(4 bytes): This value is undefined and MUST be ignored
	int reserved8 ;//(4 bytes): This value is undefined and MUST be ignored
	int reserved9 ;//(4 bytes): This value MUST be less than or equal to the number of data elements in PlcBtePapx, as specified by FibRgFcLcb97.fcPlcfBtePapx and ibRgFcLcb97.lcbPlcfBtePapx. This value MUST be ignored.
	int reserved10 ;//(4 bytes): This value is undefined and MUST be ignored.
	int reserved11 ;//(4 bytes): This value is undefined and MUST be ignored.
	int reserved12 ;//(4 bytes): This value SHOULD <26> be zero, and MUST be ignored.
	int reserved13 ;//(4 bytes): This value MUST be zero and MUST be ignored.
	int reserved14 ;//(4 bytes): This value MUST be zero and MUST be ignored.
}FibRgLw97;
typedef struct FibRgCswNew
{
	short nFibNew;//nFibNew (2 bytes): An unsigned integer that specifies the version number of the file format that is used. This value MUST be one of the following.
	//0x00D9 0x0101 0x010C 0x0112
	char rgCswNewData[8];//rgCswNewData (variable): Depending on the value of nFibNew this is one of the following.
	/*Value of nFibNew Meaning
	  0x00D9 fibRgCswNewData2000 (2 bytes)
	  0x0101 fibRgCswNewData2000 (2 bytes)
	  0x010C fibRgCswNewData2000 (2 bytes)
	  0x0112 fibRgCswNewData2007 (8 bytes)
	  */
}FibRgCswNew;

typedef struct FibRgFcLcb97
{
	unsigned int fcStshfOrig;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbStshfOrig;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int fcStshf;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An STSH that specifies the style sheet for this document begins at this offset.
	unsigned int lcbStshf;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the STSH that begins at offset fcStshf in the Table Stream. This MUST be a nonzero value.
	unsigned int fcPlcffndRef;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcffndRef begins at this offset and specifies the locations of footnote references in the Main Document, and whether those references use auto-numbering or custom symbols. If lcbPlcffndRef is zero, fcPlcffndRef is undefined and MUST be ignored.
	unsigned int lcbPlcffndRef;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcffndRef that begins at offset fcPlcffndRef in the Table Stream.
	unsigned int fcPlcffndTxt;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcffndTxt begins at this offset and specifies the locations of each block of footnote text in the Footnote Document. If lcbPlcffndTxt is zero, fcPlcffndTxt is undefined and MUST be ignored.
	unsigned int lcbPlcffndTxt;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcffndTxt that begins at offset fcPlcffndTxt in the Table Stream.  lcbPlcffndTxt MUST be zero if FibRgLw97.ccpFtn is zero, and MUST be nonzero if FibRgLw97.ccpFtn is nonzero.
	unsigned int fcPlcfandRef;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfandRef begins at this offset and specifies the dates, user initials, and locations of comments in the Main Document. If lcbPlcfandRef is zero, fcPlcfandRef is undefined and MUST be ignored.
	unsigned int lcbPlcfandRef;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfandRef at offset fcPlcfandRef in the Table Stream.
	unsigned int fcPlcfandTxt;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfandTxt begins at this offset and specifies the locations of comment text ranges in the Comment Document. If lcbPlcfandTxt is zero, fcPlcfandTxt is undefined, and MUST be ignored.
	unsigned int lcbPlcfandTxt;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfandTxt at offset fcPlcfandTxt in the Table Stream.  lcbPlcfandTxt MUST be zero if FibRgLw97.ccpAtn is zero, and MUST be nonzero if FibRgLw97.ccpAtn is nonzero.
	unsigned int fcPlcfSed;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfSed begins at this offset and specifies the locations of property lists for each section in the Main Document. If lcbPlcfSed is zero, fcPlcfSed is undefined and MUST be ignored.
	unsigned int lcbPlcfSed;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfSed that begins at offset fcPlcfSed in the Table Stream.
	unsigned int fcPlcPad;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbPlcPad;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcPlcfPhe;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Plc begins at this offset and specifies version-specific information about paragraph height. This Plc SHOULD NOT<27> be emitted and SHOULD<28> be ignored.
	unsigned int lcbPlcfPhe;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Plc at offset fcPlcfPhe in the Table Stream.
	unsigned int fcSttbfGlsy;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A SttbfGlsy that contains information about the AutoText items that are defined in this document begins at this offset.
	unsigned int lcbSttbfGlsy;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfGlsy at offset fcSttbfGlsy in the Table Stream. If base.fGlsy of the Fib that contains this FibRgFcLcb97 is zero, this value MUST be zero.
	unsigned int fcPlcfGlsy;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfGlsy that contains information about the AutoText items that are defined in this document begins at this offset.
	unsigned int lcbPlcfGlsy;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfGlsy at offset fcPlcfGlsy in the Table Stream. If base.fGlsy of the Fib that contains this FibRgFcLcb97 is zero, this value MUST be zero.
	unsigned int fcPlcfHdd;//(4 bytes): An unsigned integer that specifies the offset in the Table Stream where a Plcfhdd begins. The Plcfhdd specifies the locations of each block of header/footer text in the WordDocument Stream. If lcbPlcfHdd is 0, fcPlcfHdd is undefined and MUST be ignored.
	unsigned int lcbPlcfHdd;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Plcfhdd at offset fcPlcfHdd in the Table Stream. If there is no Plcfhdd, this value MUST be zero. A Plcfhdd MUST exist if FibRgLw97.ccpHdd indicates that there are characters in the Header Document (that is, if FibRgLw97.ccpHdd is greater than 0). Otherwise, the Plcfhdd MUST NOT exist.
	unsigned int fcPlcfBteChpx;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcBteChpx begins at the offset. fcPlcfBteChpx MUST be greater than zero, and MUST be a valid offset in the Table Stream.
	unsigned int lcbPlcfBteChpx;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcBteChpx at offset fcPlcfBteChpx in the Table Stream. lcbPlcfBteChpx MUST be greater than zero.
	unsigned int fcPlcfBtePapx;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcBtePapx begins at the offset. fcPlcfBtePapx MUST be greater than zero, and MUST be a valid offset in the Table Stream.
	unsigned int lcbPlcfBtePapx;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcBtePapx at offset fcPlcfBtePapx in the Table Stream. lcbPlcfBteChpx MUST be greater than zero.
	unsigned int fcPlcfSea;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbPlcfSea;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcSttbfFfn;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An SttbfFfn begins at this offset. This table specifies the fonts that are used in the document. If lcbSttbfFfn is 0, fcSttbfFfn is undefined and MUST be ignored.
	unsigned int lcbSttbfFfn;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfFfn at offset fcSttbfFfn in the Table Stream.
	unsigned int fcPlcfFldMom;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcFld begins at this offset and specifies the locations of field characters in the Main Document. All CPs in this PlcFld MUST be greater than or equal to 0 and less than or equal to FibRgLw97.ccpText. If lcbPlcfFldMom is zero, fcPlcfFldMom is undefined and MUST be ignored.
	unsigned int lcbPlcfFldMom;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcFld at offset fcPlcfFldMom in the Table Stream.

	unsigned int fcPlcfFldHdr;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcFld begins at this offset and specifies the locations of field characters in the Header Document. All CPs in this PlcFld are relative to the starting position of the Header Document. All CPs in this PlcFld MUST be greater than or equal to zero and less than or equal to FibRgLw97.ccpHdd. If lcbPlcfFldHdr is zero, fcPlcfFldHdr is undefined and MUST be ignored.
	unsigned int lcbPlcfFldHdr;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcFld at offset fcPlcfFldHdr in the Table Stream.
	unsigned int fcPlcfFldFtn;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcFld begins at this offset and specifies the locations of field characters in the Footnote Document. All CPs in this PlcFld are relative to the starting position of the Footnote Document. All CPs in this PlcFld MUST be greater than or equal to zero and less than or equal to FibRgLw97.ccpFtn. If lcbPlcfFldFtn is zero, fcPlcfFldFtn is undefined, and MUST be ignored.
	unsigned int lcbPlcfFldFtn;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcFld at offset fcPlcfFldFtn in the Table Stream.
	unsigned int fcPlcfFldAtn;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcFld begins at this offset and specifies the locations of field characters in the Comment Document. All CPs in this PlcFld are relative to the starting position of the Comment Document. All CPs in this PlcFld MUST be greater than or equal to zero and less than or equal to FibRgLw97.ccpAtn. If lcbPlcfFldAtn is zero, fcPlcfFldAtn is undefined and MUST be ignored.
	unsigned int lcbPlcfFldAtn;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcFld at offset fcPlcfFldAtn in the Table Stream.
	unsigned int fcPlcfFldMcr;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbPlcfFldMcr;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcSttbfBkmk;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An SttbfBkmk that contains the names of the bookmarks in the document begins at this offset. If lcbSttbfBkmk is zero, fcSttbfBkmk is undefined and MUST be ignored.  This SttbfBkmk is parallel to the PlcfBkf at offset fcPlcfBkf in the Table Stream. Each string specifies the name of the bookmark that is associated with the data element which is located at the same offset in that PlcfBkf. For this reason, the SttbfBkmk that begins at offset fcSttbfBkmk, and the PlcfBkf that begins at offset fcPlcfBkf, MUST contain the same number of elements.
	unsigned int lcbSttbfBkmk;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfBkmk at offset fcSttbfBkmk.
	unsigned int fcPlcfBkf;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfBkf that contains information about the standard bookmarks in the document begins at this offset. If lcbPlcfBkf is zero, fcPlcfBkf is undefined and MUST be ignored.  Each data element in the PlcfBkf is associated, in a one-to-one correlation, with a data element in the PlcfBkl at offset fcPlcfBkl. For this reason, the PlcfBkf that begins at offset fcPlcfBkf, and the PlcfBkl that begins at offset fcPlcfBkl, MUST contain the same number of data elements. This PlcfBkf is parallel to the SttbfBkmk at offset fcSttbfBkmk in the Table Stream. Each data element in the PlcfBkf specifies information about the bookmark that is associated with the element which is located at the same offset in that SttbfBkmk. For this reason, the PlcfBkf that begins at offset fcPlcfBkf, and the SttbfBkmk that begins at offset fcSttbfBkmk, MUST contain the same number of elements.  The largest value that a CP marking the start or end of a standard bookmark is allowed to have is the CP representing the end of all document parts.
	unsigned int lcbPlcfBkf;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfBkf at offset fcPlcfBkf.
	unsigned int fcPlcfBkl;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfBkl that contains information about the standard bookmarks in the document begins at this offset. If lcbPlcfBkl is zero, fcPlcfBkl is undefined and MUST be ignored.  Each data element in the PlcfBkl is associated, in a one-to-one correlation, with a data element in the PlcfBkf at offset fcPlcfBkf. For this reason, the PlcfBkl that begins at offset fcPlcfBkl, and the PlcfBkf that begins at offset fcPlcfBkf, MUST contain the same number of data elements.  The largest value that a CP marking the start or end of a standard bookmark is allowed to have is the value of the CP representing the end of all document parts.
	unsigned int lcbPlcfBkl;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfBkl at offset fcPlcfBkl.
	unsigned int fcCmds;//(4 bytes): An unsigned integer that specifies the offset in the Table Stream of a Tcg that specifies command-related customizations. If lcbCmds is zero, fcCmds is undefined and MUST be ignored.
	unsigned int lcbCmds;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Tcg at offset fcCmds.
	unsigned int fcUnused1;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbUnused1;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcSttbfMcr;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbSttbfMcr;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcPrDrvr;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The PrDrvr, which contains printer driver information (the names of drivers, port, and so on), begins at this offset. If lcbPrDrvr is zero, fcPrDrvr is undefined and MUST be ignored.
	unsigned int lcbPrDrvr;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PrDrvr at offset fcPrDrvr.
	unsigned int fcPrEnvPort;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The PrEnvPort that is the print environment in portrait mode begins at this offset. If lcbPrEnvPort is zero, fcPrEnvPort is undefined and MUST be ignored.
	unsigned int lcbPrEnvPort;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PrEnvPort at offset fcPrEnvPort.
	unsigned int fcPrEnvLand;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The PrEnvLand that is the print environment in landscape mode begins at this offset. If lcbPrEnvLand is zero, fcPrEnvLand is undefined and MUST be ignored.
	unsigned int lcbPrEnvLand;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PrEnvLand at offset fcPrEnvLand.
	unsigned int fcWss;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Selsf begins at this offset and specifies the last selection that was made in the Main Document. If lcbWss is zero, fcWss is undefined and MUST be ignored.
	unsigned int lcbWss;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Selsf at offset fcWss.
	unsigned int fcDop;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Dop begins at this offset.			 
	unsigned int lcbDop;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Dopat fcDop. This value MUST NOT be zero.
	unsigned int fcSttbfAssoc;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An SttbfAssoc that contains strings that are associated with the document begins at this offset.
	unsigned int lcbSttbfAssoc;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfAssoc at offset fcSttbfAssoc. This value MUST NOT be zero.
	unsigned int fcClx;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Clx begins at this offset.
	unsigned int lcbClx;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Clx at offset fcClx in the Table Stream. This value MUST be greater than zero.
	unsigned int fcPlcfPgdFtn;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbPlcfPgdFtn;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcAutosaveSource;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbAutosaveSource;//(4 bytes): This value MUST be zero and MUST be ignored.
	unsigned int fcGrpXstAtnOwners;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An array of XSTs begins at this offset. The value of cch for all XSTs in this array MUST be less than 56. The number of entries in this array is limited to 0x7FFF. This array contains the names of authors of comments in the document. The names in this array MUST be unique. If no comments are defined, lcbGrpXstAtnOwners and fcGrpXstAtnOwners MUST be zero and MUST be ignored. If any comments are in the document, fcGrpXstAtnOwners MUST point to a valid array of XSTs.
	unsigned int lcbGrpXstAtnOwners;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the XST array at offset fcGrpXstAtnOwners in the Table Stream.
	unsigned int fcSttbfAtnBkmk;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An SttbfAtnBkmk that contains information about the annotation bookmarks in the document begins at this offset. If lcbSttbfAtnBkmk is zero, fcSttbfAtnBkmk is undefined and MUST be ignored.  The SttbfAtnBkmk is parallel to the PlcfBkf at offset fcPlcfAtnBkf in the Table Stream. Each element in the SttbfAtnBkmk specifies information about the bookmark which is associated with the data element that is located at the same offset in that PlcfBkf, so the SttbfAtnBkmk beginning at offset fcSttbfAtnBkmk and the PlcfBkf beginning at offset fcPlcfAtnBkf MUST contain the same number of elements. An additional constraint upon the number of elements in the SttbfAtnBkmk is specified in the description of fcPlcfAtnBkf.
	unsigned int lcbSttbfAtnBkmk;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfAtnBkmk at offset fcSttbfAtnBkmk.
	unsigned int fcUnused2;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbUnused2;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcUnused3;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbUnused3;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcPlcSpaMom;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfSpa begins at this offset. The PlcfSpa contains shape information for the Main Document. All CPs in this PlcfSpa are relative to the starting position of the Main Document and MUST be greater than or equal to zero and less than or equal to cppText in FibRgLw97. The final CP is undefined and MUST be ignored, though it MUST be greater than the previous entry. If there are no shapes in the Main Document, lcbPlcSpaMom and fcPlcSpaMom MUST be zero and MUST be ignored. If there are shapes in the Main Document, fcPlcSpaMom MUST point to a valid PlcfSpa structure.
	unsigned int lcbPlcSpaMom;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfSpa at offset fcPlcSpaMom.
	unsigned int fcPlcSpaHdr;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfSpa begins at this offset. The PlcfSpa contains shape information for the Header Document. All CPs in this PlcfSpa are relative to the starting position of the Header Document and MUST be greater than or equal to zero and less than or equal to ccpHdd in FibRgLw97. The final CP is undefined and MUST be ignored, though this value MUST be greater than the previous entry. If there are no shapes in the Header Document, lcbPlcSpaHdr and fcPlcSpaHdr MUST both be zero and MUST be ignored. If there are shapes in the Header Document, fcPlcSpaHdr MUST point to a valid PlcfSpa structure.
	unsigned int lcbPlcSpaHdr;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfSpa at the offset fcPlcSpaHdr.
	unsigned int fcPlcfAtnBkf;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfBkf that contains information about annotation bookmarks in the document begins at this offset. If lcbPlcfAtnBkf is zero, fcPlcfAtnBkf is undefined and MUST be ignored.  Each data element in the PlcfBkf is associated, in a one-to-one correlation, with a data element in the PlcfBkl at offset fcPlcfAtnBkl. For this reason, the PlcfBkf that begins at offset fcPlcfAtnBkf, and the PlcfBkl that begins at offset fcPlcfAtnBkl, MUST contain the same number of data elements. The PlcfBkf is parallel to the SttbfAtnBkmk at offset fcSttbfAtnBkmk in the Table Stream. Each data element in the PlcfBkf specifies information about the bookmark which is associated with the element that is located at the same offset in that SttbfAtnBkmk. For this reason, the PlcfBkf that begins at offset fcPlcfAtnBkf, and the SttbfAtnBkmk that begins at offset fcSttbfAtnBkmk, MUST contain the same number of elements.  The CP range of an annotation bookmark MUST be in the Main Document part.
	unsigned int lcbPlcfAtnBkf;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfBkf at offset fcPlcfAtnBkf.
	unsigned int fcPlcfAtnBkl;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfBkl that contains information about annotation bookmarks in the document begins at this offset. If lcbPlcfAtnBkl is zero, then fcPlcfAtnBkl is undefined and MUST be ignored.  Each data element in the PlcfBkl is associated, in a one-to-one correlation, with a data element in the PlcfBkf at offset fcPlcfAtnBkf. For this reason, the PlcfBkl that begins at offset fcPlcfAtnBkl, and the PlcfBkf that begins at offset fcPlcfAtnBkf, MUST contain the same number of data elements.  The CP range of an annotation bookmark MUST be in the Main Document part.
	unsigned int lcbPlcfAtnBkl;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfBkl at offset fcPlcfAtnBkl.
	unsigned int fcPms;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Pms, which contains the current state of a print merge operation, begins at this offset. If lcbPms is zero, fcPms is undefined and MUST be ignored.
	unsigned int lcbPms;//(4 bytes): An unsigned integer which specifies the size, in bytes, of the Pms at offset fcPms.
	unsigned int fcFormFldSttbs;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbFormFldSttbs;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcPlcfendRef;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfendRef that begins at this offset specifies the locations of endnote references in the Main Document and whether those references use auto-numbering or custom symbols. If lcbPlcfendRef is zero, fcPlcfendRef is undefined and MUST be ignored.
	unsigned int lcbPlcfendRef;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfendRef that begins at offset fcPlcfendRef in the Table Stream.
	unsigned int fcPlcfendTxt;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfendTxt begins at this offset and specifies the locations of each block of endnote text in the Endnote Document. If lcbPlcfendTxt is zero, fcPlcfendTxt is undefined and MUST be ignored.
	unsigned int lcbPlcfendTxt;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfendTxt that begins at offset fcPlcfendTxt in the Table Stream.  lcbPlcfendTxt MUST be zero if FibRgLw97.ccpEdn is zero, and MUST be nonzero if FibRgLw97.ccpEdn is nonzero.
	unsigned int fcPlcfFldEdn;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcFld begins at this offset and specifies the locations of field characters in the Endnote Document. All CPs in this PlcFld are relative to the starting position of the Endnote Document. All CPs in this PlcFld MUST be greater than or equal to zero and less than or equal to FibRgLw97.ccpEdn. If lcbPlcfFldEdn is zero, fcPlcfFldEdn is undefined and MUST be ignored.
	unsigned int lcbPlcfFldEdn;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcFld at offset fcPlcfFldEdn in the Table Stream.
	unsigned int fcUnused4;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbUnused4;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcDggInfo;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An OfficeArtContent that contains information about the drawings in the document begins at this offset.
	unsigned int lcbDggInfo;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the OfficeArtContent at the offset fcDggInfo. If lcbDggInfo is zero, there MUST NOT be any drawings in the document.
	unsigned int fcSttbfRMark;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An SttbfRMark that contains the names of authors who have added revision marks or comments to the document begins at this offset. If lcbSttbfRMark is zero, fcSttbfRMark is undefined and MUST be ignored.
	unsigned int lcbSttbfRMark;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfRMark at the offset fcSttbfRMark.
	unsigned int fcSttbfCaption;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An SttbfCaption that contains information about the captions that are defined in this document begins at this offset. If lcbSttbfCaption is zero, fcSttbfCaption is undefined and MUST be ignored. If this document is not the Normal template, this value MUST be ignored.
	unsigned int lcbSttbfCaption;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfCaption at offset fcSttbfCaption in the Table Stream. If base.fDot of the Fib that contains this FibRgFcLcb97 is zero, this value MUST be zero.
	unsigned int fcSttbfAutoCaption;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A SttbfAutoCaption that contains information about the AutoCaption strings defined in this			  document begins at this offset. If lcbSttbfAutoCaption is zero, fcSttbfAutoCaption is undefined and MUST be ignored. If this document is not the Normal template, this value MUST be ignored.
	unsigned int lcbSttbfAutoCaption;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfAutoCaption at offset fcSttbfAutoCaption in the Table Stream. If base.fDot of the Fib that contains this FibRgFcLcb97 is zero, this MUST be zero.
	unsigned int fcPlcfWkb;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfWKB that contains information about all master documents and subdocuments begins at this offset.
	unsigned int lcbPlcfWkb;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfWKB at offset fcPlcfWkb in the Table Stream. If lcbPlcfWkb is zero, fcPlcfWkb is undefined and MUST be ignored.
	unsigned int fcPlcfSpl;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Plcfspl, which specifies the state of the spell checker for each text range, begins at this offset. If lcbPlcfSpl is zero, then fcPlcfSpl is undefined and MUST be ignored.
	unsigned int lcbPlcfSpl;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Plcfspl that begins at offset fcPlcfSpl in the Table Stream.
	unsigned int fcPlcftxbxTxt;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcftxbxTxt begins at this offset and specifies which ranges of text are contained in which textboxes. If lcbPlcftxbxTxt is zero, fcPlcftxbxTxt is undefined and MUST be ignored.
	unsigned int lcbPlcftxbxTxt;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcftxbxTxt that begins at offset fcPlcftxbxTxt in the Table Stream.  lcbPlcftxbxTxt MUST be zero if FibRgLw97.ccpTxbx is zero, and MUST be nonzero if FibRgLw97.ccpTxbx is nonzero.
	unsigned int fcPlcfFldTxbx;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcFld begins at this offset and specifies the locations of field characters in the Textbox Document. All CPs in this PlcFld are relative to the starting position of the Textbox Document. All CPs in this PlcFld MUST be greater than or equal to zero and less than or equal to FibRgLw97.ccpTxbx. If lcbPlcfFldTxbx is zero, fcPlcfFldTxbx is undefined and MUST be ignored.
	unsigned int lcbPlcfFldTxbx;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcFld at offset fcPlcfFldTxbx in the Table Stream.
	unsigned int fcPlcfHdrtxbxTxt;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfHdrtxbxTxt begins at this offset and specifies which ranges of text are contained in which header textboxes.
	unsigned int lcbPlcfHdrtxbxTxt;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfHdrtxbxTxt that begins at offset fcPlcfHdrtxbxTxt in the Table Stream.  lcbPlcfHdrtxbxTxt MUST be zero if FibRgLw97.ccpHdrTxbx is zero, and MUST be nonzero if FibRgLw97.ccpHdrTxbx is nonzero.
	unsigned int fcPlcffldHdrTxbx;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcFld begins at this offset and specifies the locations of field characters in the Header Textbox Document. All CPs in this PlcFld are relative to the starting position of the Header Textbox Document. All CPs in this PlcFld MUST be greater than or equal to zero and less than or equal to FibRgLw97.ccpHdrTxbx. If lcbPlcffldHdrTxbx is zero, fcPlcffldHdrTxbx is undefined, and MUST be ignored.
	unsigned int lcbPlcffldHdrTxbx;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcFld at offset fcPlcffldHdrTxbx in the Table Stream.
	unsigned int fcStwUser;//(4 bytes): An unsigned integer that specifies an offset into the Table Stream. An StwUser that specifies the user-defined variables and VBA digital signature, as specified by [MS- OSHARED] section 2.3.2, begins at this offset. If lcbStwUser is zero, fcStwUser is undefined and MUST be ignored.
	unsigned int lcbStwUser;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the StwUser at offset fcStwUser.
	unsigned int fcSttbTtmbd;//(4 bytes): An unsigned integer that specifies an offset into the Table Stream. A SttbTtmbd begins at this offset and specifies information about the TrueType fonts that are embedded in the document. If lcbSttbTtmbd is zero, fcSttbTtmbd is undefined and MUST be ignored.
	unsigned int lcbSttbTtmbd;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbTtmbd at offset fcSttbTtmbd.
	unsigned int fcCookieData;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An RgCdb begins at this offset. If lcbCookieData is zero, fcCookieData is undefined and MUST be ignored.  Otherwise, fcCookieData MAY<29> be ignored.
	unsigned int lcbCookieData;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the RgCdb at offset fcCookieData in the Table Stream.
	unsigned int fcPgdMotherOldOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated document page layout cache begins at this offset. Information SHOULD NOT<30> be emitted at this offset and SHOULD<31> be ignored. If lcbPgdMotherOldOld is zero, fcPgdMotherOldOld is undefined and MUST be ignored.
	unsigned int lcbPgdMotherOldOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated document page layout cache at offset fcPgdMotherOldOld in the Table Stream.
	unsigned int fcBkdMotherOldOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream.  Deprecated document text flow break cache begins at this offset. Information SHOULD NOT<32> be emitted at this offset and SHOULD<33> be ignored. If lcbBkdMotherOldOld is zero, fcBkdMotherOldOld is undefined and MUST be ignored.
	unsigned int lcbBkdMotherOldOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated document text flow break cache at offset fcBkdMotherOldOld in the Table Stream.
	unsigned int fcPgdFtnOldOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream.  Deprecated footnote layout cache begins at this offset. Information SHOULD NOT<34> be emitted at this offset and SHOULD<35> be ignored. If lcbPgdFtnOldOld is zero, fcPgdFtnOldOld is undefined and MUST be ignored.
	unsigned int lcbPgdFtnOldOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated footnote layout cache at offset fcPgdFtnOldOld in the Table Stream.
	unsigned int fcBkdFtnOldOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated footnote text flow break cache begins at this offset. Information SHOULD NOT<36> be emitted at this offset and SHOULD<37> be ignored. If lcbBkdFtnOldOld is zero, fcBkdFtnOldOld is undefined and MUST be ignored.
	unsigned int lcbBkdFtnOldOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated footnote text flow break cache at offset fcBkdFtnOldOld in the Table Stream.
	unsigned int fcPgdEdnOldOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated endnote layout cache begins at this offset. Information SHOULD NOT<38> be emitted at this offset and SHOULD<39> be ignored. If lcbPgdEdnOldOld is zero, fcPgdEdnOldOld is undefined and MUST be ignored.						 
	unsigned int lcbPgdEdnOldOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated endnote layout cache at offset fcPgdEdnOldOld in the Table Stream.
	unsigned int fcBkdEdnOldOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated endnote text flow break cache begins at this offset. Information SHOULD NOT<40> be emitted at this offset and SHOULD<41> be ignored. If lcbBkdEdnOldOld is zero, fcBkdEdnOldOld is undefined and MUST be ignored.
	unsigned int lcbBkdEdnOldOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated endnote text flow break cache at offset fcBkdEdnOldOld in the Table Stream.
	unsigned int fcSttbfIntlFld;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbSttbfIntlFld;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcRouteSlip;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A RouteSlip that specifies the route slip for this document begins at this offset. This value SHOULD<42> be ignored.
	unsigned int lcbRouteSlip;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the RouteSlip at offset fcRouteSlip in the Table Stream.
	unsigned int fcSttbSavedBy;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A SttbSavedBy that specifies the save history of this document begins at this offset. This value SHOULD<43> be ignored.
	unsigned int lcbSttbSavedBy;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbSavedBy at the offset fcSttbSavedBy. This value SHOULD<44> be zero.
	unsigned int fcSttbFnm;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An SttbFnm that contains information about the external files that are referenced by this document begins at this offset. If lcbSttbFnm is zero, fcSttbFnm is undefined and MUST be ignored.
	unsigned int lcbSttbFnm;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbFnm at the offset fcSttbFnm.
	unsigned int fcPlfLst;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlfLst that contains list formatting information begins at this offset. An array of LVLs is appended to the PlfLst. lcbPlfLst does not account for the array of LVLs. The size of the array of LVLs is specified by the LSTFs in PlfLst. For each LSTF whose fSimpleList is set to 0x1, there is one LVL in the array of LVLs that specifies the level formatting of the single level in the list which corresponds to the LSTF. And, for each LSTF whose fSimpleList is set to 0x0, there are 9 LVLs in the array of LVLs that specify the level formatting of the respective levels in the list which corresponds to the LSTF. This array of LVLs is in the same respective order as the LSTFs in PlfLst. If lcbPlfLst is 0, fcPlfLst is undefined and MUST be ignored.
	unsigned int lcbPlfLst;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlfLst at the offset fcPlfLst. This does not include the size of the array of LVLs that are appended to the PlfLst.
	unsigned int fcPlfLfo;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlfLfo that contains list formatting override information begins at this offset. If lcbPlfLfo is zero, fcPlfLfo is undefined and MUST be ignored.
	unsigned int lcbPlfLfo;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlfLfo at the offset fcPlfLfo.
	unsigned int fcPlcfTxbxBkd;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcftxbxBkd begins at this offset and specifies which ranges of text go inside which textboxes.
	unsigned int lcbPlcfTxbxBkd;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcftxbxBkd that begins at offset fcPlcfTxbxBkd in the Table Stream.					  lcbPlcfTxbxBkd MUST be zero if FibRgLw97.ccpTxbx is zero, and MUST be nonzero if FibRgLw97.ccpTxbx is nonzero.
	unsigned int fcPlcfTxbxHdrBkd;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfTxbxHdrBkd begins at this offset and specifies which ranges of text are contained inside which header textboxes.
	unsigned int lcbPlcfTxbxHdrBkd;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfTxbxHdrBkd that begins at offset fcPlcfTxbxHdrBkd in the Table Stream.  lcbPlcfTxbxHdrBkd MUST be zero if FibRgLw97.ccpHdrTxbx is zero, and MUST be nonzero if FibRgLw97.ccpHdrTxbx is nonzero.
	unsigned int fcDocUndoWord9;//(4 bytes): An unsigned integer that specifies an offset in the WordDocument Stream. Version-specific undo information begins at this offset. This information SHOULD NOT<45> be emitted and SHOULD<46> be ignored.
	unsigned int lcbDocUndoWord9;//(4 bytes): An unsigned integer. If this is nonzero, version-specific undo information exists at offset fcDocUndoWord9 in the WordDocument Stream.
	unsigned int fcRgbUse;//(4 bytes): An unsigned integer that specifies an offset in the WordDocument Stream.  Version-specific undo information begins at this offset. This information SHOULD NOT<47> be emitted and SHOULD<48> be ignored.
	unsigned int lcbRgbUse;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the version-specific undo information at offset fcRgbUse in the WordDocument Stream.
	unsigned int fcUsp;//(4 bytes): An unsigned integer that specifies an offset in the WordDocument Stream. Version- specific undo information begins at this offset. This information SHOULD NOT<49> be emitted and SHOULD<50> be ignored.
	unsigned int lcbUsp;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the version-specific undo information at offset fcUsp in the WordDocument Stream.
	unsigned int fcUskf;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. Version-specific undo information begins at this offset. This information SHOULD NOT<51> be emitted and SHOULD<52> be ignored.
	unsigned int lcbUskf;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the version-specific undo information at offset fcUskf in the Table Stream.
	unsigned int fcPlcupcRgbUse;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Plc begins at this offset and contains version-specific undo information. This information SHOULD NOT<53> be emitted and SHOULD<54> be ignored.
	unsigned int lcbPlcupcRgbUse;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Plc at offset fcPlcupcRgbUse in the Table Stream.
	unsigned int fcPlcupcUsp;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Plc begins at this offset and contains version-specific undo information. This information SHOULD NOT<55> be emitted and SHOULD<56> be ignored.
	unsigned int lcbPlcupcUsp;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Plc at offset fcPlcupcUsp in the Table Stream.
	unsigned int fcSttbGlsyStyle;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A SttbGlsyStyle, which contains information about the styles that are used by the AutoText items which are defined in this document, begins at this offset.					  
	unsigned int lcbSttbGlsyStyle;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbGlsyStyle at offset fcSttbGlsyStyle in the Table Stream. If base.fGlsy of the Fib that contains this FibRgFcLcb97 is zero, this value MUST be zero.
	unsigned int fcPlgosl;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlfGosl begins at the offset. If lcbPlgosl is zero, fcPlgosl is undefined and MUST be ignored.
	unsigned int lcbPlgosl;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlfGosl at offset fcPlgosl in the Table Stream.
	unsigned int fcPlcocx;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A RgxOcxInfo that specifies information about the OLE controls in the document begins at this offset. When there are no OLE controls in the document, fcPlcocx and lcbPlcocx MUST be zero and MUST be ignored. If there are any OLE controls in the document, fcPlcocx MUST point to a valid RgxOcxInfo.
	unsigned int lcbPlcocx;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the RgxOcxInfo at the offset fcPlcocx.
	unsigned int fcPlcfBteLvc;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A deprecated numbering field cache begins at this offset. This information SHOULD NOT<57> be emitted and SHOULD<58> ignored. If lcbPlcBteLvc is zero, fcPlcfBteLvc is undefined and MUST be ignored.
	unsigned int lcbPlcfBteLvc;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated numbering field cache at offset fcPlcfBteLvc in the Table Stream. This value SHOULD<59> be zero.
	unsigned int dwLowDateTime;//(4 bytes): The low-order part of a FILETIME structure, as specified by [MS- DTYP], that specifies when the document was last saved.
	unsigned int dwHighDateTime;//(4 bytes): The high-order part of a FILETIME structure, as specified by [MS- DTYP], that specifies when the document was last saved.
	unsigned int fcPlcfLvcPre10;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated list level cache begins at this offset. Information SHOULD NOT<60> be emitted at this offset and SHOULD<61> be ignored. If lcbPlcfLvcPre10 is zero, fcPlcfLvcPre10 is undefined and MUST be ignored.
	unsigned int lcbPlcfLvcPre10;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated list level cache at offset fcPlcfLvcPre10 in the Table Stream. This value SHOULD<62> be zero.
	unsigned int fcPlcfAsumy;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfAsumy begins at the offset. If lcbPlcfAsumy is zero, fcPlcfAsumy is undefined and MUST be ignored.
	unsigned int lcbPlcfAsumy;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfAsumy at offset fcPlcfAsumy in the Table Stream.
	unsigned int fcPlcfGram;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Plcfgram, which specifies the state of the grammar checker for each text range, begins at this offset. If lcbPlcfGram is zero, then fcPlcfGram is undefined and MUST be ignored.
	unsigned int lcbPlcfGram;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Plcfgram that begins at offset fcPlcfGram in the Table Stream.
	unsigned int fcSttbListNames;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A SttbListNames, which specifies the LISTNUM field names of the lists in the document, begins at this offset. If lcbSttbListNames is zero, fcSttbListNames is undefined and MUST be ignored.				  
	unsigned int lcbSttbListNames;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbListNames at the offset fcSttbListNames.
	unsigned int fcSttbfUssr;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated, version-specific undo information begins at this offset. This information SHOULD NOT<63> be emitted and SHOULD<64> be ignored.
	unsigned int lcbSttbfUssr;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated, version-specific undo information at offset fcSttbfUssr in the Table Stream.					   
}FibRgFcLcb97;

typedef struct FibRgFcLcb2000
{
	//FibRgFcLcb97 *rgFcLcb97;//rgFcLcb97 (744 bytes)
	unsigned int fcPlcfTch;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfTch begins at this offset and specifies a cache of table characters. Information at this offset SHOULD<65> be ignored. If lcbPlcfTch is zero, fcPlcfTch is undefined and MUST be ignored.
	unsigned int lcbPlcfTch;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfTch at offset fcPlcfTch.
	unsigned int fcRmdThreading;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An RmdThreading that specifies the data concerning the e-mail messages and their authors in this document begins at this offset.
	unsigned int lcbRmdThreading;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the RmdThreading at the offset fcRmdThreading. This value MUST NOT be zero.
	unsigned int fcMid;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A double-byte character Unicode string that specifies the message identifier of the document begins at this offset. This value MUST be ignored.
	unsigned int lcbMid;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the double-byte character Unicode string at offset fcMid. This value MUST be ignored.
	unsigned int fcSttbRgtplc;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A SttbRgtplc that specifies the styles of lists in the document begins at this offset. If lcbSttbRgtplc is zero, fcSttbRgtplc is undefined and MUST be ignored.
	unsigned int lcbSttbRgtplc;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbRgtplc at the offset fcSttbRgtplc.
	unsigned int fcMsoEnvelope;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An MsoEnvelopeCLSID, which specifies the envelope data as specified by [MS-OSHARED] section 2.3.8.1, begins at this offset. If lcbMsoEnvelope is zero, fcMsoEnvelope is undefined and MUST be ignored.
	unsigned int lcbMsoEnvelope;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the MsoEnvelopeCLSID at the offset fcMsoEnvelope.
	unsigned int fcPlcfLad;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Plcflad begins at this offset and specifies the language auto-detect state of each text range. If lcbPlcfLad is zero, fcPlcfLad is undefined and MUST be ignored.
	unsigned int lcbPlcfLad;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Plcflad that begins at offset fcPlcfLad in the Table Stream.  unsigned int fcRgDofr;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A variable- length array with elements of type Dofrh begins at that offset. The elements of this array are records that support the frame set and list style features. If lcbRgDofr is zero, fcRgDofr is undefined and MUST be ignored.
	unsigned int lcbRgDofr;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the array that begins at offset fcRgDofr in the Table Stream.
	unsigned int fcPlcosl;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlfCosl begins at the offset. If lcbPlcosl is zero, fcPlcosl is undefined and MUST be ignored.
	unsigned int lcbPlcosl;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlfCosl at offset fcPlcosl in the Table Stream.
	unsigned int fcPlcfCookieOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfcookieOld begins at this offset. If lcbPlcfcookieOld is zero, fcPlcfcookieOld is undefined and MUST be ignored. fcPlcfcookieOld MAY<66> be ignored.
	unsigned int lcbPlcfCookieOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfcookieOld at offset fcPlcfcookieOld in the Table Stream.
	unsigned int fcPgdMotherOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated document page layout cache begins at this offset. Information SHOULD NOT <67> be emitted at this offset and SHOULD <68> be ignored. If lcbPgdMotherOld is zero, fcPgdMotherOld is undefined and MUST be ignored.
	unsigned int lcbPgdMotherOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated document page layout cache at offset fcPgdMotherOld in the Table Stream.
	unsigned int fcBkdMotherOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated document text flow break cache begins at this offset. Information SHOULD NOT <69> be emitted at this offset and SHOULD <70> be ignored. If lcbBkdMotherOld is zero, fcBkdMotherOld is undefined and MUST be ignored.
	unsigned int lcbBkdMotherOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated document text flow break cache at offset fcBkdMotherOld in the Table Stream.
	unsigned int fcPgdFtnOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated footnote layout cache begins at this offset. Information SHOULD NOT <71> be emitted at this offset and SHOULD <72> be ignored. If lcbPgdFtnOld is zero, fcPgdFtnOld is undefined and MUST be ignored.
	unsigned int lcbPgdFtnOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated footnote layout cache at offset fcPgdFtnOld in the Table Stream.
	unsigned int fcBkdFtnOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated footnote text flow break cache begins at this offset. Information SHOULD NOT <73> be emitted at this offset and SHOULD <74> be ignored. If lcbBkdFtnOld is zero, fcBkdFtnOld is undefined and MUST be ignored.
	unsigned int lcbBkdFtnOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated footnote text flow break cache at offset fcBkdFtnOld in the Table Stream.
	unsigned int fcPgdEdnOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated endnote layout cache begins at this offset. Information SHOULD NOT <75> be emitted at this offset and SHOULD <76> be ignored. If lcbPgdEdnOld is zero, fcPgdEdnOld is undefined and MUST be ignored.
	unsigned int lcbPgdEdnOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated endnote layout cache at offset fcPgdEdnOld in the Table Stream.
	unsigned int fcBkdEdnOld;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated endnote text flow break cache begins at this offset. Information SHOULD NOT <77> be emitted at this offset and SHOULD <78> be ignored. If lcbBkdEdnOld is zero, fcBkdEdnOld is undefined and MUST be ignored.
	unsigned int lcbBkdEdnOld;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated endnote text flow break cache at offset fcBkdEdnOld in the Table Stream.				   

}FibRgFcLcb2000;
typedef struct FibRgFcLcb2002
{
	//FibRgFcLcb97 *rgFcLcb97;//rgFcLcb97 (744 bytes)
	//FibRgFcLcb2000 * rgFcLcb2000;//rgFcLcb2000 (864 bytes)
	unsigned int fcUnused1;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbUnused1;//(4 bytes): This value MUST be zero, and MUST be ignored
	unsigned int fcPlcfPgp;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PGPArray begins at this offset. If lcbPlcfPgp is 0, fcPlcfPgp is undefined and MUST be ignored.
	unsigned int lcbPlcfPgp;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PGPArray that is stored at offset fcPlcfPgp.
	unsigned int fcPlcfuim;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Plcfuim begins at this offset. If lcbPlcfuim is zero, fcPlcfuim is undefined and MUST be ignored.
	unsigned int lcbPlcfuim;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Plcfuim at offset fcPlcfuim.
	unsigned int fcPlfguidUim;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlfguidUim begins at this offset. If lcbPlfguidUim is zero, fcPlfguidUim is undefined and MUST be ignored.
	unsigned int lcbPlfguidUim;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlfguidUim at offset fcPlfguidUim.
	unsigned int fcAtrdExtra;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An AtrdExtra begins at this offset. If lcbAtrdExtra is zero, fcAtrdExtra is undefined and MUST be ignored.
	unsigned int lcbAtrdExtra;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the AtrdExtra at offset fcAtrdExtra in the Table Stream.
	unsigned int fcPlrsid;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PLRSID begins at this offset. If lcbPlrsid is zero, fcPlrsid is undefined and MUST be ignored.
	unsigned int lcbPlrsid;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PLRSID at offset fcPlrsid in the Table Stream.
	unsigned int fcSttbfBkmkFactoid;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An SttbfBkmkFactoid containing information about smart tag bookmarks in the document begins at this offset. If lcbSttbfBkmkFactoid is zero, fcSttbfBkmkFactoid is undefined and MUST be ignored.  The SttbfBkmkFactoid is parallel to the PlcfBkfd at offset fcPlcfBkfFactoid in the Table Stream. Each element in the SttbfBkmkFactoid specifies information about the bookmark that is associated with the data element which is located at the same offset in that PlcfBkfd. For this reason, the SttbfBkmkFactoid that begins at offset fcSttbfBkmkFactoid, and the PlcfBkfd that begins at offset fcPlcfBkfFactoid, MUST contain the same number of elements.
	unsigned int lcbSttbfBkmkFactoid;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfBkmkFactoid at offset fcSttbfBkmkFactoid.
	unsigned int fcPlcfBkfFactoid;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfBkfd that contains information about the smart tag bookmarks in the document begins at this offset. If lcbPlcfBkfFactoid is zero, fcPlcfBkfFactoid is undefined and MUST be ignored.  Each data element in the PlcfBkfd is associated, in a one-to-one correlation, with a data element in the Plcfbkld at offset fcPlcfBklFactoid. For this reason, the PlcfBkfd that begins at offset fcPlcfBkfFactoid, and the Plcfbkld that begins at offset fcPlcfBklFactoid, MUST contain the same number of data elements. The PlcfBkfd is parallel to the SttbfBkmkFactoid at offset fcSttbfBkmkFactoid in the Table Stream. Each data element in the PlcfBkfd specifies information about the bookmark that is associated with the element which is located at the same offset in that SttbfBkmkFactoid. For this reason, the PlcfBkfd that begins at offset fcPlcfBkfFactoid, and the SttbfBkmkFactoid that begins at offset fcSttbfBkmkFactoid, MUST contain the same number of elements.
	unsigned int lcbPlcfBkfFactoid;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfBkfd at offset fcPlcfBkfFactoid.
	unsigned int fcPlcfcookie;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Plcfcookie begins at this offset. If lcbPlcfcookie is zero, fcPlcfcookie is undefined and MUST be ignored. fcPlcfcookie MAY<79> be ignored.
	unsigned int lcbPlcfcookie;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Plcfcookie at offset fcPlcfcookie in the Table Stream.
	unsigned int fcPlcfBklFactoid;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Plcfbkld that contains information about the smart tag bookmarks in the document begins at this offset. If lcbPlcfBklFactoid is zero, fcPlcfBklFactoid is undefined and MUST be ignored.  Each data element in the Plcfbkld is associated, in a one-to-one correlation, with a data element in the PlcfBkfd at offset fcPlcfBkfFactoid. For this reason, the Plcfbkld that begins at offset fcPlcfBklFactoid, and the PlcfBkfd that begins at offset fcPlcfBkfFactoid, MUST contain the same number of data elements.
	unsigned int lcbPlcfBklFactoid;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Plcfbkld at offset fcPlcfBklFactoid.
	unsigned int fcFactoidData;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A SmartTagData begins at this offset and specifies information about the smart tag recognizers that are used in this document. If lcbFactoidData is zero, fcFactoidData is undefined and MUST be ignored.
	unsigned int lcbFactoidData;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SmartTagData at offset fcFactoidData in the Table Stream.
	unsigned int fcDocUndo;//(4 bytes): An unsigned integer that specifies an offset in the WordDocument Stream.  Version-specific undo information begins at this offset. This information SHOULD NOT<80> be emitted and SHOULD<81> be ignored.
	unsigned int lcbDocUndo;//(4 bytes): An unsigned integer. If this value is nonzero, version-specific undo information exists at offset fcDocUndo in the WordDocument Stream.
	unsigned int fcSttbfBkmkFcc;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. An SttbfBkmkFcc that contains information about the format consistency-checker bookmarks in the document begins at this offset. If lcbSttbfBkmkFcc is zero, fcSttbfBkmkFcc is undefined and MUST be ignored.  The SttbfBkmkFcc is parallel to the PlcfBkfd at offset fcPlcfBkfFcc in the Table Stream. Each element in the SttbfBkmkFcc specifies information about the bookmark that is associated with the data element which is located at the same offset in that PlcfBkfd. For this reason, the SttbfBkmkFcc that begins at offset fcSttbfBkmkFcc, and the PlcfBkfd that begins at offset fcPlcfBkfFcc, MUST contain the same number of elements.
	unsigned int lcbSttbfBkmkFcc;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfBkmkFcc at offset fcSttbfBkmkFcc.
	unsigned int fcPlcfBkfFcc;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfBkfd that contains information about format consistency-checker bookmarks in the document begins at this offset. If lcbPlcfBkfFcc is zero, fcPlcfBkfFcc is undefined and MUST be ignored.Each data element in the PlcfBkfd is associated, in a one-to-one correlation, with a data element in the PlcfBkld at offset fcPlcfBklFcc. For this reason, the PlcfBkfd that begins at offset fcPlcfBkfFcc and the PlcfBkld that begins at offset fcPlcfBklFcc MUST contain the same number of data elements. The PlcfBkfd is parallel to the SttbfBkmkFcc at offset fcSttbfBkmkFcc in the Table Stream. Each data element in the PlcfBkfd specifies information about the bookmark that is associated with the element which is located at the same offset in that SttbfBkmkFcc. For this reason, the PlcfBkfd that begins at offset fcPlcfBkfFcc and the SttbfBkmkFcc that begins at offset fcSttbfBkmkFcc MUST contain the same number of elements.
	unsigned int lcbPlcfBkfFcc;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfBkfd at offset fcPlcfBkfFcc.
	unsigned int fcPlcfBklFcc;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfBkld that contains information about the format consistency-checker bookmarks in the document begins at this offset. If lcbPlcfBklFcc is zero, fcPlcfBklFcc is undefined and MUST be ignored.  Each data element in the PlcfBkld is associated, in a one-to-one correlation, with a data element in the PlcfBkfd at offset fcPlcfBkfFcc. For this reason, the PlcfBkld that begins at offset fcPlcfBklFcc, and the PlcfBkfd that begins at offset fcPlcfBkfFcc, MUST contain the same number of data elements.
	unsigned int lcbPlcfBklFcc;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfBkld at offset fcPlcfBklFcc.
	unsigned int fcSttbfbkmkBPRepairs;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream.  An SttbfBkmkBPRepairs that contains information about the repair bookmarks in the document begins at this offset. If lcbSttbfBkmkBPRepairs is zero, fcSttbfBkmkBPRepairs is undefined and MUST be ignored.  The SttbfBkmkBPRepairs is parallel to the PlcfBkf at offset fcPlcfBkfBPRepairs in the Table Stream. Each element in the SttbfBkmkBPRepairs specifies information about the bookmark that is associated with the data element which is located at the same offset in that PlcfBkf. For this reason, the SttbfBkmkBPRepairs that begins at offset fcSttbfBkmkBPRepairs, and the PlcfBkf that begins at offset fcPlcfBkfBPRepairs, MUST contain the same number of elements.
	unsigned int lcbSttbfbkmkBPRepairs;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfBkmkBPRepairs at offset fcSttbfBkmkBPRepairs.
	unsigned int fcPlcfbkfBPRepairs;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfBkf that contains information about the repair bookmarks in the document begins at this offset. If lcbPlcfBkfBPRepairs is zero, fcPlcfBkfBPRepairs is undefined and MUST be ignored.  Each data element in the PlcfBkf is associated, in a one-to-one correlation, with a data element in the PlcfBkl at offset fcPlcfBklBPRepairs. For this reason, the PlcfBkf that begins at offset fcPlcfBkfBPRepairs, and the PlcfBkl that begins at offset fcPlcfBklBPRepairs, MUST contain the same number of data elements. The PlcfBkf is parallel to the SttbfBkmkBPRepairs at offset fcSttbfBkmkBPRepairs in the Table Stream. Each data element in the PlcfBkf specifies information about the bookmark that is associated with the element which is located at the same offset in that SttbfBkmkBPRepairs. For this reason, the PlcfBkf that begins at offset fcPlcfbkfBPRepairs, and the SttbfBkmkBPRepairs that begins at offset fcSttbfBkmkBPRepairs, MUST contain the same number of elements.  The CPs in this PlcfBkf MUST NOT exceed the CP that represents the end of the Main Document part.
	unsigned int lcbPlcfbkfBPRepairs;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfBkf at offset fcPlcfbkfBPRepairs.			
	unsigned int fcPlcfbklBPRepairs;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcfBkl that contains information about the repair bookmarks in the document begins at this offset. If lcbPlcfBklBPRepairs is zero, fcPlcfBklBPRepairs is undefined and MUST be ignored.  Each data element in the PlcfBkl is associated, in a one-to-one correlation, with a data element in the PlcfBkf at offset fcPlcfBkfBPRepairs. For this reason, the PlcfBkl that begins at offset fcPlcfBklBPRepairs, and the PlcfBkf that begins at offset fcPlcfBkfBPRepairs, MUST contain the same number of data elements.  The CPs that are contained in this PlcfBkl MUST NOT exceed the CP that represents the end of the Main Document part.
	unsigned int lcbPlcfbklBPRepairs;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcfBkl at offset fcPlcfBklBPRepairs.
	unsigned int fcPmsNew;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A new Pms, which contains the current state of a print merge operation, begins at this offset. If lcbPmsNew is zero, fcPmsNew is undefined and MUST be ignored.
	unsigned int lcbPmsNew;//(4 bytes): An unsigned integer which specifies the size, in bytes, of the Pms at offset fcPmsNew.
	unsigned int fcODSO;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. Office Data Source Object (ODSO) data that is used to perform mail merge begins at this offset. The data is stored in an array of ODSOPropertyBase items. The ODSOPropertyBase items are of variable size and are stored contiguously. The complete set of properties that are contained in the array is determined by reading each ODSOPropertyBase, until a total of lcbODSO bytes of data are read. If lcbODSO is zero, fcODSO is undefined and MUST be ignored.
	unsigned int lcbODSO;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the Office Data Source Object data at offset fcODSO in the Table Stream.
	unsigned int fcPlcfpmiOldXP;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated paragraph mark information cache begins at this offset. Information SHOULD NOT<82> be emitted at this offset and SHOULD<83> be ignored. If lcbPlcfpmiOldXP is zero, fcPlcfpmiOldXP is undefined and MUST be ignored.
	unsigned int lcbPlcfpmiOldXP;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated paragraph mark information cache at offset fcPlcfpmiOldXP in the Table Stream. This value SHOULD<84> be zero.
	unsigned int fcPlcfpmiNewXP;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated paragraph mark information cache begins at this offset. Information SHOULD NOT<85> be emitted at this offset and SHOULD<86> be ignored. If lcbPlcfpmiNewXP is zero, fcPlcfpmiNewXP is undefined and MUST be ignored.
	unsigned int lcbPlcfpmiNewXP;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated paragraph mark information cache at offset fcPlcfpmiNewXP in the Table Stream. This value SHOULD<87> be zero.
	unsigned int fcPlcfpmiMixedXP;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated paragraph mark information cache begins at this offset. Information SHOULD NOT<88> be emitted at this offset and SHOULD<89> be ignored. If lcbPlcfpmiMixedXP is zero, fcPlcfpmiMixedXP is undefined and MUST be ignored.
	unsigned int lcbPlcfpmiMixedXP;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated paragraph mark information cache at offset fcPlcfpmiMixedXP in the Table Stream.  This value SHOULD<90> be zero.
	unsigned int fcUnused2;//(4 bytes): This value is undefined and MUST be ignored.

	unsigned int lcbUnused2;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcPlcffactoid;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. A Plcffactoid, which specifies the smart tag recognizer state of each text range, begins at this offset. If lcbPlcffactoid is zero, fcPlcffactoid is undefined and MUST be ignored.
	unsigned int lcbPlcffactoid;//(4 bytes): An unsigned integer that specifies the size, in bytes of the Plcffactoid that begins at offset fcPlcffactoid in the Table Stream.
	unsigned int fcPlcflvcOldXP;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated listnum field cache begins at this offset. Information SHOULD NOT<91> be emitted at this offset and SHOULD<92> be ignored. If lcbPlcflvcOldXP is zero, fcPlcflvcOldXP is undefined and MUST be ignored.
	unsigned int lcbPlcflvcOldXP;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated listnum field cache at offset fcPlcflvcOldXP in the Table Stream. This value SHOULD<93> be zero.
	unsigned int fcPlcflvcNewXP;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated listnum field cache begins at this offset. Information SHOULD NOT<94> be emitted at this offset and SHOULD<95> be ignored. If lcbPlcflvcNewXP is zero, fcPlcflvcNewXP is undefined and MUST be ignored.
	unsigned int lcbPlcflvcNewXP;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated listnum field cache at offset fcPlcflvcNewXP in the Table Stream. This value SHOULD<96> be zero.
	unsigned int fcPlcflvcMixedXP;//(4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated listnum field cache begins at this offset. Information SHOULD NOT<97> be emitted at this offset and SHOULD<98> be ignored. If lcbPlcflvcMixedXP is zero, fcPlcflvcMixedXP is undefined and MUST be ignored.
	unsigned int lcbPlcflvcMixedXP;//(4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated listnum field cache at offset fcPlcflvcMixedXP in the Table Stream. This value SHOULD<99> be zero.		 

}FibRgFcLcb2002;

typedef struct FibRgFcLcb2003
{
	//FibRgFcLcb97 *rgFcLcb97;//rgFcLcb97 (744 bytes)
	//FibRgFcLcb2000 * rgFcLcb2000;//rgFcLcb2000 (864 bytes)
	//FibRgFcLcb2002 *rgFcLcb2002;// (1088 bytes): The contained FibRgFcLcb2002.
	int fcHplxsdr;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. An Hplxsdr structure begins at this offset. This structure specifies information about XML schema definition references.
	int lcbHplxsdr;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the Hplxsdr structure at the offset fcHplxsdr in the Table Stream. If lcbHplxsdr is zero, then fcHplxsdr is undefined and MUST be ignored.
	int fcSttbfBkmkSdt;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. An SttbfBkmkSdt that contains information about the structured document tag bookmarks in the document begins at this offset. If lcbSttbfBkmkSdt is zero, then fcSttbfBkmkSdt is undefined and MUST be ignored.  The SttbfBkmkSdt is parallel to the PlcBkfd at offset fcPlcfBkfSdt in the Table Stream. Each element in the SttbfBkmkSdt specifies information about the bookmark that is associated with the data element which is located at the same offset in that PlcBkfd. For this reason, the SttbfBkmkSdt that begins at offset fcSttbfBkmkSdt, and the PlcBkfd that begins at offset fcPlcfBkfSdt, MUST contain the same number of elements.
	int lcbSttbfBkmkSdt;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfBkmkSdt at offset fcSttbfBkmkSdt.
	int fcPlcfBkfSdt;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcBkfd that contains information about the structured document tag bookmarks in the document begins at this offset. If lcbPlcfBkfSdt is zero, fcPlcfBkfSdt is undefined and MUST be ignored.  Each data element in the PlcBkfd is associated, in a one-to-one correlation, with a data element in the PlcBkld at offset fcPlcfBklSdt. For this reason, the PlcBkfd that begins at offset fcPlcfBkfSdt, and the PlcBkld that begins at offset fcPlcfBklSdt, MUST contain the same number of data elements. The PlcBkfd is parallel to the SttbfBkmkSdt at offset fcSttbfBkmkSdt in the Table Stream. Each data element in the PlcBkfd specifies information about the bookmark that is associated with the element which is located at the same offset in that SttbfBkmkSdt. For this reason, the PlcBkfd that begins at offset fcPlcfBkfSdt, and the SttbfBkmkSdt that begins at offset fcSttbfBkmkSdt, MUST contain the same number of elements.
	int lcbPlcfBkfSdt;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcBkfd at offset fcPlcfBkfSdt.  fcPlcfBklSdt (4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcBkld that contains information about the structured document tag bookmarks in the document begins at this offset. If lcbPlcfBklSdt is zero, fcPlcfBklSdt is undefined and MUST be ignored.  Each data element in the PlcBkld is associated, in a one-to-one correlation, with a data element in the PlcBkfd at offset fcPlcfBkfSdt. For this reason, the PlcBkld that begins at offset fcPlcfBklSdt, and the PlcBkfd that begins at offset fcPlcfBkfSdt MUST contain the same number of data elements.
	int lcbPlcfBklSdt;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcBkld at offset fcPlcfBklSdt.
	int cCustomXForm;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. An array of 16-bit Unicode characters, which specifies the full path and file name of the XML Stylesheet to apply when saving this document in XML format, begins at this offset. If lcbCustomXForm is zero, fcCustomXForm is undefined and MUST be ignored.
	int lcbCustomXForm;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the array at offset fcCustomXForm in the Table Stream. This value MUST be less than or equal to 4168 and MUST be evenly divisible by two.
	int fcSttbfBkmkProt;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. An SttbfBkmkProt that contains information about range-level protection bookmarks in the document begins at this offset. If lcbSttbfBkmkProt is zero, fcSttbfBkmkProt is undefined and MUST be ignored.  The SttbfBkmkProt is parallel to the PlcBkf at offset fcPlcfBkfProt in the Table Stream. Each element in the SttbfBkmkProt specifies information about the bookmark that is associated with the data element which is located at the same offset in that PlcBkf. For this reason, the SttbfBkmkProt that begins at offset fcSttbfBkmkProt, and the PlcBkf that begins at offset fcPlcfBkfProt, MUST contain the same number of elements.
	int lcbSttbfBkmkProt;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbfBkmkProt at offset fcSttbfBkmkProt.
	int fcPlcfBkfProt;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcBkf that contains information about range-level protection bookmarks in the document begins at this offset. If lcbPlcfBkfProt is zero, then fcPlcfBkfProt is undefined and MUST be ignored.  Each data element in the PlcBkf is associated, in a one-to-one correlation, with a data element in the PlcBkl at offset fcPlcfBklProt. For this reason, the PlcBkf that begins at offset fcPlcfBkfProt, and the PlcBkl that begins at offset fcPlcfBklProt, MUST contain the same number of data elements. The PlcBkf is parallel to the SttbfBkmkProt at offset fcSttbfBkmkProt in the Table Stream. Each data element in the PlcBkf specifies information about the bookmark that is associated with the element which is located at the same offset in that SttbfBkmkProt. For this reason, the PlcBkf that begins at offset fcPlcfBkfProt, and the SttbfBkmkProt that begins at offset fcSttbfBkmkProt, MUST contain the same number of elements.
	int lcbPlcfBkfProt;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcBkf at offset fcPlcfBkfProt.
	int fcPlcfBklProt;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. A PlcBkl containing information about range-level protection bookmarks in the document begins at this offset. If lcbPlcfBklProt is zero, then fcPlcfBklProt is undefined and MUST be ignored.  Each data element in the PlcBkl is associated in a one-to-one correlation with a data element in the PlcBkf at offset fcPlcfBkfProt, so the PlcBkl beginning at offset fcPlcfBklProt and the PlcBkf beginning at offset fcPlcfBkfProt MUST contain the same number of data elements.
	int lcbPlcfBklProt;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcBkl at offset fcPlcfBklProt.
	int fcSttbProtUser;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. A SttbProtUser that specifies the usernames that are used for range-level protection begins at this offset.
	int lcbSttbProtUser;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the SttbProtUser at the offset fcSttbProtUser.
	int fcUnused;// (4 bytes): This value MUST be zero, and MUST be ignored.
	int lcbUnused;// (4 bytes): This value MUST be zero, and MUST be ignored.
	int fcPlcfpmiOld;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream.  Deprecated paragraph mark information cache begins at this offset. Information SHOULD NOT<100> be emitted at this offset and SHOULD<101> be ignored. If lcbPlcfpmiOld is zero, then fcPlcfpmiOld is undefined and MUST be ignored.
	int lcbPlcfpmiOld;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated paragraph mark information cache at offset fcPlcfpmiOld in the Table Stream. SHOULD<102> be zero.
	int fcPlcfpmiOldInline;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream.  Deprecated paragraph mark information cache begins at this offset. Information SHOULD NOT<103> be emitted at this offset and SHOULD<104> be ignored. If lcbPlcfpmiOldInline is zero, then fcPlcfpmiOldInline is undefined and MUST be ignored.
	int lcbPlcfpmiOldInline;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated paragraph mark information cache at offset fcPlcfpmiOldInline in the Table Stream.  SHOULD<105> be zero.
	int fcPlcfpmiNew;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream.  Deprecated paragraph mark information cache begins at this offset. Information SHOULD NOT<106> be emitted at this offset and SHOULD<107> be ignored. If lcbPlcfpmiNew is zero, then fcPlcfpmiNew is undefined and MUST be ignored.
	int lcbPlcfpmiNew;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated paragraph mark information cache at offset fcPlcfpmiNew in the Table Stream. SHOULD<108> be zero.
	int fcPlcfpmiNewInline;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream.  Deprecated paragraph mark information cache begins at this offset. Information SHOULD NOT<109> be emitted at this offset and SHOULD<110> be ignored. If lcbPlcfpmiNewInline is zero, then fcPlcfpmiNewInline is undefined and MUST be ignored.
	int lcbPlcfpmiNewInline;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated paragraph mark information cache at offset fcPlcfpmiNewInline in the Table Stream.  SHOULD<111> be zero.
	int fcPlcflvcOld;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. Deprecated listnum field cache begins at this offset. Information SHOULD NOT<112> be emitted at this offset and SHOULD<113> be ignored. If lcbPlcflvcOld is zero, then fcPlcflvcOld is undefined and MUST be ignored.
	int lcbPlcflvcOld;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated listnum field cache at offset fcPlcflvcOld in the Table Stream. SHOULD<114> be zero.
	int fcPlcflvcOldInline;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream.  Deprecated listnum field cache begins at this offset. Information SHOULD NOT<115> be emitted at this offset and SHOULD<116> be ignored. If lcbPlcflvcOldInline is zero, fcPlcflvcOldInline is undefined and MUST be ignored.
	int lcbPlcflvcOldInline;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated listnum field cache at offset fcPlcflvcOldInline in the Table Stream. SHOULD<117> be zero.
	int fcPlcflvcNew;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream.  Deprecated listnum field cache begins at this offset. Information SHOULD NOT<118> be emitted at this offset and SHOULD<119> be ignored. If lcbPlcflvcNew is zero, fcPlcflvcNew is undefined and MUST be ignored.
	int lcbPlcflvcNew;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated listnum field cache at offset fcPlcflvcNew in the Table Stream. SHOULD<120> be zero.
	int fcPlcflvcNewInline;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream.  Deprecated listnum field cache begins at this offset. Information SHOULD NOT<121> be emitted at this offset and SHOULD<122> be ignored. If lcbPlcflvcNewInline is zero, fcPlcflvcNewInline is undefined and MUST be ignored.
	int lcbPlcflvcNewInline;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated listnum field cache at offset fcPlcflvcNewInline in the Table Stream.  SHOULD<123> be zero.
	int fcPgdMother;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream.  Deprecated document page layout cache begins at this offset. Information SHOULD NOT <124> be emitted at this offset and SHOULD <125> be ignored. If lcbPgdMother is zero, fcPgdMother is undefined and MUST be ignored.
	int lcbPgdMother;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated document page layout cache at offset fcPgdMother in the Table Stream.  
	int fcBkdMother;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream.  Deprecated document text flow break cache begins at this offset. Information SHOULD NOT <126> be emitted at this offset and SHOULD <127> be ignored. If lcbBkdMother is zero, then fcBkdMother is undefined and MUST be ignored.
	int lcbBkdMother;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated document text flow break cache at offset fcBkdMother in the Table Stream.
	int fcAfdMother;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. Deprecated document author filter cache begins at this offset. Information SHOULD NOT <128> be emitted at this offset and SHOULD <129> be ignored. If lcbAfdMother is zero, then fcAfdMother is undefined and MUST be ignored.
	int lcbAfdMother;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated document author filter cache at offset fcAfdMother in the Table Stream.  fcPgdFtn (4 bytes): An unsigned integer that specifies an offset in the Table Stream. Deprecated footnote layout cache begins at this offset. Information SHOULD NOT <130> be emitted at this offset and SHOULD <131> be ignored. If lcbPgdFtn is zero, then fcPgdFtn is undefined and MUST be ignored.
	int lcbPgdFtn;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated footnote layout cache at offset fcPgdFtn in the Table Stream.
	int fcBkdFtn;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated footnote text flow break cache begins at this offset. Information SHOULD NOT <132> be emitted at this offset and SHOULD <133> be ignored. If lcbBkdFtn is zero, fcBkdFtn is undefined and MUST be ignored.
	int lcbBkdFtn;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated footnote text flow break cache at offset fcBkdFtn in the Table Stream.  
	int fcAfdFtn;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated footnote author filter cache begins at this offset. Information SHOULD NOT <134> be emitted at this offset and SHOULD <135> be ignored. If lcbAfdFtn is zero, fcAfdFtn is undefined and MUST be ignored.
	int lcbAfdFtn;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated footnote author filter cache at offset fcAfdFtn in the Table Stream.
	int fcPgdEdn;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated endnote layout cache begins at this offset. Information SHOULD NOT <136> be emitted at this offset and SHOULD <137> be ignored. If lcbPgdEdn is zero, then fcPgdEdn is undefined and MUST be ignored.
	int lcbPgdEdn;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated endnote layout cache at offset fcPgdEdn in the Table Stream.
	int fcBkdEdn;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. The deprecated endnote text flow break cache begins at this offset. Information SHOULD NOT <138> be emitted at this offset and SHOULD <139> be ignored. If lcbBkdEdn is zero, fcBkdEdn is undefined and MUST be ignored.
	int lcbBkdEdn;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated endnote text flow break cache at offset fcBkdEdn in the Table Stream.
	int fcAfdEdn;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. Deprecated endnote author filter cache begins at this offset. Information SHOULD NOT <140> be emitted at this offset and SHOULD <141> be ignored. If lcbAfdEdn is zero, then fcAfdEdn is undefined and MUST be ignored. 
	int lcbAfdEdn;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated endnote author filter cache at offset fcAfdEdn in the Table Stream.
	int fcAfd;// (4 bytes): An unsigned integer that specifies an offset in the Table Stream. A deprecated AFD structure begins at this offset. Information SHOULD NOT<142> be emitted at this offset and SHOULD<143> be ignored. If lcbAfd is zero, fcAfd is undefined and MUST be ignored.  
	int lcbAfd;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the deprecated AFD structure at offset fcAfd in the Table Stream.
}FibRgFcLcb2003;

typedef struct FibRgFcLcb2007
{
	//FibRgFcLcb97 *rgFcLcb97;//rgFcLcb97 (744 bytes)
	//FibRgFcLcb2000 * rgFcLcb2000;//rgFcLcb2000 (864 bytes)
	//FibRgFcLcb2002 *rgFcLcb2002;// (1088 bytes): The contained FibRgFcLcb2002.
	//FibRgFcLcb2003 * rgFcLcb2003;// (1312 bytes)
	unsigned int fcPlcfmthd;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbPlcfmthd;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcSttbfBkmkMoveFrom;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbSttbfBkmkMoveFrom;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcPlcfBkfMoveFrom;//(4 bytes): This value is undefined and MUST be ignored
	unsigned int lcbPlcfBkfMoveFrom;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcPlcfBklMoveFrom;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbPlcfBklMoveFrom;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcSttbfBkmkMoveTo;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbSttbfBkmkMoveTo;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcPlcfBkfMoveTo;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbPlcfBkfMoveTo;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcPlcfBklMoveTo;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbPlcfBklMoveTo;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcUnused1;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbUnused1;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcUnused2;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbUnused2;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcUnused3;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbUnused3;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcSttbfBkmkArto;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbSttbfBkmkArto;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcPlcfBkfArto;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbPlcfBkfArto;//(4 bytes): This value MUST be zero, and MUST be ignored
	unsigned int fcPlcfBklArto;//(4 bytes): Undefined and MUST be ignored.
	unsigned int lcbPlcfBklArto;//(4 bytes): MUST be zero, and MUST be ignored.
	unsigned int fcArtoData;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbArtoData;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcUnused4;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbUnused4;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcUnused5;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbUnused5;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcUnused6;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbUnused6;//(4 bytes): This value MUST be zero, and MUST be ignored.
	unsigned int fcOssTheme;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbOssTheme;//(4 bytes): This value SHOULD<144> be zero, and MUST be ignored.
	unsigned int fcColorSchemeMapping;//(4 bytes): This value is undefined and MUST be ignored.
	unsigned int lcbColorSchemeMapping;//(4 bytes): This value SHOULD<145> be zero, and MUST be ignored.					
} FibRgFcLcb2007;



typedef struct FibRgFcLcb
{
	FibRgFcLcb97 * rgFcLcb97;//rgFcLcb97 (744 bytes)
	FibRgFcLcb2000 * rgFcLcb2000;//rgFcLcb2000 (864 bytes)
	FibRgFcLcb2002 * rgFcLcb2002;// (1088 bytes): The contained FibRgFcLcb2002.
	FibRgFcLcb2003 * rgFcLcb2003;// (1312 bytes)
	FibRgFcLcb2007 * rgFcLcb2007;// (1350 bytes): The FibRgFcLcb.
} FibRgFcLcb;

typedef struct FIB
{
	FibBase * base;//32 bytes;
	short csw;//An unsigned integer that specifies the count of 16-bit values corresponding to fibRgW that follow. MUST be 0x000E
	FibRgW97 * fibrgw;//28bytes;
	short cslw;//An unsigned integer that specifies the count of 32-bit values corresponding to fibRgLw that follow. MUST be 0x0016.
	FibRgLw97 * fibRgLw;// 88 bytes;
	unsigned short cbRgFcLcb;// (2 bytes): An unsigned integer that specifies the count of 64-bit values corresponding to fibRgFcLcbBlob that follow. This MUST be one of the following values, depending on the value of nFib.
	/*
	   Value of nFib cbRgFcLcb
	   0x00C1 0x005D
	   0x00D9 0x006C
	   0x0101 0x0088
	   0x010C 0x00A4
	   0x0112 0x00B7
	   */


	FibRgFcLcb * fibRgFcLcbBlob;// (variable): The FibRgFcLcb.

	short cswNew;// (2 bytes): An unsigned integer that specifies the count of 16-bit values corresponding to fibRgCswNew that follow. This MUST be one of the following values, depending on the value of nFib
	/*
	   Value of nFib cswNew
	   0x00C1 0
	   0x00D9 0x0002
	   0x0101 0x0002
	   0x010C 0x0002
	   0x0112 0x0005
	   */
	FibRgCswNew *fibRgCswNew;// (variable): If cswNew is nonzero, this is fibRgCswNew. Otherwise, it is not present in the file.
	short nFib;
}FIB;

/*
   typedef struct FcCompressed
   {
fc:30;//fc (30 bits): An unsigned integer that specifies an offset in the WordDocument Stream where the text starts. If fCompressed is zero, the text is an array of 16-bit Unicode characters starting at offset fc. If fCompressed is 1, the text starts at offset fc/2 and is an array of 8-bit Unicode characters, except for the values which are mapped to Unicode characters as follows.
A - fCompressed (1 bit): A bit that specifies whether the text is compressed.
A:1;//A bit that specifies whether the text is compressed.
B:1;//This bit MUST be zero, and MUST be ignored.
}FcCompressed;
*/
	typedef struct FcCompressed
{//4 bytes
	unsigned int fc:30; //fc (30 bits): An unsigned integer that specifies an offset in the WordDocument Stream where the text starts. If fCompressed is zero, the text is an array of 16-bit Unicode characters starting at offset fc. If fCompressed is 1, the text starts at offset fc/2 and is an array of 8-bit Unicode characters, except for the values which are mapped to Unicode characters as follows.
	/*
	   Byte Unicode Character
	   0x82 0x201A
	   0x83 0x0192
	   0x84 0x201E
	   0x85 0x2026
	   0x86 0x2020
	   0x87 0x2021
	   0x88 0x02C6
	   0x89 0x2030
	   0x8A 0x0160
	   0x8B 0x2039
	   0x8C 0x0152
	   0x91 0x2018
	   0x92 0x2019
	   0x93 0x201C
	   0x94 0x201D
	   0x95 0x2022
	   0x96 0x2013
	   0x97 0x2014
	   0x98 0x02DC
	   0x99 0x2122
	   0x9A 0x0161
	   0x9B 0x203A
	   0x9C 0x0153
	   0x9F 0x0178
	   */
	unsigned int A:1; //A - fCompressed (1 bit): A bit that specifies whether the text is compressed.
	unsigned int B:1; //B - r1 (1 bit): This bit MUST be zero, and MUST be ignored.
}FcCompressed;	
typedef struct Pcd
{//8bytes
	unsigned short A:1; //A - fNoParaLast (1 bit): If this bit is 1, the text MUST NOT contain a paragraph mark.
	unsigned short B:1; //B - fR1 (1 bit): This field is undefined and MUST be ignored.
	unsigned short C:1; //C - fDirty (1 bit): This field MUST be 0.
	unsigned short fR2:13; //fR2 (13 bits): This field is undefined and MUST be ignored.
	short fc;//fc (4 bytes): An FcCompressed structure that specifies the location of the text in the WordDocument Stream.
	short fc2;
	unsigned short prm; //prm (2 bytes): A Prm structure that specifies additional properties for this text. These properties are used as part of the algorithms in sections 2.4.6.1 (Direct Paragraph Formatting) and 2.4.6.2 (Direct Character Formatting).
}Pcd;
typedef struct PlcPcd {
	unsigned int * aCP;//An array of CPs that specifies the starting points of text ranges. The end of each range is the beginning of the next range. All CPs MUST be greater than or equal to zero. If any of the fields ccpFtn, ccpHdd, ccpMcr, ccpAtn, ccpEdn, ccpTxbx, or ccpHdrTxbx from FibRgLw97 are nonzero, then the last CP MUST be equal to the sum of those fields plus ccpText+1.  Otherwise, the last CP MUST be equal to ccpText.
	Pcd * aPcd;//An array of Pcds (8 bytes each) that specify the location of text in the WordDocument stream and any additional properties of the text. If aPcd[i].fc.fCompressed is 1, then the byte offset of the last character of the text referenced by aPcd[i] is given by the following. 
	//aPcd[i].fc.fc/2 + aPcd[i+1]-aCP[i];
	//Otherwise, the byte offset of the last character of the text referenced by aPcd[i] is given by the following. 
	//aPcd[i].fc.fc + (aPcd[i+1]-aCP[i])*2;
	//Because aCP MUST be sorted in ascending order and MUST NOT contain duplicate CPs, (aCP[i+1]-aCP[i])>0, for all valid indexes i of aPcd. Because a PLC MUST contain one more CP than a data element, i+1 is a valid index of aCP if i is a valid index of aPcd.
}PlcPcd;

typedef struct PnFkpPapx
{//The PnFkpPapx structure specifies the offset of a PapxFkp in the WordDocument Stream.
	unsigned int pn:22;// (22 bits): An unsigned integer that specifies the offset in the WordDocument Stream of a PapxFkp structure. The PapxFkp structure begins at an offset of pn¡Á512.
	unsigned int unused:10;// (10 bits): This value is undefined and MUST be ignored.
}PnFkpPapx;

typedef struct BxPap
{//The BxPap structure specifies the offset of a PapxInFkp in PapxFkp.
	unsigned char bOffset;// (1 byte): An unsigned integer that specifies the offset of a PapxInFkp in a PapxFkp. The offset of the PapxInFkp is bOffset*2. If bOffset is 0 then there is no PapxInFkp for this paragraph and this paragraph has the default properties as specified in section 2.6.2.
	char reserved[12];// (12 bytes): Specifies version-specific paragraph height information. This value SHOULD<204> be 0 and SHOULD<205> be ignored
}BxPap;
typedef struct PapxFkp
{ //The PapxFkp structure maps paragraphs, table rows, and table cells to their properties. A PapxFkp structure is 512 bytes in size, with cpara in the last byte. The elements of rgbx specify the locations of PapxInFkp structures that start at offsets between the end of rgbx and cpara within this PapxFkp structure.
	unsigned int* rgfc;// (variable): An array of 4-byte unsigned integers. Each element of this array specifies an offset in the WordDocument Stream where a paragraph of text begins, or where an end of row mark exists. This array MUST be sorted in ascending order and MUST NOT contain duplicates. Each paragraph begins immediately after the end of the previous paragraph. The count of elements that this array contains is cpara incremented by 1. The last element does not specify the beginning of a paragraph; instead it specifies the end of the last paragraph.
	BxPap * rgbx;// (variable): An array of BxPap, followed by PapxInFkp structures. The elements of this array, which has cpara elements and parallels rgfc, each specify the offset of one of the PapxInFkp structures in this PapxFkp structure.  Each PapxInFkp specifies the paragraph properties for the paragraph at the corresponding offset in rgfc or the table properties for the table row whose end of row mark is located at the corresponding offset in rgfc.
	char cpara;// (1 byte): An unsigned integer that specifies the total number of paragraphs, table rows, or table cells for which this PapxFkp structure specifies formatting. This field occupies the last byte of the PapxFkp structure The value of this field MUST be at least 0x01, and MUST NOT exceed 0x1D because that would cause rgfc and rgb to expand and PapxFkp to exceed 512 bytes.
}PapxFkp;

typedef struct PlcBtePapx
{//The PlcBtePapx structure is a PLC that specifies paragraph, table row, or table cell properties as described later. Where most PLCs map CPs to data, the PlcBtePapx maps stream offsets to data instead. The offsets in aFC partition a portion of the WordDocument stream into adjacent ranges.  Consider the collection of paragraphs, table rows, and table cells whose last character occurs at an offset in the WordDocument stream larger than or equal to aFC[i] but smaller than aFC[i+1]. Then, aPnBtePapx[i] specifies the properties of these paragraphs, table rows, or table cells.  A PlcBtePapx MUST NOT contain duplicate stream offsets. Each data element of PlcBtePapx is 4 bytes long.
	unsigned int* aFC;// (variable): An array of unsigned integers. Each element in this array specifies an offset in the WordDocument stream. The elements of aFC MUST be sorted in ascending order, and there MUST NOT be any duplicate entries.
	PnFkpPapx * aPnBtePapx;// (variable): An array of PnFkpPapx. The ith entry in aPnBtePapx is a PnFkpPapx that specifies the properties of all paragraphs, table rows, and table cells whose last character occurs at an offset in the WordDocument stream larger than or equal to aFC[i] but smaller than aFC[i+1]; aPnBtePapx MUST contain one less entry than aFC.
}PlcBtePapx;


typedef struct Pcdt{
	char clxt;// (1 byte): This value MUST be 0x02.
	unsigned int lcb;// (4 bytes): An unsigned integer that specifies the size, in bytes, of the PlcPcd structure.
	PlcPcd plcpcd;// (variable): A PlcPcd structure. As with all Plc elements, the size that is specified by lcb MUST result in a whole number of Pcd structures in this PlcPcd structure.
}Pcdt;
typedef struct Prc{
	char clxt;// (1 byte): This value MUST be 0x01.
	short cbGrpprl;// (2 bytes): A signed integer that specifies the size of GrpPrl, in bytes. This value MUST be less than or equal to 0x3FA2.
	char * GrpPrl;// (variable): An array of Prl elements. GrpPrl contains a whole number of Prl elements.
}Prc;
typedef struct Clx{
	Prc * RgPrc;// (variable): An array of Prc. If this array is empty, the first byte of the Clx MUST be 0x02.  0x02 is invalid as the first byte of a Prc, but required for the Pcdt.

	int numCP;
	int numPcd;
	Pcdt * pcdt;// (variable): A Pcdt.
}Clx;


typedef struct DocFile{
	ByteArray *bytearray;
	OleHeader *header;
	FIB * fib;
	Clx * clx;
}DocFile;

void DocFile_free(DocFile * file);

FibBase * FibBase_read(ByteArray *bytearray)
{
	int position = bytearray->position;
	FibBase * fibbase = malloc(sizeof(FibBase));
	memset(fibbase,0,sizeof(FibBase));
	fibbase->wIdent = ByteArray_readInt16(bytearray);
	if(fibbase->wIdent!=0xA5EC)
	{
		//printf("not a WORD file!\n");
		free(fibbase);
		return NULL;
	}
	fibbase->nFib= ByteArray_readInt16(bytearray);
	fibbase->unused= ByteArray_readInt16(bytearray);
	fibbase->lid= ByteArray_readInt16(bytearray);
	fibbase->pnNext= ByteArray_readInt16(bytearray);
	//fibbase->flag = ByteArray_readInt16(bytearray);
	ByteArray_readBytes(bytearray,2,(char*)&(fibbase->flag));
	fibbase->nFibBack = ByteArray_readInt16(bytearray);
	if(fibbase->nFibBack != 0xbf && fibbase->nFibBack != 0xc1)
	{
		//printf("nFibBack error! %x\n",fibbase->nFibBack);
		free(fibbase);
		return NULL;
	}
	fibbase->lKey = ByteArray_readInt32(bytearray);
	fibbase->envr = ByteArray_readByte(bytearray);
	//fibbase->flag2= ByteArray_readByte(bytearray);
	ByteArray_readBytes(bytearray,1,(char*)&(fibbase->flag2));
	bytearray->position += 12;//reserved 12 bytes
	//printf("FibBase (%d bytes == 32) read ok!\n",bytearray->position-position);
	return fibbase;
}
FibRgLw97 * FibRgLW97_read(ByteArray *bytearray)
{
	int position = bytearray->position;

	FibRgLw97 * fibRgLw = malloc(sizeof(FibRgLw97));
	memset(fibRgLw,0,sizeof(FibRgLw97));
	fibRgLw->cbMac = ByteArray_readInt32(bytearray);
	bytearray->position+=8;
	fibRgLw->ccpText= ByteArray_readInt32(bytearray);
	fibRgLw->ccpFtn= ByteArray_readInt32(bytearray);
	fibRgLw->ccpHdd= ByteArray_readInt32(bytearray);
	bytearray->position+=4;
	fibRgLw->ccpAtn= ByteArray_readInt32(bytearray);
	fibRgLw->ccpEdn= ByteArray_readInt32(bytearray);
	fibRgLw->ccpTxbx= ByteArray_readInt32(bytearray);
	fibRgLw->ccpHdrTxbx= ByteArray_readInt32(bytearray);
	bytearray->position+=44;
	/*
	printf("fibRgLw cbMac:%d, ccpText:%d, ccpFtn:%d, ccpHdd:%d, ccpAtn:%d, ccpEdn:%d, ccpTxbx:%d, ccpHdrTxbx:%d, (%d)\n"
			,fibRgLw->cbMac
			,fibRgLw->ccpText
			,fibRgLw->ccpFtn
			,fibRgLw->ccpHdd
			,fibRgLw->ccpAtn
			,fibRgLw->ccpEdn
			,fibRgLw->ccpTxbx
			,fibRgLw->ccpHdrTxbx
			,sizeof(FibRgLw97)
		  );
		  */

	bytearray->position = position + 88;
	//printf("FibRgLW97_read(%d bytes == 88) read ok!\n",bytearray->position-position);
	return fibRgLw;
}


FibRgFcLcb * FibRgFcLcb_read(ByteArray*bytearray)
{
	//int position = bytearray->position;
	FibRgFcLcb* fibrgfclcb = malloc(sizeof(FibRgFcLcb));
	memset(fibrgfclcb,0,sizeof(FibRgFcLcb));

	int i = 0;
	fibrgfclcb->rgFcLcb97 = malloc(744);
	memset(fibrgfclcb->rgFcLcb97,0,744);
	while(i<744)
	{
		int * p = (int*)((char*)fibrgfclcb->rgFcLcb97+ i);
		*p = ByteArray_readInt32(bytearray);
		i+= 4;
	}
	//printf("[0x%x],%d==744 == %d\n",position,sizeof(FibRgFcLcb97),bytearray->position-position);
	fibrgfclcb->rgFcLcb2000 = malloc(864-744);
	memset(fibrgfclcb->rgFcLcb2000,0,864-744);
	while(i<864)
	{
		int * p = ((int*)((char*)fibrgfclcb->rgFcLcb2000+ i));
		*p = ByteArray_readInt32(bytearray);
		i+= 4;
	}
	fibrgfclcb->rgFcLcb2002 = malloc(1088-864);
	memset(fibrgfclcb->rgFcLcb2002,0,1088-864);
	while(i<1088)
	{
		int * p = (int*)((char*)fibrgfclcb->rgFcLcb2002+ i);
		*p = ByteArray_readInt32(bytearray);
		i+= 4;
	}
	fibrgfclcb->rgFcLcb2003 = malloc(1312-1088);
	memset(fibrgfclcb->rgFcLcb2003,0,1312-1088);
	while(i<1312)
	{
		int * p = (int*)((char*)fibrgfclcb->rgFcLcb2003+ i);
		*p = ByteArray_readInt32(bytearray);
		i+= 4;
	}
	//printf("[0x%x],%d = 1312-1088 = %d\n",position,sizeof(FibRgFcLcb2003),bytearray->position-position - position);
	fibrgfclcb->rgFcLcb2007 = malloc(sizeof(FibRgFcLcb2007));
	memset(fibrgfclcb->rgFcLcb2007,0,sizeof(FibRgFcLcb2007));
	while(i<1312 + sizeof(FibRgFcLcb2007))
	{
		int * p = (int*)((char*)fibrgfclcb->rgFcLcb2007+ i);
		*p = ByteArray_readInt32(bytearray);
		i+= 4;
	}
	//printf("[0x%x],%d = %d\n",position,sizeof(FibRgFcLcb2007),bytearray->position-position - position);

	//printf("FibRgFcLcb (%d)\n" , sizeof(FibRgFcLcb));
	return fibrgfclcb;
}


void FIB_free(FIB * fib)
{
	if(fib)
	{
		if(fib->base){
			free(fib->base);
		}
		free(fib);
	}
}

FibRgW97 * FibRgW97_read(ByteArray*bytearray)
{//28 bytes;
	int position = bytearray->position;
	FibRgW97 * fibrgw = malloc(sizeof(FibRgW97));
	memset(fibrgw,0,sizeof(FibRgW97));
	bytearray->position = position + 28;
	//printf("FibRgW97_read (%d bytes == 28) read ok!\n",bytearray->position-position);
	return fibrgw;
}
FibRgCswNew * FibRgCswNew_read(ByteArray * bytearray)
{
	//printf("FibRgCswNew_read\n");
	FibRgCswNew * fibrgcswnew  = malloc(sizeof(FibRgCswNew ));
	memset(fibrgcswnew,0,sizeof(FibRgCswNew ));
	fibrgcswnew->nFibNew = ByteArray_readInt16(bytearray);// (2 bytes): An unsigned integer that specifies the version number of the file format that is used. This value MUST be one of the following.  Value
	switch(fibrgcswnew->nFibNew)
	{
		case 0x00D9 :
			//fibrgcswnew->rgCswNewData[0] = ByteArray_readInt16(bytearray);
			break;
		case 0x0101 :
			//fibrgcswnew->rgCswNewData[0] = ByteArray_readInt16(bytearray);
			break;
		case 0x010C :
			//fibrgcswnew->rgCswNewData[0] = ByteArray_readInt16(bytearray);
			break;
		case 0x0112 :
			//fibrgcswnew->rgCswNewData[0] = ByteArray_readInt16(bytearray);
			//fibrgcswnew->rgCswNewData[2] = ByteArray_readInt16(bytearray);
			//fibrgcswnew->rgCswNewData[4] = ByteArray_readInt16(bytearray);
			//fibrgcswnew->rgCswNewData[6] = ByteArray_readInt16(bytearray);
			break;
		default:
			//printf("fibrgcswnew ERROR!");
			break;
	}
	return  fibrgcswnew ;
}

FIB * DocFile_readFib(DocFile *file)
{/*{{{*/
	if(file->fib)
		return file->fib;

	ByteArray * bytearray = file->bytearray;
	StorageEntry * entry = Ole_getEntryByName(file->header,"WordDocument");
	if(entry)
	{
		bytearray->position = Ole_getEntryPostion(file->header,entry);
	}else{
		//printf("{WordDocument not found}\n");
		return 0;
	}

	FIB * fib = malloc(sizeof(FIB));
	memset(fib,0,sizeof(FIB));
	//printf("read fib at[0x%x]\n ",bytearray->position);
	fib->base = FibBase_read(bytearray);//32 bytes
	fib->csw = ByteArray_readInt16(bytearray);//csw (2 bytes): An unsigned integer that specifies the count of 16-bit values corresponding to fibRgW that follow. MUST be 0x000E.
	if(fib->csw!=0xe)
	{
		//printf("error fibrgw number!\n");
		FIB_free(fib);
		return NULL;
	}
	fib->fibrgw = FibRgW97_read(bytearray);//28 bytes
	fib->cslw = ByteArray_readInt16(bytearray);//cslw (2 bytes): An unsigned integer that specifies the count of 32-bit values corresponding to fibRgLw that follow. MUST be 0x0016.
	//printf("read cslw at[0x%x]\n ",bytearray->position);
	if(fib->cslw != 0x16)
	{
		//printf("error cslw number!\n");
		FIB_free(fib);
		return NULL;
	}
	fib->fibRgLw = FibRgLW97_read(bytearray); //fibRgLw (88 bytes): The FibRgLw97.
	fib->cbRgFcLcb = ByteArray_readInt16(bytearray);//cbRgFcLcb (2 bytes): An unsigned integer that specifies the count of 64-bit values corresponding to fibRgFcLcbBlob that follow. This MUST be one of the following values, depending on the value of nFib.
	//printf("read fibRgLw at[0x%x]\n ",bytearray->position);
	//0x00C1 fibRgFcLcb97
	switch(fib->cbRgFcLcb)
	{
		case 0x005D : // case 0x00C1 :
			//printf("%x,%x\n",0x00C1,fib->base->nFib);
			fib->base->nFib = 0x00C1;
			//printf("FibRgFcLcb97,");
			break;
		case 0x006C :// case 0x00D9 :
			//printf("%x,%x\n",0x00D9,fib->base->nFib);
			fib->base->nFib = 0x00D9;
			//printf("FibRgFcLcb2000,");
			break;
		case 0x0088 :// case 0x0101 :
			//printf("%x,%x\n",0x0101,fib->base->nFib);
			fib->base->nFib = 0x0101;
			//printf("FibRgFcLcb2002,");
			break;
		case 0x00A4 :// case 0x010C :
			//printf("%x,%x\n",0x010C,fib->base->nFib);
			fib->base->nFib = 0x010C;
			//printf("FibRgFcLcb2003,");
			break;
		case 0x00B7 :// case 0x0112:
			//printf("%x,%x\n",0x0112,fib->base->nFib);
			fib->base->nFib = 0x0112;
			//printf("FibRgFcLcb2007,");
			break;
		default:
			//printf("cbRgFcLcb ERROR!!!!\n");
			FIB_free(fib);
			return NULL;
	}
	//printf("cbRgFcLcb :%x,%x\n",fib->cbRgFcLcb,fib->base->nFib);
	//return NULL;
	//char * fibRgFcLcbBlob = malloc(fib->cbRgFcLcb*8);
	//ByteArray_readBytes(bytearray,fib->cbRgFcLcb*8,fibRgFcLcbBlob); //fibRgFcLcbBlob (variable): The FibRgFcLcb.
	//fib->fibRgFcLcbBlob = (FibRgFcLcb*)fibRgFcLcbBlob;
	int position = bytearray->position;
	fib->fibRgFcLcbBlob = FibRgFcLcb_read(bytearray);
	switch(fib->base->nFib)
	{
		case 0x00c1:
			bytearray->position = position + 744;
			break;
		case 0x00D9:
			bytearray->position = position + 864;
			break;
		case 0x0101:
			bytearray->position = position + 1088;
			break;
		case 0x010C:
			bytearray->position = position + 1312;
			break;
		case 0x0112:
			bytearray->position = position + 1350;
			break;
	}
	//bytearray->position = position + fib->cbRgFcLcb * 8;

	fib->cswNew = ByteArray_readInt16(bytearray);//cswNew (2 bytes): An unsigned integer that specifies the count of 16-bit values corresponding to fibRgCswNew that follow. This MUST be one of the following values, depending on the value of nFib.  Value of nFib cswNew
	switch(fib->cswNew)
	{
		case 0x02 :
			break;
		case 0x05 :
			break;
		case 0:
			break;
		default:
			printf("cswNew ERROR!!!!\n");
			FIB_free(fib);
			return NULL;

	}
	//fibRgCswNew (variable): If cswNew is nonzero, this is fibRgCswNew. Otherwise, it is not present in the file.
	if(fib->cswNew)
	{
		fib->fibRgCswNew = FibRgCswNew_read(bytearray);
		fib->nFib = fib->fibRgCswNew->nFibNew;
	}else{//cswNew == 0;
		fib->nFib = fib->base->nFib;
	}
	//printf("nFib:0x%x\n",fib->nFib);
	//printf("read fib end at[0x%x]\n ",bytearray->position);
	return fib;
}/*}}}*/

Clx * Clx_read(DocFile * file)
{
	if(file->fib==NULL)
		file->fib = DocFile_readFib(file);
	if(file->fib==NULL)
	{
		return NULL;
	}

	StorageEntry * entry = NULL;
	if(file->fib->base->flag.G)//fWhichTblStm;
	{
		entry = Ole_getEntryByName(file->header,"1Table");
	}else{
		entry = Ole_getEntryByName(file->header,"0Table");
	}
	if(entry == NULL)
	{
		//printf("Table Stream not found");
		return 0;
	}
	ByteArray * bytearray = file->bytearray;
	unsigned int offset = file->fib->fibRgFcLcbBlob->rgFcLcb97->fcClx;
	unsigned int size = file->fib->fibRgFcLcbBlob->rgFcLcb97->lcbClx;
	int position = Ole_getEntryPostion(file->header,entry);
	//printf("read clx at[0x%x]\n ",position);
	bytearray->position = position + offset;
	Clx * clx = malloc(sizeof(Clx));
	memset(clx,0,sizeof(Clx));
	position = bytearray->position;
	//printf("-------------read clx at[0x%x],%d,\n ",position,size);

	while(bytearray->position - position < size){
		Prc * prc  = malloc(sizeof(Prc));
		prc->clxt = ByteArray_readByte(bytearray);
		if(prc->clxt==0x02)
		{
			Pcdt * pcdt = malloc(sizeof(Pcdt));
			clx->pcdt = pcdt;

			pcdt->clxt = 0x02;
			pcdt->lcb = ByteArray_readInt32(bytearray);
			//printf("Pcdt size:%d\n",pcdt->lcb);

			pcdt->plcpcd.aCP = malloc((pcdt->lcb+8)/2);

			unsigned int ccpText = file->fib->fibRgLw->ccpText;
			unsigned int ccpFtn = file->fib->fibRgLw->ccpFtn;
			unsigned int ccpHdd = file->fib->fibRgLw->ccpHdd;
			//unsigned int ccpMcr= file->fib->fibRgLw->ccpMcr;
			unsigned int ccpAtn= file->fib->fibRgLw->ccpAtn;
			unsigned int ccpEdn= file->fib->fibRgLw->ccpEdn;
			unsigned int ccpTxbx= file->fib->fibRgLw->ccpTxbx;
			unsigned int ccpHdrTxbx = file->fib->fibRgLw->ccpHdrTxbx;
			//int sum = ccpFtn + ccpHdd + ccpMcr + ccpAtn + ccpEdn + ccpTxbx + ccpHdrTxbx;
			int sum = ccpFtn + ccpHdd + ccpAtn + ccpEdn + ccpTxbx + ccpHdrTxbx;

			int i = 0;
			int numCP = 0;
			while(i<(pcdt->lcb))
			{
				unsigned int acp = ByteArray_readInt32(bytearray);
				if(acp>0x7fffffff)
				{
					//printf("ERROR acp\n");
					return 0;
				}
				pcdt->plcpcd.aCP[numCP] = acp;
				//printf("(%d)0x%x %x,",i, pcdt->plcpcd.aCP[numCP],1<<30);
				//printf("(%d)0x%x ,",i, pcdt->plcpcd.aCP[numCP]);
				i+= 4;
				numCP++;
				if((sum > 0 && acp == ccpText + sum + 1) || acp == ccpText) {
					break;
				}
			}
			//bytearray->position -= 4;
			clx->numCP = numCP;
			int len = pcdt->lcb - i;
			clx->numPcd = len/8;
			pcdt->plcpcd.aPcd = malloc(len);
			memset(pcdt->plcpcd.aPcd,0,len);
			ByteArray_readBytes(bytearray,len,(char*)pcdt->plcpcd.aPcd);
			//printf("\nccpText : 0x%x, numCP :%d, numPcd:%d\n",ccpText,clx->numCP,clx->numPcd);
			i = 0;
			while(i<clx->numPcd)
			{
				Pcd * pcd = &(pcdt->plcpcd.aPcd[i]);
				FcCompressed * fc = (FcCompressed*)&(pcd->fc);

				//if(pcd->C) printf("XXXXXXXXXXXXXXXXXXXx\n");

				if(fc->B ){
					//printf("XXXXXXXXXXXXXXXXXXXx%d,%x,%x,\n",i,fc->A,fc->fc);
					//fc->fc &= 0x7fffffff;
				}
				++i;
			}
			break;
		}else if(prc->clxt==0x01){
			//printf("Prc size:%d\n",prc->clxt);
			short cbGrpprl = ByteArray_readInt16(bytearray);
			prc->GrpPrl = malloc(cbGrpprl+2);
			prc->cbGrpprl = cbGrpprl;
			//printf("Prc size:%d!\n",prc->cbGrpprl);
			ByteArray_readBytes(bytearray,prc->cbGrpprl,prc->GrpPrl);
		}else{
			//printf("ERROR Prc! XXXXXXXXXXXXXXXXXXXx\n");
			return NULL;
			break;
		}
	}
	return clx;
}


DocFile * DocFile_parse(ByteArray * bytearray,char * out)
{
	DocFile * file = malloc(sizeof(DocFile));
	memset(file,0,sizeof(DocFile));

	file->bytearray = bytearray;
	OleHeader * header = Ole_read(NULL,file->bytearray);
	if(header==NULL)
		return 0;
	file->header = header;
	file->clx = Clx_read(file);
	if(file->clx){
		PlcPcd * plcpcd = &(file->clx->pcdt->plcpcd);
		unsigned int * aCP = plcpcd->aCP;
		Pcd * aPcd = plcpcd->aPcd;
		int length = file->clx->numCP;

		StorageEntry * entry = Ole_getEntryByName(file->header,"WordDocument");
		if(entry==NULL){
			//printf("{WordDocument not found}\n");
			return 0;
		}
		int position = Ole_getEntryPostion(file->header,entry);
		Pcd* pcd = NULL;
		int i=0;
		while(i<length-1)
		{
			pcd = &(aPcd[i]);
			FcCompressed * fc = (FcCompressed*)&(pcd->fc);
			//if(fc->B) printf("XXXXXXXXXXXXXXXXXXXx%d,0x%x,%x==0,0x%x\n",i,fc->A,fc->B,fc->fc);
			unsigned int acp = aCP[i];//character position
			unsigned int acp2 = aCP[i+1];//character position
			unsigned int start = 0; 
			unsigned int end=0;
			if(fc->A){//fCompressed
				start = fc->fc/2 + position;
				end = fc->fc/2 + (acp2 - acp -1) + position;
				bytearray->position = start;
				//printf("u8 string [0x%x] (%x->%x):\n",bytearray->position,start,end);
				snprintf(out+strlen(out),end-start,"%s",bytearray->data,bytearray->position);
				ByteArray_prints(bytearray,end-start);
			}else{
				start = fc->fc + position;
				end = fc->fc  + 2*(acp2-acp-1)+ position;
				bytearray->position = start;
				//printf("u16 string [0x%x] (%x->%x):\n",bytearray->position,start,end);
				ByteArray_print16(bytearray,(end-start)/2);
			}
			++i;
		}
	}
	return 0;
}
void PlcPcd_free(PlcPcd* plcpcd)
{
	if(plcpcd->aCP)
		free(plcpcd->aCP);
	if(plcpcd->aPcd)
		free(plcpcd->aPcd);
}

void Pcdt_free(Pcdt * pcdt)
{
	if(pcdt)
	{
		PlcPcd_free(&pcdt->plcpcd);
		free(pcdt);
	}
}
void Prc_free(Prc* reprc)
{
	if(reprc)
	{
		if(reprc->GrpPrl)
		{
			free(reprc->GrpPrl);
		}
		free(reprc);
	}
}

void Clx_free(Clx * clx)
{
	if(clx)
	{
		if(clx->RgPrc)
		{
			Prc_free(clx->RgPrc);
		}
		if(clx->pcdt)
		{
			Pcdt_free(clx->pcdt);
		}

		free(clx);
	}
}
void DocFile_free(DocFile * file)
{
	if(file)
	{
		if(file->header)
			Ole_free(file->header);
		if(file->fib)
			FIB_free(file->fib);
		if(file->clx)
			Clx_free(file->clx);
		free(file);
	}
}

#ifdef test_doc
int main()
{
	//char * filename = "test.doc";
	//char * filename = "excel_.doc";
	char * filename = "RAR.doc";
	//char * filename = "PDF.doc";
	FILE * _file = fopen(filename,"rb");
	int fileLen = fseek(_file,0,SEEK_END);
	fileLen = ftell(_file);
	rewind(_file);
	//printf("%s,%d\n",filename,fileLen);

	ByteArray * bytearray = ByteArray_new(fileLen);
	fread(bytearray->data,1,fileLen,_file);
	fclose(_file);

	char * out = malloc(fileLen);
	memset(out,0,fileLen);
	DocFile * file = DocFile_parse(bytearray,out);
	printf("%s",out);
	free(out);
	printf("\n%x",file);
	DocFile_free(file);
	ByteArray_free(bytearray);
	return 0;
}

#endif

#ifdef LITTLE_ENDIAN
int little_endian= 1; 
#else
int little_endian= 0; 
#endif

char ByteArray_readByte(ByteArray * bytearray)
{
	if(bytearray->position < bytearray->length)
	{
		int c = *(bytearray->data+bytearray->position);
		//printf("%x\n",c);
		bytearray->position++;
		return c;
	}
	return 0;
}

void ByteArray_readBytes(ByteArray * bytearray,int size,char * bytes)
{
	if(bytes && size >0 && size + bytearray->position <= bytearray->length)
	{
		memcpy(bytes,bytearray->data+bytearray->position,size);
		bytearray->position+=size;
	}
}
unsigned short int ByteArray_readInt16(ByteArray * bytearray)
{
	unsigned short c = 0;
	if(bytearray->position+1 < bytearray->length)
	{
		if(little_endian){
			c = *((unsigned short*)(bytearray->data+bytearray->position));
		}else{
			c = *(bytearray->data+bytearray->position + 1)&0xff ;
			c += (*(bytearray->data+bytearray->position)&0xff) <<8 ;
		}
		bytearray->position+=2;
	}
	return c;
}
unsigned int ByteArray_readInt32(ByteArray * bytearray)
{
	if(bytearray->position+3 < bytearray->length)
	{
		unsigned int c;
		if(little_endian){
			c = *((int*)(bytearray->data+bytearray->position));
		}else{
			c = (*(bytearray->data+bytearray->position+3))&0xff ;
			c += (*(bytearray->data+bytearray->position+2)&0xff) << 8;
			c += (*(bytearray->data+bytearray->position+1)&0xff) << 16;
			c += (*(bytearray->data+bytearray->position)&0xff) << 24;
		}
		bytearray->position+=4;
		return c;
	}
	return 0;
}
int u16ToUtf8(unsigned short w,unsigned char * ret)
{
	if(w==0xa0)
		w = ' ';
	if(w)
	{
		int length = 0;
		if(w < (1<<7)){//1 byte
			if(w == 0x0d)//½áÊø·ûºÅ
			{
				ret[length++]='\n';
			}else if(w==2){
			}else if(isprint(w)){
				ret[length++]= w;
			}
		}else if(w < (1<<11)){// 2 bytes
			ret[length] = (3<<6);//2 bytes
			ret[length++] = (w>>6) & 0x1f;
			ret[length] = (w) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
		}else if(w < (1<<16)){// 3 bytes
			// Î¢Èí / 5fae
			ret[length] += (7<<5);
			ret[length++] += ((w>>12) & 0xf);
			ret[length] = ((w>>6) & 0x3f);
			ret[length++] += ((1<<7) & 0xff);
			ret[length] = ((w) & 0x3f);
			ret[length++] += ((1<<7) & 0xff);
		}else if(w < (1<<21)){// 4 bytes
			ret[length] = (15<<4);
			ret[length++] += (w >> 18) & 0x7;
			ret[length] = (w >> 12) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
			ret[length] = (w >> 6) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
			ret[length] = (w) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
		}else if(w < (1<<26)){// 5 bytes
			ret[length] = (31<<3);//5 bytes
			ret[length++] = (w >> 18) & 0x3f;
			ret[length] = (w >> 12) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
			ret[length] = (w >> 6) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
			ret[length] = (w) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
		}else if(w < (1<<31)){// 6 bytes
			ret[length] = (63<<2);//6 bytes
			ret[length++] += (w >> 30) & 0x1;
			ret[length] = (w >> 24) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
			ret[length] = (w >> 18) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
			ret[length] = (w >> 12) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
			ret[length] = (w >> 6) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
			ret[length] = (w) & 0x3f;
			ret[length++] += ((1<<7) & 0xff);
		}else{
		}
		ret[length++]='\0';
		return length;
	}
	return 0;
}
char * ByteArray_readUtf16(ByteArray * bytearray,int n)//need free
{
	if(bytearray==NULL || n<=0)
		return NULL;
	int position = bytearray->position;

	char * ret = malloc(n*4+1);
	memset(ret,0,n*3);
	int num = 0;
	unsigned short c = ByteArray_readInt16(bytearray);
	while(c)
	{
		char z[8];
		memset(z,0,8);
		u16ToUtf8(c,(unsigned char*)z);
		sprintf(ret+strlen(ret),"%s",z);
		c = ByteArray_readInt16(bytearray);
		while(c == 0x0d)//½áÊø·ûºÅ
		{
			sprintf(ret+strlen(ret),"\n");
			c = ByteArray_readInt16(bytearray);
			++num;
		}
		++num;
		if(num>n && n>0)break;
	}
	//printf("(%d)\n",num);
	bytearray->position = position+n*2;
	return ret;
}

int ByteArray_print16(ByteArray*bytearray,int n)
{
	if(bytearray==NULL || n<= 0 || bytearray->position+n*2>bytearray->length)
		return 0;
	int position = bytearray->position;
	char * ret = ByteArray_readUtf16(bytearray,n);
	if(ret){
		int length = strlen(ret);
		printf("%s\n",ret);
		free(ret);
		bytearray->position = position;
		return length;
	}
	bytearray->position = position;
	return 0;
}

char *ByteArray_readU8(ByteArray * bytearray,int n)//need free
{
	if(bytearray==NULL || bytearray->position>=bytearray->length)
		return 0;
	int len = n+1;
	char * ret = malloc(len);
	memset(ret,0,len);

	int i=0;
	while(i<n && bytearray->position < bytearray->length)
	{
		unsigned char c = ByteArray_readByte(bytearray);
		if(c == 0x0d)//½áÊø·ûºÅ
			c = '\n';
		unsigned short w = 0;
		switch(c)
		{
			case 0x82:
				w=0x201A;
				break;
			case 0x83:
				w=0x0192;
				break;
			case 0x84:
				w=0x201E;
				break;
			case 0x85:
				w=0x2026;
				break;
			case 0x86:
				w=0x2020;
				break;
			case 0x87:
				w=0x2021;
				break;
			case 0x88:
				w=0x02C6;
				break;
			case 0x89:
				w=0x2030;
				break;
			case 0x8A:
				w=0x0160;
				break;
			case 0x8B:
				w=0x2039;
				break;
			case 0x8C:
				w=0x0152;
				break;
			case 0x91:
				w=0x2018;
				break;
			case 0x92:
				w=0x2019;
				break;
			case 0x93:
				w=0x201C;
				break;
			case 0x94:
				w=0x201D;
				break;
			case 0x95:
				w=0x2022;
				break;
			case 0x96:
				w=0x2013;
				break;
			case 0x97:
				w=0x2014;
				break;
			case 0x98:
				w=0x02DC;
				break;
			case 0x99:
				w=0x2122;
				break;
			case 0x9A:
				w=0x0161;
				break;
			case 0x9B:
				w=0x203A;
				break;
			case 0x9C:
				w=0x0153;
				break;
			case 0x9F:
				w=0x0178;
				break;
			default:
				break;
		}
		if(w){
			char s[8];
			memset(ret,0,8);
			len += u16ToUtf8(w,(unsigned char*)s);
			ret = realloc(ret,len);
			sprintf(ret+strlen(ret),"%s",s);
		}else{
			//if(isprint(c))
			sprintf(ret+strlen(ret),"%c",c);
		}
		++i;
	}
	return ret;
}

int ByteArray_prints(ByteArray*bytearray,int num)
{
	if(bytearray==NULL || bytearray->position>=bytearray->length)
		return 0;
	char * p = bytearray->data + bytearray->position;
	char * ret = ByteArray_readU8(bytearray,num);
	if(num>0 && num<0x1000)
	{
		printf("%s",ret);
		free(ret);
	}else if(strlen(p)){
		printf("%s",p);
	}
	return 0;
}

void ByteArray_printx(ByteArray*bytearray,int num)
{
	int i = 0;
	char *bytes = NULL;
	if(num > 0){
		bytes = malloc(num);
		ByteArray_readBytes(bytearray,num,bytes);
	}
	while(i < num)
	{
		char c = bytes[i];
		printf("%x ",(char)c & 0xff);
		++i;
	}
	if(bytes)
		free(bytes);
}


ByteArray * ByteArray_new(int size)
{
	ByteArray * bytearray = malloc(sizeof(ByteArray));
	memset(bytearray,0,sizeof(ByteArray));
	if(size>0){
		bytearray->length = size;
		bytearray->data = malloc(size);
	}
	return bytearray;
}
void ByteArray_free(ByteArray*bytearray)
{
	if(bytearray)
	{
		if(bytearray->length>0)
			free(bytearray->data);
		free(bytearray);
	}
}
ByteArray*ByteArray_resize(ByteArray*bytearray,int size)
{
	if(size<=0)
	{
		ByteArray_free(bytearray);
		return NULL;
	}

	if(bytearray==NULL)
		bytearray = ByteArray_new(size);

	if(bytearray->length==0){
		bytearray->data = malloc(size);
	}else{
		bytearray->data = realloc(bytearray->data,size);
	}
	bytearray->length = size;
	return bytearray;
}
ByteArray * ByteArray_writeBytes(ByteArray*bytearray,char*bytes,int length)
{
	if(length<=0 || bytes==NULL)
		return bytearray;
	if(bytearray==NULL)
		bytearray = ByteArray_new(length);
	if(bytearray->position+length>bytearray->length){
		bytearray = ByteArray_resize(bytearray,bytearray->length+length);
	}
	memcpy(bytearray->data+bytearray->position,bytes,length);
	bytearray->position += length;
	return bytearray;
}
void ByteArray_rewind(ByteArray*bytearray)
{
	if(bytearray)
		bytearray->position=0;
}



#ifdef as_api
#include "AS3.h"

//Method exposed to ActionScript
//Takes a String and echos it
static AS3_Val echo(void* self, AS3_Val args)
{
	AS3_Val byteArray;
	AS3_ArrayValue( args, "AS3ValType", &byteArray);
	AS3_Val length = AS3_GetS(byteArray, "length");  

	sztrace("length getted!");
	int len = AS3_IntValue(length);
	//if(len>0) return length;

	char *data=NULL;
	data=malloc(len);
	memset(data,0,len);
	int fileLen = AS3_ByteArray_readBytes(data,byteArray, len);
	char *out= NULL;
	out = malloc(fileLen);
	memset(out,0,fileLen);



	ByteArray * bytearray = ByteArray_new(fileLen);
	bytearray->data = data;

	DocFile * file = DocFile_parse(bytearray,out);
	DocFile_free(file);
	ByteArray_free(bytearray);

	return AS3_String(out);
}

int main()
{
	//define the methods exposed to ActionScript
	//typed as an ActionScript Function instance
	AS3_Val echoMethod = AS3_Function( NULL, echo );

	// construct an object that holds references to the functions
	AS3_Val result = AS3_Object( "echo: AS3ValType", echoMethod );

	// Release
	AS3_Release( echoMethod );

	// notify that we initialized -- THIS DOES NOT RETURN!
	AS3_LibInit( result );

	// should never get here!
	return 0;
}
#endif

