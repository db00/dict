/*
 *
 gcc -g -Wall testgl2.c -I"../SDL2/include/" -lSDL2 && ./a.out
 */
#include "SDL.h"
#include "SDL_opengl.h"

typedef struct GL_Context
{
#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
#include "SDL_glfuncs.h"
#undef SDL_PROC
} GL_Context;


/* Undefine this if you want a flat cube instead of a rainbow cube */
#define SHADED_CUBE

static GL_Context ctx;

int LoadContext(GL_Context * data)
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
			return SDL_SetError("Couldn't load GL2 function %s: %s\n", #func, SDL_GetError()); \
		} \
	} while ( 0 );
#endif /* _SDL_NOGETPROCADDR_ */

#include "SDL_glfuncs.h"
#undef SDL_PROC
	return 0;
}


static void Render()
{
	ctx.glClearColor(0.0, 0.0, 0.0, 1.0);
	ctx.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ctx.glBegin(GL_QUADS);

	static float cube[8][3] = {
		{0.5, 0.5, -0.5},
		{0.5, -0.5, -0.5},
		{-0.5, -0.5, -0.5},
		{-0.5, 0.5, -0.5},
		{-0.5, 0.5, 0.5},
		{0.5, 0.5, 0.5},
		{0.5, -0.5, 0.5},
		{-0.5, -0.5, 0.5}
	};
#ifdef SHADED_CUBE
	static float color[8][3] = {
		{1.0, 1.0, 0.0},
		{1.0, 0.0, 0.0},
		{0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{0.0, 1.0, 1.0},
		{1.0, 1.0, 1.0},
		{1.0, 0.0, 1.0},
		{0.0, 0.0, 1.0}
	};
	ctx.glColor3fv(color[0]);
	ctx.glVertex3fv(cube[0]);
	ctx.glColor3fv(color[1]);
	ctx.glVertex3fv(cube[1]);
	ctx.glColor3fv(color[2]);
	ctx.glVertex3fv(cube[2]);
	ctx.glColor3fv(color[3]);
	ctx.glVertex3fv(cube[3]);

	ctx.glColor3fv(color[3]);
	ctx.glVertex3fv(cube[3]);
	ctx.glColor3fv(color[4]);
	ctx.glVertex3fv(cube[4]);
	ctx.glColor3fv(color[7]);
	ctx.glVertex3fv(cube[7]);
	ctx.glColor3fv(color[2]);
	ctx.glVertex3fv(cube[2]);

	ctx.glColor3fv(color[0]);
	ctx.glVertex3fv(cube[0]);
	ctx.glColor3fv(color[5]);
	ctx.glVertex3fv(cube[5]);
	ctx.glColor3fv(color[6]);
	ctx.glVertex3fv(cube[6]);
	ctx.glColor3fv(color[1]);
	ctx.glVertex3fv(cube[1]);

	ctx.glColor3fv(color[5]);
	ctx.glVertex3fv(cube[5]);
	ctx.glColor3fv(color[4]);
	ctx.glVertex3fv(cube[4]);
	ctx.glColor3fv(color[7]);
	ctx.glVertex3fv(cube[7]);
	ctx.glColor3fv(color[6]);
	ctx.glVertex3fv(cube[6]);

	ctx.glColor3fv(color[5]);
	ctx.glVertex3fv(cube[5]);
	ctx.glColor3fv(color[0]);
	ctx.glVertex3fv(cube[0]);
	ctx.glColor3fv(color[3]);
	ctx.glVertex3fv(cube[3]);
	ctx.glColor3fv(color[4]);
	ctx.glVertex3fv(cube[4]);

	ctx.glColor3fv(color[6]);
	ctx.glVertex3fv(cube[6]);
	ctx.glColor3fv(color[1]);
	ctx.glVertex3fv(cube[1]);
	ctx.glColor3fv(color[2]);
	ctx.glVertex3fv(cube[2]);
	ctx.glColor3fv(color[7]);
	ctx.glVertex3fv(cube[7]);
#else /* flat cube */
	ctx.glColor3f(1.0, 0.0, 0.0);
	ctx.glVertex3fv(cube[0]);
	ctx.glVertex3fv(cube[1]);
	ctx.glVertex3fv(cube[2]);
	ctx.glVertex3fv(cube[3]);

	ctx.glColor3f(0.0, 1.0, 0.0);
	ctx.glVertex3fv(cube[3]);
	ctx.glVertex3fv(cube[4]);
	ctx.glVertex3fv(cube[7]);
	ctx.glVertex3fv(cube[2]);

	ctx.glColor3f(0.0, 0.0, 1.0);
	ctx.glVertex3fv(cube[0]);
	ctx.glVertex3fv(cube[5]);
	ctx.glVertex3fv(cube[6]);
	ctx.glVertex3fv(cube[1]);

	ctx.glColor3f(0.0, 1.0, 1.0);
	ctx.glVertex3fv(cube[5]);
	ctx.glVertex3fv(cube[4]);
	ctx.glVertex3fv(cube[7]);
	ctx.glVertex3fv(cube[6]);

	ctx.glColor3f(1.0, 1.0, 0.0);
	ctx.glVertex3fv(cube[5]);
	ctx.glVertex3fv(cube[0]);
	ctx.glVertex3fv(cube[3]);
	ctx.glVertex3fv(cube[4]);

	ctx.glColor3f(1.0, 0.0, 1.0);
	ctx.glVertex3fv(cube[6]);
	ctx.glVertex3fv(cube[1]);
	ctx.glVertex3fv(cube[2]);
	ctx.glVertex3fv(cube[7]);
#endif /* SHADED_CUBE */
	ctx.glEnd();

	ctx.glMatrixMode(GL_MODELVIEW);
	ctx.glRotatef(5.0, 1.0, 1.0, 1.0);
}

int main(int argc, char *argv[])
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_SetError("SDL_INIT_VIDEO ERROR!\n");
		return 1;
	}

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
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);//or -1
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);//gldebug ..not Available in sdl2.0.4,dont know why.
#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__) || defined(__NACL__) //|| define(linux)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);//or 1
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);//or 1

	SDL_Window *window;
	window = SDL_CreateWindow("", 0, 0, 240, 320, SDL_WINDOW_OPENGL);
	SDL_ShowWindow(window);
	static SDL_GLContext context;
	context = SDL_GL_CreateContext(window);
	/* Important: call this *after* creating the context */
	if (LoadContext(&ctx) < 0) {
		SDL_Log("Could not load GL functions\n");
		exit(2);
		return 0;
	}
	SDL_GL_SetSwapInterval(1);  /* disable vsync. */


	ctx.glMatrixMode(GL_PROJECTION);//gl
	ctx.glLoadIdentity();//gl
	ctx.glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);//gl
	ctx.glMatrixMode(GL_MODELVIEW);//gl
	//ctx.glMatrixMode(GL_PROJECTION);
	ctx.glLoadIdentity();//gl
	ctx.glEnable(GL_DEPTH_TEST);
	ctx.glDepthFunc(GL_LESS);
	ctx.glShadeModel(GL_SMOOTH);//gl
	/* Main render loop */
	int done=100;
	while (done>0) 
	{
		SDL_GL_MakeCurrent(window, context);
		Render();
		SDL_GL_SwapWindow(window);
		--done;
	}
	return 0;
}
