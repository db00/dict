/**
 * @file myttf.h
 gcc -Wall -g -I"/usr/include/SDL2/" utf8.c array.c tween.c ease.c textfield.c matrix.c sprite.c sdlstring.c mystring.c myregex.c files.c -lSDL2_ttf -lSDL2 -lm -D debugtext && ./a.out
 gcc -Wall -g -I"../SDL2/include/" -I"../SDL2_image/" -I"../SDL2_ttf/" array.c tween.c ease.c textfield.c matrix.c sprite.c sdlstring.c mystring.c files.c -lSDL2_ttf -lmingw32 -lSDL2main -lSDL2 -D debugtext && a
 gcc -Wall -I"../SDL2/include/" -I"../SDL2_image/" -I"../SDL2_ttf/" array.c tween.c ease.c httploader.c ipstring.c base64.c sdlstring.c textfield.c matrix.c sprite.c mystring.c files.c -lSDL2_image -lSDL2_ttf -lm -lSDL2 -D debugtext -lssl -lcrypto && ./a.out
 *  
 * 	6217 0029 5010 1015 186
 *	1917086952
 *	6902577682
 *	5809216
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-07-21
 */
#ifndef _MY_TTF_H
#define _MY_TTF_H

#include "myfont.h"


#ifndef max
#define max(x,y) ((x)<(y))?(y):(x)
#endif


//单个字符
typedef struct TextWord{
	int numbyte;//字节数
	int w;
	int h;
}TextWord;

typedef struct TextLine{
	int lineId;//行号
	int indexInText;//文本块的开始字符
	int numbyte;//文本行字节数
	char * text;//文本开始位置指针
	SDL_Rect rect;//显示图像大小位置

	Array * words;//Array of TextWord
	//TextWord * lastWord;//结尾字符
	//struct TextLine * next;
}TextLine;


typedef struct TextField{
	//TextFormat * format;//字体
	TTF_Font * font;
	Array * textLines;//Array of TextLine
	//TextLine * lines;//
	//TextLine * lastLine;//最后一行
	//int numLines ;// [只读] 定义多行文本字段中的文本行数。
	char*text ;// 作为文本字段中当前文本的字符串。
	SDL_Color * textColor ;// 文本字段中文本的颜色（采用十六进制格式）。
	SDL_Color * backgroundColor; // 文本字段背景的颜色。
	SDL_Color * borderColor ;// 文本字段边框的颜色。
	int textHeight ;// [只读] 文本的高度，以像素为单位。
	int textWidth ;// [只读] 文本的宽度，以像素为单位。
	int mouseWheelEnabled ;// 一个布尔值，表示当用户单击某个文本字段并滚动鼠标滚轮时，Flash Player 是否自动滚动多行文本字段。
	Sprite * sprite;
	int length ;// [只读] 文本字段中的字符数。
	int x;
	int y;
	int w;
	int h;
	SDL_bool staticHeight;//true为固定高度，false根据文本内容变化的动态高度
	int scrollV ;// 文本在文本字段中的垂直位置。
	//SDL_mutex * mutex;
	float posRate;//文本当前显示的位置，值域:[0,1]
	Sprite * posSprite;//文本当前显示的位置，值域:[0,1]

	void (* wordSelect)(char *);//
	/*
	   int maxScrollV ;// [只读] scrollV 的最大值。

	   int selectable ;// 一个布尔值，表示文本字段是否可选。
	   int wordWrap ;// 一个布尔值，表示文本字段是否自动换行。
	   int autoSize;// 控制文本字段的自动大小调整和对齐。
	   int multiline ;// 表示字段是否为多行文本字段。
	   int alwaysShowSelection; // 如果设置为 true 且文本字段没有焦点，Flash Player 将以灰色突出显示文本字段中的所选内容。
	   int defaultTextFormat ;// 指定应用于新插入文本（例如，用户输入的文本或使用 replaceSelectedText() 方法插入的文本）的格式。
	   int antiAliasType; // 用于此文本字段的消除锯齿类型。
	   char*htmlText;// 包含文本字段内容的 HTML 表示形式。
	   int bottomScrollV;// [只读] 一个整数（从 1 开始的索引），表示指定文本字段中当前可以看到的最后一行。
	   int caretIndex ;// [只读] 插入点（尖号）位置的索引。
	   int condenseWhite ;// 一个布尔值，指定是否删除具有 HTML 文本的文本字段中的额外空白（空格、换行符等等）。
	   int displayAsPassword ;// 指定文本字段是否是密码文本字段。
	   int embedFonts;// 指定是否使用嵌入字体轮廓进行呈现。
	   int gridFitType;// 用于此文本字段的网格固定类型。
	   int maxChars ;// 文本字段中最多可包含的字符数（即用户输入的字符数）。
	   int maxScrollH ;// [只读] scrollH 的最大值。
	   int restricts;// 表示用户可输入到文本字段中的字符集。
	   int scrollH ;// 当前水平滚动位置。
	   int selectionBeginIndex ;// [只读] 当前所选内容中第一个字符从零开始的字符索引值。
	   int selectionEndIndex ;// [只读] 当前所选内容中最后一个字符从零开始的字符索引值。
	   int sharpness ;// 此文本字段中字型边缘的清晰度。
	   int styleSheet ;// 将样式表附加到文本字段。
	   int textInteractionMode ;// [只读] 交互模式属性，默认值为 TextInteractionMode.NORMAL。
	   int thickness ;// 此文本字段中字型边缘的粗细。
	   int type ;// 文本字段的类型。
	   int useRichTextClipboard ;// 指定在复制和粘贴文本时是否同时复制和粘贴其格式。
	   */
}TextField;
char * TextField_getWordAtPoint(TextField * textfield, int x, int y);// 在 x 和 y 参数指定的位置返回从零开始的字符索引值。
char * TextField_getLineText(TextField * textfield,int lineIndex);// 返回 lineIndex 参数指定的行的文本。
int TextField_getLineIndexAtPoint(TextField * textfield,int x, int y);// 在 x 和 y 参数指定的位置返回从零开始的行索引值。
/*
 *	
 int TextField_getLineIndexOfChar(int charIndex);// 返回 charIndex 参数指定的字符所在的行的索引值（从零开始）。
 getCharBoundaries(charIndex:int):Rectangle// 返回一个矩形，该矩形是字符的边框。
 getCharIndexAtPoint(x:Number, y:Number):int// 在 x 和 y 参数指定的位置返回从零开始的字符索引值。
 getFirstCharInParagraph(charIndex:int):int// 如果给定一个字符索引，则返回同一段落中第一个字符的索引。
 getImageReference(id:String):DisplayObject// 返回给定 id 或已使用 <img> 标签添加到 HTML 格式文本字段中的图像或 SWF 文件的 DisplayObject 引用。
 getLineLength(lineIndex:int):int// 返回特定文本行中的字符数。
 getLineMetrics(lineIndex:int):flash.text:TextLineMetrics// 返回给定文本行的度量信息。
 getLineOffset(lineIndex:int):int// 返回 lineIndex 参数指定的行中第一个字符的字符索引。
 getParagraphLength(charIndex:int):int// 如果给定一个字符索引，则返回包含给定字符的段落的长度。
 getTextFormat(beginIndex:int = -1, endIndex:int = -1):flash.text:TextFormat// 返回 TextFormat 对象，其中包含 beginIndex 和 endIndex 参数指定的文本范围的格式信息。
 isFontCompatible(fontName:String, fontStyle:String):Boolean// [静态] 如果具有指定的 fontName 和 fontStyle（其中的 Font.fontType 为 flash.text.FontType.EMBEDDED）的嵌入字体可用，则将返回 true。
 replaceSelectedText(value:String):void// 使用 value 参数的内容替换当前所选内容。
 replaceText(beginIndex:int, endIndex:int, newText:String):void// 将 beginIndex 和 endIndex 参数指定的字符范围替换为 newText 参数的内容。
 setSelection(beginIndex:int, endIndex:int):void// 将第一个字符和最后一个字符的索引值（使用 beginIndex 和 endIndex 参数指定）指定的文本设置为所选内容。
 setTextFormat(format:flash.text:TextFormat, beginIndex:int = -1, endIndex:int = -1):void// 将 format 参数指定的文本格式应用于文本字段中的指定文本。
 */




TextField * TextField_appendText(TextField*textfield,char *s);// 将字符串追加到文本字段的文本的末尾。
TextField * TextField_setText(TextField*textfield,char *s);

TextField * TextField_new();
TextLine * TextLine_new();
TextWord * Textword_new(TTF_Font*font,Uint8*);
void TextField_clear();
void TextLine_clear();
void TextWord_clear();
void TextField_setScrollV(TextField* textfield,int i);
int TextField_getMaxScrollV(TextField *textfield);


#endif
