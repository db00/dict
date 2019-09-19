/**
 * @file graphics.c
 gcc -g -Wall -I"../SDL2/include/" graphics.c files.c mystring.c myregex.c array.c  sprite.c matrix.c  -lm -lSDL2 -D debug_graphics && ./a.out
 * sprite 3d graphics using sampler
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2016-02-02
 */
#include "graphics.h"
#ifdef debug_graphics

/*
   Graphics_beginBitmapFill(bitmap:BitmapData, matrix:Matrix = null, repeat:Boolean = true, smooth:Boolean = false):void// 用位图图像填充绘图区。 Graphics 
   beginFill(color:uint, alpha:Number = 1.0):void// 指定一种简单的单一颜色填充，在绘制时该填充将在随后对其它 Graphics 方法（如 lineTo() 或 drawCircle()）的调用中使用。 Graphics 
   beginGradientFill(type:String, colors:Array, alphas:Array, ratios:Array, matrix:Matrix = null, spreadMethod:String = "pad", interpolationMethod:String = "rgb", focalPointRatio:Number = 0):void// 指定一种渐变填充，用于随后调用对象的其它 Graphics 方法（如 lineTo() 或 drawCircle()）。 Graphics 
   beginShaderFill(shader:Shader, matrix:Matrix = null):void// 为对象指定着色器填充，供随后调用其它 Graphics 方法（如 lineTo() 或 drawCircle()）时使用。 Graphics 
   clear():void// 清除绘制到此 Graphics 对象的图形，并重置填充和线条样式设置。 Graphics 
   copyFrom(sourceGraphics:Graphics):void// 将源 Graphics 对象中的所有绘画命令复制到执行调用的 Graphics 对象中。 Graphics 
   curveTo(controlX:Number, controlY:Number, anchorX:Number, anchorY:Number):void// 使用当前线条样式和由 (controlX, controlY) 指定的控制点绘制一条从当前绘画位置开始到 (anchorX, anchorY) 结束的曲线。 Graphics 
   drawCircle(x:Number, y:Number, radius:Number):void// 绘制一个圆。 Graphics 
   drawEllipse(x:Number, y:Number, width:Number, height:Number):void// 绘制一个椭圆。 Graphics 
   drawGraphicsData(graphicsData:Vector.<IGraphicsData>):void// 提交一系列 IGraphicsData 实例来进行绘图。 Graphics 
   drawPath(commands:Vector.<int>, data:Vector.<Number>, winding:String = "evenOdd"):void// 提交一系列绘制命令。 Graphics 
   drawRect(x:Number, y:Number, width:Number, height:Number):void// 绘制一个矩形。 Graphics 
   drawRoundRect(x:Number, y:Number, width:Number, height:Number, ellipseWidth:Number, ellipseHeight:Number = NaN):void// 绘制一个圆角矩形。 Graphics 
   drawTriangles(vertices:Vector.<Number>, indices:Vector.<int> = null, uvtData:Vector.<Number> = null, culling:String = "none"):void// 呈现一组三角形（通常用于扭曲位图），并为其指定三维外观。 Graphics 
   endFill():void// 对从上一次调用 beginFill()、beginGradientFill() 或 beginBitmapFill() 方法之后添加的直线和曲线应用填充。 Graphics 
   hasOwnProperty(name:String):Boolean// 指示对象是否已经定义了指定的属性。 Object 
   isPrototypeOf(theClass:Object):Boolean// 指示 Object 类的实例是否在指定为参数的对象的原型链中。 Object 
   lineBitmapStyle(bitmap:BitmapData, matrix:Matrix = null, repeat:Boolean = true, smooth:Boolean = false):void// 指定一个位图，用于绘制线条时的线条笔触。 Graphics 
   lineGradientStyle(type:String, colors:Array, alphas:Array, ratios:Array, matrix:Matrix = null, spreadMethod:String = "pad", interpolationMethod:String = "rgb", focalPointRatio:Number = 0):void// 指定一种渐变，用于绘制线条时的笔触。 Graphics 
   lineShaderStyle(shader:Shader, matrix:Matrix = null):void// 指定一个着色器以用于绘制线条时的线条笔触。 Graphics 
   lineStyle(thickness:Number = NaN, color:uint = 0, alpha:Number = 1.0, pixelHinting:Boolean = false, scaleMode:String = "normal", caps:String = null, joints:String = null, miterLimit:Number = 3):void// 指定一种线条样式以用于随后对 lineTo() 或 drawCircle() 等 Graphics 方法的调用。 Graphics 
   lineTo(x:Number, y:Number):void// 使用当前线条样式绘制一条从当前绘画位置开始到 (x, y) 结束的直线；当前绘画位置随后会设置为 (x, y)。 Graphics 
   moveTo(x:Number, y:Number):void// 将当前绘画位置移动到 (x, y)。 
   */

typedef struct UserData
{
	GLuint programObject;
	GLint positionLoc;
	GLint colorLoc;
	GLint mvpLoc;
	GLfloat *vertices;
	GLfloat *normals;
	GLfloat *texCoords;
	GLuint  *indices;
} UserData;

static void Data3d_destroy(Sprite * sprite)
{
	if(sprite == NULL || sprite->data3d==NULL)
		return;
	UserData * data3d = sprite->data3d;
	if(data3d){
		if(data3d->vertices)free(data3d->vertices);
		if(data3d->indices)free(data3d->indices);
		if(data3d->normals)free(data3d->normals);
		if(data3d->texCoords)free(data3d->texCoords);
		free(data3d);
	}
	sprite->data3d= NULL;
}

static void Data3d_show(Sprite*sprite)
{
	UserData * userData = sprite->data3d;
	if(userData==NULL)
		return ;
	if(userData->programObject == 0)
	{
		GLbyte vShaderStr[] =  
			"uniform mat4 u_mvpMatrix;    \n"
			"attribute vec4 vPosition;    \n"
			"attribute vec4 color;    \n"
			"varying vec4 vcolor;    \n"
			"void main()                  \n"
			"{                            \n"
			"   vcolor = color;  \n"
			"   gl_Position = u_mvpMatrix * vec4(vPosition);  \n"//
			"}                            \n";

		GLbyte fShaderStr[] =  
#ifndef HAVE_OPENGL
			"precision mediump float;\n"
#endif
			"varying vec4 vcolor;    \n"
			"void main()                                  \n"
			"{                                            \n"
			"  gl_FragColor = vec4 ( vcolor );\n"
			"}                                            \n";

		GLuint vertexShader;
		GLuint fragmentShader;
		GLuint programObject;
		GLint linked;
		// Load the vertex/fragment shaders
		vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
		fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );
		// Create the program object
		programObject = GL_CHECK(gles2.glCreateProgram ());
		if ( programObject == 0 ){
			SDL_Log( "programObject==0\n");
			return ;
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
				char infoLog [(sizeof(char) * infoLen )];
				GL_CHECK(gles2.glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog ));
				SDL_Log( "Error linking program:\n%s\n", infoLog );
				//esLogMessage ( "Error linking program:\n%s\n", infoLog );            
			}
			GL_CHECK(gles2.glDeleteProgram ( programObject ));
			return ;
		}
		userData->programObject = programObject;
		if(programObject){
			userData->positionLoc = GL_CHECK(gles2.glGetAttribLocation ( programObject, "vPosition" ));
			SDL_Log("positionLoc:%d\n",userData->positionLoc);
			userData->colorLoc = GL_CHECK(gles2.glGetAttribLocation( programObject, "color"));
			SDL_Log("colorLoc:%d\n",userData->colorLoc);
			userData->mvpLoc = GL_CHECK(gles2.glGetUniformLocation( programObject, "u_mvpMatrix" ));
			SDL_Log("mvpLoc:%d\n",userData->mvpLoc);
		}
		SDL_Log( "linking program:\n%d\n", programObject);
		//GL_CHECK(gles2.glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f ));
	}

	if(userData->programObject)
	{
		GL_CHECK(gles2.glUseProgram ( userData->programObject));
	}else{
		return;
	}

	GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f, 
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f };
	GLfloat colors[]={
		1.0,0.0,.0,1.0,
		0.0,1.0,0.0,1.0,
		0.0,.0,1.0,1.0,
	};
	GLfloat colors2[]={
		1.0,1.0,1.0,1.0,
		1.0,1.0,1.0,1.0,
		1.0,1.0,1.0,1.0,
	};
	GLushort indices[] = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8
	};

	if(userData->mvpLoc>=0){
		Sprite_matrix(sprite);
		GL_CHECK(gles2.glUniformMatrix4fv( userData->mvpLoc, 1, GL_FALSE, (GLfloat*) &sprite->mvpMatrix.rawData[0][0]));
	}
	GL_CHECK(gles2.glDisable( GL_CULL_FACE ));
	// Load the vertex data
	GL_CHECK(gles2.glVertexAttribPointer ( userData->positionLoc, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), vVertices ));
	GL_CHECK(gles2.glEnableVertexAttribArray ( userData->positionLoc ));
	GL_CHECK(gles2.glVertexAttribPointer ( userData->colorLoc, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), colors));
	GL_CHECK(gles2.glEnableVertexAttribArray ( userData->colorLoc));
	GL_CHECK(gles2.glDrawArrays ( GL_TRIANGLES, 0, 3 ));
	//GL_CHECK(gles2.glDrawArrays( GL_LINES,0, 8));//, GL_UNSIGNED_SHORT, indices);
	GL_CHECK(gles2.glVertexAttribPointer ( userData->colorLoc, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), colors2));
	GL_CHECK(gles2.glEnableVertexAttribArray ( userData->colorLoc));
	GL_CHECK(gles2.glDrawElements ( GL_LINE_LOOP, 3, GL_UNSIGNED_SHORT, indices));
	//GL_CHECK(gles2.glDrawElements ( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, indices));
	//eglSwapBuffers ( esContext->eglDisplay, esContext->eglSurface );
}


static void mouseDown(SpriteEvent*e)
{/*{{{*/
	//Sprite * sprite1 = Sprite_getChildByName(stage,"sprite1");
	SDL_Log("mouseDown:-----------------------------%s,%d,%d,\n"
			,e->target->name
			,stage->mouse->x
			,stage->mouse->y
		   );
	if(e->target->parent)
		Sprite_addChild(e->target->parent,e->target);
	if(e->target!= stage->sprite)e->target->y++;
}/*}}}*/

static void mouseMove(SpriteEvent*e)
{/*{{{*/
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
}/*}}}*/


#include "files.h"
int main(int argc, char *argv[])
{/*{{{*/
	Stage_init();
	Sprite_addEventListener(stage->sprite,SDL_MOUSEBUTTONDOWN,mouseDown);
	if(stage->GLEScontext == NULL){
		//return 0;
	}else {
		UserData userData;
		memset(&userData,0,sizeof(UserData));

		Sprite*sprite = Sprite_new();
		//sprite->x = 100;
		//sprite->y = 100;
		sprite->w = 100;
		sprite->h = 100;
		sprite->data3d = &userData;
		sprite->showFunc = Data3d_show;
		sprite->destroyFunc = Data3d_destroy;
		//Sprite_addChild(stage->sprite,sprite);

		Sprite*container= Sprite_new();
		Sprite_addChildAt(stage->sprite,container,0);
		Sprite_addChild(container,sprite);
		//container->x = stage->stage_w/2;
		//container->y = stage->stage_h/2;
		//container->w = stage->stage_w;
		//container->h = stage->stage_h;
		container->alpha = 0.8;
		container->surface = SDL_LoadBMP(decodePath("~/sound/1.bmp"));

		Sprite_addEventListener(container,SDL_MOUSEMOTION,mouseMove);
		//Sprite_addEventListener(sprite,SDL_MOUSEMOTION,mouseMove);
	}
	Stage_loopEvents();
	exit(0);
	return 0;
}/*}}}*/
#endif
