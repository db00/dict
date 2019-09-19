/*
 *
 gcc -Wall MipMap2D.c -lm -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/"  -lSDL2 -lSDL2_ttf && ./a.out
 gcc earth.c  -L. -lGLESv2 -lmingw32 -lSDL2_test -lSDL2main -lSDL2 -lSDL2_ttf && a 
 gcc Simple_TextureCubemap.c  -lSDL2_test -lSDL2 -lSDL2_ttf -lm && ./a.out &&
 */
#ifndef __GLES2BASE__
#define __GLES2BASE__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "SDL_opengles2.h"
#include "SDL_test_common.h"
#include "SDL.h"
#define PI 3.1415926535897932384626433832795f
#define NUM_PARTICLES	1000
#define PARTICLE_SIZE   7
typedef struct GLES2_Context
{
#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
#include "SDL_gles2funcs.h"
#undef SDL_PROC
} GLES2_Context;
typedef struct
{
	const char * videodriver;
	int window_w;
	int window_h;
	int num_windows;
	SDL_Window **windows;
} CommonState;
CommonState *state;

static SDL_GLContext *context = NULL;
GLES2_Context gles2;
static int LoadContext(GLES2_Context * data)
{
#if SDL_VIDEO_DRIVER_UIKIT
#define __SDL_NOGETPROCADDR__
#elif SDL_VIDEO_DRIVER_ANDROID
#define __SDL_NOGETPROCADDR__
#elif SDL_VIDEO_DRIVER_PANDORA
#define __SDL_NOGETPROCADDR__
#endif
	//#if defined __SDL_NOGETPROCADDR__
	//#define SDL_PROC(ret,func,params) data->func=func;
	//#else
#define SDL_PROC(ret,func,params) \
	do { \
		data->func = SDL_GL_GetProcAddress(#func); \
		if ( ! data->func ) { \
			SDL_Log("Couldn't load GLES2 function %s: %s\n", #func, SDL_GetError()); \
			return SDL_SetError("Couldn't load GLES2 function %s: %s\n", #func, SDL_GetError()); \
		} \
	} while ( 0 );
	//#endif /* _SDL_NOGETPROCADDR_ */
#include "SDL_gles2funcs.h"
#undef SDL_PROC
	return 0;
}
#define GL_CHECK(x) \
	x; \
{ \
	GLenum glError = gles2.glGetError(); \
	if(glError != GL_NO_ERROR) { \
		SDL_Log("glGetError() = %i (0x%.8x) at line %i\n", glError, glError, __LINE__); \
		quit(1); \
	} \
}


typedef struct {
	GLfloat   m[4][4];
} ESMatrix;
typedef struct UserData
{
	// Uniform locations
	GLint  mvpLoc;
	// Vertex daata
	// Rotation angle
	GLfloat   angle;
	// MVP matrix
	ESMatrix  mvpMatrix;

	GLuint shader_frag, shader_vert;
	GLint  normalLoc;
	int      numIndices;
	GLfloat *vertices;
	GLfloat *normals;
	GLfloat *texCoords;
	GLuint  *indices;
	// Attribute locations
	GLint baseMapLoc;
	GLint lightMapLoc;
	// Texture handle
	GLuint baseMapTexId;
	GLuint lightMapTexId;
	// Handle to a program object
	GLuint programObject;
	// Attribute locations
	GLint  positionLoc;
	GLint  texCoordLoc;
	// Sampler location
	GLint samplerLoc;
	// Attribute locations
	GLint  lifetimeLoc;
	GLint  startPositionLoc;
	GLint  endPositionLoc;
	// Offset location
	GLint offsetLoc;
	GLint timeLoc;
	GLint colorLoc;
	GLint centerPositionLoc;
	// Texture handle
	GLuint textureId;
	float particleData[ NUM_PARTICLES * PARTICLE_SIZE ];
	float time;
} UserData;
void Draw ( UserData * userData);
int Init ( UserData * userData);
GLuint LoadShader(GLenum type, const char *shaderSrc)
{
	GLuint shader; 
	GLint compiled; 
	// Create the shader object 
	shader = gles2.glCreateShader(type); 
	if(shader == 0) 
		return 0; 
	// Load the shader source 
	gles2.glShaderSource(shader, 1, &shaderSrc, NULL); 
	// Compile the shader 
	gles2.glCompileShader(shader); 
	// Check the compile status 
	gles2.glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled); 
	if(!compiled) 
	{ 
		GLint infoLen = 0; 
		gles2.glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen); 
		if(infoLen > 1) 
		{ 
			char* infoLog = malloc(sizeof(char) * infoLen); 
			gles2.glGetShaderInfoLog(shader, infoLen, NULL, infoLog); 
			SDL_Log("Error compiling shader:\n%s\n", infoLog); 
			free(infoLog); 
		} 
		gles2.glDeleteShader(shader); 
		return 0; 
	} 
	return shader; 
}
int power_of_two(int input)
{
	/*return input;*/
	int value = 1;

	while (value < input) {
		value <<= 1;
	}
	return value;
} 
void quit(int rc)
{
	int i;
	if (context != NULL) {
		for (i = 0; i < state->num_windows; i++) {
			if (context[i]) {
				SDL_GL_DeleteContext(context[i]);
			}
		}
		SDL_free(context);
	}
	exit(rc);
}
/**
*/
SDL_Surface *get_font(int ptsize,const char*message,SDL_Color*forecolor)
{
	const char*ttf_file = "C:/WINDOWS/Fonts/simkai.ttf";
#if defined(__ANDROID__)
	ttf_file ="/system/fonts/DroidSansFallback.ttf";	
#elif defined(linux)
	ttf_file ="/usr/share/fonts/adobe-source-han-sans-cn/SourceHanSansCN-Light.otf";	
#endif
	if ( TTF_Init() < 0 ) {
		fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
		SDL_Quit();
		return NULL;
	}
	TTF_Font * font = TTF_OpenFont(ttf_file, ptsize);
	int renderstyle = TTF_STYLE_NORMAL;
	TTF_SetFontStyle(font, renderstyle);
	SDL_Color fcolor = {0xff,0xff,0xff,0};
	SDL_Surface* text;
	text = TTF_RenderUTF8_Shaded(font, message, *forecolor,fcolor);
	SDL_Log("font height:%d,",TTF_FontHeight(font));
	TTF_CloseFont(font);
	return text;
}
Uint32 _then=0;
int wait_per_frame()
{
	Uint32 fps = 24;
	Uint32 _now = SDL_GetTicks();
	//printf("now: %d,",_now);
	int waitms = _then + 1000.0f/fps - _now;
	if(waitms >0){
		//printf("wait %d\n",waitms);
		SDL_Delay(waitms);
	}
	_then = _now;
	return 0;
}

static void initGL()
{
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);//模板测试
	SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#if !defined(__MACOS__)
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);//or -1
#endif
#ifndef HAVE_OPENGL
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);//
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);//or 1
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);//or 1
}

int main(int argc, char *argv[])
{
	SDL_DisplayMode mode;
	UserData *datas;
	/* Initialize test framework */
	//state = CommonCreateState(argv, SDL_INIT_VIDEO);
	state = (CommonState *)SDL_calloc(1, sizeof(*state));
	state->num_windows = 1;
	if (!state) {
		return 1;
	}
	int i;
	initGL();
	state->windows = (SDL_Window **) SDL_malloc(state->num_windows * sizeof(*state->windows));
	for (i = 0; i < state->num_windows; ++i) {
		state->windows[i] = SDL_CreateWindow("title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS);
		if (!state->windows[i]) {
			fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
			return SDL_FALSE;
		}
		SDL_ShowWindow(state->windows[i]);
	}
	context = SDL_calloc(state->num_windows, sizeof(context));
	if (context == NULL) {
		SDL_Log("Out of memory!\n");
		quit(2);
	}
	/* Create OpenGL ES contexts */
	for (i = 0; i < state->num_windows; i++) {
		context[i] = SDL_GL_CreateContext(state->windows[i]);
		if (!context[i]) {
			SDL_Log("SDL_GL_CreateContext(): %s\n", SDL_GetError());
			quit(2);
		}
	}
	/* Important: call this *after* creating the context */
	if(gles2.glGetError==NULL)
		if (LoadContext(&gles2) < 0) {
			SDL_Log("Could not load GLES2 functions\n");
			quit(2);
		}
	//SDL_GL_SetSwapInterval(1);
	SDL_GL_SetSwapInterval(0);
	SDL_GetCurrentDisplayMode(0, &mode);
	state->window_h = mode.h;
	state->window_w = mode.w;
	datas = SDL_calloc(state->num_windows, sizeof(UserData));
	int status;
	/* Set rendering settings for each context */
	for (i = 0; i < state->num_windows; ++i) {
		status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
		if (status) {
			SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
			/* Continue for next window */
			continue;
		}
		Init(&datas[i]);
	}
	/* Main render loop */
	Uint32 then, now, frames;
	SDL_Event event;
	frames = 0;
	then = SDL_GetTicks();
	int done;
	done = 0;
	_then = then;
	while (!done) {
		/* Check for events */
		++frames;
		while (SDL_PollEvent(&event) && !done) {
			switch (event.type) {
				case SDL_WINDOWEVENT:
					switch (event.window.event) {
						case SDL_WINDOWEVENT_RESIZED:
							for (i = 0; i < state->num_windows; ++i) {
								if (event.window.windowID == SDL_GetWindowID(state->windows[i])) {
									status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
									if (status) {
										SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
										break;
									}
									/* Change view port to the new window dimensions */
									state->window_w = event.window.data1;
									state->window_h = event.window.data2;
									/* Update window content */
									Draw(&datas[i]); SDL_GL_SwapWindow(state->windows[i]);
									break;
								}
							}
							break;
					}
				case SDL_KEYDOWN: 
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							done = 1;
							break;
					}
			}
		}
		if (!done) {
			for (i = 0; i < state->num_windows; ++i) {
				status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
				if (status) {
					SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
					/* Continue for next window */
					continue;
				}
				Draw(&datas[i]);
				SDL_GL_SwapWindow(state->windows[i]);
			}
		}
		wait_per_frame();
	}
	/* Print out some timing information */
	now = SDL_GetTicks();
	if (now > then) {
		SDL_Log("%2.2f frames per second\n",
				((double) frames * 1000) / (now - then));
	}
#if !defined(__ANDROID__)    
	quit(0);
#endif    
	return 0;
}
GLuint CreateSimpleTextureCubemap(SDL_Surface * surface)
{
	GLuint textureId;
	// Generate a texture object
	gles2.glGenTextures ( 1, &textureId );
	// Bind the texture object
	gles2.glBindTexture ( GL_TEXTURE_CUBE_MAP, textureId );

	if(surface){
		int w, h;
		SDL_Surface *image;
		SDL_Rect area;
		SDL_BlendMode saved_mode;

		/* Use the surface width and height expanded to powers of 2 */
		w = power_of_two(surface->w);
		h = power_of_two(surface->h);
		//h =w;
		image = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN     /* OpenGL RGBA masks */
				0x000000FF,
				0x0000FF00,
				0x00FF0000,
				0xFF000000
#else
				0xFF000000,
				0x00FF0000,
				0x0000FF00,
				0x000000FF
#endif
				);
		if (image == NULL) {
			SDL_Log("no image");
			return 0;
		}
		/* Save the alpha blending attributes */
		SDL_GetSurfaceBlendMode(surface, &saved_mode);
		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

		/* Copy the surface into the GL texture image */
		area.x = 0;
		area.y = 0;
		area.w = surface->w;
		area.h = surface->h;
		SDL_BlitSurface(surface, &area, image, &area);

		/* Restore the alpha blending attributes */
		SDL_SetSurfaceBlendMode(surface, saved_mode);

		/*
		 *
		 void glTexSubImage2D( GLenum   target,  
		 GLint   level,  
		 GLint   xoffset,  
		 GLint   yoffset,  
		 GLsizei   width,  
		 GLsizei   height,  
		 GLenum   format,  
		 GLenum   type,  
		 const GLvoid *   data); 

		 * glTexImage3OES(GL_TEXTURE_3D_OES, GLint level, GLenum  internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void*  pixels)
		 gles2.glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		 */
		//		 gles2.glTexImage3DOES(GL_TEXTURE_3D_OES, 0, GL_RGB, w, h, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		/*
		*/
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		/*
		   gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		   gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		   gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		   gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		   gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
		   */


		SDL_FreeSurface(image);     /* No longer needed */
	}else{
		// Six 1x1 RGB faces
		GLubyte cubePixels[6][3] = {
			// Face 0 - Red
			{255, 0, 0},
			// Face 1 - Green,
			{0, 255, 0}, 
			// Face 3 - Blue
			{0, 0, 255},
			// Face 4 - Yellow
			{255, 255, 0},
			// Face 5 - Purple
			{255, 0, 255},
			// Face 6 - White
			{255, 255, 255}
		};
		// Load the cube face - Positive X
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[0] );
		// Load the cube face - Negative X
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[1] );
		// Load the cube face - Positive Y
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[2] );
		// Load the cube face - Negative Y
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[3] );
		// Load the cube face - Positive Z
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[4] );
		// Load the cube face - Negative Z
		gles2.glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[5] );
	}
	// Set the filtering mode
	gles2.glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	gles2.glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	return textureId;
}
/**
 *
 userData->numIndices = esGenSphere ( 20, 0.75f, &userData->vertices, &userData->normals, NULL, &userData->indices );
 */
int esGenSphere ( int numSlices, float radius, GLfloat **vertices, GLfloat **normals, GLfloat **texCoords, GLuint **indices )
{
	int i;
	int j;
	int numParallels = numSlices / 2;
	int numVertices = ( numParallels + 1 ) * ( numSlices + 1 );
	int numIndices = numParallels * numSlices * 6;
	float angleStep = (2.0f * PI) / ((float) numSlices);

	// Allocate memory for buffers
	if ( vertices != NULL )
		*vertices = malloc ( sizeof(GLfloat) * 3 * numVertices );

	if ( normals != NULL )
		*normals = malloc ( sizeof(GLfloat) * 3 * numVertices );

	if ( texCoords != NULL )
		*texCoords = malloc ( sizeof(GLfloat) * 2 * numVertices );

	if ( indices != NULL )
		*indices = malloc ( sizeof(GLuint) * numIndices );

	for ( i = 0; i < numParallels + 1; i++ )
	{
		for ( j = 0; j < numSlices + 1; j++ )
		{
			int vertex = ( i * (numSlices + 1) + j ) * 3; 

			if ( vertices )
			{
				(*vertices)[vertex + 0] = radius * sinf ( angleStep * (float)i ) *
					sinf ( angleStep * (float)j );
				(*vertices)[vertex + 1] = radius * cosf ( angleStep * (float)i );
				(*vertices)[vertex + 2] = radius * sinf ( angleStep * (float)i ) *
					cosf ( angleStep * (float)j );
			}

			if ( normals )
			{
				(*normals)[vertex + 0] = (*vertices)[vertex + 0] / radius;
				(*normals)[vertex + 1] = (*vertices)[vertex + 1] / radius;
				(*normals)[vertex + 2] = (*vertices)[vertex + 2] / radius;
			}

			if ( texCoords )
			{
				int texIndex = ( i * (numSlices + 1) + j ) * 2;
				/*texIndex = (numSlices+1)*(numParallels+1)*2-texIndex;*/
				(*texCoords)[texIndex + 0] = (float) j / (float) numSlices;
				(*texCoords)[texIndex + 1] = -( 1.0f - (float) i ) / (float) (numParallels - 1 );
			}
		}
	}

	// Generate the indices
	if ( indices != NULL )
	{
		GLuint *indexBuf = (*indices);
		for ( i = 0; i < numParallels ; i++ ) 
		{
			for ( j = 0; j < numSlices; j++ )
			{
				*indexBuf++  = i * ( numSlices + 1 ) + j;
				*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + j;
				*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + ( j + 1 );

				*indexBuf++ = i * ( numSlices + 1 ) + j;
				*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + ( j + 1 );
				*indexBuf++ = i * ( numSlices + 1 ) + ( j + 1 );
			}
		}
	}
	return numIndices;
}
///
//  Generate an RGB8 checkerboard image
//
GLubyte* GenCheckImage( int width, int height, int checkSize )
{
	int x,
		y;
	GLubyte *pixels = malloc( width * height * 3 );
	if ( pixels == NULL )
		return NULL;
	for ( y = 0; y < height; y++ )
		for ( x = 0; x < width; x++ )
		{
			GLubyte rColor = 0;
			GLubyte bColor = 0;
			if ( ( x / checkSize ) % 2 == 0 )
			{
				rColor = 255 * ( ( y / checkSize ) % 2 );
				bColor = 255 * ( 1 - ( ( y / checkSize ) % 2 ) );
			}
			else
			{
				bColor = 255 * ( ( y / checkSize ) % 2 );
				rColor = 255 * ( 1 - ( ( y / checkSize ) % 2 ) );
			}
			pixels[(y * height + x) * 3] = rColor;
			pixels[(y * height + x) * 3 + 1] = 0;
			pixels[(y * height + x) * 3 + 2] = bColor; 
		} 
	return pixels;
}
///
// Create a mipmapped 2D texture image 
//
GLuint CreateTexture2D( )
{
	// Texture object handle
	GLuint textureId;
	int    width = 256,
		   height = 256;
	GLubyte *pixels;
	pixels = GenCheckImage( width, height, 64 );
	if ( pixels == NULL )
		return 0;
	// Generate a texture object
	gles2.glGenTextures ( 1, &textureId );
	// Bind the texture object
	gles2.glBindTexture ( GL_TEXTURE_2D, textureId );
	// Load mipmap level 0
	gles2.glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );
	// Set the filtering mode
	gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	return textureId;
}
char* esLoadTGA ( char *fileName, int *width, int *height )
{
	char *buffer = NULL;
	FILE *f;
	unsigned char tgaheader[12];
	unsigned char attributes[6];
	unsigned int imagesize;

	f = fopen(fileName, "rb");
	if(f == NULL) return NULL;

	if(fread(&tgaheader, sizeof(tgaheader), 1, f) == 0)
	{
		fclose(f);
		return NULL;
	}

	if(fread(attributes, sizeof(attributes), 1, f) == 0)
	{
		fclose(f);
		return 0;
	}

	*width = attributes[1] * 256 + attributes[0];
	*height = attributes[3] * 256 + attributes[2];
	imagesize = attributes[4] / 8 * *width * *height;
	buffer = malloc(imagesize);
	if (buffer == NULL)
	{
		fclose(f);
		return 0;
	}

	if(fread(buffer, 1, imagesize, f) != imagesize)
	{
		free(buffer);
		return NULL;
	}
	fclose(f);
	return buffer;
}
GLuint esLoadProgram ( const char *vertShaderSrc, const char *fragShaderSrc )
{
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint programObject;
	GLint linked;
	// Load the vertex/fragment shaders
	vertexShader = LoadShader ( GL_VERTEX_SHADER, vertShaderSrc );
	if ( vertexShader == 0 )
		return 0;
	fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fragShaderSrc );
	if ( fragmentShader == 0 )
	{
		gles2.glDeleteShader( vertexShader );
		return 0;
	}
	// Create the program object
	programObject = gles2.glCreateProgram ( );
	if ( programObject == 0 )
		return 0;
	gles2.glAttachShader ( programObject, vertexShader );
	gles2.glAttachShader ( programObject, fragmentShader );
	// Link the program
	gles2.glLinkProgram ( programObject );
	// Check the link status
	gles2.glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );
	if ( !linked ) 
	{
		GLint infoLen = 0;
		gles2.glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
		if ( infoLen > 1 )
		{
			char* infoLog = malloc (sizeof(char) * infoLen );
			/*gles2.glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );*/
			SDL_Log( "Error linking program:\n%s\n", infoLog );            
			free ( infoLog );
		}
		gles2.glDeleteProgram ( programObject );
		return 0;
	}
	// Free up no longer needed shader resources
	gles2.glDeleteShader ( vertexShader );
	gles2.glDeleteShader ( fragmentShader );
	return programObject;
}
GLuint CreateSimpleTexture2D( )
{
	// Texture object handle
	GLuint textureId;

	// 2x2 Image, 3 bytes per pixel (R, G, B)
	GLubyte pixels[4 * 3] =
	{  
		255,   0,   0, // Red
		0, 255,   0, // Green
		0,   0, 255, // Blue
		255, 255,   0  // Yellow
	};

	// Use tightly packed data
	gles2.glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

	// Generate a texture object
	gles2.glGenTextures ( 1, &textureId );

	// Bind the texture object
	gles2.glBindTexture ( GL_TEXTURE_2D, textureId );

	// Load the texture
	gles2.glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );

	// Set the filtering mode
	gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	return textureId;

}
GLuint SDL_GL_LoadTexture(SDL_Surface * surface, GLfloat * texcoord)
{
	GLuint texture;
	int w, h;
	SDL_Surface *image;
	SDL_Rect area;
	SDL_BlendMode saved_mode;

	/* Use the surface width and height expanded to powers of 2 */
	w = power_of_two(surface->w);
	h = power_of_two(surface->h);
	h = w;
	if(texcoord){
		texcoord[0] = 0.0f;         /* Min X */
		texcoord[1] = 0.0f;         /* Min Y */
		texcoord[2] = (GLfloat) surface->w / w;     /* Max X */
		texcoord[3] = (GLfloat) surface->h / h;     /* Max Y */
	}

	image = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN     /* OpenGL RGBA masks */
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0xFF000000
#else
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF
#endif
			);
	if (image == NULL) {
		return 0;
	}

	/* Save the alpha blending attributes */
	SDL_GetSurfaceBlendMode(surface, &saved_mode);
	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

	/* Copy the surface into the GL texture image */
	area.x = 0;
	area.y = 0;
	area.w = surface->w;
	area.h = surface->h;
	SDL_BlitSurface(surface, &area, image, &area);

	/* Restore the alpha blending attributes */
	SDL_SetSurfaceBlendMode(surface, saved_mode);

	/* Create an OpenGL texture for the image */
	gles2.glGenTextures(1, &texture);
	gles2.glBindTexture(GL_TEXTURE_2D, texture);
	gles2.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gles2.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gles2.glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
	SDL_FreeSurface(image);     /* No longer needed */
	return texture;
}
GLuint LoadTexture ( char *fileName )
{
	int width,
		height;
	char *buffer = esLoadTGA ( fileName, &width, &height );
	GLuint texId;

	if ( buffer == NULL )
	{
		SDL_Log( "Error loading (%s) image.\n", fileName );
		return 0;
	}

	gles2.glGenTextures ( 1, &texId );
	gles2.glBindTexture ( GL_TEXTURE_2D, texId );

	// GL_TEXTURE_2D, GL_PROXY_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, or GL_PROXY_TEXTURE_CUBE_MAP. 
	gles2.glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer );
	gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	free ( buffer );

	return texId;
}
int esGenCube ( float scale, GLfloat **vertices, GLfloat **normals, GLfloat **texCoords, GLuint **indices )
{
	int i;
	int numVertices = 24;
	int numIndices = 36;

	GLfloat cubeVerts[] =
	{
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f,  0.5f, 0.5f,
		0.5f,  0.5f, 0.5f, 
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
	};

	GLfloat cubeNormals[] =
	{
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
	};

	GLfloat cubeTex[] =
	{
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
	};

	// Allocate memory for buffers
	if ( vertices != NULL )
	{
		*vertices = malloc ( sizeof(GLfloat) * 3 * numVertices );
		memcpy( *vertices, cubeVerts, sizeof( cubeVerts ) );
		for ( i = 0; i < numVertices * 3; i++ )
		{
			(*vertices)[i] *= scale;
		}
	}

	if ( normals != NULL )
	{
		*normals = malloc ( sizeof(GLfloat) * 3 * numVertices );
		memcpy( *normals, cubeNormals, sizeof( cubeNormals ) );
	}

	if ( texCoords != NULL )
	{
		*texCoords = malloc ( sizeof(GLfloat) * 2 * numVertices );
		memcpy( *texCoords, cubeTex, sizeof( cubeTex ) ) ;
	}


	// Generate the indices
	if ( indices != NULL )
	{
		GLuint cubeIndices[] =
		{
			0, 2, 1,
			0, 3, 2, 
			4, 5, 6,
			4, 6, 7,
			8, 9, 10,
			8, 10, 11, 
			12, 15, 14,
			12, 14, 13, 
			16, 17, 18,
			16, 18, 19, 
			20, 23, 22,
			20, 22, 21
		};

		*indices = malloc ( sizeof(GLuint) * numIndices );
		memcpy( *indices, cubeIndices, sizeof( cubeIndices ) );
	}

	return numIndices;
}
void esMatrixMultiply(ESMatrix *result, ESMatrix *srcA, ESMatrix *srcB)
{
	ESMatrix    tmp;
	int         i;

	for (i=0; i<4; i++)
	{
		tmp.m[i][0] =	(srcA->m[i][0] * srcB->m[0][0]) +
			(srcA->m[i][1] * srcB->m[1][0]) +
			(srcA->m[i][2] * srcB->m[2][0]) +
			(srcA->m[i][3] * srcB->m[3][0]) ;

		tmp.m[i][1] =	(srcA->m[i][0] * srcB->m[0][1]) + 
			(srcA->m[i][1] * srcB->m[1][1]) +
			(srcA->m[i][2] * srcB->m[2][1]) +
			(srcA->m[i][3] * srcB->m[3][1]) ;

		tmp.m[i][2] =	(srcA->m[i][0] * srcB->m[0][2]) + 
			(srcA->m[i][1] * srcB->m[1][2]) +
			(srcA->m[i][2] * srcB->m[2][2]) +
			(srcA->m[i][3] * srcB->m[3][2]) ;

		tmp.m[i][3] =	(srcA->m[i][0] * srcB->m[0][3]) + 
			(srcA->m[i][1] * srcB->m[1][3]) +
			(srcA->m[i][2] * srcB->m[2][3]) +
			(srcA->m[i][3] * srcB->m[3][3]) ;
	}
	memcpy(result, &tmp, sizeof(ESMatrix));
}
void esFrustum(ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ)
{
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farZ - nearZ;
	ESMatrix    frust;

	if ( (nearZ <= 0.0f) || (farZ <= 0.0f) ||
			(deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f) )
		return;

	frust.m[0][0] = 2.0f * nearZ / deltaX;
	frust.m[0][1] = frust.m[0][2] = frust.m[0][3] = 0.0f;

	frust.m[1][1] = 2.0f * nearZ / deltaY;
	frust.m[1][0] = frust.m[1][2] = frust.m[1][3] = 0.0f;

	frust.m[2][0] = (right + left) / deltaX;
	frust.m[2][1] = (top + bottom) / deltaY;
	frust.m[2][2] = -(nearZ + farZ) / deltaZ;
	frust.m[2][3] = -1.0f;

	frust.m[3][2] = -2.0f * nearZ * farZ / deltaZ;
	frust.m[3][0] = frust.m[3][1] = frust.m[3][3] = 0.0f;

	esMatrixMultiply(result, &frust, result);
}
void esPerspective(ESMatrix *result, float fovy, float aspect, float nearZ, float farZ)
{
	GLfloat frustumW, frustumH;

	frustumH = tanf( fovy / 360.0f * PI ) * nearZ;
	frustumW = frustumH * aspect;

	esFrustum( result, -frustumW, frustumW, -frustumH, frustumH, nearZ, farZ );
}
void esMatrixLoadIdentity(ESMatrix *result)
{
	memset(result, 0x0, sizeof(ESMatrix));
	result->m[0][0] = 1.0f;
	result->m[1][1] = 1.0f;
	result->m[2][2] = 1.0f;
	result->m[3][3] = 1.0f;
}
void esTranslate(ESMatrix *result, GLfloat tx, GLfloat ty, GLfloat tz)
{
	result->m[3][0] += (result->m[0][0] * tx + result->m[1][0] * ty + result->m[2][0] * tz);
	result->m[3][1] += (result->m[0][1] * tx + result->m[1][1] * ty + result->m[2][1] * tz);
	result->m[3][2] += (result->m[0][2] * tx + result->m[1][2] * ty + result->m[2][2] * tz);
	result->m[3][3] += (result->m[0][3] * tx + result->m[1][3] * ty + result->m[2][3] * tz);
}
void esRotate(ESMatrix *result, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	GLfloat sinAngle, cosAngle;
	GLfloat mag = sqrtf(x * x + y * y + z * z);

	sinAngle = sinf ( angle * PI / 180.0f );
	cosAngle = cosf ( angle * PI / 180.0f );
	if ( mag > 0.0f )
	{
		GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs;
		GLfloat oneMinusCos;
		ESMatrix rotMat;

		x /= mag;
		y /= mag;
		z /= mag;

		xx = x * x;
		yy = y * y;
		zz = z * z;
		xy = x * y;
		yz = y * z;
		zx = z * x;
		xs = x * sinAngle;
		ys = y * sinAngle;
		zs = z * sinAngle;
		oneMinusCos = 1.0f - cosAngle;

		rotMat.m[0][0] = (oneMinusCos * xx) + cosAngle;
		rotMat.m[0][1] = (oneMinusCos * xy) - zs;
		rotMat.m[0][2] = (oneMinusCos * zx) + ys;
		rotMat.m[0][3] = 0.0F; 

		rotMat.m[1][0] = (oneMinusCos * xy) + zs;
		rotMat.m[1][1] = (oneMinusCos * yy) + cosAngle;
		rotMat.m[1][2] = (oneMinusCos * yz) - xs;
		rotMat.m[1][3] = 0.0F;

		rotMat.m[2][0] = (oneMinusCos * zx) - ys;
		rotMat.m[2][1] = (oneMinusCos * yz) + xs;
		rotMat.m[2][2] = (oneMinusCos * zz) + cosAngle;
		rotMat.m[2][3] = 0.0F; 

		rotMat.m[3][0] = 0.0F;
		rotMat.m[3][1] = 0.0F;
		rotMat.m[3][2] = 0.0F;
		rotMat.m[3][3] = 1.0F;

		esMatrixMultiply( result, &rotMat, result );
	}
}

void Update ( UserData *userData , float deltaTime )
{
	ESMatrix perspective;
	ESMatrix modelview;
	float    aspect;

	// Compute a rotation angle based on time to rotate the cube
	userData->angle += ( deltaTime * 40.0f );
	if( userData->angle >= 360.0f )
		userData->angle -= 360.0f;

	// Compute the window aspect ratio
	aspect = (GLfloat) state->window_w/ (GLfloat) state->window_h;

	// Generate a perspective matrix with a 60 degree FOV
	esMatrixLoadIdentity( &perspective );
	esPerspective( &perspective, 60.0f, aspect, 1.0f, 20.0f );

	// Generate a model view matrix to rotate/translate the cube
	esMatrixLoadIdentity( &modelview );

	// Translate away from the viewer
	esTranslate( &modelview, 0.0, 0.0, -2.0 );

	// Rotate the cube
	esRotate( &modelview, userData->angle, .0, 1.0, .0 );

	// Compute the final MVP by multiplying the 
	// modevleiw and perspective matrices together
	esMatrixMultiply( &userData->mvpMatrix, &modelview, &perspective );
}
#endif
