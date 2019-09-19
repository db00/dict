/**
 * @file sprite.h
e:\Documents\wxDevCppP7\App\DevCpp\MinGW32\bin\gcc -Wall -I"." files.c mystring.c myregex.c -I"../SDL2/include/" -D GLchar=char sprite.c matrix.c array.c regex.c -lGLESv2 -lmingw32 -lSDL2main -lSDL2 -Ddebug_sprite && a
e:\Documents\wxDevCppP7\App\DevCpp\MinGW32\bin\gcc -Wall -I"." files.c mystring.c myregex.c -I"../SDL2/include/" sprite.c matrix.c array.c regex.c -lopengl32 -lmingw32 -lSDL2main -lSDL2 -Ddebug_sprite && a
gcc -Wall -I"../SDL2/include/" array.c sprite.c matrix.c files.c mystring.c myregex.c -lSDL2 -lm -Ddebug_sprite && ./a.out
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-07-21
 */
#ifndef sprite_h
#define sprite_h

#include "SDL.h"
#include "SDL_platform.h"
#include "matrix.h"
#include "array.h"

//#define HAVE_OPENGL
#if defined(__MACOS__) || ( !defined(__IPHONEOS__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__) && !defined(__NACL__)) && !defined(linux) && !defined(__WIN32__) && !defined(__WIN64__)
#ifndef HAVE_OPENGL
#define HAVE_OPENGL
#endif
#endif

#ifndef HAVE_OPENGL
#include "SDL_opengles2.h"
#else
#include "SDL_opengl.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif


typedef struct GLES2_Context
{
#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
#ifndef HAVE_OPENGL
#include "SDL_gles2funcs.h"
#else
#include "SDL_glfuncs.h"
#endif
#undef SDL_PROC
} GLES2_Context;


#define GL_CHECK(x) \
	x; \
{ \
	GLenum glError = gles2.glGetError(); \
	if(glError != GL_NO_ERROR) { \
		SDL_Log("glGetError() = %i (0x%.8x) at line %i\t", glError, glError, __LINE__); \
		quit(1);\
	} \
}

//int LoadContext(GLES2_Context * data);

typedef struct SpriteEvent{
	Uint32 type;
	SDL_Event *e;//对应SDL_Event
	void(*func)(struct SpriteEvent*);//
	struct Sprite*target;
	unsigned int lastEventTime;//ensure not dispatch only once
}SpriteEvent;

typedef struct Point3d{
	int x;
	int y;
	int z;
	float scaleX;
	float scaleY;
	float scaleZ;
	int rotationX;
	int rotationY;
	int rotationZ;
}Point3d;

typedef struct Data3d
{//默认3d结构体
	GLuint vboIds[2];
	GLuint programObject;
	GLint  positionLoc;
	GLint  ambientLoc;//光照强度
	GLint  lightDirection;//光照position
	GLint  normalLoc;
	GLint  texCoordLoc;
	GLint  samplerLoc;
	GLint  mvpLoc;
	GLint  alphaLoc;

	int      numIndices;
	int      numVertices;
	GLfloat *vertices;
	GLfloat *normals;
	GLfloat *texCoords;
	GLuint  *indices;
} Data3d;

typedef struct World3d{//
	float fovy;
	float aspect;
	float nearZ;
	float farZ;
	Matrix3D perspective;
} World3d;

typedef struct Sprite{
	int is3D;//1:is3D,0:2d;
	char* name;
	int visible;
	SDL_Rect * Bounds;//鼠标响应区,(窗口坐标)
	SDL_Rect * mask;//遮罩区域，(窗口坐标)
	SDL_Surface * surface;//
	SDL_Texture * texture;//显示信息
	SDL_Rect * dragRect;//非null可拖拽,(窗口坐标)
	SDL_bool canDrag;//可拖拽
	struct Sprite*parent;//显示在舞台的父节点
	Array *children;//Sprite Array 子节点数组;
	SDL_bool mouseChildren;//鼠标可点击到子节点

	Array *events;//SpriteEvent Array;添加的事件


	//gl data
	Matrix3D *globeToLocalMatrix;
	Matrix3D *localToGlobeMatrix;
	Matrix3D  mvpMatrix;

	GLuint textureId;
	//GLuint renderbuffer;
	//GLuint framebuffer;

	void *data3d;//默认是 Data3d 结构体
	void (*showFunc)(struct Sprite*);// 其他3d显示
	void (*destroyFunc)(struct Sprite*);// 其他3d销毁
	GLfloat ambient[4];//光照强度
#ifdef __IPHONEOS__
	GLfloat *texCoords;//
#endif

	int filter;

	void * tween;//动画效果
	void (*Tween_kill)(void*,int);//动画效果清除,arguments[0]:tween,arguments[1]:toEnd

	int stopPropagation;//防止对事件流中当前节点的后续节点中的所有事件侦听器进行处理。

	int x;//(窗口坐标)
	int y;//(窗口坐标)
	int z;//(窗口坐标)

	int w;//(窗口坐标)
	int h;//(窗口坐标)

	GLfloat alpha;

	float scaleX;
	float scaleY;
	float scaleZ;

	int rotationX;
	int rotationY;
	int rotationZ;

	void * obj;//继承属性
	void * other;//其他属性
} Sprite;

typedef struct Stage{
	Sprite * currentTarget;
	SDL_GLContext * GLEScontext;
	SDL_Renderer * renderer;//renderer
	SDL_Window* window;//
	Sprite* focus;//舞台焦点
	Sprite* sprite;//舞台焦点
	SDL_Point*mouse;//鼠标位置
	int stage_w;
	int stage_h;
	unsigned int lastEventTime;
	unsigned int numsprite;//
	World3d *world;
	void * sound;
	GLfloat ambient[4];//光照强度
	GLfloat lightDirection[3];//光照方向
}Stage;

typedef void (*EventFunc)(SpriteEvent*); 

Data3d * Data3D_init();
Data3d * Data3D_new();
void Data3d_reset(Data3d * data3D);
void Data3d_set(Data3d * data3D,Data3d * _data3D);


extern Stage *stage;
Stage * Stage_init();
void Stage_loopEvents();

extern GLES2_Context gles2;

Sprite * Sprite_new();
float Sprite_getAlpha(Sprite * sprite);
Point3d *Sprite_localToGlobal(Sprite*sprite,Point3d *p);
Point3d *Sprite_GlobalToLocal(Sprite*sprite,Point3d *p);
Sprite*Sprite_removeChild(Sprite*parent,Sprite*sprite);
Sprite * Sprite_getChildByIndex(Sprite * sprite,int index);
int Sprite_getChildIndex(Sprite*parent,Sprite*sprite);
SDL_bool Sprite_contains(Sprite*parent,Sprite*sprite);
Sprite* Sprite_getChildByName(Sprite*sprite,const char*name);
int Sprite_dispatchEvent(Sprite*sprite,const SDL_Event *event);
int Sprite_addEventListener(Sprite*sprite,Uint32 type,EventFunc func);
int Sprite_removeEventListener(Sprite*sprite,Uint32 type,EventFunc func);
void Sprite_removeEvents(Sprite * sprite);
Sprite*Sprite_addChildAt(Sprite*parent,Sprite*sprite,int index);
Sprite * Sprite_addChild(Sprite*parent,Sprite*sprite);
Sprite* getSpriteByStagePoint(int x,int y);
Sprite*Sprite_removeChildAt(Sprite*sprite,int index);
int Sprite_destroy(Sprite*sprite);

void Sprite_fullcenter(Sprite*sprite,int x,int y,int w,int h);//sprite在矩形中充满居中
void Sprite_fullcenterRect(Sprite*sprite,SDL_Rect*rect);//sprite在矩形中充满居中
void Sprite_center(Sprite*sprite,int x,int y,int w,int h);//sprite在矩形中居中
void Sprite_centerRect(Sprite*sprite,SDL_Rect*rect);//sprite在矩形中居中

int Sprite_removeChildren(Sprite*sprite);
int Stage_redraw();
int Sprite_limitPosion(Sprite*target,SDL_Rect*rect);
SDL_Rect * Sprite_getBorder(Sprite*sprite,SDL_Rect*rect);

void Sprite_destroySurface(Sprite*sprite);
void Sprite_destroyTexture(Sprite*sprite);

SDL_Color * uintColor(Uint32 _color);

//Sprite * Sprite_newText(char *s,int fontSize,Uint32 fontColor,Uint32 bgColor);//new 一行文本生成一个sprite
//void Sprite_alertText(char * s);//弹出一行警告文本
//SDL_Surface * Httploader_loadimg(char * url);

//extern void Sprite_show3d(Sprite*sprite);//3d 必须实现
float xto3d(int x);
float yto3d(int y);
float zto3d(int z);
float wto3d(int x);
float hto3d(int x);
int wfrom3d(float w);
int hfrom3d(float h);
int xfrom3d(float x);
int yfrom3d(float y);
int zfrom3d(float z);

SDL_UserEvent*UserEvent_new(Uint32 type,Sint32 code,void*data1,void*data2);
void UserEvent_clear(SDL_UserEvent * event);

GLuint LoadShader(GLenum type, GLbyte * shaderSrc);

void Sprite_translate(Sprite*sprite,int _x,int _y,int _z);
void Sprite_rotate(Sprite*sprite,int _rotationX,int _rotationY,int _rotationZ);
void Sprite_scale(Sprite*sprite,float scaleX,float scaleY,float scaleZ);

GLuint SDL_GL_LoadTexture(Sprite * sprite, GLfloat * texcoord);
int Sprite_getTextureId(Sprite * sprite);
SDL_Surface * Surface_new(int width,int height);
SDL_Surface * RGBA_surface(SDL_Surface * surface);
SDL_Surface * Stage_readpixel(Sprite *stage,SDL_Rect* rect);
void Sprite_setSurface(Sprite*sprite,SDL_Surface * surface);
void quit(int rc);
void Sprite_matrix(Sprite *sprite);
int Window_resize(int w,int h);
int Sprite_getVisible(Sprite*sprite);
void Sprite_stopPropagation(Sprite * target);
GLuint esLoadProgram ( GLbyte * vertShaderSrc, GLbyte *fragShaderSrc );
#endif
