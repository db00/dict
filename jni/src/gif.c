/**
 * @file gif.c
 gcc -Wall -I"../SDL2/include/" gif.c ease.c array.c files.c mystring.c myregex.c tween.c matrix.c sprite.c -lSDL2 -D test_gif -lm  dgif_lib.c gif_err.c gifalloc.c  && ./a.out
 gcc -Wall -I"../SDL2/include/" gif.c ease.c array.c mystring.c tween.c matrix.c sprite.c -lmingw32 -lSDL2main -lSDL2 -D test_gif -lm  dgif_lib.c gif_err.c gifalloc.c  && a
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2016-01-08
 */
#include "gif.h"
static void PrintGifError(int ErrorCode) {
	const char *Err = GifErrorString(ErrorCode);

	if (Err != NULL)
		fprintf(stderr, "GIF-LIB error: %s.\n", Err);
	else
		fprintf(stderr, "GIF-LIB undefined error %d.\n", ErrorCode);
}

/******************************************************************************
  The real screen dumping routine.
 ******************************************************************************/
static unsigned char *DumpScreen2RGB(
		ColorMapObject *ColorMap,
		GifRowType *ScreenBuffer,
		int ScreenWidth, int ScreenHeight)
{
	int i, j;
	GifRowType GifRow;
	GifColorType *ColorMapEntry;

	unsigned char *Buffer, *BufferP;

	if ((Buffer = (unsigned char *) malloc(ScreenWidth*ScreenHeight*4)) == NULL)
	{
		printf("Failed to allocate memory required, aborted.");
	}
	//printf("\n%dx%d\n",ScreenWidth,ScreenHeight);
	memset(Buffer,0,ScreenHeight*ScreenWidth*4);

	for (i = 0; i < ScreenHeight; i++) {
		GifRow = ScreenBuffer[i];
		printf("\b\b\b\b%-4d", ScreenHeight - i);
		BufferP = Buffer + i * (ScreenWidth * 4);
		for (j = 0 ; j < ScreenWidth; j++) {
			ColorMapEntry = &ColorMap->Colors[GifRow[j]];

#if SDL_BYTEORDER == SDL_LIL_ENDIAN     /* OpenGL RGBA masks */
			*BufferP++ = ColorMapEntry->Red;
			*BufferP++ = ColorMapEntry->Green;
			*BufferP++ = ColorMapEntry->Blue;
			*BufferP++ = 0xff;
#else
			*BufferP++ = 0xff;
			*BufferP++ = ColorMapEntry->Blue;
			*BufferP++ = ColorMapEntry->Green;
			*BufferP++ = ColorMapEntry->Red;
#endif
		}
	}

	return Buffer;
}

static int readFunc(GifFileType * giffile, GifByteType * bytes, int len)
{
	char* ptr = (char*)(giffile->UserData);
	memcpy(bytes, ptr, len);
	giffile->UserData = ptr + len;
	return len;
}

Array * Surface_gif(char *data, int * delay)
{
	Array * surfaces=NULL;
	int	i, j, Size, Row, Col, Width, Height, ExtCode, Count;
	GifRecordType RecordType;
	GifByteType *Extension;
	GifRowType *ScreenBuffer;
	GifFileType *GifFile;
	int
		InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */
		InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */
	int ImageNum = 0;
	ColorMapObject *ColorMap;
	int Error;

	GifFile = DGifOpen(data, readFunc, &Error);
	if (GifFile == NULL) {
		PrintGifError(Error);
		exit(EXIT_FAILURE);
	}
	/* 
	 * Allocate the screen as vector of column of rows. Note this
	 * screen is device independent - it's the screen defined by the
	 * GIF file parameters.
	 */
	if ((ScreenBuffer = (GifRowType *) malloc(GifFile->SHeight * sizeof(GifRowType))) == NULL)
		printf("Failed to allocate memory required, aborted.");

	Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
	if ((ScreenBuffer[0] = (GifRowType) malloc(Size)) == NULL)
		printf("Failed to allocate memory required, aborted.");

	for (i = 0; i < GifFile->SWidth; i++)  /* Set its color to BackGround. */
		ScreenBuffer[0][i] = GifFile->SBackGroundColor;
	for (i = 1; i < GifFile->SHeight; i++) {
		/* Allocate the other rows, and set their color to background too: */
		if ((ScreenBuffer[i] = (GifRowType) malloc(Size)) == NULL)
			printf("Failed to allocate memory required, aborted.");

		memcpy(ScreenBuffer[i], ScreenBuffer[0], Size);
	}

	/* Scan the content of the GIF file and load the image(s) in: */
	do {
		if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) {
			PrintGifError(GifFile->Error);
			exit(EXIT_FAILURE);
		}
		//printf("ImageCount:%d\n",GifFile->ImageCount);
		switch (RecordType) {
			case IMAGE_DESC_RECORD_TYPE:
				if (DGifGetImageDesc(GifFile) == GIF_ERROR) {
					PrintGifError(GifFile->Error);
					exit(EXIT_FAILURE);
				}

				Row = GifFile->Image.Top; /* Image Position relative to Screen. */
				Col = GifFile->Image.Left;
				Width = GifFile->Image.Width;
				Height = GifFile->Image.Height;


				printf("\n IMAGE_DESC_RECORD_TYPE Image %d at (%d, %d) [%dx%d]:     ",  ++ImageNum, Col, Row, Width, Height);
				if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth || GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) {
					fprintf(stderr, "Image %d is not confined to screen dimension, aborted.\n",ImageNum);
					exit(EXIT_FAILURE);
				}
				if (GifFile->Image.Interlace) {
					/* Need to perform 4 passes on the images: */
					for (Count = i = 0; i < 4; i++)
						for (j = Row + InterlacedOffset[i]; j < Row + Height; j += InterlacedJumps[i]) {
							printf("\b\b\b\b%-4d", Count++);
							if (DGifGetLine(GifFile, &ScreenBuffer[j][Col], Width) == GIF_ERROR) {
								PrintGifError(GifFile->Error);
								exit(EXIT_FAILURE);
							}
						}
				} else {
					for (i = 0; i < Height; i++) {
						printf("\b\b\b\b%-4d", i);
						if (DGifGetLine(GifFile, &ScreenBuffer[Row++][Col], Width) == GIF_ERROR) {
							PrintGifError(GifFile->Error);
							exit(EXIT_FAILURE);
						}
					}
				}

				ColorMap = (GifFile->Image.ColorMap ? GifFile->Image.ColorMap : GifFile->SColorMap);
				if (ColorMap == NULL) {
					fprintf(stderr, "Gif Image does not have a colormap\n");
					exit(EXIT_FAILURE);
				}

				SDL_Surface * surface = Surface_new(GifFile->SWidth,GifFile->SHeight);
				char * pixels = (char*)DumpScreen2RGB(ColorMap, ScreenBuffer, GifFile->SWidth, GifFile->SHeight);
				memcpy(surface->pixels,pixels,GifFile->SWidth*GifFile->SHeight*4);
				if(pixels)free(pixels);
				pixels=NULL;
				surfaces = Array_setByIndex( surfaces,(GifFile->ImageCount)-1, surface);
				//surface->refcount++;
				break;
			case EXTENSION_RECORD_TYPE:
				//printf("\tEXTENSION_RECORD_TYPE");
				/* Skip any extension blocks in file: */
				if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) {
					PrintGifError(GifFile->Error);
					exit(EXIT_FAILURE);
				}
				while (Extension != NULL) {
					if (ExtCode == GRAPHICS_EXT_FUNC_CODE) {
						GraphicsControlBlock gcb;
						if (DGifExtensionToGCB(Extension[0], Extension+1, &gcb) == GIF_ERROR) {
							PrintGifError(GifFile->Error);
							exit(EXIT_FAILURE);
						}
						printf("\tDisposal Mode: %d\n", gcb.DisposalMode);
						printf("\tUser Input Flag: %d\n", gcb.UserInputFlag);
						printf("\tTransparency on: %s\n",
								gcb.TransparentColor != -1 ? "yes" : "no");
						printf("\tDelayTime: %d\n", gcb.DelayTime);
						printf("\tTransparent Index: %d\n", gcb.TransparentColor);
						if(delay)*delay = gcb.DelayTime*10;
					}
					if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) {
						PrintGifError(GifFile->Error);
						exit(EXIT_FAILURE);
					}
				}
				break;
			case TERMINATE_RECORD_TYPE:
				printf("\tTERMINATE_RECORD_TYPE\n");
				break;
			default:		    /* Should be trapped by DGifGetRecordType. */
				break;
		}
	} while (RecordType != TERMINATE_RECORD_TYPE);

	//return NULL;
	if(ScreenBuffer)
	{
		free(ScreenBuffer);
		ScreenBuffer = NULL;
	}

	if (DGifCloseFile(GifFile, &Error) == GIF_ERROR)
	{
		PrintGifError(Error);
		exit(EXIT_FAILURE);
	}
	return surfaces;
}
#ifdef test_gif 
#include "files.h"
int main(int argc, char *argv[])
{
	size_t fileLen;
	char * data = readfile("~/sound/wait.gif",&fileLen);

	Stage_init();

	int numSurface = 0;
	int delay= 0;
	Array* surfaces = Surface_gif(data,&delay);
	if(surfaces)
		numSurface = surfaces->length;
	Sprite*sprite = Sprite_new(NULL);
	Sprite_addChild(stage->sprite,sprite);
	//sprite->x= 100;
	//sprite->y= 100;
	//sprite->w= 50;
	//sprite->h= 120;
	TweenObj * tweenObj = (TweenObj*)TweenObj_new(sprite);
	tweenObj->end->alpha = 1.0;
	Tween * tween = tween_to(sprite,delay*numSurface,tweenObj);
	tween->loop = 1;
	if(surfaces)
		tween->surfaces = surfaces;
	Stage_loopEvents();
	return 0;
}
#endif

/***
 *
 byte#  hexadecimal  text or
 (hex)               value     Meaning
0:     47 49 46
38 39 61     GIF89a    Header
Logical Screen Descriptor
6:     90 01        400        - width in pixels
8:     90 01        400        - height in pixels
A:     F7                      - GCT follows for 256 colors with resolution 3 x 8bits/primary
B:     00           0          - background color #0
C:     00                      - default pixel aspect ratio
D:                            Global Color Table
:
30D:   21 FF                  Application Extension block
30F:   0B           11         - eleven bytes of data follow
310:   4E 45 54
53 43 41
50 45        NETSCAPE   - 8-character application name
32 2E 30     2.0        - application "authentication code"
31B:   03           3          - three more bytes of data
31C:   01           1          - data sub-block index (always 1)
31D:   FF FF        65535      - unsigned number of repetitions
31F:   00                      - end of App Extension block
320:   21 F9                  Graphic Control Extension for frame #1
322:   04           4          - four bytes of data follow
323:   08                      - bit-fields 3x:3:1:1, 000|010|0|0 -> Restore to bg color
324:   09 00                   - 0.09 sec delay before painting next frame
326:   00                      - no transparent color
327:   00                      - end of GCE block
328:   2C                     Image Descriptor
329:   00 00 00 00  (0,0)      - NW corner of frame at 0, 0
32D:   90 01 90 01  (400,400)  - Frame width and height: 400 x 400
331:   00                      - no local color table; no interlace
332:   08           8         LZW min code size
333:   FF           255       - 255 bytes of LZW encoded image data follow
334:                data
433:   FF           255       - 255 bytes of LZW encoded image data follow
data
:
92BA:  00                    - end of LZW data for this frame
92BB:  21 F9                 Graphic Control Extension for frame #2
:                                                            :
153B7B:21 F9                 Graphic Control Extension for frame #44
:
15CF35:3B                    File terminator
 *
 */
