/*
 *
 gcc Multisample.c  -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/"  -L. -lGLESv2 -lm -lSDL2 -lSDL2_ttf &&  ./a.out
 gcc Multisample.c  -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/"  -L. -lGLESv2 -lmingw32 -lSDL2_test -lSDL2main -lSDL2 -lSDL2_ttf && a 
 gcc Multisample.c SDL_test_common.c -lm -lSDL2 -lSDL2_ttf && ./a.out && ./a.out
 gcc Multisample.c -llibGLESv2 -lSDL2_test -lm -lmingw32 -lSDL2main -lSDL2   && a
 adb push 2.c /sdcard/ &&
 there is a bug in xp,can not show correctly

 opengl 模板测试 glStencilOp glStencilFunc
 下面来设置蒙板缓存和蒙板测试。
 首先我们启用蒙板测试，这样就可以修改蒙板缓存中的值。

 下面我们来解释蒙板测试函数的含义：
 当你使用glEnable(GL_STENCIL_TEST)启用蒙板测试之后，蒙板函数用于确定一个颜色片段是应该丢弃还是保留（被绘制）。蒙板缓存区 中的值与参考值ref进行比较，比较标准是func所指定的比较函数。参考值和蒙板缓存区的值都可以与掩码进行为AND操作。蒙板测试的结果还导致蒙板缓 存区根据glStencilOp函数所指定的行为进行修改。
 void glStencilFunc(GLenum func,  GLint ref,  GLuint mask);
 func的参数值如下：

 常量	含义
 GL_NEVER	从不通过蒙板测试
 GL_ALWAYS	总是通过蒙板测试
 GL_LESS	只有参考值<(蒙板缓存区的值&mask)时才通过
 GL_LEQUAL	只有参考值<=(蒙板缓存区的值&mask)时才通过
 GL_EQUAL	只有参考值=(蒙板缓存区的值&mask)时才通过
 GL_GEQUAL	只有参考值>=(蒙板缓存区的值&mask)时才通过
 GL_GREATER	只有参考值>(蒙板缓存区的值&mask)时才通过
 GL_NOTEQUAL	只有参考值!=(蒙板缓存区的值&mask)时才通过
 接下来我们解释glStencilOp函数，它用来根据比较结果修改蒙板缓存区中的值，它的函数原形为：
 void glStencilOp(GLenum sfail, GLenum zfail, GLenum zpass)，各个参数的含义如下：
 sfail 当蒙板测试失败时所执行的操作
 zfail 当蒙板测试通过，深度测试失败时所执行的操作
 zpass 当蒙板测试通过，深度测试通过时所执行的操作

 具体的操作包括以下几种
 GL_KEEP	保持当前的蒙板缓存区值
 GL_ZERO	把当前的蒙板缓存区值设为0
 GL_REPLACE	用glStencilFunc函数所指定的参考值替换蒙板参数值
 GL_INCR	增加当前的蒙板缓存区值，但限制在允许的范围内
 GL_DECR	减少当前的蒙板缓存区值，但限制在允许的范围内
 GL_INVERT	将当前的蒙板缓存区值进行逐位的翻转
 当完成了以上操作后我们绘制一个地面，当然现在你什么也看不到，它只是把覆盖地面的蒙板缓存区中的相应位置设为1。

 glEnable(GL_STENCIL_TEST);				// 启用蒙板缓存
 glStencilFunc(GL_ALWAYS, 1, 1);			// 设置蒙板测试总是通过，参考值设为1，掩码值也设为1
 glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);		// 设置当深度测试不通过时，保留蒙板中的值不变。如果通过则使用参考值替换蒙板值
 glDisable(GL_DEPTH_TEST);				// 禁用深度测试
 DrawFloor();					// 绘制地面
 */
#include "gles2base0.h"
#include "SDL.h"

#define NumTests  4
int Init (UserData *userData)
{


	GLbyte vShaderStr[] =  
		"attribute vec4 a_position;   \n"
		"void main()                  \n"
		"{                            \n"
		"   gl_Position = a_position; \n"
		"}                            \n";

	GLbyte fShaderStr[] =  
		"precision mediump float;  \n"
		"uniform vec4  u_color;    \n"
		"void main()               \n"
		"{                         \n"
		"  gl_FragColor = u_color; \n"
		"}                         \n";

	// Load the shaders and get a linked program object
	userData->programObject = esLoadProgram ( vShaderStr, fShaderStr );

	// Get the attribute locations
	userData->positionLoc = gles2.glGetAttribLocation ( userData->programObject, "a_position" );

	// Get the sampler location
	userData->colorLoc = gles2.glGetUniformLocation ( userData->programObject, "u_color" );

	// Set the clear color
	gles2.glClearColor ( 0.f, 0.0f, 0.0f, 0.0f );

	// Set the stencil clear value
	gles2.glClearStencil ( 0x1 );

	// Set the depth clear value
	gles2.glClearDepthf( 0.75f );

	// Enable the depth and stencil tests
	gles2.glEnable( GL_DEPTH_TEST );
	gles2.glEnable( GL_STENCIL_TEST );

	return 1;
}

///
// Initialize the stencil buffer values, and then use those
//   values to control rendering
//
void Draw ( UserData *userData)
{
	int  i;
	GLfloat vVertices[] = { 
		-0.75f,  0.25f,  0.50f, // Quad #0
		-0.25f,  0.25f,  0.50f,
		-0.25f,  0.75f,  0.50f,
		-0.75f,  0.75f,  0.50f,
		0.25f,  0.25f,  0.90f, // Quad #1
		0.75f,  0.25f,  0.90f,
		0.75f,  0.75f,  0.90f,
		0.25f,  0.75f,  0.90f,
		-0.75f, -0.75f,  0.50f, // Quad #2
		-0.25f, -0.75f,  0.50f,
		-0.25f, -0.25f,  0.50f,
		-0.75f, -0.25f,  0.50f,
		0.25f, -0.75f,  0.50f, // Quad #3
		0.75f, -0.75f,  0.50f,
		0.75f, -0.25f,  0.50f,
		0.25f, -0.25f,  0.50f,
		-1.00f, -1.00f,  0.00f, // Big Quad
		1.00f, -1.00f,  0.00f,
		1.00f,  1.00f,  0.00f,
		-1.00f,  1.00f,  0.00f
	};

	GLubyte indices[][6] = { 
		{  0,  1,  2,  0,  2,  3 }, // Quad #0
		{  4,  5,  6,  4,  6,  7 }, // Quad #1
		{  8,  9, 10,  8, 10, 11 }, // Quad #2
		{ 12, 13, 14, 12, 14, 15 }, // Quad #3
		{ 16, 17, 18, 16, 18, 19 }  // Big Quad
	};

	GLfloat  colors[NumTests][4] = { 
		{ 1.0f, 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f, 0.0f }
	};

	GLint   numStencilBits;
	GLuint  stencilValues[NumTests] = { 
		0x7, // Result of test 0
		0x0, // Result of test 1
		0x2, // Result of test 2
		0xff // Result of test 3.  We need to fill this
			//  value in a run-time
	};

	// Set the viewport
	gles2.glViewport ( 0, 0, state->window_w, state->window_h);

	// Clear the color, depth, and stencil buffers.  At this
	//   point, the stencil buffer will be 0x1 for all pixels
	gles2.glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	// Use the program object
	gles2.glUseProgram ( userData->programObject );

	// Load the vertex position
	gles2.glVertexAttribPointer ( userData->positionLoc, 3, GL_FLOAT, 
			GL_FALSE, 0, vVertices );

	gles2.glEnableVertexAttribArray ( userData->positionLoc );

	// Test 0:
	//
	// Initialize upper-left region.  In this case, the
	//   stencil-buffer values will be replaced because the
	//   stencil test for the rendered pixels will fail the
	//   stencil test, which is
	//
	//        ref   mask   stencil  mask
	//      ( 0x7 & 0x3 ) < ( 0x1 & 0x7 )   3<1 fail GL_REPLACE 7 
	//
	//   The value in the stencil buffer for these pixels will
	//   be 0x7.
	//
	gles2.glStencilFunc( GL_LESS, 0x7, 0x3 );
	gles2.glStencilOp( GL_REPLACE, GL_DECR, GL_DECR );
	gles2.glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices[0] );

	// Test 1:
	//
	// Initialize the upper-right region.  Here, we'll decrement
	//   the stencil-buffer values where the stencil test passes
	//   but the depth test fails.  The stencil test is
	//
	//        ref  mask    stencil  mask
	//      ( 0x3 & 0x3 ) > ( 0x1 & 0x3 ) 3>1 pass 0.9>.75 depth fail GL_DECR 1
	//
	//    but where the geometry fails the depth test.  The
	//    stencil values for these pixels will be 0x0.
	//
	gles2.glStencilFunc( GL_GREATER, 0x3, 0x3 );
	gles2.glStencilOp( GL_KEEP, GL_DECR, GL_KEEP );
	gles2.glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices[1] );

	// Test 2:
	//
	// Initialize the lower-left region.  Here we'll increment 
	//   (with saturation) the stencil value where both the
	//   stencil and depth tests pass.  The stencil test for
	//   these pixels will be
	//
	//        ref  mask     stencil  mask
	//      ( 0x1 & 0x3 ) == ( 0x1 & 0x3 ) 1==1 pass .5<.75 depth pass GL_INCR 1
	//
	//   The stencil values for these pixels will be 0x2.
	//
	gles2.glStencilFunc( GL_EQUAL, 0x1, 0x3 );
	gles2.glStencilOp( GL_KEEP, GL_INCR, GL_INCR );
	gles2.glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices[2] );

	// Test 3:
	//
	// Finally, initialize the lower-right region.  We'll invert
	//   the stencil value where the stencil tests fails.  The
	//   stencil test for these pixels will be
	//
	//        ref   mask    stencil  mask
	//      ( 0x2 & 0x1 ) == ( 0x1 & 0x1 ) 0==1 fail GL_INVERT
	//
	//   The stencil value here will be set to ~((2^s-1) & 0x1),
	//   (with the 0x1 being from the stencil clear value),
	//   where 's' is the number of bits in the stencil buffer
	//
	gles2.glStencilFunc( GL_EQUAL, 0x2, 0x1 );
	gles2.glStencilOp( GL_INVERT, GL_KEEP, GL_KEEP );
	gles2.glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices[3] );

	// Since we don't know at compile time how many stecil bits are present,
	//   we'll query, and update the value correct value in the
	//   stencilValues arrays for the fourth tests.  We'll use this value
	//   later in rendering.
	gles2.glGetIntegerv( GL_STENCIL_BITS, &numStencilBits );

	stencilValues[3] = ~(((1 << numStencilBits) - 1) & 0x1) & 0xff;

	// Use the stencil buffer for controlling where rendering will
	//   occur.  We diable writing to the stencil buffer so we
	//   can test against them without modifying the values we
	//   generated.
	gles2.glStencilMask( 0x0 );

	for ( i = 0; i < NumTests; ++i )
	{
		//7,0,2,ff, 
		gles2.glStencilFunc( GL_EQUAL, stencilValues[i], 0xff );
		gles2.glUniform4fv( userData->colorLoc, 1, colors[i] );
		gles2.glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices[4] );
	}

	/*eglSwapBuffers ( esContext->eglDisplay, esContext->eglSurface );*/
}


