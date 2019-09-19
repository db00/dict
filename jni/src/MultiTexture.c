/*
 *
   gcc MultiTexture.c ../SDL2/src/test/SDL_test_common.c -I"../SDL2_ttf/" -I"../SDL2/include/" -I"../SDL2_image/" -lGLESv2 -lm -lSDL2 -lSDL2_ttf && ./a.out
   gcc MultiTexture.c -I"../SDL2_ttf/" -I"../SDL2/include/" -I"../SDL2_image/" -L. -lGLESv2 -lmingw32 -lSDL2_test -lSDL2main -lSDL2 -lSDL2_ttf && a 
   gcc MultiTexture.c SDL_test_common.c -lm -lSDL2 -lSDL2_ttf && ./a.out && ./a.out
   gcc MultiTexture.c -llibGLESv2 -lSDL2_test -lm -lmingw32 -lSDL2main -lSDL2   && a
   adb push 2.c /sdcard/ &&
   Copyright (r) 1997-2014 Sam Lantinga <slouken@libsdl.org>

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely.
   */
#include "gles2base0.h"
#include "SDL.h"
/**
 *
"precision mediump float; \n"
"uniform sampler2D s_baseMap; \n"
"varying vec2 v_texCoord; \n"
"uniform float u_blurStep; \n"
"void main(void) \n"
"{ \n"
"   vec4 sample0, \n"
"        sample1, \n"
"        sample2, \n"
"        sample3; \n"
"         \n"
"   float step = u_blurStep / 100.0; \n"
"         \n"
"   sample0 = texture2D(s_baseMap,  \n"
"                       vec2(v_texCoord.x - step,  \n"
"                            v_texCoord.y - step)); \n"
"   sample1 = texture2D(s_baseMap,  \n"
"                       vec2(v_texCoord.x + step,  \n"
"                            v_texCoord.y + step)); \n"
"   sample2 = texture2D(s_baseMap,  \n"
"                       vec2(v_texCoord.x + step,  \n"
"                            v_texCoord.y - step)); \n"
"   sample3 = texture2D(s_baseMap,  \n"
"                       vec2(v_texCoord.x - step,  \n"
"                            v_texCoord.y + step)); \n"
"                         \n"
"   gl_FragColor = (sample0 + sample1 + sample2 + sample3) / 4.0; \n"
"} \n";
 */

///
// Initialize the shader and program object
//
int Init (UserData *userData)
{
	GLbyte vShaderStr[] =  
		"attribute vec4 a_position;   \n"
		"attribute vec2 a_texCoord;   \n"
		"varying vec2 v_texCoord;     \n"
		"void main()                  \n"
		"{                            \n"
		"   gl_Position = a_position; \n"
		"   v_texCoord = a_texCoord;  \n"
		"}                            \n";

	GLbyte fShaderStr[] =  
		"precision mediump float;                            \n"
		"varying vec2 v_texCoord;                            \n"
		"uniform sampler2D s_baseMap;                        \n"
		"uniform sampler2D s_lightMap;                       \n"
		"void main()                                         \n"
		"{                                                   \n"
		"  vec4 baseColor;                                   \n"
		"  vec4 lightColor;                                  \n"
		"                                                    \n"
		"  baseColor = texture2D( s_baseMap, v_texCoord );   \n"
		"  lightColor = texture2D( s_lightMap, v_texCoord ); \n"
		"  gl_FragColor = baseColor * (lightColor + 0.25);   \n"
		"}                                                   \n";

	// Load the shaders and get a linked program object
	userData->programObject = esLoadProgram ( vShaderStr, fShaderStr );

	// Get the attribute locations
	userData->positionLoc = gles2.glGetAttribLocation ( userData->programObject, "a_position" );
	userData->texCoordLoc = gles2.glGetAttribLocation ( userData->programObject, "a_texCoord" );

	// Get the sampler location
	userData->baseMapLoc = gles2.glGetUniformLocation ( userData->programObject, "s_baseMap" );
	userData->lightMapLoc = gles2.glGetUniformLocation ( userData->programObject, "s_lightMap" );

	// Load the textures
	userData->baseMapTexId = LoadTexture ( "../../a/basemap.tga" );
	userData->lightMapTexId = LoadTexture ( "../../a/lightmap.tga" );

	if ( userData->baseMapTexId == 0 || userData->lightMapTexId == 0 )
		return 0;

	gles2.glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
	return 1;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( UserData *userData)
{
	GLfloat vVertices[] = {
		-1.f,  1.f, 0.0f,  // Position 0
		0.0f,  0.0f,        // TexCoord 0 
		-1.f, -1.f, 0.0f,  // Position 1
		0.0f,  1.0f,        // TexCoord 1
		1.f, -1.f, 0.0f,  // Position 2
		1.0f,  1.0f,        // TexCoord 2
		1.f,  1.f, 0.0f,  // Position 3
		1.0f,  0.0f         // TexCoord 3
	};
	GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

	// Set the viewport
	gles2.glViewport ( 0, 0, state->window_w, state->window_h);

	// Clear the color buffer
	gles2.glClear ( GL_COLOR_BUFFER_BIT );

	// Use the program object
	gles2.glUseProgram ( userData->programObject );

	// Load the vertex position
	gles2.glVertexAttribPointer ( userData->positionLoc, 3, GL_FLOAT, 
			GL_FALSE, 5 * sizeof(GLfloat), vVertices );
	// Load the texture coordinate
	gles2.glVertexAttribPointer ( userData->texCoordLoc, 2, GL_FLOAT,
			GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );

	gles2.glEnableVertexAttribArray ( userData->positionLoc );
	gles2.glEnableVertexAttribArray ( userData->texCoordLoc );

	// Bind the base map
	gles2.glActiveTexture ( GL_TEXTURE0 );
	gles2.glBindTexture ( GL_TEXTURE_2D, userData->baseMapTexId );

	// Set the base map sampler to texture unit to 0
	gles2.glUniform1i ( userData->baseMapLoc, 0 );

	// Bind the light map
	gles2.glActiveTexture ( GL_TEXTURE1 );
	gles2.glBindTexture ( GL_TEXTURE_2D, userData->lightMapTexId );

	// Set the light map sampler to texture unit 1
	gles2.glUniform1i ( userData->lightMapLoc, 1 );

	gles2.glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

	/*eglSwapBuffers ( esContext->eglDisplay, esContext->eglSurface );*/
}

