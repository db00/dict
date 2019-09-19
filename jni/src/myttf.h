/**
 * @file myttf.h
 gcc -Wall -I"../SDL2/include/"  -I"../SDL2_ttf/" myfont.c array.c utf8.c update.c httploader.c ipstring.c urlcode.c base64.c  bytearray.c zip.c files.c myregex.c sdlstring.c myttf.c matrix.c sprite.c mystring.c  -lSDL2_ttf -lz -lssl -lcrypto -lpthread -lm -lSDL2 -D debugtext  && ./a.out
 *  
 * 	6217 0029 5010 1015 186
 *	1917086952
 *	6902577682
 *	5809216
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-07-21
 */
#ifndef _MYTTF_H
#define _MYTTF_H

#include "myfont.h"


#ifndef max
#define max(x,y) ((x)<(y))?(y):(x)
#endif




typedef struct Text{
	char * text ;// 作为文本字段中当前文本的字符串。
	int length ;// [只读] 文本字段中的字符数。
	int dealedlen;//文本块的开始字符

	TTF_Font * font;

	SDL_Color * textColor ;// 文本字段中文本的颜色（采用十六进制格式）。
	SDL_Color * backgroundColor; // 文本字段背景的颜色。
	SDL_Color * borderColor ;// 文本字段边框的颜色。

	Sprite * sprite;

	int x;
	int y;
	int w;
	int h;

	Array * lines;

	void (* wordSelect)(char *);//
}Text;
/*
 *	
 getCharBoundaries(charIndex:int):Rectangle// 返回一个矩形，该矩形是字符的边框。
 getCharIndexAtPoint(x:Number, y:Number):int// 在 x 和 y 参数指定的位置返回从零开始的字符索引值。
 getFirstCharInParagraph(charIndex:int):int// 如果给定一个字符索引，则返回同一段落中第一个字符的索引。
 getImageReference(id:String):DisplayObject// 返回给定 id 或已使用 <img> 标签添加到 HTML 格式文本字段中的图像或 SWF 文件的 DisplayObject 引用。
 getLineIndexAtPoint(x:Number, y:Number):int// 在 x 和 y 参数指定的位置返回从零开始的行索引值。
 getLineIndexOfChar(charIndex:int):int// 返回 charIndex 参数指定的字符所在的行的索引值（从零开始）。
 getLineLength(lineIndex:int):int// 返回特定文本行中的字符数。
 getLineMetrics(lineIndex:int):flash.text:TextLineMetrics// 返回给定文本行的度量信息。
 getLineOffset(lineIndex:int):int// 返回 lineIndex 参数指定的行中第一个字符的字符索引。
 getLineText(lineIndex:int):String// 返回 lineIndex 参数指定的行的文本。
 getParagraphLength(charIndex:int):int// 如果给定一个字符索引，则返回包含给定字符的段落的长度。
 getTextFormat(beginIndex:int = -1, endIndex:int = -1):flash.text:TextFormat// 返回 TextFormat 对象，其中包含 beginIndex 和 endIndex 参数指定的文本范围的格式信息。
 isFontCompatible(fontName:String, fontStyle:String):Boolean// [静态] 如果具有指定的 fontName 和 fontStyle（其中的 Font.fontType 为 flash.text.FontType.EMBEDDED）的嵌入字体可用，则将返回 true。
 replaceSelectedText(value:String):void// 使用 value 参数的内容替换当前所选内容。
 replaceText(beginIndex:int, endIndex:int, newText:String):void// 将 beginIndex 和 endIndex 参数指定的字符范围替换为 newText 参数的内容。
 setSelection(beginIndex:int, endIndex:int):void// 将第一个字符和最后一个字符的索引值（使用 beginIndex 和 endIndex 参数指定）指定的文本设置为所选内容。
 setTextFormat(format:flash.text:TextFormat, beginIndex:int = -1, endIndex:int = -1):void// 将 format 参数指定的文本格式应用于文本字段中的指定文本。
 */




Text * Text_appendText(Text * textfield,char *s);// 将字符串追加到文本字段的文本的末尾。
Text * Text_setText(Text * textfield,char *s);

Text * Text_new();
void Text_clear();


#endif
