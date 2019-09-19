/*
   Copyright (r) 1997-2014 Sam Lantinga <slouken@libsdl.org>
   gcc -Wall gles2.c -I"../SDL2/include/" -lSDL2  -lm  && ./a.out 
   gcc -Wall gles2.c -I"../SDL2/include/" -lmingw32 -lSDL2main -lSDL2 -lopengl32 && a
   gcc -Wall gles2.c ../SDL2/src/test/SDL_test_common.c -I"../SDL2/include/" -lSDL2  -lm -lGLESv2 && ./a.out
   gcc -Wall gles2.c -I"../SDL2/include/" -lSDL2  -lm -lGLESv2 && ./a.out
   gcc -Wall gles2.c -I"../SDL2/include/" -lSDL2 -lm -lGLESv2 && ./a.out

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely.
   */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined(__IPHONEOS__) && defined(__ANDROID__) && defined(__EMSCRIPTEN__) && defined(__NACL__) //&& !linux
#include "SDL_opengles2.h"
#else
#include "SDL_opengl.h"
#endif
#include "SDL.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

typedef struct shader_data
{
	GLuint shader_program, shader_frag, shader_vert;

	GLint attr_position;
	GLint attr_color, attr_mvp;

	int angle_x, angle_y, angle_z;

} shader_data;

typedef struct GLES2_Context
{
#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
#include "SDL_gles2funcs.h"
#undef SDL_PROC
} GLES2_Context;

GLES2_Context gles2;
typedef struct Sprite{
	SDL_GLContext context;
	shader_data datas;
	int w;
	int h;
	SDL_Window *window;
}Sprite;

Sprite * stage;

static int LoadContext(GLES2_Context * data)
{
#if SDL_VIDEO_DRIVER_UIKIT
#define __SDL_NOGETPROCADDR__
#elif SDL_VIDEO_DRIVER_ANDROID
#define __SDL_NOGETPROCADDR__
#elif SDL_VIDEO_DRIVER_PANDORA
#define __SDL_NOGETPROCADDR__
#endif

#if defined __SDL_NOGETPROCADDR__
#define SDL_PROC(ret,func,params) data->func=func;
#else
#define SDL_PROC(ret,func,params) \
	do { \
		data->func = SDL_GL_GetProcAddress(#func); \
		if ( ! data->func ) { \
			return SDL_SetError("Couldn't load GLES2 function %s: %s\n", #func, SDL_GetError()); \
		} \
	} while ( 0 );
#endif /* _SDL_NOGETPROCADDR_ */

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


/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void quit(int rc)
{/*{{{*/
	if (stage->context != NULL) {
		if (stage->context) {
			SDL_GL_DeleteContext(stage->context);
		}
		//SDL_free(stage->context);
	}

	SDL_VideoQuit();
	SDL_Quit();
	exit(rc);
}/*}}}*/
/* 
 * Simulates desktop's glRotatef. The matrix is returned in column-major 
 * order. 
 */
static void rotate_matrix(float angle, float x, float y, float z, float *r)
{/*{{{*/
	float radians, c, s, c1, u[3], length;
	int i, j;

	radians = (float)(angle * M_PI) / 180.0f;

	c = SDL_cosf(radians);
	s = SDL_sinf(radians);

	c1 = 1.0f - SDL_cosf(radians);

	length = (float)SDL_sqrt(x * x + y * y + z * z);

	u[0] = x / length;
	u[1] = y / length;
	u[2] = z / length;

	for (i = 0; i < 16; i++) {
		r[i] = 0.0;
	}

	r[15] = 1.0;

	for (i = 0; i < 3; i++) {
		r[i * 4 + (i + 1) % 3] = u[(i + 2) % 3] * s;
		r[i * 4 + (i + 2) % 3] = -u[(i + 1) % 3] * s;
	}

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			r[i * 4 + j] += c1 * u[i] * u[j] + (i == j ? c : 0.0f);
		}
	}
}/*}}}*/

/* 
 * Simulates gluPerspectiveMatrix 
 */
static void perspective_matrix(float fovy, float aspect, float znear, float zfar, float *r)
{/*{{{*/
	int i;
	float f;

	//f = 1.0f/SDL_tanf(fovy * 0.5f);
	f = 1.0f/tan(fovy * 0.5f);

	for (i = 0; i < 16; i++) {
		r[i] = 0.0;
	}

	r[0] = f / aspect;
	r[5] = f;
	r[10] = (znear + zfar) / (znear - zfar);
	r[11] = -1.0f;
	r[14] = (2.0f * znear * zfar) / (znear - zfar);
	r[15] = 0.0f;
}/*}}}*/

/* 
 * Multiplies lhs by rhs and writes out to r. All matrices are 4x4 and column
 * major. In-place multiplication is supported.
 */
static void multiply_matrix(float *lhs, float *rhs, float *r)
{/*{{{*/
	int i, j, k;
	float tmp[16];

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			tmp[j * 4 + i] = 0.0;

			for (k = 0; k < 4; k++) {
				tmp[j * 4 + i] += lhs[k * 4 + i] * rhs[j * 4 + k];
			}
		}
	}

	for (i = 0; i < 16; i++) {
		r[i] = tmp[i];
	}
}/*}}}*/

/* 
 * Create shader, load in source, compile, dump debug as necessary.
 *
 * shader: Pointer to return created shader ID.
 * source: Passed-in shader source code.
 * shader_type: Passed to GL, e.g. GL_VERTEX_SHADER.
 */
void process_shader(GLuint *shader, const char * source, GLint shader_type)
{
	GLint status = GL_FALSE;
	const char *shaders[1] = { NULL };
	char buffer[1024];
	GLsizei length;

	/* Create shader and load into GL. */
	*shader = GL_CHECK(gles2.glCreateShader(shader_type));

	shaders[0] = source;

	GL_CHECK(gles2.glShaderSource(*shader, 1, shaders, NULL));

	/* Clean up shader source. */
	shaders[0] = NULL;

	/* Try compiling the shader. */
	GL_CHECK(gles2.glCompileShader(*shader));
	GL_CHECK(gles2.glGetShaderiv(*shader, GL_COMPILE_STATUS, &status));

	/* Dump debug info (source and log) if compilation failed. */
	if(status != GL_TRUE) {
		gles2.glGetProgramInfoLog(*shader, sizeof(buffer), &length, &buffer[0]);
		buffer[length] = '\0';
		SDL_Log("Shader compilation failed: %s", buffer);fflush(stderr);
		quit(-1);
	}
}

/* 3D data. Vertex range -0.5..0.5 in all axes.
 * Z -0.5 is near, 0.5 is far. */
const float _vertices[] =
{
	/* Front face. */
	/* Bottom left */
	-0.5,  0.5, -0.5,
	0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,
	/* Top right */
	-0.5,  0.5, -0.5,
	0.5,  0.5, -0.5,
	0.5, -0.5, -0.5,
	/* Left face */
	/* Bottom left */
	-0.5,  0.5,  0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5,  0.5,
	/* Top right */
	-0.5,  0.5,  0.5,
	-0.5,  0.5, -0.5,
	-0.5, -0.5, -0.5,
	/* Top face */
	/* Bottom left */
	-0.5,  0.5,  0.5,
	0.5,  0.5, -0.5,
	-0.5,  0.5, -0.5,
	/* Top right */
	-0.5,  0.5,  0.5,
	0.5,  0.5,  0.5,
	0.5,  0.5, -0.5,
	/* Right face */
	/* Bottom left */
	0.5,  0.5, -0.5,
	0.5, -0.5,  0.5,
	0.5, -0.5, -0.5,
	/* Top right */
	0.5,  0.5, -0.5,
	0.5,  0.5,  0.5,
	0.5, -0.5,  0.5,
	/* Back face */
	/* Bottom left */
	0.5,  0.5,  0.5,
	-0.5, -0.5,  0.5,
	0.5, -0.5,  0.5,
	/* Top right */
	0.5,  0.5,  0.5,
	-0.5,  0.5,  0.5,
	-0.5, -0.5,  0.5,
	/* Bottom face */
	/* Bottom left */
	-0.5, -0.5, -0.5,
	0.5, -0.5,  0.5,
	-0.5, -0.5,  0.5,
	/* Top right */
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	0.5, -0.5,  0.5,
};

const float _colors[] =
{
	/* Front face */
	/* Bottom left */
	1.0, 0.0, 0.0, /* red */
	0.0, 0.0, 1.0, /* blue */
	0.0, 1.0, 0.0, /* green */
	/* Top right */
	1.0, 0.0, 0.0, /* red */
	1.0, 1.0, 0.0, /* yellow */
	0.0, 0.0, 1.0, /* blue */
	/* Left face */
	/* Bottom left */
	1.0, 1.0, 1.0, /* white */
	0.0, 1.0, 0.0, /* green */
	0.0, 1.0, 1.0, /* cyan */
	/* Top right */
	1.0, 1.0, 1.0, /* white */
	1.0, 0.0, 0.0, /* red */
	0.0, 1.0, 0.0, /* green */
	/* Top face */
	/* Bottom left */
	1.0, 1.0, 1.0, /* white */
	1.0, 1.0, 0.0, /* yellow */
	1.0, 0.0, 0.0, /* red */
	/* Top right */
	1.0, 1.0, 1.0, /* white */
	0.0, 0.0, 0.0, /* black */
	1.0, 1.0, 0.0, /* yellow */
	/* Right face */
	/* Bottom left */
	1.0, 1.0, 0.0, /* yellow */
	1.0, 0.0, 1.0, /* magenta */
	0.0, 0.0, 1.0, /* blue */
	/* Top right */
	1.0, 1.0, 0.0, /* yellow */
	0.0, 0.0, 0.0, /* black */
	1.0, 0.0, 1.0, /* magenta */
	/* Back face */
	/* Bottom left */
	0.0, 0.0, 0.0, /* black */
	0.0, 1.0, 1.0, /* cyan */
	1.0, 0.0, 1.0, /* magenta */
	/* Top right */
	0.0, 0.0, 0.0, /* black */
	1.0, 1.0, 1.0, /* white */
	0.0, 1.0, 1.0, /* cyan */
	/* Bottom face */
	/* Bottom left */
	0.0, 1.0, 0.0, /* green */
	1.0, 0.0, 1.0, /* magenta */
	0.0, 1.0, 1.0, /* cyan */
	/* Top right */
	0.0, 1.0, 0.0, /* green */
	0.0, 0.0, 1.0, /* blue */
	1.0, 0.0, 1.0, /* magenta */
};

const char* _shader_vert_src = 
" attribute vec4 av4position; "
" attribute vec3 av3color; "
" uniform mat4 mvp; "
" varying vec3 vv3color; "
" void main() { "
"    vv3color = av3color; "
"    gl_Position = mvp * av4position; "
" } ";

const char* _shader_frag_src = 
#if defined(__IPHONEOS__) && defined(__ANDROID__) && defined(__EMSCRIPTEN__) && defined(__NACL__) //&& !linux
" precision lowp float; "
#endif
" varying vec3 vv3color; "
" void main() { "
"    gl_FragColor = vec4(vv3color, 1.0); "
" } ";

static void Render(Sprite * sprite)
{
	shader_data * data = &(sprite->datas);
	if(data==NULL)return;
	float matrix_rotate[16], matrix_modelview[16], matrix_perspective[16], matrix_mvp[16];

	/* 
	 * Do some rotation with Euler angles. It is not a fixed axis as
	 * quaterions would be, but the effect is cool. 
	 */
	rotate_matrix((float)data->angle_x, 1.0f, 0.0f, 0.0f, matrix_modelview);
	rotate_matrix((float)data->angle_y, 0.0f, 1.0f, 0.0f, matrix_rotate);

	multiply_matrix(matrix_rotate, matrix_modelview, matrix_modelview);

	rotate_matrix((float)data->angle_z, 0.0f, 1.0f, 0.0f, matrix_rotate);

	multiply_matrix(matrix_rotate, matrix_modelview, matrix_modelview);

	/* Pull the camera back from the cube */
	matrix_modelview[14] -= 2.5;

	perspective_matrix(45.0f, (float)stage->w /stage->h, 0.01f, 100.0f, matrix_perspective);
	multiply_matrix(matrix_perspective, matrix_modelview, matrix_mvp);

	GL_CHECK(gles2.glUniformMatrix4fv(data->attr_mvp, 1, GL_FALSE, matrix_mvp));

	data->angle_x += 3;
	data->angle_y += 2;
	data->angle_z += 1;

	data->angle_x %= 360;
	data->angle_y %= 360;
	data->angle_z %= 360;

	GL_CHECK(gles2.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
	GL_CHECK(gles2.glDrawArrays(GL_TRIANGLES, 0, 36));
}


void loop()
{
	SDL_GL_MakeCurrent(stage->window, stage->context);
	Render(stage);
	SDL_GL_SwapWindow(stage->window);
}

int main(int argc, char *argv[])
{
	SDL_VideoInit(SDL_GetVideoDriver(0));
	stage = (Sprite*)malloc(sizeof(Sprite));
	memset(stage,0,sizeof(Sprite));
	stage->window = SDL_CreateWindow("title", 240, 0, 240, 240, SDL_WINDOW_OPENGL);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#if defined(__IPHONEOS__) && defined(__ANDROID__) && defined(__EMSCRIPTEN__) && defined(__NACL__) //&& !linux
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif

	stage->w = 240;
	stage->h = 240;

	stage->context = (SDL_GLContext*)SDL_GL_CreateContext(stage->window);
	LoadContext(&gles2);

	SDL_GL_SetSwapInterval(1);


	shader_data *data;
	data = &(stage->datas);
	data->angle_x = 0; data->angle_y = 0; data->angle_z = 0;

	process_shader(&data->shader_vert, _shader_vert_src, GL_VERTEX_SHADER);
	process_shader(&data->shader_frag, _shader_frag_src, GL_FRAGMENT_SHADER);

	data->shader_program = GL_CHECK(gles2.glCreateProgram());
	GL_CHECK(gles2.glAttachShader(data->shader_program, data->shader_vert));
	GL_CHECK(gles2.glAttachShader(data->shader_program, data->shader_frag));
	GL_CHECK(gles2.glLinkProgram(data->shader_program));

	data->attr_position = GL_CHECK(gles2.glGetAttribLocation(data->shader_program, "av4position"));
	data->attr_color = GL_CHECK(gles2.glGetAttribLocation(data->shader_program, "av3color"));
	data->attr_mvp = GL_CHECK(gles2.glGetUniformLocation(data->shader_program, "mvp"));

	GL_CHECK(gles2.glUseProgram(data->shader_program));
	GL_CHECK(gles2.glEnableVertexAttribArray(data->attr_position));
	GL_CHECK(gles2.glEnableVertexAttribArray(data->attr_color));
	GL_CHECK(gles2.glVertexAttribPointer(data->attr_position, 3, GL_FLOAT, GL_FALSE, 0, _vertices));
	GL_CHECK(gles2.glVertexAttribPointer(data->attr_color, 3, GL_FLOAT, GL_FALSE, 0, _colors));
	GL_CHECK(gles2.glEnable(GL_CULL_FACE));
	GL_CHECK(gles2.glEnable(GL_DEPTH_TEST));

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(loop, 0, 1);
#else

	int i =100;
	while (i) {
		--i;
		loop();
	}
#endif

#if !defined(__ANDROID__) && !defined(__NACL__)  
	quit(0);
#endif    
	exit(0);
	return 0;
}
