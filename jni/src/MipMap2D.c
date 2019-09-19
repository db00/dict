/**
 *
   gcc MipMap2D.c -lm -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/"   -lGLESv2 -lSDL2 -lSDL2_ttf && ./a.out
   gcc MipMap2D.c  -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/"  -L"." -lGLESv2 -lmingw32  -lSDL2main -lSDL2 -lSDL2_ttf && a 
 */
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// MipMap2D.c
//
//    This is a simple example that demonstrates generating a mipmap chain
//    and rendering with it
//
//#include <stdlib.h>
#include "gles2base.h"

///
//  From an RGB8 source image, generate the next level mipmap
//
GLboolean GenMipMap2D( GLubyte *src, GLubyte **dst, int srcWidth, int srcHeight, int *dstWidth, int *dstHeight )
{
   int x,
       y;
   int texelSize = 3;

   *dstWidth = srcWidth / 2;
   if ( *dstWidth <= 0 )
      *dstWidth = 1;

   *dstHeight = srcHeight / 2;
   if ( *dstHeight <= 0 )
      *dstHeight = 1;

   *dst = malloc ( sizeof(GLubyte) * texelSize * (*dstWidth) * (*dstHeight) );
   if ( *dst == NULL )
      return GL_FALSE;

   for ( y = 0; y < *dstHeight; y++ )
   {
      for( x = 0; x < *dstWidth; x++ )
      {
         int srcIndex[4];
         float r = 0.0f,
               g = 0.0f,
               b = 0.0f;
         int sample;

         // Compute the offsets for 2x2 grid of pixels in previous
         // image to perform box filter
         srcIndex[0] = 
            (((y * 2) * srcWidth) + (x * 2)) * texelSize;
         srcIndex[1] = 
            (((y * 2) * srcWidth) + (x * 2 + 1)) * texelSize; 
         srcIndex[2] = 
            ((((y * 2) + 1) * srcWidth) + (x * 2)) * texelSize;
         srcIndex[3] = 
            ((((y * 2) + 1) * srcWidth) + (x * 2 + 1)) * texelSize;

         // Sum all pixels
         for ( sample = 0; sample < 4; sample++ )
         {
            r += src[srcIndex[sample]];
            g += src[srcIndex[sample] + 1];
            b += src[srcIndex[sample] + 2];
         }

         // Average results
         r /= 4.0;
         g /= 4.0;
         b /= 4.0;

         // Store resulting pixels
         (*dst)[ ( y * (*dstWidth) + x ) * texelSize ] = (GLubyte)( r );
         (*dst)[ ( y * (*dstWidth) + x ) * texelSize + 1] = (GLubyte)( g );
         (*dst)[ ( y * (*dstWidth) + x ) * texelSize + 2] = (GLubyte)( b );
      }
   }

   return GL_TRUE;
}

///
// Create a mipmapped 2D texture image 
//
GLuint CreateMipMappedTexture2D( )
{
   // Texture object handle
   GLuint textureId;
   int    width = 256,
          height = 256;
   int    level;
   GLubyte *pixels;
   GLubyte *prevImage;
   GLubyte *newImage;
      
   pixels = GenCheckImage( width, height, 8 );
   if ( pixels == NULL )
      return 0;

   // Generate a texture object
   gles2.glGenTextures ( 1, &textureId );

   // Bind the texture object
   gles2.glBindTexture ( GL_TEXTURE_2D, textureId );

   // Load mipmap level 0
   gles2.glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, width, height, 
                  0, GL_RGB, GL_UNSIGNED_BYTE, pixels );
   
   level = 1;
   prevImage = &pixels[0];
   
   while ( width > 1 && height > 1 )
   {
      int newWidth,
          newHeight;

      // Generate the next mipmap level
      GenMipMap2D( prevImage, &newImage, width, height, 
                   &newWidth, &newHeight );

      // Load the mipmap level
      gles2.glTexImage2D( GL_TEXTURE_2D, level, GL_RGB, 
                    newWidth, newHeight, 0, GL_RGB,
                    GL_UNSIGNED_BYTE, newImage );

      // Free the previous image
      free ( prevImage );

      // Set the previous image for the next iteration
      prevImage = newImage;
      level++;

      // Half the width and height
      width = newWidth;
      height = newHeight;
   }

   free ( newImage );

   // Set the filtering mode
   gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
   gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

   return textureId;

}


///
// Initialize the shader and program object
//
//int Init ( ESContext *esContext )
int Init ( UserData*userData)
{
   //UserData *userData = esContext->userData;
   char vShaderStr[] =
      "uniform float u_offset;      \n"
      "attribute vec4 a_position;   \n"
      "attribute vec2 a_texCoord;   \n"
      "varying vec2 v_texCoord;     \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position; \n"
      "   gl_Position.x += u_offset;\n"
      "   v_texCoord = a_texCoord;  \n"
      "}                            \n";
   
   char fShaderStr[] =  
      "precision mediump float;                            \n"
      "varying vec2 v_texCoord;                            \n"
      "uniform sampler2D s_texture;                        \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
      "}                                                   \n";

   // Load the shaders and get a linked program object
   userData->programObject = esLoadProgram ( vShaderStr, fShaderStr );

   // Get the attribute locations
   userData->positionLoc = gles2.glGetAttribLocation ( userData->programObject, "a_position" );
   userData->texCoordLoc = gles2.glGetAttribLocation ( userData->programObject, "a_texCoord" );
   
   // Get the sampler location
   userData->samplerLoc = gles2.glGetUniformLocation ( userData->programObject, "s_texture" );

   // Get the offset location
   userData->offsetLoc = gles2.glGetUniformLocation( userData->programObject, "u_offset" );

   // Load the texture
   userData->textureId = CreateMipMappedTexture2D ();

   gles2.glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return 1;
}

///
// Draw a triangle using the shader pair created in Init()
//
//void Draw ( ESContext *esContext )
void Draw ( UserData*userData)
{
   //UserData *userData = esContext->userData;
   GLfloat vVertices[] = { -0.5f,  0.5f, 0.0f, 1.5f,  // Position 0
                            0.0f,  0.0f,              // TexCoord 0 
                           -0.5f, -0.5f, 0.0f, 0.75f, // Position 1
                            0.0f,  1.0f,              // TexCoord 1
                            0.5f, -0.5f, 0.0f, 0.75f, // Position 2
                            1.0f,  1.0f,              // TexCoord 2
                            0.5f,  0.5f, 0.0f, 1.5f,  // Position 3
                            1.0f,  0.0f               // TexCoord 3
                         };
   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
      
   // Set the viewport
   //gles2.glViewport ( 0, 0, esContext->width, esContext->height );
   
   // Clear the color buffer
   gles2.glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   gles2.glUseProgram ( userData->programObject );

   // Load the vertex position
   gles2.glVertexAttribPointer ( userData->positionLoc, 4, GL_FLOAT, 
                           GL_FALSE, 6 * sizeof(GLfloat), vVertices );
   // Load the texture coordinate
   gles2.glVertexAttribPointer ( userData->texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 6 * sizeof(GLfloat), &vVertices[4] );

   gles2.glEnableVertexAttribArray ( userData->positionLoc );
   gles2.glEnableVertexAttribArray ( userData->texCoordLoc );

   // Bind the texture
   gles2.glActiveTexture ( GL_TEXTURE0 );
   gles2.glBindTexture ( GL_TEXTURE_2D, userData->textureId );

   // Set the sampler texture unit to 0
   gles2.glUniform1i ( userData->samplerLoc, 0 );

   // Draw quad with nearest sampling
   gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   gles2.glUniform1f ( userData->offsetLoc, -0.6f );   
   gles2.glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

   // Draw quad with trilinear filtering
   gles2.glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
   gles2.glUniform1f ( userData->offsetLoc, 0.6f );
   gles2.glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

   //eglSwapBuffers ( esContext->eglDisplay, esContext->eglSurface );
}
