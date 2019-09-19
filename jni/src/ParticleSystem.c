/*
 *
   gcc ParticleSystem.c  -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/"  -L. -lGLESv2 -lSDL2 -lSDL2_ttf -lm && ./a.out
   gcc ParticleSystem.c  -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/"  -L. -lGLESv2 -lmingw32  -lSDL2main -lSDL2 -lSDL2_ttf && a 
   gcc ParticleSystem.c -L. -lGLESv2 -lmingw32 -lSDL2_test -lSDL2main -lSDL2 -lSDL2_ttf && a 
   gcc ParticleSystem.c SDL_test_common.c -lm -lSDL2 -lSDL2_ttf && ./a.out && ./a.out
   gcc ParticleSystem.c -llibGLESv2 -lSDL2_test -lm -lmingw32 -lSDL2main -lSDL2   && a
   adb push 2.c /sdcard/ &&
   Copyright (r) 1997-2014 Sam Lantinga <slouken@libsdl.org>

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely.
   */

#include "SDL.h"
#include "gles2base0.h"

///
// Initialize the shader and program object
//
int Init ( UserData *userData)
{
	int i;

	GLbyte vShaderStr[] =
		"uniform float u_time;		                           \n"
		"uniform vec3 u_centerPosition;                       \n"
		"attribute float a_lifetime;                          \n"
		"attribute vec3 a_startPosition;                      \n"
		"attribute vec3 a_endPosition;                        \n"
		"varying float v_lifetime;                            \n"
		"void main()                                          \n"
		"{                                                    \n"
		"  if ( u_time <= a_lifetime )                        \n"
		"  {                                                  \n"
		"    gl_Position.xyz = a_startPosition +              \n"
		"                      (u_time * a_endPosition);      \n"
		"    gl_Position.xyz += u_centerPosition;             \n"
		"    gl_Position.w = 1.0;                             \n"
		"  }                                                  \n"
		"  else                                               \n"
		"     gl_Position = vec4( -1000, -1000, 0, 0 );       \n"
		"  v_lifetime = 1.0 - ( u_time / a_lifetime );        \n"
		"  v_lifetime = clamp ( v_lifetime, 0.0, 1.0 );       \n"
		"  gl_PointSize = ( v_lifetime * v_lifetime ) * 40.0; \n"
		"}";

	GLbyte fShaderStr[] =  
		"precision mediump float;                             \n"
		"uniform vec4 u_color;		                           \n"
		"varying float v_lifetime;                            \n"
		"uniform sampler2D s_texture;                         \n"
		"void main()                                          \n"
		"{                                                    \n"
		"  vec4 texColor;                                     \n"
		"  texColor = texture2D( s_texture, gl_PointCoord );  \n"
		"  gl_FragColor = vec4( u_color ) * texColor;         \n"
		"  gl_FragColor.a *= v_lifetime;                      \n"
		"}                                                    \n";

	// Load the shaders and get a linked program object
	userData->programObject = esLoadProgram ( vShaderStr, fShaderStr );

	// Get the attribute locations
	userData->lifetimeLoc = gles2.glGetAttribLocation ( userData->programObject, "a_lifetime" );
	userData->startPositionLoc = gles2.glGetAttribLocation ( userData->programObject, "a_startPosition" );
	userData->endPositionLoc = gles2.glGetAttribLocation ( userData->programObject, "a_endPosition" );

	// Get the uniform locations
	userData->timeLoc = gles2.glGetUniformLocation ( userData->programObject, "u_time" );
	userData->centerPositionLoc = gles2.glGetUniformLocation ( userData->programObject, "u_centerPosition" );
	userData->colorLoc = gles2.glGetUniformLocation ( userData->programObject, "u_color" );
	userData->samplerLoc = gles2.glGetUniformLocation ( userData->programObject, "s_texture" );

	gles2.glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );

	// Fill in particle data array
	srand ( 0 );
	for ( i = 0; i < NUM_PARTICLES; i++ )
	{
		float *particleData = &userData->particleData[i * PARTICLE_SIZE];

		// Lifetime of particle
		(*particleData++) = ( (float)(rand() % 10000) / 10000.0f );

		// End position of particle
		(*particleData++) = ( (float)(rand() % 10000) / 5000.0f ) - 1.0f;
		(*particleData++) = ( (float)(rand() % 10000) / 5000.0f ) - 1.0f;
		(*particleData++) = ( (float)(rand() % 10000) / 5000.0f ) - 1.0f;

		// Start position of particle
		(*particleData++) = ( (float)(rand() % 10000) / 40000.0f ) - 0.125f;
		(*particleData++) = ( (float)(rand() % 10000) / 40000.0f ) - 0.125f;
		(*particleData++) = ( (float)(rand() % 10000) / 40000.0f ) - 0.125f;

	}

	// Initialize time to cause reset on first update
	userData->time = 1.0f;

	/*userData->textureId = LoadTexture ( "smoke.tga" );*/
	userData->textureId= LoadTexture ( "../../a/basemap.tga" );
	if ( userData->textureId <= 0 )
	{
		SDL_Log("load texture Error!");
		return SDL_FALSE;
	}

	return SDL_TRUE;
}

///
//  Update time-based variables
//
void _Update (
		UserData *userData
		, float deltaTime )
{

	userData->time += deltaTime;

	if ( userData->time >= 1.0f )
	{
		float centerPos[3];
		float color[4];

		userData->time = 0.0f;

		// Pick a new start location and color
		centerPos[0] = ( (float)(rand() % 10000) / 10000.0f ) - 0.5f;
		centerPos[1] = ( (float)(rand() % 10000) / 10000.0f ) - 0.5f;
		centerPos[2] = ( (float)(rand() % 10000) / 10000.0f ) - 0.5f;

		gles2.glUniform3fv ( userData->centerPositionLoc, 1, &centerPos[0] );

		// Random color
		color[0] = ( (float)(rand() % 10000) / 20000.0f ) + 0.5f;
		color[1] = ( (float)(rand() % 10000) / 20000.0f ) + 0.5f;
		color[2] = ( (float)(rand() % 10000) / 20000.0f ) + 0.5f;
		color[3] = 0.5;

		gles2.glUniform4fv ( userData->colorLoc, 1, &color[0] );
	}

	// Load uniform time variable
	gles2.glUniform1f ( userData->timeLoc, userData->time );
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw (
		UserData *userData
		)
{

	_Update(userData,0.02);
	// Set the viewport
	gles2.glViewport ( 0, 0, state->window_w, state->window_h);

	// Clear the color buffer
	gles2.glClear ( GL_COLOR_BUFFER_BIT );

	// Use the program object
	gles2.glUseProgram ( userData->programObject );

	// Load the vertex attributes
	gles2.glVertexAttribPointer ( userData->lifetimeLoc, 1, GL_FLOAT, 
			GL_FALSE, PARTICLE_SIZE * sizeof(GLfloat), 
			userData->particleData );

	gles2.glVertexAttribPointer ( userData->endPositionLoc, 3, GL_FLOAT,
			GL_FALSE, PARTICLE_SIZE * sizeof(GLfloat),
			&userData->particleData[1] );

	gles2.glVertexAttribPointer ( userData->startPositionLoc, 3, GL_FLOAT,
			GL_FALSE, PARTICLE_SIZE * sizeof(GLfloat),
			&userData->particleData[4] );


	gles2.glEnableVertexAttribArray ( userData->lifetimeLoc );
	gles2.glEnableVertexAttribArray ( userData->endPositionLoc );
	gles2.glEnableVertexAttribArray ( userData->startPositionLoc );
	// Blend particles
	gles2.glEnable ( GL_BLEND );
	gles2.glBlendFunc ( GL_SRC_ALPHA, GL_ONE );

	// Bind the texture
	gles2.glActiveTexture ( GL_TEXTURE0 );
	gles2.glBindTexture ( GL_TEXTURE_2D, userData->textureId );
	gles2.glEnable ( GL_TEXTURE_2D );

	// Set the sampler texture unit to 0
	gles2.glUniform1i ( userData->samplerLoc, 0 );

	gles2.glDrawArrays( GL_POINTS, 0, NUM_PARTICLES );

	/*eglSwapBuffers ( esContext->eglDisplay, esContext->eglSurface );*/
}

