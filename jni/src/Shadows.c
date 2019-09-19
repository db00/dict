/**
 * @file Shadows.c
   gcc Shadows.c -lm -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/"  -lSDL2 -lSDL2_ttf && ./a.out
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2017-08-18
 */

// The MIT License (MIT)
//
// Copyright (c) 2013 Dan Ginsburg, Budirijanto Purnomo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

//
// Book:      OpenGL(R) ES 3.0 Programming Guide, 2nd Edition
// Authors:   Dan Ginsburg, Budirijanto Purnomo, Dave Shreiner, Aaftab Munshi
// ISBN-10:   0-321-93388-5
// ISBN-13:   978-0-321-93388-1
// Publisher: Addison-Wesley Professional
// URLs:      http://www.opengles-book.com
//            http://my.safaribooksonline.com/book/animation-and-3d/9780133440133
//
// Shadows.c
//
//    Demonstrates shadow rendering with depth texture and 6x6 PCF
//
//#include <stdlib.h>
//#include <math.h>
//#include "esUtil.h"
#include "gles2base.h"

#define POSITION_LOC    0
#define COLOR_LOC       1
#define TRUE 1
#define FALSE 0

typedef struct
{
   // Handle to a program object
   GLuint sceneProgramObject;
   GLuint shadowMapProgramObject;

   // Uniform locations
   GLint  sceneMvpLoc;
   GLint  shadowMapMvpLoc;
   GLint  sceneMvpLightLoc;
   GLint  shadowMapMvpLightLoc;

   // Sampler location
   GLint shadowMapSamplerLoc;

   // shadow map Texture handle
   GLuint shadowMapTextureId;
   GLuint shadowMapBufferId;
   GLuint shadowMapTextureWidth;
   GLuint shadowMapTextureHeight;

   // VBOs of the model
   GLuint groundPositionVBO;
   GLuint groundIndicesIBO;
   GLuint cubePositionVBO;
   GLuint cubeIndicesIBO;
   
   // Number of indices
   int    groundNumIndices;
   int    cubeNumIndices;

   // dimension of grid
   int    groundGridSize;

   // MVP matrices
   ESMatrix  groundMvpMatrix;
   ESMatrix  groundMvpLightMatrix;
   ESMatrix  cubeMvpMatrix;
   ESMatrix  cubeMvpLightMatrix;

   float eyePosition[3];
   float lightPosition[3];
} UserData;

///
// Initialize the MVP matrix
//
//int InitMVP ( ESContext *esContext )
void InitMVP( UserData*userData)
{
   ESMatrix perspective;
   ESMatrix ortho;
   ESMatrix modelview;
   ESMatrix model;
   ESMatrix view;
   float    aspect;
   //UserData *userData = esContext->userData;
   
   // Compute the window aspect ratio
   aspect = (GLfloat) esContext->width / (GLfloat) esContext->height;
   
   // Generate a perspective matrix with a 45 degree FOV for the scene rendering
   //esMatrixLoadIdentity ( &perspective );
   //esPerspective ( &perspective, 45.0f, aspect, 0.1f, 100.0f );

   // Generate an orthographic projection matrix for the shadow map rendering
   //esMatrixLoadIdentity ( &ortho );
   //esOrtho ( &ortho, -10, 10, -10, 10, -30, 30 );

   // GROUND
   // Generate a model view matrix to rotate/translate the ground
   esMatrixLoadIdentity ( &model );

   // Center the ground
   esTranslate ( &model, -2.0f, -2.0f, 0.0f );
   esScale ( &model, 10.0f, 10.0f, 10.0f );
   esRotate ( &model, 90.0f, 1.0f, 0.0f, 0.0f );

   // create view matrix transformation from the eye position
   //esMatrixLookAt ( &view, userData->eyePosition[0], userData->eyePosition[1], userData->eyePosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );

   //esMatrixMultiply ( &modelview, &model, &view );

   // Compute the final ground MVP for the scene rendering by multiplying the 
   // modelview and perspective matrices together
   //esMatrixMultiply ( &userData->groundMvpMatrix, &modelview, &perspective );

   // create view matrix transformation from the light position
   esMatrixLookAt ( &view, 
                    userData->lightPosition[0], userData->lightPosition[1], userData->lightPosition[2],
                    0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f );

   esMatrixMultiply ( &modelview, &model, &view );

   // Compute the final ground MVP for the shadow map rendering by multiplying the 
   // modelview and ortho matrices together
   esMatrixMultiply ( &userData->groundMvpLightMatrix, &modelview, &ortho );

   // CUBE
   // position the cube
   esMatrixLoadIdentity ( &model );
   esTranslate ( &model, 5.0f, -0.4f, -3.0f );
   esScale ( &model, 1.0f, 2.5f, 1.0f );
   esRotate ( &model, -15.0f, 0.0f, 1.0f, 0.0f );

   // create view matrix transformation from the eye position
   esMatrixLookAt ( &view, 
                    userData->eyePosition[0], userData->eyePosition[1], userData->eyePosition[2],
                    0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f );

   esMatrixMultiply ( &modelview, &model, &view );
   
   // Compute the final cube MVP for scene rendering by multiplying the 
   // modelview and perspective matrices together
   esMatrixMultiply ( &userData->cubeMvpMatrix, &modelview, &perspective );

   // create view matrix transformation from the light position
   esMatrixLookAt ( &view, 
                    userData->lightPosition[0], userData->lightPosition[1], userData->lightPosition[2],
                    0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f );

   esMatrixMultiply ( &modelview, &model, &view );
   
   // Compute the final cube MVP for shadow map rendering by multiplying the 
   // modelview and ortho matrices together
   esMatrixMultiply ( &userData->cubeMvpLightMatrix, &modelview, &ortho );

   //return TRUE;
}

//int InitShadowMap ( ESContext *esContext )
void InitShadowMap( UserData*userData)
{
   //UserData *userData = esContext->userData;
   GLenum none = GL_NONE;
   GLint defaultFramebuffer = 0;

   // use 1K by 1K texture for shadow map
   userData->shadowMapTextureWidth = userData->shadowMapTextureHeight = 1024;

   GL_CHECK(gles2.glGenTextures ( 1, &userData->shadowMapTextureId ));
   GL_CHECK(gles2.glBindTexture ( GL_TEXTURE_2D, userData->shadowMapTextureId ));
   GL_CHECK(gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ));
   GL_CHECK(gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ));
   GL_CHECK(gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE ));
   GL_CHECK(gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE ));
        
   // Setup hardware comparison
   GL_CHECK(gles2.glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE ));
   GL_CHECK(gles2.glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL ));
        
   glTexImage2D ( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                  userData->shadowMapTextureWidth, userData->shadowMapTextureHeight, 
                  0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );

   GL_CHECK(gles2.glBindTexture ( GL_TEXTURE_2D, 0 ));

   GL_CHECK(gles2.glGetIntegerv ( GL_FRAMEBUFFER_BINDING, &defaultFramebuffer ));

   // setup fbo
   GL_CHECK(gles2.glGenFramebuffers ( 1, &userData->shadowMapBufferId ));
   GL_CHECK(gles2.glBindFramebuffer ( GL_FRAMEBUFFER, userData->shadowMapBufferId ));

   GL_CHECK(gles2.glDrawBuffers ( 1, &none ));
   
   GL_CHECK(gles2.glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, userData->shadowMapTextureId, 0 ));

   GL_CHECK(gles2.glActiveTexture ( GL_TEXTURE0 ));
   GL_CHECK(gles2.glBindTexture ( GL_TEXTURE_2D, userData->shadowMapTextureId ));
 
   if ( GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus ( GL_FRAMEBUFFER ) )
   {
      //return FALSE;
	  return;
   }

   GL_CHECK(gles2.glBindFramebuffer ( GL_FRAMEBUFFER, defaultFramebuffer ));

   //return TRUE;
}

///
// Initialize the shader and program object
//
//int Init ( ESContext *esContext )
int Init ( UserData*userData)
{
   GLfloat *positions;
   GLuint *indices;

   //UserData *userData = esContext->userData;
   const char vShadowMapShaderStr[] =  
      "#version 300 es                                  \n"
      "uniform mat4 u_mvpLightMatrix;                   \n"
      "layout(location = 0) in vec4 a_position;         \n"
      "out vec4 v_color;                                \n"
      "void main()                                      \n"
      "{                                                \n"
      "   gl_Position = u_mvpLightMatrix * a_position;  \n"
      "}                                                \n";
   
   const char fShadowMapShaderStr[] =  
      "#version 300 es                                  \n"
      "precision lowp float;                            \n"
      "void main()                                      \n"
      "{                                                \n"
      "}                                                \n";

    const char vSceneShaderStr[] =  
      "#version 300 es                                   \n"
      "uniform mat4 u_mvpMatrix;                         \n"
      "uniform mat4 u_mvpLightMatrix;                    \n"
      "layout(location = 0) in vec4 a_position;          \n"
      "layout(location = 1) in vec4 a_color;             \n"
      "out vec4 v_color;                                 \n"
      "out vec4 v_shadowCoord;                           \n"
      "void main()                                       \n"
      "{                                                 \n"
      "   v_color = a_color;                             \n"
      "   gl_Position = u_mvpMatrix * a_position;        \n"
      "   v_shadowCoord = u_mvpLightMatrix * a_position; \n"
      "                                                  \n"
      "   // transform from [-1,1] to [0,1];             \n"
      "   v_shadowCoord = v_shadowCoord * 0.5 + 0.5;     \n"
      "}                                                 \n";
   
   const char fSceneShaderStr[] =  
      "#version 300 es                                                \n"
      "precision lowp float;                                          \n"
      "uniform lowp sampler2DShadow s_shadowMap;                      \n"
      "in vec4 v_color;                                               \n"
      "in vec4 v_shadowCoord;                                         \n"
      "layout(location = 0) out vec4 outColor;                        \n"
      "                                                               \n"
      "float lookup ( float x, float y )                              \n"
      "{                                                              \n"
      "   float pixelSize = 0.002; // 1/500                           \n"
      "   vec4 offset = vec4 ( x * pixelSize * v_shadowCoord.w,       \n"
      "                        y * pixelSize * v_shadowCoord.w,       \n"
      "                        -0.005 * v_shadowCoord.w, 0.0 );       \n"
      "   return textureProj ( s_shadowMap, v_shadowCoord + offset ); \n"
      "}                                                              \n"
      "                                                               \n"
      "void main()                                                    \n"
      "{                                                              \n"
      "   // 3x3 kernel with 4 taps per sample, effectively 6x6 PCF   \n"
      "   float sum = 0.0;                                            \n"
      "   float x, y;                                                 \n"
      "   for ( x = -2.0; x <= 2.0; x += 2.0 )                        \n"
      "      for ( y = -2.0; y <= 2.0; y += 2.0 )                     \n"
      "         sum += lookup ( x, y );                               \n"
      "                                                               \n"
      "   // divide sum by 9.0                                        \n"
      "   sum = sum * 0.11;                                           \n"
      "   outColor = v_color * sum;                                   \n"
      "}                                                              \n";

   // Load the shaders and get a linked program object
   userData->shadowMapProgramObject = esLoadProgram ( vShadowMapShaderStr, fShadowMapShaderStr );
   userData->sceneProgramObject = esLoadProgram ( vSceneShaderStr, fSceneShaderStr );

   // Get the uniform locations
   userData->sceneMvpLoc = GL_CHECK(gles2.glGetUniformLocation ( userData->sceneProgramObject, "u_mvpMatrix" ));
   userData->shadowMapMvpLoc = GL_CHECK(gles2.glGetUniformLocation ( userData->shadowMapProgramObject, "u_mvpMatrix" ));
   userData->sceneMvpLightLoc = GL_CHECK(gles2.glGetUniformLocation ( userData->sceneProgramObject, "u_mvpLightMatrix" ));
   userData->shadowMapMvpLightLoc = GL_CHECK(gles2.glGetUniformLocation ( userData->shadowMapProgramObject, "u_mvpLightMatrix" ));

   // Get the sampler location
   userData->shadowMapSamplerLoc = GL_CHECK(gles2.glGetUniformLocation ( userData->sceneProgramObject, "s_shadowMap" ));

   // Generate the vertex and index data for the ground
   userData->groundGridSize = 3;
   //userData->groundNumIndices = esGenSquareGrid( userData->groundGridSize, &positions, &indices );

   // Index buffer object for the ground model
   GL_CHECK(gles2.glGenBuffers ( 1, &userData->groundIndicesIBO ));
   GL_CHECK(gles2.glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, userData->groundIndicesIBO ));
   GL_CHECK(gles2.glBufferData ( GL_ELEMENT_ARRAY_BUFFER, userData->groundNumIndices * sizeof( GLuint ), indices, GL_STATIC_DRAW ));
   GL_CHECK(gles2.glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, 0 ));
   free( indices );

   // Position VBO for ground model
   GL_CHECK(gles2.glGenBuffers ( 1, &userData->groundPositionVBO ));
   GL_CHECK(gles2.glBindBuffer ( GL_ARRAY_BUFFER, userData->groundPositionVBO ));
   glBufferData ( GL_ARRAY_BUFFER, userData->groundGridSize * userData->groundGridSize * sizeof( GLfloat ) * 3, 
                  positions, GL_STATIC_DRAW );
   free( positions );

   // Generate the vertex and index date for the cube model
   userData->cubeNumIndices = esGenCube ( 1.0f, &positions,
                                          NULL, NULL, &indices );

   // Index buffer object for cube model
   GL_CHECK(gles2.glGenBuffers ( 1, &userData->cubeIndicesIBO ));
   GL_CHECK(gles2.glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, userData->cubeIndicesIBO ));
   GL_CHECK(gles2.glBufferData ( GL_ELEMENT_ARRAY_BUFFER, sizeof( GLuint ) * userData->cubeNumIndices, indices, GL_STATIC_DRAW ));
   GL_CHECK(gles2.glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, 0 ));
   free( indices );

   // Position VBO for cube model
   GL_CHECK(gles2.glGenBuffers ( 1, &userData->cubePositionVBO ));
   GL_CHECK(gles2.glBindBuffer ( GL_ARRAY_BUFFER, userData->cubePositionVBO ));
   GL_CHECK(gles2.glBufferData ( GL_ARRAY_BUFFER, 24 * sizeof( GLfloat ) * 3, positions, GL_STATIC_DRAW ));
   free( positions );

   // setup transformation matrices
   userData->eyePosition[0] = -5.0f;
   userData->eyePosition[1] = 3.0f;
   userData->eyePosition[2] = 5.0f;
   userData->lightPosition[0] = 10.0f;
   userData->lightPosition[1] = 5.0f;
   userData->lightPosition[2] = 2.0f;
   
   // create depth texture
   if ( !InitShadowMap( esContext ) )
   {
      return FALSE;
   }

   GL_CHECK(gles2.glClearColor ( 1.0f, 1.0f, 1.0f, 0.0f ));

   // disable culling
   GL_CHECK(gles2.glDisable ( GL_CULL_FACE ));

   // enable depth test
   GL_CHECK(gles2.glEnable ( GL_DEPTH_TEST ));

   return TRUE;
}

///
// Draw the model
//
//void DrawScene ( ESContext *esContext, GLint mvpLoc, GLint mvpLightLoc )
void DrawScene( UserData*userData,GLint mvpLoc,GLint mvpLightLoc)
{
   //UserData *userData = esContext->userData;
 
   // Draw the ground
   // Load the vertex position
   GL_CHECK(gles2.glBindBuffer ( GL_ARRAY_BUFFER, userData->groundPositionVBO ));
   glVertexAttribPointer ( POSITION_LOC, 3, GL_FLOAT, 
                           GL_FALSE, 3 * sizeof(GLfloat), (const void*)NULL );
   GL_CHECK(gles2.glEnableVertexAttribArray ( POSITION_LOC ));   

   // Bind the index buffer
   GL_CHECK(gles2.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, userData->groundIndicesIBO ));

   // Load the MVP matrix for the ground model
   GL_CHECK(gles2.glUniformMatrix4fv ( mvpLoc, 1, GL_FALSE, (GLfloat*) &userData->groundMvpMatrix.m[0][0] ));
   GL_CHECK(gles2.glUniformMatrix4fv ( mvpLightLoc, 1, GL_FALSE, (GLfloat*) &userData->groundMvpLightMatrix.m[0][0] ));

   // Set the ground color to light gray
   GL_CHECK(gles2.glVertexAttrib4f ( COLOR_LOC, 0.9f, 0.9f, 0.9f, 1.0f ));

   GL_CHECK(gles2.glDrawElements ( GL_TRIANGLES, userData->groundNumIndices, GL_UNSIGNED_INT, (const void*)NULL ));

   // Draw the cube
   // Load the vertex position
   GL_CHECK(gles2.glBindBuffer( GL_ARRAY_BUFFER, userData->cubePositionVBO ));
   glVertexAttribPointer ( POSITION_LOC, 3, GL_FLOAT, 
                           GL_FALSE, 3 * sizeof(GLfloat), (const void*)NULL );
   GL_CHECK(gles2.glEnableVertexAttribArray ( POSITION_LOC ));   

   // Bind the index buffer
   GL_CHECK(gles2.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, userData->cubeIndicesIBO ));

   // Load the MVP matrix for the cube model
   GL_CHECK(gles2.glUniformMatrix4fv ( mvpLoc, 1, GL_FALSE, (GLfloat*) &userData->cubeMvpMatrix.m[0][0] ));
   GL_CHECK(gles2.glUniformMatrix4fv ( mvpLightLoc, 1, GL_FALSE, (GLfloat*) &userData->cubeMvpLightMatrix.m[0][0] ));

   // Set the cube color to red
   GL_CHECK(gles2.glVertexAttrib4f ( COLOR_LOC, 1.0f, 0.0f, 0.0f, 1.0f ));

   GL_CHECK(gles2.glDrawElements ( GL_TRIANGLES, userData->cubeNumIndices, GL_UNSIGNED_INT, (const void*)NULL ));
}

//void Draw ( ESContext *esContext )
void Draw ( UserData*userData)
{
   //UserData *userData = esContext->userData;
   GLint defaultFramebuffer = 0;

   // Initialize matrices
   //InitMVP ( esContext );
   InitMVP ( userData );

   GL_CHECK(gles2.glGetIntegerv ( GL_FRAMEBUFFER_BINDING, &defaultFramebuffer ));

   // FIRST PASS: Render the scene from light position to generate the shadow map texture
   GL_CHECK(gles2.glBindFramebuffer ( GL_FRAMEBUFFER, userData->shadowMapBufferId ));

   // Set the viewport
   //GL_CHECK(gles2.glViewport ( 0, 0, userData->shadowMapTextureWidth, userData->shadowMapTextureHeight ));

   // clear depth buffer
   GL_CHECK(gles2.glClear( GL_DEPTH_BUFFER_BIT ));

   // disable color rendering, only write to depth buffer
   GL_CHECK(gles2.glColorMask ( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE ));

   // reduce shadow rendering artifact
   GL_CHECK(gles2.glEnable ( GL_POLYGON_OFFSET_FILL ));
   GL_CHECK(gles2.glPolygonOffset( 5.0f, 100.0f ));

   GL_CHECK(gles2.glUseProgram ( userData->shadowMapProgramObject ));
   //DrawScene ( esContext, userData->shadowMapMvpLoc, userData->shadowMapMvpLightLoc );
   DrawScene ( userData, userData->shadowMapMvpLoc, userData->shadowMapMvpLightLoc );

   GL_CHECK(gles2.glDisable( GL_POLYGON_OFFSET_FILL ));

   // SECOND PASS: Render the scene from eye location using the shadow map texture created in the first pass
   GL_CHECK(gles2.glBindFramebuffer ( GL_FRAMEBUFFER, defaultFramebuffer ));
   GL_CHECK(gles2.glColorMask ( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE ));

   // Set the viewport
   //GL_CHECK(gles2.glViewport ( 0, 0, esContext->width, esContext->height ));
   
   // Clear the color and depth buffers
   GL_CHECK(gles2.glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ));
   GL_CHECK(gles2.glClearColor ( 1.0f, 1.0f, 1.0f, 0.0f ));

   // Use the scene program object
   GL_CHECK(gles2.glUseProgram ( userData->sceneProgramObject ));

   // Bind the shadow map texture
   GL_CHECK(gles2.glActiveTexture ( GL_TEXTURE0 ));
   GL_CHECK(gles2.glBindTexture ( GL_TEXTURE_2D, userData->shadowMapTextureId ));

   // Set the sampler texture unit to 0
   GL_CHECK(gles2.glUniform1i ( userData->shadowMapSamplerLoc, 0 ));

   //DrawScene ( esContext, userData->sceneMvpLoc, userData->sceneMvpLightLoc );
   DrawScene ( userData, userData->sceneMvpLoc, userData->sceneMvpLightLoc );
}

///
// Cleanup
//
//void Shutdown ( ESContext *esContext )
void Shutdown( UserData*userData)
{
   //UserData *userData = esContext->userData;

   GL_CHECK(gles2.glDeleteBuffers( 1, &userData->groundPositionVBO ));
   GL_CHECK(gles2.glDeleteBuffers( 1, &userData->groundIndicesIBO ));

   GL_CHECK(gles2.glDeleteBuffers( 1, &userData->cubePositionVBO ));
   GL_CHECK(gles2.glDeleteBuffers( 1, &userData->cubeIndicesIBO ));
   
   // Delete shadow map
   GL_CHECK(gles2.glBindFramebuffer ( GL_FRAMEBUFFER, userData->shadowMapBufferId ));
   GL_CHECK(gles2.glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0 ));
   GL_CHECK(gles2.glBindFramebuffer ( GL_FRAMEBUFFER, 0 ));
   GL_CHECK(gles2.glDeleteFramebuffers ( 1, &userData->shadowMapBufferId ));
   GL_CHECK(gles2.glDeleteTextures ( 1, &userData->shadowMapTextureId ));

   // Delete program object
   GL_CHECK(gles2.glDeleteProgram ( userData->sceneProgramObject ));
   GL_CHECK(gles2.glDeleteProgram ( userData->shadowMapProgramObject ));
}

/*
//int esMain ( ESContext *esContext )
void esMain( UserData*userData)
{
   esContext->userData = malloc ( sizeof( UserData ) );

   esCreateWindow ( esContext, "Shadow Rendering", 500, 500, ES_WINDOW_RGB | ES_WINDOW_DEPTH );
   
   if ( !Init ( esContext ) )
   {
      return GL_FALSE;
   }

   esRegisterShutdownFunc ( esContext, Shutdown );
   esRegisterDrawFunc ( esContext, Draw );
   
   return GL_TRUE;
}
*/

