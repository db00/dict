/**
 * @file sprite.c
 gcc -g -Wall -I"../SDL2/include/"  sprite.c matrix.c array.c files.c regex.c myregex.c mystring.c -lSDL2 -lm -Ddebug_sprite && ./a.out
 gcc -g -Wall -I"../SDL2/include/" -I"." -D GLchar=char sprite.c array.c matrix.c -lGLESv2 -lmingw32 -lSDL2main -lSDL2 -Ddebug_sprite && a
 gcc -g -Wall -I"/usr/include/SDL2" array.c sprite.c matrix.c files.c mystring.c myregex.c -lSDL2 -lm -Ddebug_sprite && ./a.out
 apt-get install -y libpcap-dev libsqlite3-dev sqlite3 libpcap0.8-dev libssl-dev build-essential iw tshark


 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-07-21
 */
#include "sprite.h"

GLES2_Context gles2;
Stage * stage = NULL;
//static SDL_mutex *mutex = NULL;

static int _Stage_redraw();

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
			SDL_Log("Couldn't load GLES2 function %s: %s\n", #func, SDL_GetError()); \
			return SDL_SetError("Couldn't load GLES2 function %s: %s\n", #func, SDL_GetError()); \
		} \
	} while ( 0 );
#endif /* _SDL_NOGETPROCADDR_ */

#ifdef HAVE_OPENGL
#include "SDL_glfuncs.h"
#else
#include "SDL_gles2funcs.h"
#endif
#undef SDL_PROC
	return 0;
}
static Data3d * data2D;

void quit(int rc)
{
	//SDL_DestroyMutex(mutex);
	if(stage){
		if (stage->GLEScontext != NULL) {
			if (stage->GLEScontext) {
				GL_CHECK(gles2.glDeleteProgram(data2D->programObject));
				SDL_GL_DeleteContext(stage->GLEScontext);
			}
			stage->GLEScontext = NULL;
		}
		Sprite_destroy(stage->sprite);
		stage->sprite = NULL;
		free(stage);
		stage=NULL;
	}

	//TTF_Quit();
	SDL_VideoQuit();
	SDL_Quit();
	exit(rc);
}

static int power_of_two(int input)
{
	/*return input;*/
	int value = 1;

	while (value < input) {
		value <<= 1;
	}
	return value;
} 


SDL_Color * uintColor(Uint32 _color)
{
	SDL_Color * color = (SDL_Color*)malloc(sizeof(SDL_Color));
	memset(color,0,sizeof(*color));
#if SDL_BYTEORDER == SDL_LIL_ENDIAN     /* OpenGL RGBA masks */
	color->r = (Uint8)((_color>>24) & 0xff);
	color->g = (Uint8)((_color>>16) & 0xff);
	color->b = (Uint8)((_color>>8) & 0xff);
	color->a = (Uint8)((_color) & 0xff);
#else
	color->r = (Uint8)((_color>>24) & 0xff);
	color->g = (Uint8)((_color>>16) & 0xff);
	color->b = (Uint8)((_color>>8) & 0xff);
	color->a = (Uint8)((_color) & 0xff);
#endif
	return color;
}

//need free
SDL_Surface * RGBA_surface(SDL_Surface * surface)
{
	if(surface==NULL)
		return NULL;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	return SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
#else
	return SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);//
#endif
	//SDL_Surface * image = Surface_new(surface->w,surface->h);
	//SDL_BlitSurface(surface, NULL, image, NULL);
	//return image;
}

#ifdef __IPHONEOS__
SDL_Surface * Surface_size3D(SDL_Surface * surface)
{
	int w, h;
	w = power_of_two(surface->w);
	h = power_of_two(surface->h);
	SDL_Surface * image = Surface_new(w,h);
	SDL_BlitScaled(surface, NULL, image, NULL);
	SDL_FreeSurface(surface);
	return image;
}
#endif

GLuint SDL_GL_LoadTexture(Sprite * sprite, GLfloat * texcoord)
{
	SDL_Surface * surface = sprite->surface;
	GLint maxRenderbufferSize;
	GL_CHECK(gles2.glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize));
	if((maxRenderbufferSize <= surface->w) || (maxRenderbufferSize <= surface->h))
	{
		SDL_Log("cannot use framebuffer objects as we need to create a depth buffer as a renderbuffer object");
		return 0;
	}

	GLuint texture;
	SDL_Surface *image;
	SDL_Rect area;
	SDL_BlendMode saved_mode;


	/* Use the surface width and height expanded to powers of 2 */
	int w, h;
	if(texcoord){//change size
		w = power_of_two(surface->w);
		h = power_of_two(surface->h);
		texcoord[0] = 0.0f;         /* Min X */
		texcoord[1] = 0.0f;         /* Min Y */
		texcoord[2] = (GLfloat) surface->w / w;     /* Max X */
		texcoord[3] = (GLfloat) surface->h / h;     /* Max Y */
	}else{//size not changed
		w = surface->w;
		h = surface->h;
	}

	image = Surface_new(w,h);
	if (image == NULL) {
		return 0;
	}

	/* Save the alpha blending attributes */
	SDL_GetSurfaceBlendMode(surface, &saved_mode);
	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
	//SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
	//SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_ADD);
	//SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_MOD);

	/* Copy the surface into the GL texture image */
	area.x = 0;
	area.y = 0;
	area.w = surface->w;
	area.h = surface->h;
	SDL_BlitSurface(surface, &area, image, &area);
	//SDL_SetColorKey(image,SDL_TRUE,0x000000ff);

	/* Restore the alpha blending attributes */
	SDL_SetSurfaceBlendMode(surface, saved_mode);

	/* Create an OpenGL texture for the image */
	GL_CHECK(gles2.glGenTextures(1, &texture));
	GL_CHECK(gles2.glBindTexture(GL_TEXTURE_2D, texture));
	GL_CHECK(gles2.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CHECK(gles2.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CHECK(gles2.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CHECK(gles2.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	GL_CHECK(gles2.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels));
	GL_CHECK(gles2.glBindTexture(GL_TEXTURE_2D, 0));

	SDL_FreeSurface(image);     /* No longer needed */
	return texture;
}

float Sprite_getAlpha(Sprite * sprite)
{
	float alpha = sprite->alpha;
	Sprite * s = sprite->parent;
	while(s)
	{
		alpha *=s->alpha;
		s = s->parent;
	}
	return alpha;
}
int Sprite_getVisible(Sprite*sprite)
{
	if(sprite){
		Sprite*cur = sprite;
		while(cur)
		{
			if(cur->visible==0)
			{
				return 0;
			}
			cur = cur->parent;
		}
		return 1;
	}
	return 0;
}
void Sprite_matrix(Sprite *sprite)
{
	Point3d *p = Sprite_localToGlobal(sprite,NULL);

	Matrix3D modelview;
	// Generate a model view matrix to rotate/translate the cube
	Matrix_identity( &modelview);

	esScale(&modelview,p->scaleX/sprite->scaleX,p->scaleY/sprite->scaleY,p->scaleZ/sprite->scaleZ);
	// Rotate 
	esRotate( &modelview, -p->rotationX+sprite->rotationX, 1.0, 0.0, 0.0 );
	esRotate( &modelview, -p->rotationY+sprite->rotationY, 0.0, 1.0, 0.0 );
	esRotate( &modelview, -p->rotationZ+sprite->rotationZ, 0.0, 0.0, 1.0 );

	esTranslate(&modelview, xto3d(p->x-sprite->x), yto3d(p->y-sprite->y), zto3d(p->z-sprite->z));


	// Compute the final MVP by multiplying the 
	// modevleiw and perspective matrices together
	esMatrixMultiply( &sprite->mvpMatrix, &modelview, &stage->world->perspective);

	esRotate( &sprite->mvpMatrix, -sprite->rotationX, 1.0, 0.0, 0.0 );
	esRotate( &sprite->mvpMatrix, -sprite->rotationY, 0.0, 1.0, 0.0 );
	esRotate( &sprite->mvpMatrix, -sprite->rotationZ, 0.0, 0.0, 1.0 );
	esScale(&sprite->mvpMatrix,sprite->scaleX,sprite->scaleY,sprite->scaleZ);
	esTranslate(&sprite->mvpMatrix,
			wto3d(sprite->x),
			-hto3d(sprite->y),
			zto3d(sprite->z)
			);

	free(p);
}


SDL_Rect * Sprite_getBorder(Sprite*sprite,SDL_Rect*rect)
{
	if(rect==NULL || sprite == stage->sprite){
		return NULL;
	}
	if(rect==NULL)
	{
		rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
		memset(rect,0,sizeof(SDL_Rect));
	}
	if(sprite->Bounds){
		free(sprite->Bounds);
		sprite->Bounds = NULL;
	}
	if(sprite->mask)
	{
		sprite->Bounds = malloc(sizeof(SDL_Rect));
		sprite->Bounds->x = sprite->mask->x;
		sprite->Bounds->y = sprite->mask->y;
		sprite->Bounds->w = sprite->mask->w;
		sprite->Bounds->h = sprite->mask->h;
		return sprite->Bounds;
	}
	if(sprite->is3D){
		SDL_Rect *r = (SDL_Rect*)malloc(sizeof(SDL_Rect));
		r->w = rect->w;
		r->h = rect->h;
		r->x = rect->x - rect->w/2;
		r->y = rect->y - rect->h/2;
		if(sprite->Bounds)
			free(sprite->Bounds);
		sprite->Bounds= r;//SDL_UnionRect(rect,sprite->Bounds,sprite->Bounds);
		rect = r;
	}else{
		sprite->Bounds= rect;//SDL_UnionRect(rect,sprite->Bounds,sprite->Bounds);
	}
	/*
	   SDL_Log("sprite->Bounds:%d,%d,%d,%d\n",
	   sprite->Bounds->x,
	   sprite->Bounds->y,
	   sprite->Bounds->w,
	   sprite->Bounds->h
	   );
	   */


	Sprite*curParent = sprite->parent;
	while(curParent && curParent!=stage->sprite)
	{
		int i = 0;
		while(i<curParent->children->length)
		{
			Sprite*target = Sprite_getChildByIndex(curParent,i);
			if(target->Bounds){
				SDL_Rect* Bounds = malloc(sizeof(*Bounds));
				SDL_UnionRect(curParent->Bounds,target->Bounds,Bounds);
				if(curParent->Bounds){
					free(curParent->Bounds);
				}
				curParent->Bounds = Bounds;
			}else{
				//return NULL;
			}
			++i;
		}
		curParent = curParent-> parent;
	}
	return rect;
}

Sprite * Sprite_new()
{
	Sprite*sprite = NULL;
	if(sprite==NULL){
		//SDL_Log("new ");
		sprite = (Sprite*)malloc(sizeof(Sprite));
		memset(sprite,0,sizeof(Sprite));
	}
	if(sprite->name==NULL)
	{
		sprite->name = (char*)malloc(16);
		memset(sprite->name,0,16);
		sprintf(sprite->name,"sprite%d",stage->numsprite++);
		//SDL_Log("%s\n",sprite->name);
	}

	sprite->scaleX = 1.0;
	sprite->scaleY = 1.0;
	sprite->scaleZ = 1.0;

	sprite->mouseChildren = SDL_TRUE;
	sprite->alpha = 1.0;
	sprite->visible = 1;
	return sprite;
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

	//#ifndef __MACOS__
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

int Window_resize(int w,int h)
{
#ifndef __ANDROID__
	stage->stage_w = w;
	stage->stage_h = h;
	SDL_SetWindowSize(stage->window,w,h);
	if(stage->world){
		stage->world->aspect = (GLfloat)stage->stage_w/stage->stage_h; // Compute the window aspect ratio
		//stage->world->fovy = 53.13010235415598; //atan(1/stage->world->aspect)*180/M_PI; // Generate a perspective matrix with a 53.13010235415598 degree FOV
		stage->world->fovy = atan(4.0/3)*180/M_PI; // Generate a perspective matrix with a 53.13010235415598 degree FOV
		stage->world->nearZ = 1.0f;
		stage->world->farZ = 20.0f;
		Matrix_identity(&stage->world->perspective);
		esPerspective( &stage->world->perspective, stage->world->fovy, stage->world->aspect, stage->world->nearZ, stage->world->farZ);
		// Translate away from the viewer
		esTranslate(&stage->world->perspective,0,0,-2.0);
	}
#endif
	return 0;
}

Stage * Stage_init() 
{
	int is3D = 1;
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0) {
		SDL_SetError("SDL_INIT_VIDEO ERROR!\n");
		return NULL;
	}
	/*
	   if(!TTF_WasInit())
	   {
	   if ( TTF_Init() < 0 ) {
	   SDL_SetError("Couldn't initialize TTF: %s\n",SDL_GetError());
	   quit(0);
	   return NULL;
	   }
	   }
	   */
	if(stage==NULL)
	{
		stage = malloc(sizeof(Stage));
		memset(stage,0,sizeof(Stage));

		stage->sprite = Sprite_new();
		if(stage->renderer==NULL && stage->GLEScontext == NULL)
		{
			SDL_DisplayMode mode;
			SDL_GetCurrentDisplayMode(0, &mode);
			SDL_Log("screen size:%dx%d\n",mode.w,mode.h);
#ifdef __ANDROID__
			stage->stage_w = mode.w;
			stage->stage_h = mode.h;
#else
			stage->stage_w = 400;
			stage->stage_h = (int)((float)stage->stage_w*16.0/12.0);
#endif

			stage->window = SDL_CreateWindow("title",
					SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
					stage->stage_w, stage->stage_h,
					(is3D?SDL_WINDOW_OPENGL:0)|SDL_WINDOW_RESIZABLE);
			if(stage->window){
				SDL_ShowWindow(stage->window);
				SDL_GetWindowSize(stage->window, &stage->stage_w, &stage->stage_h);
			}else{
				quit(1);
				return NULL;
			}

			if(is3D)
			{
				initGL();
				stage->GLEScontext = SDL_GL_CreateContext(stage->window);
				if (LoadContext(&gles2) < 0) {
					SDL_Log("Could not load GLES2 functions\n");
					quit(2);
					return stage;
				}
				//SDL_GL_SetSwapInterval(1);
				SDL_GL_SetSwapInterval(0);  /* disable vsync. */
#ifdef HAVE_OPENGL
				SDL_Log("OpenGL2.0");
				gles2.glMatrixMode(GL_PROJECTION);//gl
				gles2.glLoadIdentity();//gl
				gles2.glOrtho(-1.0, 1.0, -1.0, 1.0, 1.0, 20.0);//gl
				//gles2.glMatrixMode(GL_MODELVIEW);//gl
				gles2.glMatrixMode(GL_PROJECTION);
				gles2.glLoadIdentity();//gl
				//gles2.glEnable(GL_DEPTH_TEST);
				gles2.glDepthFunc(GL_LESS);
				gles2.glShadeModel(GL_SMOOTH);//gl
				gles2.glTranslatef(0,0,-2.0);
#endif
				if(stage->world == NULL)
				{
					//SDL_Log("init 3d world");
					stage->world = (World3d *)malloc(sizeof(World3d));
					stage->world->aspect = (GLfloat)stage->stage_w/stage->stage_h; // Compute the window aspect ratio
					//stage->world->fovy = 53.13010235415598; //atan(1/stage->world->aspect)*180/M_PI; // Generate a perspective matrix with a 53.13010235415598 degree FOV
					stage->world->fovy = atan(4.0/3)*180/M_PI; // Generate a perspective matrix with a 53.13010235415598 degree FOV
					stage->world->nearZ = 1.0f;
					stage->world->farZ = 20.0f;
					Matrix_identity(&stage->world->perspective);
					esPerspective( &stage->world->perspective, stage->world->fovy, stage->world->aspect, stage->world->nearZ, stage->world->farZ);
					// Translate away from the viewer
					esTranslate(&stage->world->perspective,0,0,-2.0);
#if 1
					stage->lightDirection[0]=1.0;
					stage->lightDirection[1]=1.0;
					stage->lightDirection[2]=-1.0;
#endif
					//esRotate( &stage->world->perspective, 10, .0, 0.0, 1.0);
					//esMatrixMultiply(&stage->world->perspective,&stage->world->perspective,);

				}
				//int w,h; SDL_GetWindowSize(stage->window, &w, &h);
				//SDL_GL_MakeCurrent(stage->window, stage->GLEScontext);
				//SDL_GL_GetDrawableSize(stage->window, &stage->stage_w, &stage->stage_h);
				//GL_CHECK(gles2.glViewport(0, 0, stage->stage_w , stage->stage_h));
				//return NULL;
				Data3D_init();
			}
			/*
			   else
			   {
			   stage->renderer = SDL_CreateRenderer(stage->window, -1, SDL_RENDERER_ACCELERATED);
			   SDL_SetRenderDrawColor(stage->renderer, 0x0f, 0xf, 0xf, 0x0);
			   SDL_RenderClear(stage->renderer);
			   }
			   */
		}
		stage->sprite->visible = 1;
	}
	return stage;
}

Point3d * Sprite_localToGlobal(Sprite*sprite,Point3d *p)
{
	if(sprite == NULL)
		return NULL;
	//Point3d *p = NULL;
	p = NULL;
	if(p==NULL)
	{
		p = (Point3d*)malloc(sizeof(Point3d));
		p->x = 0;
		p->y = 0;
		p->z = 0;
		p->scaleX = 1.0;
		p->scaleY = 1.0;
		p->scaleZ = 1.0;
		p->rotationX = 0;
		p->rotationY = 0;
		p->rotationZ = 0;
	}

	Sprite * cur = sprite;
	while(cur)
	{
		p->x += cur->x*cur->scaleX;
		p->y += cur->y*cur->scaleY;
		p->z += cur->z*cur->scaleZ;
		p->scaleX *= cur->scaleX;
		p->scaleY *= cur->scaleY;
		p->scaleZ *= cur->scaleZ;
		p->rotationX += cur->rotationX;
		p->rotationY += cur->rotationY;
		p->rotationZ += cur->rotationZ;
		cur=cur->parent;
	}
	return p;
}



Point3d * Sprite_GlobalToLocal(Sprite*sprite,Point3d*p)
{
	if(sprite == NULL)
		return NULL;
	if(p==NULL)
		p = malloc(sizeof(Point3d));
	p->x -= sprite->x;
	p->y -= sprite->y;

	Sprite * curParent = sprite->parent;
	while(curParent)
	{
		p->x -=curParent->x;
		p->y -=curParent->y;
		curParent = curParent->parent;
	}
	return p;
}


int Sprite_getTextureId(Sprite * sprite)
{
	if(sprite->textureId == 0){
		if(sprite->surface == NULL) {
			//SDL_Log("no surface!\n");
			//return _data3D;
			return 0;
		}else{
			//SDL_Log("has surface!\n");
		}
		if(sprite->surface ) {
#ifdef __IPHONEOS__
			if(sprite->is3D){
				if(sprite->texCoords){
					free(sprite->texCoords);
					sprite->texCoords = NULL;
				}
				sprite->surface = Surface_size3D(sprite->surface);
			}else{
				sprite->texCoords = malloc(4*sizeof(GLfloat));
			}
			sprite->textureId = SDL_GL_LoadTexture(sprite, sprite->texCoords);
#else
			sprite->textureId = SDL_GL_LoadTexture(sprite, NULL);
#endif
			if(sprite->textureId==0)
				//return _data3D;
				return 0;
			//sprite->x = 0; sprite->y = 0;
			if((sprite->w==0 && sprite->h==0) || sprite==stage->sprite){
				sprite->w = sprite->surface->w;
				sprite->h = sprite->surface->h;
			}
			Sprite_destroySurface(sprite);
		}else{
			SDL_Log("notexture!\n");
			//return _data3D;
			return 0;
		}
	}
	return sprite->textureId;

}

static GLuint * Sprite_getIndices(Sprite * sprite)
{
	Data3d*_data3D = sprite->data3d;
	if(_data3D->indices==NULL){
		GLuint indices[] = {0,1,2,0,2,3};
		_data3D->numIndices = sizeof(indices)/sizeof(GLuint);
		_data3D->indices = malloc(_data3D->numIndices*sizeof(unsigned int));
		memcpy(_data3D->indices,indices,sizeof(indices));
		return _data3D->indices;
	}
	return _data3D->indices;
}
static GLfloat * Sprite_getVertices(Sprite * sprite)
{
	Data3d*_data3D = sprite->data3d;
	// Load the vertex position
	if(_data3D->positionLoc>=0){
		if(_data3D->vertices==NULL)
		{
			float _x=0.0,_y=0.0,_w,_h;
			_w = wto3d(sprite->w);
			_h = hto3d(sprite->h);

			GLfloat vertices[] = {
				_x,		_y,		0.0f,	// Position 0	//top left
				_x,		_y-_h,	0.0f,	// Position 1	//bottom left
				_x+_w,	_y-_h,	0.0f,	// Position 2	//bottom right
				_x+_w,	_y,		0.0f	// Position 3	//top right
			};

			_data3D->numVertices = 4;
			_data3D->vertices = (GLfloat*)malloc(sizeof(vertices));
			memcpy(_data3D->vertices,vertices,sizeof(vertices));
		}
	}
	return _data3D->vertices;
}

static GLfloat * Sprite_getNormals(Sprite * sprite)
{
	Data3d*_data3D = sprite->data3d;
	// Load the vertex normals 
	if(_data3D->normalLoc>=0){
		if(_data3D->normals==NULL){
			GLfloat normals[] = {
				0.0f,0.0f,0.0f,	// Position 0	//top left
				0.0f,0.0f,0.0f,	// Position 1	//bottom left
				0.0f,0.0f,0.0f,	// Position 2	//bottom right
				0.0f,0.0f,0.0f	// Position 3	//top right
			};
			_data3D->normals = (GLfloat*)malloc(sizeof(normals));
			memcpy(_data3D->normals,normals,sizeof(normals));
		}
	}
	return _data3D->normals;
}


static GLfloat * Sprite_getTexCoords(Sprite * sprite)
{
	Data3d*_data3D = sprite->data3d;
	// Load the texture coordinate
	if(_data3D->texCoordLoc>=0){
		if(_data3D->texCoords==NULL ){
#ifdef __IPHONEOS__
			GLfloat _w = sprite->texCoords[2];
			GLfloat _h = sprite->texCoords[3];
#else
			GLfloat _w = 1.0f;
			GLfloat _h = 1.0f;
#endif
			GLfloat texCoords[] = {
				0.0f,  0.0f,        // TexCoord 0 
				0.0f, _h,        // TexCoord 1
				_w,  _h,        // TexCoord 2
				_w,  0.0f         // TexCoord 3
			};
			_data3D->texCoords = (GLfloat*)malloc(sizeof(texCoords));
			memcpy(_data3D->texCoords,texCoords,sizeof(texCoords));
		}
	}
	return _data3D->texCoords;
}

static void Sprite_getAmbient(Sprite * sprite)
{
	Data3d*_data3D = sprite->data3d;
	if(_data3D->ambientLoc >=0){
		if( stage->ambient[0]==0 && stage->ambient[1]==0 && stage->ambient[2]==0 && stage->ambient[3]==0){
			stage->ambient[0]=.9;
			stage->ambient[1]=.9;
			stage->ambient[2]=.9;
			stage->ambient[3]=1.0;
		}
		if( sprite->ambient[0]==0 && sprite->ambient[1]==0 && sprite->ambient[2]==0 && sprite->ambient[3]==0){
			sprite->ambient[0]=stage->ambient[0];
			sprite->ambient[1]=stage->ambient[1];
			sprite->ambient[2]=stage->ambient[2];
			sprite->ambient[3]=stage->ambient[3];
		}
	}
}

static void Data3d_show(Sprite*sprite)
{/*{{{*/
	Data3d*_data3D = sprite->data3d;

	//SDL_Log("%s\n",sprite->name);
	if(_data3D==NULL){
		_data3D = Data3D_new();
		if(_data3D->programObject == 0){
			Data3d * data2D = Data3D_init();
			Data3d_set(_data3D,data2D);
		}
		sprite->data3d = _data3D;
	}

	sprite->textureId = Sprite_getTextureId(sprite);
	if(sprite->textureId==0)
		return;

	//贴图透明度
	GL_CHECK(gles2.glEnable(GL_BLEND));
	GL_CHECK(gles2.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	//GL_CHECK(gles2.glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR));
	//
	GL_CHECK(gles2.glCullFace ( GL_BACK ));
	//GL_CHECK(gles2.glCullFace ( GL_FRONT));
	GL_CHECK(gles2.glEnable ( GL_CULL_FACE ));

	//GL_CHECK(gles2.glDepthFunc(GL_LESS));
	GL_CHECK(gles2.glDepthFunc(GL_LEQUAL));
	//GL_CHECK(gles2.glEnable(GL_DEPTH_TEST));
	//GL_CHECK(gles2.glDisable ( GL_DEPTH_TEST));
	//GL_CHECK(gles2.glEnable( GL_STENCIL_TEST ));
	if(sprite->mask)
	{
		GL_CHECK(gles2.glEnable( GL_SCISSOR_TEST));
		//gles2.glScissor((sprite->mask->x),stage->stage_h-(sprite->mask->y),(sprite->mask->w),(sprite->mask->h));
		gles2.glScissor(sprite->mask->x,stage->stage_h-(sprite->mask->y+sprite->mask->h),sprite->mask->w,sprite->mask->h);
	}

	if(_data3D->programObject)
	{
		GL_CHECK(gles2.glUseProgram ( _data3D->programObject ));
	}else{
		return;
	}

	_data3D->normals = Sprite_getNormals(sprite);
	if(_data3D->normals)
	{
		GL_CHECK(gles2.glVertexAttribPointer ( _data3D->normalLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), _data3D->normals));
		GL_CHECK(gles2.glEnableVertexAttribArray ( _data3D->normalLoc));
	}

	_data3D->texCoords = Sprite_getTexCoords(sprite);
	if(_data3D->texCoords)
	{
		GL_CHECK(gles2.glVertexAttribPointer ( _data3D->texCoordLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), _data3D->texCoords));
		GL_CHECK(gles2.glEnableVertexAttribArray ( _data3D->texCoordLoc ));
	}

	// Bind the texture
	if(sprite->textureId>0){
		GL_CHECK(gles2.glActiveTexture ( GL_TEXTURE0 ));
		GL_CHECK(gles2.glBindTexture ( GL_TEXTURE_2D, sprite->textureId ));
	}
	// Set the sampler texture unit to 0
	if(_data3D->samplerLoc>=0)
	{
		GL_CHECK(gles2.glUniform1i( _data3D->samplerLoc, 0));
	}

	if(_data3D->alphaLoc>=0)
	{
		GL_CHECK(gles2.glUniform1f( _data3D->alphaLoc, Sprite_getAlpha(sprite)));
	}


	Sprite_getAmbient(sprite);
	GL_CHECK(gles2.glUniform4f( _data3D->ambientLoc, sprite->ambient[0], sprite->ambient[1], sprite->ambient[2], sprite->ambient[3]));
	if(_data3D->lightDirection>=0){
		GL_CHECK(gles2.glUniform3f(_data3D->lightDirection, stage->lightDirection[0], stage->lightDirection[1], stage->lightDirection[2]));
	}

	if(_data3D->mvpLoc>=0){
		Sprite_matrix(sprite);
		GL_CHECK(gles2.glUniformMatrix4fv( _data3D->mvpLoc, 1, GL_FALSE, (GLfloat*) &sprite->mvpMatrix.rawData[0][0]));
	}

	_data3D->vertices = Sprite_getVertices(sprite);
	_data3D->indices = Sprite_getIndices(sprite);
	/*
	   if ( _data3D->vboIds[0] == 0 && _data3D->vboIds[1] == 0 )
	   {
	   GL_CHECK(gles2.glGenBuffers ( 2, _data3D->vboIds));
	   GL_CHECK(gles2.glBindBuffer ( GL_ARRAY_BUFFER, _data3D->vboIds[0] ));
	   GL_CHECK(gles2.glBufferData ( GL_ARRAY_BUFFER, 3*sizeof(GLfloat)*_data3D->numVertices, _data3D->vertices, GL_STATIC_DRAW ));
	   GL_CHECK(gles2.glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, _data3D->vboIds[1] ));
	   GL_CHECK(gles2.glBufferData ( GL_ELEMENT_ARRAY_BUFFER, 3*sizeof(GLushort)*_data3D->numIndices, _data3D->indices, GL_STATIC_DRAW ));
	   }
	   GL_CHECK(gles2.glBindBuffer ( GL_ARRAY_BUFFER, _data3D->vboIds[0] ));
	   GL_CHECK(gles2.glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, _data3D->vboIds[1] ));
	   */

	if(_data3D->vertices)
	{
		GL_CHECK(gles2.glEnableVertexAttribArray ( _data3D->positionLoc ));
		GL_CHECK(gles2.glVertexAttribPointer ( _data3D->positionLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), _data3D->vertices));
		//GL_CHECK(gles2.glVertexAttribPointer ( _data3D->positionLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0));
	}

	if(_data3D->numIndices>0){
		GL_CHECK(gles2.glDrawElements(GL_TRIANGLES, _data3D->numIndices, GL_UNSIGNED_INT, _data3D->indices));
		//GL_CHECK(gles2.glDrawElements(GL_TRIANGLES, _data3D->numIndices, GL_UNSIGNED_INT,0));
	}

	GL_CHECK(gles2.glBindBuffer ( GL_ARRAY_BUFFER, 0 ));
	GL_CHECK(gles2.glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, 0));
	if(_data3D->positionLoc>=0)
	{
		GL_CHECK(gles2.glDisableVertexAttribArray(_data3D->positionLoc));
	}
	if(_data3D->normalLoc>=0)
	{
		GL_CHECK(gles2.glDisableVertexAttribArray(_data3D->normalLoc));
	}
	if(_data3D->texCoordLoc>=0)
	{
		GL_CHECK(gles2.glDisableVertexAttribArray(_data3D->texCoordLoc));
	}
	GL_CHECK(gles2.glDisable( GL_SCISSOR_TEST));
	//GL_CHECK(gles2.glEnable ( GL_DEPTH_TEST));
	//return _data3D;
	return ;
}/*}}}*/


Sprite * Sprite_show(Sprite*sprite)
{
	if(sprite==NULL)
		return NULL;
	if(Sprite_getVisible(sprite)==0) {
		return sprite;
	}

	//SDL_Log("show:%s\n",sprite->name);
	if(stage->GLEScontext){
		if(sprite->showFunc == NULL){
			sprite->showFunc = Data3d_show;
		}
		sprite->showFunc(sprite);
		//return sprite;
	}else if(sprite->texture || sprite->surface){
		if(sprite->texture==NULL){
			if(sprite->surface){
				if((sprite->w) * (sprite->h)==0) {
					sprite->w = sprite->surface->w;
					sprite->h = sprite->surface->h;
				}
				sprite->texture = SDL_CreateTextureFromSurface(stage->renderer, sprite->surface);
				Sprite_destroySurface(sprite);
			}
		}
	}

	SDL_Rect* rect =(SDL_Rect*)malloc(sizeof(SDL_Rect));
	memset(rect,0,sizeof(SDL_Rect));
	int rotation=0;
	int centerX=0;
	int centerY=0;
	float scaleX = 1.0;
	float scaleY = 1.0;

	Sprite *curSprite=sprite;
	while(curSprite){
		rect->x += curSprite->x;
		rect->y += curSprite->y;
		if(curSprite->rotationZ)
		{
			rotation += curSprite->rotationZ;
		}
		scaleX *= curSprite->scaleX;
		scaleY *= curSprite->scaleY;
		curSprite = curSprite->parent;
	}
	rect->w = sprite->w * scaleX;
	rect->h = sprite->h * scaleY;
	Sprite_getBorder(sprite,rect);
	//SDL_Log("Sprite_getBorder:%s,%d,%d,%d,%d\n",sprite->name,rect->x,rect->y,rect->w,rect->h);

	if(sprite->texture){
		SDL_SetTextureAlphaMod(sprite->texture, sprite->alpha*0xff);
		SDL_SetTextureColorMod(sprite->texture, sprite->alpha*0xff, sprite->alpha*0xff, sprite->alpha*0xff);
		if(rotation!=0){
			SDL_Point center;
			center.x = centerX;
			center.y = centerY;
			{//绕中心旋转
				//center.x = sprite->w/2;
				//center.y = sprite->h/2;
			}
			SDL_RendererFlip flip = SDL_FLIP_NONE;
			SDL_RenderCopyEx(stage->renderer, sprite->texture, NULL, rect, rotation,&center,flip);
		}else{
			SDL_RenderCopy(stage->renderer, sprite->texture, NULL, rect);
		}
	}

	if(sprite->children){
		int i = 0;
		while(i<sprite->children->length) {
			Sprite*child = Sprite_getChildByIndex(sprite,i);
			if(Sprite_getVisible(child)){
				Sprite_show(child);
			}
			++i;
		}
	}

	return sprite;
}

int Sprite_getChildIndex(Sprite*parent,Sprite*sprite)
{
	if(parent && sprite && sprite->parent==parent)
	{
		return Array_getIndexByValue(parent->children,sprite);
	}
	return -1;
}


Sprite * Sprite_removeChild(Sprite*parent,Sprite*sprite)
{
	if(parent==NULL || sprite ==NULL)
		return sprite;
	if(stage->currentTarget==sprite || Sprite_contains(stage->currentTarget,sprite))
		stage->currentTarget = NULL;
	if(stage->focus ==sprite || Sprite_contains(stage->focus,sprite))
		stage->focus= NULL;

	if(parent!= sprite->parent || sprite->parent == NULL) {
		return sprite;
	}
	int i = Sprite_getChildIndex(parent,sprite);
	if(i>=0)
		return Sprite_removeChildAt(parent,i);
	return sprite;
}


Sprite * Sprite_getChildByIndex(Sprite * sprite,int index)
{
	return Array_getByIndex(sprite->children,index);
}

SDL_bool Sprite_contains(Sprite*parent,Sprite*sprite)
{
	if(parent==NULL || sprite==NULL)
		return SDL_FALSE;
	Sprite * curParent = sprite->parent;
	while(curParent && curParent!=parent)
	{
		curParent = curParent->parent;
	}
	if(curParent==parent)
		return SDL_TRUE;
	return SDL_FALSE;
}

Sprite * Sprite_getChildByName(Sprite*sprite,const char*name)
{
	if(sprite->children==NULL)
		return NULL;
	int i= 0;
	while(i< sprite->children->length)
	{
		Sprite*child = Sprite_getChildByIndex(sprite,i);
		if(strcmp(child->name,name)==0)
			return child;
		++i;
	}
	return NULL;
}



SDL_UserEvent * UserEvent_new(Uint32 type,Sint32 code,void*func,void*param)
{
	if(type==0){//SDL_USEREVENT
		type = SDL_RegisterEvents(1);
	}
	SDL_Event * event=NULL;
	SDL_Event e;
	if (type!= ((Uint32)-1)) {
		if(type == SDL_USEREVENT)
		{
			event = &e;// (SDL_Event*)malloc(sizeof(SDL_UserEvent));
		}else{
			event = (SDL_Event*)malloc(sizeof(SDL_UserEvent));
		}
		memset(event,0,sizeof(SDL_UserEvent));
		event->type = type;
		event->user.code = code;
		event->user.data1 = func;
		event->user.data2 = param;
		SDL_PushEvent(event);
	}
	if(type == SDL_USEREVENT)//no need free
		event = NULL;
	return (SDL_UserEvent*)event;
}
void UserEvent_clear(SDL_UserEvent * event)
{
	if(event)
		free(event);
}

void Sprite_removeEvents(Sprite * sprite)
{
	if(stage->currentTarget==sprite)
		stage->currentTarget = NULL;
	if(stage->focus==sprite)
		stage->focus= NULL;
	if(sprite && sprite->events)
	{
		Array_freeEach(sprite->events);
		sprite->events=NULL;
	}
}

int Sprite_dispatchEvent(Sprite*sprite,const SDL_Event *event)
{
	if(stage->focus!=sprite && stage->currentTarget!=sprite && sprite!=stage->sprite)
		return 1;
	if(sprite==NULL || sprite->events == NULL)
		return 2;
	if(event==NULL)
		return 3;
	stage->currentTarget = sprite;
	int i = 0;
	while(stage->currentTarget==sprite && sprite->events && i < sprite->events->length)
	{
		SpriteEvent*e = Array_getByIndex(sprite->events,i);
		if(stage->currentTarget==sprite && e && sprite && sprite->events && sprite->name) 
		{
			if(stage->currentTarget==sprite && event->type && event->type == e->type){
				e->e = (SDL_Event*)event;
				if(e->func!=NULL){
					if(e->lastEventTime != event->motion.timestamp){
						//SDL_Log("lastEventTime :%d",e->lastEventTime);
						if(stage->currentTarget==sprite)
							e->func(e);
					}
					e->lastEventTime = event->motion.timestamp;
				}
			}
		}
		++i;
	}
	return 0;
}

static SpriteEvent * Sprite_hasEvent(Sprite*sprite,Uint32 type,EventFunc func)
{
	if(sprite==NULL)
		return 0;
	if(sprite->events){
		int i = 0;
		while(i<sprite->events->length)
		{
			SpriteEvent*e = (SpriteEvent*)Array_getByIndex(sprite->events,i);
			if(e->type==type && e->func==func)
			{
				return e;
			}
			++i;
		}
	}
	return NULL;
}

int Sprite_addEventListener(Sprite*sprite,Uint32 type,EventFunc func)
{
	if(sprite == NULL)
		return -1;
	if(SDL_HasEvent(type) == SDL_FALSE) {//事件不在事件列表
		//type= SDL_RegisterEvents(1);
	}
	if(Sprite_hasEvent(sprite,type,func)){
		return -2;
		//Sprite_removeEventListener(sprite,type,func);
	}else{
	}

	SpriteEvent*e = (SpriteEvent*)malloc(sizeof(*e));
	memset(e,0,sizeof(*e));
	e->type = type;
	e->func = func;
	e->target = sprite;
	e->lastEventTime = stage->lastEventTime;

	if(sprite->events==NULL){
		sprite->events = Array_new();
		sprite->events = Array_setByIndex(sprite->events,0,e);
	}else{//events 中的空的;e 放到 e->events 的开头
		sprite->events = Array_insert(sprite->events,0,e);
	}
	//SDL_Log("%s,sprite->events->length:%d,",sprite->name,sprite->events->length);
	return 0;
}

int Sprite_eventDestroy(SpriteEvent*e)
{
	if(e){
		free(e);
	}
	return 0;
}

int Sprite_removeEventListener(Sprite*sprite,Uint32 type,EventFunc func)
{
	if(stage->currentTarget==sprite)
		stage->currentTarget = NULL;
	if(sprite && type && func && sprite->events)
	{
		int i = sprite->events->length;
		while(i>0){
			--i;
			SpriteEvent*_e = Array_getByIndex(sprite->events,i);
			if(_e->target == sprite && type == _e->type && _e->func == func )
			{
				if(sprite->events->length==1)
					Sprite_removeEvents(sprite);
				else
					sprite->events = Array_removeByIndex(sprite->events,i);
				break;
			}
		}
	}
	if(stage->currentTarget==sprite)
		stage->currentTarget = NULL;
	return 0;
}


Sprite * Sprite_addChildAt(Sprite*parent,Sprite*sprite,int index)
{
	if(parent==NULL || sprite == NULL)
		return sprite;

	if(sprite->parent)
		Sprite_removeChild(sprite->parent,sprite);

	sprite->parent = parent;

	if(parent->children==NULL)
		parent->children = Array_new();
	if(index > parent->children->length)
		index = parent->children->length;
	if(index < 0)
		index = 0;

	parent->children = Array_insert(parent->children,index,sprite);

	return sprite;
}

Sprite*Sprite_addChild(Sprite*parent,Sprite*sprite)
{
	if(parent==NULL)
		return sprite;
	if(sprite==NULL)
		return NULL;
	if(parent->children==NULL)
		parent->children = Array_new();
	return Sprite_addChildAt(parent,sprite,parent->children->length);
}

#if !SDL_VERSION_ATLEAST(2,0,4)
SDL_bool SDL_PointInRect(const SDL_Point *p, const SDL_Rect *r)
{
	return ( (p->x >= r->x) && (p->x < (r->x + r->w)) &&
			(p->y >= r->y) && (p->y < (r->y + r->h)) ) ? SDL_TRUE : SDL_FALSE;
}
#endif

//is the point in the sprite
static int isPointInSprite(Sprite * sprite,SDL_Point * p)
{
	if(sprite->Bounds && Sprite_getVisible(sprite))
	{
		if(SDL_PointInRect(p,sprite->Bounds))
		{
			return 1;
		}
	}
	return 0;
}

//find which child the point is in
Sprite * getPointChild(Sprite * sprite,SDL_Point* p)
{
	if(sprite->mouseChildren == SDL_FALSE)
		return sprite;
	if(sprite->children)
	{
		int i = sprite->children->length;
		while(i>0)
		{
			--i;
			Sprite*child= Sprite_getChildByIndex(sprite,i);
			if(child && isPointInSprite(child,p))
				return child;
		}
	}
	return sprite;
}

//从上到下找出,点(x,y)所在的sprite的最后一级,直至mouseChildren=false,或无子集.
Sprite* getSpriteByStagePoint(int x,int y)
{
	stage->mouse->x = x;
	stage->mouse->y = y;

	Sprite * target = stage->sprite;
	Sprite * child = getPointChild(stage->sprite,stage->mouse);
	while(child!=target)//
	{
		target = child;
		child = getPointChild(target,stage->mouse);
	}
	return target;
}

Sprite * Sprite_removeChildAt(Sprite*sprite,int index)
{
	if(sprite == NULL || sprite->children==NULL || index<0 || index>=sprite->children->length)
		return NULL;
	Sprite*child = Sprite_getChildByIndex(sprite,index);
	if(child){
		child->parent = NULL;
		sprite->children = Array_removeByIndex(sprite->children,index);

		if(sprite->children->length==0)
		{
			Array_clear(sprite->children);
			sprite->children = NULL;
		}
	}
	return child;
}
/**
 *
 * surface->refcount--;
 * Sprite_setSurface():surface->refcount ++;
 */
void Sprite_destroySurface(Sprite*sprite)
{
	if(sprite->surface){
		sprite->surface->refcount--;
		if(sprite->surface->refcount<=0){
			//SDL_Log("surface destroyed:%d\n",sprite->surface->refcount);
			SDL_FreeSurface(sprite->surface);
		}else{
			//SDL_Log("surface refcount--:%d\n",sprite->surface->refcount);
		}
		sprite->surface = NULL;
	}
}

static void Data3d_destroy(Sprite * sprite)
{
	if(sprite == NULL || sprite->data3d==NULL)
		return;
	Data3d * data3d = sprite->data3d;
	if(data3d){
		if(data3d->vertices)free(data3d->vertices);
		if(data3d->indices)free(data3d->indices);
		if(data3d->normals)free(data3d->normals);
		if(data3d->texCoords)free(data3d->texCoords);
		free(data3d);
	}
	sprite->data3d = NULL;
	//sprite->is3D = 0;
}


void Sprite_destroyTexture(Sprite*sprite)
{
	if(stage->GLEScontext){
		if(sprite->destroyFunc==NULL)
			sprite->destroyFunc = Data3d_destroy;
		sprite->destroyFunc(sprite);
	}else{
		if(sprite->texture){
			SDL_DestroyTexture(sprite->texture);
			sprite->texture = NULL;
		}
	}
	if(sprite->textureId){
		GL_CHECK(gles2.glDeleteTextures(1,&(sprite->textureId)));
		//GL_CHECK(gles2.glDeleteRenderbuffers(1, &sprite->renderbuffer));
		//GL_CHECK(gles2.glDeleteFramebuffers(1, &sprite->framebuffer));
		//GL_CHECK(gles2.glDeleteBuffers(sizeof(sprite->vboIds)/sizeof(GLuint), &sprite->vboIds));

		sprite->textureId = 0;
	}

}

int Sprite_destroy(Sprite*sprite)
{
	if(sprite==NULL)
		return 1;

	if(sprite->parent){
		Sprite_removeChild(sprite->parent,sprite);
		sprite->parent = NULL;
	}

	if(sprite->children)
	{
		Sprite_removeChildren(sprite);
		sprite->children = NULL;
	}

	if(sprite->events){
		Sprite_removeEvents(sprite);
		sprite->events = NULL;
	}
	if(sprite->tween && sprite->Tween_kill)
	{
		sprite->Tween_kill(sprite->tween,0);
		sprite->tween = NULL;
		sprite->Tween_kill = NULL;
	}

	Sprite_destroySurface(sprite);
	Sprite_destroyTexture(sprite);

#ifdef __IPHONEOS__
	if(sprite->texCoords){
		free(sprite->texCoords);
	}
#endif
	if(sprite->Bounds){
		free(sprite->Bounds);
	}
	if(sprite->name){
		free(sprite->name);
	}
	free(sprite);
	return 0;
}
int Sprite_removeChildren(Sprite*sprite)
{
	if(sprite==NULL)
		return 1;
	if(Sprite_contains(sprite,stage->currentTarget))
		stage->currentTarget = NULL;
	if(Sprite_contains(sprite,stage->focus))
		stage->focus = NULL;
	while(sprite->children && sprite->children->length>0)
	{
		Sprite*child = Sprite_getChildByIndex(sprite,0);
		if(child && child->children && child->children->length>0){
			Sprite_removeChildren(child);
			child->children = NULL;
		}
		Sprite_removeChildAt(sprite,0);
	}
	if(sprite->children)
		Array_clear(sprite->children);
	sprite->children = NULL;
	return 0;
}

static int _Stage_redraw()
{
	if(stage->GLEScontext){
		SDL_GL_MakeCurrent(stage->window, stage->GLEScontext);
		GL_CHECK(gles2.glViewport ( 0, 0, stage->stage_w, stage->stage_h));
		GL_CHECK(gles2.glClear ( GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT));
		//GL_CHECK(gles2.glClearDepthf(1.0));
		Sprite_show(stage->sprite);
		SDL_GL_SwapWindow(stage->window);
	}else if(stage && stage->renderer){
		SDL_RenderClear(stage->renderer);
		Sprite_show(stage->sprite);
		SDL_RenderPresent(stage->renderer);
	}
	return 0;
}

int Stage_redraw()
{
	UserEvent_new(SDL_USEREVENT,0,_Stage_redraw,NULL);//_Stage_redraw
	return 0;
}

SDL_Surface * Surface_new(int width,int height)
{
	return SDL_CreateRGBSurface(0, width, height, 32, 
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			0xff000000,
			0x00ff0000,
			0x0000ff00,
			0x000000ff
#else
			0x000000ff,
			0x0000ff00,
			0x00ff0000,
			0xff000000
#endif
			);
}

int Sprite_limitPosion(Sprite*target,SDL_Rect*rect)
{
	if(target && rect){
		//SDL_Log("limit_rect:%d,%d,%d,%d\n",rect->x,rect->y,rect->w,rect->h);
		if(target->x < rect->x)
			target->x = rect->x;

		if(target->y < rect->y)
			target->y = rect->y;

		if(target->x > rect->x + rect->w)
			target->x = rect->x + rect->w;

		if(target->y > rect->y + rect->h)
			target->y = rect->y + rect->h;
	}
	return 0;
}

void Sprite_preventDefault(Sprite * target)
{
}

static int stopPropagation=0;
void Sprite_stopPropagation(Sprite * target)
{
	//target->stopPropagation = 1;
	stopPropagation = 1;
}

static void bubbleEvent(Sprite*target,SDL_Event*event)
{
	stage->currentTarget = target;
	stage->lastEventTime = event->motion.timestamp;
	while(target && target!= stage->sprite && target == stage->currentTarget){
		if(target->events && Sprite_getVisible(target) && target->name)
		{
			//SDL_Log("bubbleEvent:%s",target->name);
			if(target==stage->currentTarget)
				Sprite_dispatchEvent(target,event);
		}
		if(stopPropagation){
			stopPropagation = 0;
			return;
		}
		if(target->stopPropagation){
			target->stopPropagation = 0;
			return;
		}
		if(target && stage->currentTarget==target)
		{
			target = target->parent;
			stage->currentTarget = target;
		}
	}
	if(stage->sprite->events)
		Sprite_dispatchEvent(stage->sprite,event);
	stage->currentTarget = NULL;
}


static int button_messagebox(void *eventNumber)
{
	const SDL_MessageBoxButtonData buttons[] = {
		{
			SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
			0,
			"取消"
		},
		{
			SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
			1,
			"确定"
		},
	};

	SDL_MessageBoxData data = {
		SDL_MESSAGEBOX_INFORMATION,
		NULL, /* no parent window */
		"退出",
		"click the button '确定' to quit!",
		2,
		buttons,
		NULL /* Default color scheme */
	};

	int button = -1;
	int success = 0;
	if (eventNumber) {
		data.message = "This is a custom messagebox from a background thread.";
	}

	success = SDL_ShowMessageBox(&data, &button);
	if (success == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error Presenting MessageBox: %s\n", SDL_GetError());
		if (eventNumber) {
			SDL_UserEvent event;
			event.type = (intptr_t)eventNumber;
			SDL_PushEvent((SDL_Event*)&event);
			return 1;
		} else {
			quit(2);
		}
	}
	SDL_Log("Pressed button: %d, %s\n", button, button == 0 ? "Cancel" : "OK");

	if (eventNumber) {
		SDL_UserEvent event;
		event.type = (intptr_t)eventNumber;
		SDL_PushEvent((SDL_Event*)&event);
	}

	return button;
}

void setStageMouse(int x,int y){
	if(stage->mouse == NULL){
		stage->mouse = (SDL_Point*)malloc(sizeof(Point3d));
	}
	stage->mouse->x = x;
	stage->mouse->y = y;
}

int PrintEvent(const SDL_Event * event)
{
	Sprite*target = NULL;
	switch(event->type)
	{
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			setStageMouse(event->button.x,event->button.y);
			target = getSpriteByStagePoint(stage->mouse->x,stage->mouse->y);
			stage->focus = target;
			break;
		case SDL_MOUSEMOTION:
			setStageMouse(event->motion.x,event->motion.y);
			target = getSpriteByStagePoint(stage->mouse->x,stage->mouse->y);
			if(target)
			{
				if(target->canDrag && event->motion.state){
					if(abs(event->motion.xrel)<20 && abs(event->motion.yrel)<20)
					{
						target->x += event->motion.xrel;
						target->y += event->motion.yrel;
					}
					Sprite_limitPosion(target,target->dragRect);
					//SDL_Log("motion->state: %d",event->motion.state);
					//if(target->mouse->x)
					//_Stage_redraw();
				}
			}
			//stage->focus = target;
			break;
		case SDL_FINGERMOTION :
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
			setStageMouse(event->tfinger.x,event->tfinger.y);
			target = getSpriteByStagePoint(event->tfinger.x,event->tfinger.y);
			stage->focus = target;
			break;
		case SDL_MOUSEWHEEL:
			/*
			   SDL_Log("SDL_MOUSEWHEEL:timestamp:%d,windowID:%d,which:%d,deltaX:%d,deltaY:%d\n",//",direction:%d\n",
			   event->wheel.timestamp,
			   event->wheel.windowID,
			   event->wheel.which,
			   event->wheel.x,
			   event->wheel.y
			//,event->wheel.direction
			);
			*/
			target = getSpriteByStagePoint(stage->mouse->x,stage->mouse->y);
			//stage->focus = target;
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if(event->type==SDL_KEYUP && event->key.repeat==0)
			{
				if(event->key.keysym.sym== SDLK_AC_BACK || event->key.keysym.sym == SDLK_ESCAPE){
#if defined(__IPHONEOS__) || defined(__ANDROID__)
#else
					return 1;
#endif
					return button_messagebox(NULL);
					int success = SDL_ShowSimpleMessageBox(
							SDL_MESSAGEBOX_ERROR,
							"退出!",
							"退出!",
							NULL);
					if (success == -1) {
						SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error Presenting MessageBox: %s\n", SDL_GetError());
					}else{
						return 1;
					}
				}else{
					//destroyQuitSprite();
				}
				//_Stage_redraw();
			}
			//break;
		case SDL_TEXTINPUT:
		case SDL_TEXTEDITING:
			if(stage->focus){
				Sprite_dispatchEvent(stage->focus,event);//舞台事件
			}else{
				SDL_Log("no focus\n");
			}
			if(stage->focus!=stage->sprite){
				Sprite_dispatchEvent(stage->sprite,event);//舞台事件
			}
			return 0;
			break;
		case SDL_QUIT:
			SDL_Log("Program quit after %i ticks", event->quit.timestamp);
			quit(0);
			return 1;
			break;

		case SDL_USEREVENT:
			//SDL_Log("SDL_UserEvent _Stage_redraw,%d",event->user.timestamp);
			((void (*)(void*))(event->user.data1))(event->user.data2);
			return 0;
			break;
		case SDL_SYSWMEVENT:
			//SDL_Log("SDL_SYSWMEVENT:timestamp:%d,event->syswm.msg->version:%s\n",
			//event->syswm.timestamp,
			//event->syswm.msg->version);
			return 0;
			break;
		case SDL_WINDOWEVENT:
			switch (event->window.event) {
				case SDL_WINDOWEVENT_SHOWN:
					SDL_Log("Window %d shown", event->window.windowID);
					_Stage_redraw();
					break;
				case SDL_WINDOWEVENT_HIDDEN:
					SDL_Log("Window %d hidden", event->window.windowID);
					break;
				case SDL_WINDOWEVENT_EXPOSED:
					SDL_Log("Window %d exposed", event->window.windowID);
					_Stage_redraw();
					break;
				case SDL_WINDOWEVENT_MOVED:
					SDL_Log("Window %d moved to %d,%d",
							event->window.windowID, event->window.data1,
							event->window.data2);
					break;
				case SDL_WINDOWEVENT_RESIZED:
					SDL_Log("Window %d resized to %dx%d",
							event->window.windowID, event->window.data1,
							event->window.data2);
					Window_resize(event->window.data1,event->window.data2);
					//stage->stage_w = event->window.data1;
					//stage->stage_h = event->window.data2;
					_Stage_redraw();
					break;
				case SDL_WINDOWEVENT_MINIMIZED:
					SDL_Log("Window %d minimized", event->window.windowID);
					break;
				case SDL_WINDOWEVENT_MAXIMIZED:
					SDL_Log("Window %d maximized", event->window.windowID);
					break;
				case SDL_WINDOWEVENT_RESTORED:
					SDL_Log("Window %d restored", event->window.windowID);
					_Stage_redraw();
					break;
				case SDL_WINDOWEVENT_ENTER:
					SDL_Log("Mouse entered window %d",
							event->window.windowID);
					break;
				case SDL_WINDOWEVENT_LEAVE:
					SDL_Log("Mouse left window %d", event->window.windowID);
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					SDL_Log("Window %d gained keyboard focus",
							event->window.windowID);
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					SDL_Log("Window %d lost keyboard focus",
							event->window.windowID);
					break;
				case SDL_WINDOWEVENT_CLOSE:
					SDL_Log("Window %d closed", event->window.windowID);
					break;
				default:
					SDL_Log("Window %d got unknown event %d",
							event->window.windowID, event->window.event);
					break;
			}
			return 0;
			break;
		default:
			SDL_Log("unknown event XXXXXXXXXX \n");
			return 0;
	}
	//SDL_SpinLock lock = 0;
	//SDL_AtomicLock(&lock);
	/*
	   if (SDL_LockMutex(mutex) < 0) {
	   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock mutex: %s", SDL_GetError());
	   quit(1);
	   }
	   */
	//mouse event:
	if(target) {
		bubbleEvent(target,(SDL_Event*)event);//事件冒泡
	}
	/*
	   if (SDL_UnlockMutex(mutex) < 0) {
	   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't unlock mutex: %s", SDL_GetError());
	   quit(1);
	   }
	   */
	//SDL_AtomicUnlock(&lock);
	return 0;
}


void Stage_loopEvents()
{
	/*
	   if ((mutex = SDL_CreateMutex()) == NULL) {
	   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create mutex: %s\n", SDL_GetError());
	   quit(1);
	   }
	   */

	if(stage && stage->sprite->children)_Stage_redraw();
	int done=0;
	while (!done) {
		SDL_Event event;
		//memset(&event,0,sizeof(event));
		//#if SDL_VIDEO_DRIVER_RPI || defined(__ANDROID__)
		if(SDL_WaitEvent(&event))//embed device
			//#else
			//if(SDL_PollEvent(&event)) //pc
			//#endif
		{
			if(SDL_QUIT == event.type) {
				done = 1;
				quit(0);
				break;
			}
			if(PrintEvent(&event)){
				break;
			}
		}
	}
}

GLuint LoadShader(GLenum type, GLbyte *shaderSrc)
{
	GLuint shader; 
	GLint compiled; 
	// Create the shader object 
	shader = GL_CHECK(gles2.glCreateShader(type)); 
	if(shader == 0) 
		return 0; 
	// Load the shader source 
	GL_CHECK(gles2.glShaderSource((GLuint)shader, (GLsizei)1, (const GLchar**)&shaderSrc, NULL)); 
	// Compile the shader 
	GL_CHECK(gles2.glCompileShader(shader)); 
	// Check the compile status 
	GL_CHECK(gles2.glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled)); 
	if(!compiled) 
	{ 
		GLint infoLen = 0; 
		GL_CHECK(gles2.glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen)); 
		if(infoLen > 1) 
		{ 
			char* infoLog = malloc(sizeof(char) * infoLen); 
			GL_CHECK(gles2.glGetShaderInfoLog(shader, infoLen, NULL, infoLog)); 
			SDL_Log("Error compiling shader:\n%s\n%s", infoLog,shaderSrc); 
			free(infoLog); 
		} 
		GL_CHECK(gles2.glDeleteShader(shader)); 
		return 0; 
	} 
	return shader; 
}

GLuint esLoadProgram ( GLbyte * vertShaderSrc, GLbyte * fragShaderSrc )
{
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint programObject;
	GLint linked;
	// Load the vertex/fragment shaders
	vertexShader = LoadShader ( GL_VERTEX_SHADER, vertShaderSrc );
	if ( vertexShader == 0 )
	{
		SDL_Log( "Error vertexShader ==0");            
		return 0;
	}
	fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fragShaderSrc );
	if ( fragmentShader == 0 )
	{
		GL_CHECK(gles2.glDeleteShader( vertexShader ));
		SDL_Log( "Error fragmentShader==0");            
		return 0;
	}
	// Create the program object
	programObject = GL_CHECK(gles2.glCreateProgram ( ));
	if ( programObject == 0 ){
		SDL_Log( "Error programObject==0");            
		return 0;
	}
	GL_CHECK(gles2.glAttachShader ( programObject, vertexShader ));
	GL_CHECK(gles2.glAttachShader ( programObject, fragmentShader ));
	// Link the program
	GL_CHECK(gles2.glLinkProgram ( programObject ));
	// Check the link status
	GL_CHECK(gles2.glGetProgramiv ( programObject, GL_LINK_STATUS, &linked ));
	if ( !linked ) 
	{
		GLint infoLen = 0;
		GL_CHECK(gles2.glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen ));
		if ( infoLen > 1 )
		{
			char* infoLog = malloc (sizeof(char) * infoLen );
			/*GL_CHECK(gles2.glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog ));*/
			SDL_Log( "Error linking program:\n%s\n", infoLog );            
			free ( infoLog );
		}
		GL_CHECK(gles2.glDeleteProgram ( programObject ));
		return 0;
	}
	// Free up no longer needed shader resources
	GL_CHECK(gles2.glDeleteShader ( vertexShader ));
	GL_CHECK(gles2.glDeleteShader ( fragmentShader ));
	return programObject;
}

void Data3d_reset(Data3d * data3D)
{
	if(data3D)
	{
		data3D->alphaLoc = -1;
		data3D->ambientLoc= -1;
		data3D->lightDirection = -1;
		data3D->mvpLoc= -1;
		data3D->normalLoc = -1;
		data3D->positionLoc = -1;
		data3D->samplerLoc = -1;
		data3D->texCoordLoc = -1;
	}
}
void Data3d_set(Data3d * data3D,Data3d * _data3D)
{
	if(data3D)
	{
		data3D->programObject = _data3D->programObject;

		data3D->alphaLoc = _data3D->alphaLoc;
		data3D->ambientLoc = _data3D->ambientLoc;
		data3D->lightDirection = _data3D->lightDirection;
		data3D->mvpLoc= _data3D->mvpLoc;
		data3D->normalLoc = _data3D->normalLoc;
		data3D->positionLoc = _data3D->positionLoc;
		data3D->samplerLoc = _data3D->samplerLoc;
		data3D->texCoordLoc = _data3D->texCoordLoc;
	}
}
Data3d* Data3D_new()
{
	Data3d * data3D = (Data3d*)malloc(sizeof(Data3d));
	memset(data3D,0,sizeof(Data3d));
	Data3d_reset(data3D);
	return data3D;
}

Data3d * Data3D_init()
{
	if(data2D==NULL){
		data2D = Data3D_new();
		GLbyte vShaderStr[] =  
			"uniform mat4 u_mvpMatrix;    \n"
			"attribute vec2 a_texCoord;   \n"
			"attribute vec3 a_position;   \n"
			"attribute vec3 a_normal;   \n"
			"varying vec2 v_texCoord;     \n"
			"varying vec3 v_normal;     \n"
			"varying mat4 v_matrix;     \n"
			"void main()                  \n"
			"{                            \n"
			"	gl_Position = u_mvpMatrix * vec4(a_position,1.0);  \n"//
			"	v_texCoord = a_texCoord;  \n"
			"	v_normal = a_normal;  \n"
			"	v_matrix = u_mvpMatrix;  \n"
			"}                            \n";

		/* fragment shader */
		GLbyte fShaderStr[] =  
			//#ifndef HAVE_OPENGL
			//"precision mediump float;                            \n"
			//#endif
			"#ifdef GL_ES\n"
			"precision mediump float;							\n"
			"#endif												\n"
			//"uniform mat4 u_mvpMatrix;    \n"
			"varying mat4 v_matrix;     \n"
			"uniform vec3 lightDirection;    \n"
			"uniform float u_alpha;   		\n"
			"uniform sampler2D s_texture;                        \n"
			"uniform vec4 Ambient;" // sets lighting level, same across many vertices
			"varying vec2 v_texCoord;                            \n"
			"varying vec3 v_normal;     \n"
			"void main()                                         \n"
			"{                                                   \n"
			"	vec4 vsampler = texture2D( s_texture, v_texCoord );\n"
			"	vec4 color = vsampler*Ambient;"//环境光
			"	float alpha = color.w;"//环境光
			//"	gl_FragColor = min(color,1.0);\n"
			"	\n"
			"	if( v_normal!=vec3(0.0)){\n"
			//"		float l = dot(vec4(u_mvpMatrix*vec4(v_normal,1.0)),vec4(lightDirection,1.0));\n"//单面光照\n"
			"		float l = dot(normalize(vec4(v_matrix*vec4(v_normal,1.0))),normalize(vec4(lightDirection,1.0)))*.5;"//dot(vec4(u_mvpMatrix*vec4(v_normal,1.0)),vec4(lightDirection,1.0));\n"//单面光照\n"
			"		if(lightDirection!=vec3(0.0))"
			"			color += max(0.0,l);\n"//单面光照
			"	}\n"
			"	if(u_alpha>=0.0&&u_alpha<1.0)\n"
			"		color =vec4(vec3(color),alpha*u_alpha);\n"
			"	gl_FragColor = min(color,1.0);\n"
			"}                                                  \n";

		data2D->programObject = esLoadProgram ( vShaderStr, fShaderStr );
		//SDL_Log("programObject:%d\n",data2D->programObject);

		if(data2D->programObject){
			data2D->normalLoc= GL_CHECK(gles2.glGetAttribLocation( data2D->programObject, "a_normal"));
			//SDL_Log("normalLoc:%d\n",data2D->normalLoc);
			data2D->texCoordLoc = GL_CHECK(gles2.glGetAttribLocation ( data2D->programObject, "a_texCoord" ));
			//SDL_Log("texCoordLoc:%d\n",data2D->texCoordLoc);
			data2D->samplerLoc = GL_CHECK(gles2.glGetUniformLocation ( data2D->programObject, "s_texture" ));
			//SDL_Log("samplerLoc:%d\n",data2D->samplerLoc);
			data2D->alphaLoc = GL_CHECK(gles2.glGetUniformLocation (data2D->programObject, "u_alpha"));
			//SDL_Log("alphaLoc:%d\n",data2D->alphaLoc);
			data2D->mvpLoc = GL_CHECK(gles2.glGetUniformLocation( data2D->programObject, "u_mvpMatrix" ));
			//SDL_Log("mvpLoc:%d\n",data2D->mvpLoc);
			data2D->positionLoc = GL_CHECK(gles2.glGetAttribLocation ( data2D->programObject, "a_position" ));
			//SDL_Log("positionLoc:%d\n",data2D->positionLoc);
			data2D->ambientLoc = GL_CHECK(gles2.glGetUniformLocation ( data2D->programObject, "Ambient" ));
			//SDL_Log("ambientLoc:%d\n",data2D->ambientLoc);
			data2D->lightDirection= GL_CHECK(gles2.glGetUniformLocation( data2D->programObject, "lightDirection"));
			//SDL_Log("lightDirection:%d\n",data2D->lightDirection);
		}
	}
	return data2D;
}

void Sprite_translate(Sprite*sprite,int _x,int _y,int _z)
{
	sprite->x = _x;
	sprite->y = _y;
	sprite->z = _z;
}

void Sprite_scale(Sprite*sprite,float scaleX,float scaleY,float scaleZ)
{
	sprite->scaleX = scaleX;
	sprite->scaleY = scaleY;
	sprite->scaleZ = scaleZ;
}


void Sprite_rotate(Sprite*sprite,int _rotationX,int _rotationY,int _rotationZ)
{
	sprite->rotationX = _rotationX % 360;
	sprite->rotationY = _rotationY % 360;
	sprite->rotationZ = _rotationZ % 360;
}

//2D->3D: 映射至第四像限, x:+,y:- , x,y各加屏幕大小的一半
float wto3d(int x){
	return x*2.0/stage->stage_w*stage->world->aspect;
}
float hto3d(int y){
	return y*2.0/stage->stage_h;
}

float xto3d(int x){
	return (x*2.0/stage->stage_w -1.0)*stage->world->aspect;
}
float yto3d(int y){
	return 1.0 - y*2.0/stage->stage_h;
}
float zto3d(int z){
	return z*1.0/stage->stage_w;
}
int zfrom3d(float z){
	return (int)((z)*stage->stage_w);
}
int xfrom3d(float x){
	return (int)((x/stage->world->aspect + 1.0)*stage->stage_w/2.0);
}
int yfrom3d(float y){
	return (int)((1.0 - y)*stage->stage_h/2.0);
}
int wfrom3d(float w){
	return (int)((w/stage->world->aspect)*stage->stage_w/2.0);
}
int hfrom3d(float h){
	return (int)((h)*stage->stage_h/2.0);
}


/**
 *
 * surface->refcount ++;
 * Sprite_destroySurface():surface->refcount--;
 */
void Sprite_setSurface(Sprite * sprite,SDL_Surface * surface)
{
	if(sprite==NULL)
		return;
	//sprite->w=0; sprite->h=0;
	sprite->textureId = 0;
	Sprite_destroySurface(sprite);
	Sprite_destroyTexture(sprite);
	if(surface){
		sprite->surface = surface;
		surface->refcount++;
	}

}


void Sprite_center(Sprite*sprite,int x,int y,int w,int h)
{
	if(sprite == NULL)return;
	if(sprite->w * sprite->h > 0){
	}else if(sprite->surface){
		sprite->w = sprite->surface->w;
		sprite->h = sprite->surface->h;
	}
	sprite->x = x + w/2 - sprite->w/2;
	sprite->y = y + h/2 - sprite->h/2;
}
void Sprite_centerRect(Sprite*sprite,SDL_Rect*rect)
{
	if(sprite && rect)
	{
		Sprite_center(sprite,rect->x,rect->y,rect->w,rect->h);
	}
}

void Sprite_fullcenter(Sprite*sprite,int x,int y,int w,int h)
{
	if(sprite == NULL)return;
	if(sprite->w * sprite->h == 0){
		if(sprite->surface){
			sprite->w = sprite->surface->w;
			sprite->h = sprite->surface->h;
		}else{
			return;
		}
	}
	float scaleX = (float)w/sprite->w;
	float scaleY = (float)h/sprite->h;
	float scale = scaleX<scaleY?scaleX:scaleY;
	sprite->w = scale*sprite->w;
	sprite->h = scale*sprite->h;
	Sprite_center(sprite,x,y,w,h);
}
void Sprite_fullcenterRect(Sprite*sprite,SDL_Rect*rect)
{
	if(sprite && rect)
	{
		Sprite_fullcenter(sprite,rect->x,rect->y,rect->w,rect->h);
	}
}

SDL_Surface * Stage_readpixel(Sprite *sprite,SDL_Rect* rect)
{
	int w = rect->w;
	int h = rect->h;
	int x = rect->x;
	int y = rect->y;
	SDL_Surface * image = Surface_new(w,h);
	if (image == NULL) {
		return NULL;
	}
	int line = 0;
	for(line=0;line<h;line++){
		GL_CHECK(gles2.glReadPixels(x,  y+line,  w,  1,  GL_RGBA, GL_UNSIGNED_BYTE,  (char*)(image->pixels)+w*(h-line-1)*4));
	}
	return image;
}

#ifdef debug_sprite


#include "httploader.h"

extern Data3d * data2D;
static void mouseDown(SpriteEvent*e)
{
	if(e->target == stage->sprite)return;//e->target->y++;
	//Sprite * sprite1 = Sprite_getChildByName(stage->sprite,"sprite1");
	SDL_Log("mouseDown:-----------------------------%s,%d,%d,\n"
			,e->target->name
			,stage->mouse->x
			,stage->mouse->y
		   );
	if(e->target->parent)
		Sprite_addChild(e->target->parent,e->target);
}

static void mouseMove(SpriteEvent*e)
{
	Sprite * sprite = (Sprite *)e->target;
	SDL_Event * event = (SDL_Event*)e->e;
	if(e->target->parent)
		Sprite_addChild(e->target->parent,e->target);

	/*
	   event->motion.timestamp,
	   event->motion.windowID,
	   event->motion.which,
	   event->motion.state,
	   event->motion.xrel,
	   event->motion.yrel
	   */
	if(event->motion.state){
		sprite->rotationX += event->motion.yrel;
		sprite->rotationY += event->motion.xrel;
		Sprite_rotate(sprite  ,sprite->rotationX,sprite->rotationY,sprite->rotationZ);
		//Sprite_translate(sprite ,sprite->x + event->motion.xrel ,sprite->y + event->motion.yrel ,sprite->z);
		//Sprite_scale(sprite ,sprite->scaleX *(1 + event->motion.yrel*.01) ,sprite->scaleY *(1 + event->motion.yrel*.01),sprite->scaleZ *(1 + event->motion.yrel*.001));
		Stage_redraw();
	}
}

#include "files.h"
int main(int argc, char *argv[])
{
    SDL_SetMainReady();
	Stage_init();
#if SDL_VERSION_ATLEAST(2,0,5)
	SDL_Log("set opacity\n");
	//SDL_SetWindowOpacity(stage->window,.5);
#endif
#ifdef HAVE_OPENGL
	SDL_Log("HAVE_OPENGL\n");
#else
	SDL_Log("OPENGL_ES\n");
#endif
	Sprite_addEventListener(stage->sprite,SDL_MOUSEBUTTONDOWN,mouseDown);
	char * path = decodePath("~/sound/1.bmp");
	//char * path = ("1.bmp");
#ifdef __ANDROID__
	//path = ("/sdcard/sound/1.bmp");
#endif

	if(stage->GLEScontext){
		Sprite*sprite = Sprite_new();
		sprite->is3D = 1;
		sprite->surface = (SDL_LoadBMP(path));
		SDL_Log("surface size:%dx%d",sprite->surface->w,sprite->surface->h);
		Data3d *_data3D = sprite->data3d;
		if(_data3D==NULL){
			_data3D = Data3D_new();
			if(_data3D->programObject==0){
				data2D = Data3D_init();
				Data3d_set(_data3D,data2D);
			}
			sprite->data3d = _data3D;
			_data3D->numIndices = esGenSphere ( 20, .75f, &_data3D->vertices, &_data3D->normals, &_data3D->texCoords, &_data3D->indices );
			int numSlices = 20;
			_data3D->numIndices = esGenSphere ( numSlices, .75f, &_data3D->vertices, &_data3D->normals, &_data3D->texCoords, &_data3D->indices );
			int numParallels = numSlices / 2;
			_data3D->numVertices = ( numParallels + 1 ) * ( numSlices + 1 );
			//_data3D->numIndices = esGenSphere ( 20, 15.f, &_data3D->vertices, &_data3D->normals, &_data3D->texCoords, &_data3D->indices );
			//_data3D->numIndices = esGenCube(  0.75f, &_data3D->vertices, &_data3D->normals, &_data3D->texCoords, &_data3D->indices );
		}
		sprite->alpha = 0.5;
		Sprite*contener= Sprite_new();
		Sprite_addChild(stage->sprite,contener);
		Sprite_addChild(contener,sprite);
		contener->x = stage->stage_w/2;
		contener->y = stage->stage_h/2;
		contener->w = stage->stage_w;
		contener->h = stage->stage_h;
		Sprite_addEventListener(sprite,SDL_MOUSEMOTION,mouseMove);
		/***
		  Sprite*sprite2 = Sprite_new();
		  sprite2->surface = SDL_LoadBMP(path);
		//sprite2->texture = SDL_CreateTextureFromSurface(stage->renderer,sprite2->surface);
		//sprite2->filter= 3;
		sprite2->rotationZ =90;
		sprite2->x =0;
		sprite2->y =0;
		//sprite2->w =stage->stage_w;
		//sprite2->h =stage->stage_h;
		//sprite2->scaleX =2.0;
		//sprite2->scaleY =2.0;
		sprite2->mouseChildren = SDL_FALSE;
		sprite2->canDrag = SDL_TRUE;
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = 0;
		rect.h = stage->stage_h-sprite2->h;
		sprite2->dragRect = &rect;
		Sprite_addChild(stage->sprite,sprite2);

		Sprite_addEventListener(sprite2,SDL_MOUSEBUTTONDOWN,mouseDown);
		//Sprite_removeEventListener(sprite2,SDL_MOUSEBUTTONDOWN,mouseDown);
		*/
	}else{
		SDL_Log("-------------\n");
	}
	Sprite*sprite3 = Sprite_new();
	sprite3->surface = (SDL_LoadBMP(path));
	sprite3->filter= 3;
	sprite3->y = 0;
	sprite3->x = 0;
	sprite3->w = 100;
	sprite3->h = 100;
	sprite3->scaleX = 2.0;
	SDL_Rect rect;
	rect.x = 10;
	rect.y = 20;
	rect.w = 50;
	rect.h = 50;
	sprite3->mask= &rect;
	sprite3->canDrag = SDL_TRUE;
	Sprite_addChild(stage->sprite,sprite3);
	//sprite3->visible = 0;
	Sprite_addEventListener(sprite3,SDL_MOUSEBUTTONDOWN,mouseDown);
	/***
	  SDL_Log("stage ----------- size:%dx%d\n",stage->stage_w,stage->stage_h);
	  Sprite *sprite4 = Sprite_new();
	  sprite4->surface = Httploader_loadimg("http://res1.huaien.com/images/tx.jpg");
	  sprite4->alpha = .9;
	  sprite4->filter= 1;
	  sprite4->w =stage->stage_w;
	  sprite4->h =stage->stage_h;
	  if(sprite4->surface){
	//sprite4->rotationZ = 180;
	Sprite_center(sprite4,0,0,stage->stage_w,stage->stage_h);
	Sprite_addChildAt(stage,sprite4,0);
	}else{
	Sprite_destroy(sprite4);
	}
	*/
	//Stage_redraw();
	//Sprite_alertText("hello,一切正常！");
	Stage_loopEvents();
	//SDL_Delay(20);
	//exit(0);
	return 0;
	/**
	  SDL_SaveBMP(Stage_readpixel(stage,sprite4->Bounds),"stage.bmp");
	  */
}
#endif
