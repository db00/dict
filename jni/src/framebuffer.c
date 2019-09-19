/**
 * @file framebuffer.c
 gcc -g -Wall framebuffer.c sprite.c matrix.c array.c -lSDL2 -lm -I"../SDL2/include/" && ./a.out  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2017-08-11
 */
#include "sprite.h"

// FramebufferObject.js (c) matsuda and kanda
// Vertex shader program
char * VSHADER_SOURCE =
"attribute vec4 a_Position;\n" 
"attribute vec2 a_TexCoord;\n"
"uniform mat4 u_MvpMatrix;\n"
"varying vec2 v_TexCoord;\n"
"void main() {\n"
"  gl_Position = u_MvpMatrix * a_Position;\n"
"  v_TexCoord = a_TexCoord;\n"
"}\n";

// Fragment shader program
char * FSHADER_SOURCE =
"#ifdef GL_ES\n"
"precision mediump float;\n"
"#endif\n"
"uniform sampler2D u_Sampler;\n"
"varying vec2 v_TexCoord;\n"
"void main() {\n"
"  gl_FragColor = texture2D(u_Sampler, v_TexCoord);\n"
"}\n";

// Size of off screen
int OFFSCREEN_WIDTH = 256;
int OFFSCREEN_HEIGHT = 256;
GLint program;

typedef struct Buffer{
	GLuint buffer;
	GLint num;
	GLint type;
}Buffer;
Buffer * Buffer_new()
{
	Buffer * o = malloc(sizeof(Buffer));
	memset(o,0,sizeof(Buffer));
	return o;
}
typedef struct Object{
	Buffer * vertexBuffer;
	Buffer * texCoordBuffer;
	Buffer * indexBuffer;
	Sprite * sprite;
	GLint numIndices;
}Object;
Object * Object_new()
{
	Object * o = malloc(sizeof(Object));
	memset(o,0,sizeof(Object));
	return o;
}

Buffer * initArrayBufferForLaterUse(GLfloat * data,int num,GLenum type) {
	// Create a buffer object
	Buffer * buffer = Buffer_new();
	gles2.glGenBuffers(1,&buffer->buffer);
	// Write date into the buffer object
	gles2.glBindBuffer(GL_ARRAY_BUFFER, buffer->buffer);
	gles2.glBufferData(GL_ARRAY_BUFFER, num, data, GL_STATIC_DRAW);

	// Store the necessary information to assign the object to the attribute variable later
	buffer->num = num;
	buffer->type = type;

	return buffer;
}
Buffer * initElementArrayBufferForLaterUse(GLuint * data,int num, GLenum type) {
	// Create a buffer object
	Buffer * buffer = Buffer_new();
	gles2.glGenBuffers(1,&buffer->buffer);
	// Write date into the buffer object

	gles2.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->buffer);
	gles2.glBufferData(GL_ELEMENT_ARRAY_BUFFER,num, data, GL_STATIC_DRAW);

	buffer->type = type;

	return buffer;
}

Object * initVertexBuffersForCube() {
	// Create a cube
	//    v6----- v5
	//   /|      /|
	//  v1------v0|
	//  | |     | |
	//  | |v7---|-|v4
	//  |/      |/
	//  v2------v3

	// Vertex coordinates
	GLfloat vertices[] = { 
		1.0, 1.0, 1.0,  -1.0, 1.0, 1.0,  -1.0,-1.0, 1.0,   1.0,-1.0, 1.0,    // v0-v1-v2-v3 front
		1.0, 1.0, 1.0,   1.0,-1.0, 1.0,   1.0,-1.0,-1.0,   1.0, 1.0,-1.0,    // v0-v3-v4-v5 right
		1.0, 1.0, 1.0,   1.0, 1.0,-1.0,  -1.0, 1.0,-1.0,  -1.0, 1.0, 1.0,    // v0-v5-v6-v1 up
		-1.0, 1.0, 1.0,  -1.0, 1.0,-1.0,  -1.0,-1.0,-1.0,  -1.0,-1.0, 1.0,    // v1-v6-v7-v2 left
		-1.0,-1.0,-1.0,   1.0,-1.0,-1.0,   1.0,-1.0, 1.0,  -1.0,-1.0, 1.0,    // v7-v4-v3-v2 down
		1.0,-1.0,-1.0,  -1.0,-1.0,-1.0,  -1.0, 1.0,-1.0,   1.0, 1.0,-1.0     // v4-v7-v6-v5 back
	};

	// Texture coordinates
	GLfloat texCoords[] = {
		1.0, 1.0,   0.0, 1.0,   0.0, 0.0,   1.0, 0.0,    // v0-v1-v2-v3 front
		0.0, 1.0,   0.0, 0.0,   1.0, 0.0,   1.0, 1.0,    // v0-v3-v4-v5 right
		1.0, 0.0,   1.0, 1.0,   0.0, 1.0,   0.0, 0.0,    // v0-v5-v6-v1 up
		1.0, 1.0,   0.0, 1.0,   0.0, 0.0,   1.0, 0.0,    // v1-v6-v7-v2 left
		0.0, 0.0,   1.0, 0.0,   1.0, 1.0,   0.0, 1.0,    // v7-v4-v3-v2 down
		0.0, 0.0,   1.0, 0.0,   1.0, 1.0,   0.0, 1.0     // v4-v7-v6-v5 back
	};

	// Indices of the vertices
	GLuint indices[] = {
		0, 1, 2,   0, 2, 3,    // front
		4, 5, 6,   4, 6, 7,    // right
		8, 9,10,   8,10,11,    // up
		12,13,14,  12,14,15,    // left
		16,17,18,  16,18,19,    // down
		20,21,22,  20,22,23     // back
	};

	Object * o = Object_new();  // Create the "Object" object to return multiple objects.

	o->vertexBuffer = initArrayBufferForLaterUse( vertices, 3, GL_FLOAT);
	o->texCoordBuffer = initArrayBufferForLaterUse( texCoords, 2, GL_FLOAT);
	o->indexBuffer = initElementArrayBufferForLaterUse( indices,6*6, GL_UNSIGNED_BYTE);

	o->numIndices = sizeof(indices)/sizeof(GLuint);

	// Unbind the buffer object
	gles2.glBindBuffer(GL_ARRAY_BUFFER, 0);
	gles2.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return o;
}
Object * initVertexBuffersForPlane() {
	// Create face
	//  v1------v0
	//  |        | 
	//  |        |
	//  |        |
	//  v2------v3

	// Vertex coordinates
	GLfloat vertices[] = {
		1.0, 1.0, 0.0,  -1.0, 1.0, 0.0,  -1.0,-1.0, 0.0,   1.0,-1.0, 0.0    // v0-v1-v2-v3
	};

	// Texture coordinates
	GLfloat texCoords[] = {1.0, 1.0,   0.0, 1.0,   0.0, 0.0,   1.0, 0.0};

	// Indices of the vertices
	GLuint indices[] = {0, 1, 2,   0, 2, 3};

	Object * o = Object_new(); // Create the "Object" object to return multiple objects.

	// Write vertex information to buffer object
	o->vertexBuffer = initArrayBufferForLaterUse(vertices, 3, GL_FLOAT);
	o->texCoordBuffer = initArrayBufferForLaterUse(texCoords, 2, GL_FLOAT);
	o->indexBuffer = initElementArrayBufferForLaterUse(indices,6, GL_UNSIGNED_BYTE);

	o->numIndices = sizeof(indices)/sizeof(GLuint);

	// Unbind the buffer object
	gles2.glBindBuffer(GL_ARRAY_BUFFER, 0);
	gles2.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return o;
}

GLuint initFramebufferObject() {
	GLuint framebuffer, texture, depthBuffer;
	// Create a frame buffer object (FBO)
	gles2.glGenFramebuffers(1,&framebuffer);

	// Create a texture object and set its size and parameters
	gles2.glGenTextures(1,&texture); // Create a texture object
	gles2.glBindTexture(GL_TEXTURE_2D, texture); // Bind the object to target
	gles2.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, OFFSCREEN_WIDTH, OFFSCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	gles2.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	/*
	   framebuffer.texture = texture; // Store the texture object
	   */

	// Create a renderbuffer object and Set its size and parameters
	//depthBuffer = gles2.createRenderbuffer(); // Create a renderbuffer object
	gles2.glGenRenderbuffers(1,&depthBuffer);
	gles2.glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer); // Bind the object to target
	gles2.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, OFFSCREEN_WIDTH, OFFSCREEN_HEIGHT);

	// Attach the texture and the renderbuffer object to the FBO
	gles2.glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	gles2.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	gles2.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	// Check if FBO is configured correctly
	GLuint e = gles2.glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (GL_FRAMEBUFFER_COMPLETE != e) {
		return 0;
	}

	// Unbind the buffer object
	gles2.glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gles2.glBindTexture(GL_TEXTURE_2D, 0);
	gles2.glBindRenderbuffer(GL_RENDERBUFFER, 0);


	return framebuffer;
}

// Assign the buffer objects and enable the assignment
void initAttributeVariable(GLuint a_attribute,Buffer * buffer) {
	gles2.glBindBuffer(GL_ARRAY_BUFFER, buffer->buffer);
	gles2.glVertexAttribPointer(a_attribute, buffer->num, buffer->type, 0, 0, 0);
	gles2.glEnableVertexAttribArray(a_attribute);
}


/*
// Coordinate transformation matrix
*/
Matrix3D g_modelMatrix;// = Matrix4_new();
Matrix3D g_mvpMatrix;// = Matrix();

void drawTexturedObject( Object * o, GLuint texture) {
	// Assign the buffer objects and enable the assignment
	Data3d * data2D = o->sprite->data3d;//Data3D_new();
	initAttributeVariable( data2D->positionLoc, o->vertexBuffer);    // Vertex coordinates
	initAttributeVariable( data2D->texCoordLoc, o->texCoordBuffer);  // Texture coordinates
	// Bind the texture object to the target
	gles2.glActiveTexture(GL_TEXTURE0);
	gles2.glBindTexture(GL_TEXTURE_2D, texture);

	// Draw
	gles2.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o->indexBuffer->buffer);
	gles2.glDrawElements(GL_TRIANGLES, o->numIndices, o->indexBuffer->type, 0);
}

void drawTexturedCube(Object * o, float angle, GLuint texture, Matrix3D * viewProjMatrix) {
	Data3d * data2D = o->sprite->data3d;//Data3D_new();
	/*
	// Calculate a model matrix
	g_modelMatrix.setRotate(20.0, 1.0, 0.0, 0.0);
	g_modelMatrix.rotate(angle, 0.0, 1.0, 0.0);
	// Calculate the model view project matrix and pass it to u_MvpMatrix
	g_mvpMatrix.set(viewProjMatrix);
	g_mvpMatrix.multiply(g_modelMatrix);

*/
	//gles2.glUniformMatrix4fv(program.u_MvpMatrix, 0, g_mvpMatrix.elements);
	gles2.glUniformMatrix4fv(data2D->mvpLoc,1, GL_FALSE, &g_mvpMatrix.rawData[0][0]);
	drawTexturedObject(o, texture);
}
void drawTexturedPlane( Object * o, float angle, GLuint texture, Matrix3D * viewProjMatrix) {
	Data3d * data2D = o->sprite->data3d;//Data3D_new();
	/*
	// Calculate a model matrix
	g_modelMatrix.setTranslate(0, 0, 1);
	g_modelMatrix.rotate(20.0, 1.0, 0.0, 0.0);
	g_modelMatrix.rotate(angle, 0.0, 1.0, 0.0);

	// Calculate the model view project matrix and pass it to u_MvpMatrix
	g_mvpMatrix.set(viewProjMatrix);
	g_mvpMatrix.multiply(g_modelMatrix);
	*/
	//GL_CHECK(gles2.glUniformMatrix4fv( data2D->mvpLoc, 1, GL_FALSE, (GLfloat*) &sprite->mvpMatrix.rawData[0][0]));
	gles2.glUniformMatrix4fv(data2D->mvpLoc,1, GL_FALSE, &g_mvpMatrix.rawData[0][0]);

	drawTexturedObject( o, texture);
}
void draw(GLuint fbo,Object * plane,Object * cube,float angle,GLuint texture, Matrix3D * viewProjMatrix, Matrix3D * viewProjMatrixFBO) {
	gles2.glBindRenderbuffer(GL_FRAMEBUFFER, fbo);              // Change the drawing destination to FBO
	gles2.glViewport(0, 0, OFFSCREEN_WIDTH, OFFSCREEN_HEIGHT); // Set a viewport for FBO

	gles2.glClearColor(0.2, 0.2, 0.4, 1.0); // Set clear color (the color is slightly changed)
	gles2.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear FBO

	drawTexturedCube( cube, angle, texture, viewProjMatrixFBO);   // Draw the cube

	gles2.glBindFramebuffer(GL_FRAMEBUFFER, 0);        // Change the drawing destination to color buffer
	gles2.glViewport(0, 0, stage->stage_w, stage->stage_h);

	gles2.glClearColor(0.0, 0.0, 0.0, 1.0);
	gles2.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color buffer

	//drawTexturedPlane( plane, angle, fbo.texture, viewProjMatrix);  // Draw the plane
}




void drawTexturedCube2( Object * o,float angle,GLuint texture, Matrix3D * viewpProjMatrix,Matrix3D * u_MvpMatrix) {
	Data3d * data2D = o->sprite->data3d;//Data3D_new();
	// Calculate a model matrix
	/*
	   g_modelMatrix.rotate(20.0, 1.0, 0.0, 0.0);
	   g_modelMatrix.rotate(angle, 0.0, 1.0, 0.0);
	   g_modelMatrix.scale(1, 1, 1);

	// Calculate the model view project matrix and pass it to u_MvpMatrix
	g_mvpMatrix.set(vpMatrix);
	g_mvpMatrix.multiply(g_modelMatrix);

	*/
	//gles2.glUniformMatrix4fv(u_MvpMatrix, 0, g_mvpMatrix.elements);
	gles2.glUniformMatrix4fv(data2D->mvpLoc,1, GL_FALSE, &g_mvpMatrix.rawData[0][0]);
	drawTexturedObject(o, texture);
}

GLuint fbo;
Object * plane;
Object * cube;
GLuint texture;
Matrix3D viewProjMatrix;// = new Matrix4();   // Prepare view projection matrix for color buffer
Matrix3D viewProjMatrixFBO;// = new Matrix4();   // Prepare view projection matrix for FBO

void showFunc(Sprite * sprite)
{
	float currentAngle = 0.0;  // Update current rotation angle
	draw( fbo, plane, cube, currentAngle, texture, &viewProjMatrix, &viewProjMatrixFBO);
}

int main()
{
	Stage_init();


	Sprite * sprite = Sprite_new();
	sprite->is3D = SDL_TRUE;
	sprite->surface = SDL_LoadBMP("1.bmp");
	Sprite_addChild(stage->sprite,sprite);
	//sprite->showFunc = showFunc;
	Data3d * data2D = Data3D_new();
	Data3d_reset(data2D);
	data2D->programObject = esLoadProgram( VSHADER_SOURCE, FSHADER_SOURCE);
	program = data2D->programObject;
	if(data2D->programObject){
		data2D->positionLoc = GL_CHECK(gles2.glGetAttribLocation ( data2D->programObject, "a_Position" ));
		data2D->texCoordLoc = GL_CHECK(gles2.glGetAttribLocation ( data2D->programObject, "a_TexCoord" ));
		data2D->mvpLoc = GL_CHECK(gles2.glGetUniformLocation( data2D->programObject, "u_MvpMatrix" ));
		data2D->samplerLoc= gles2.glGetUniformLocation(data2D->programObject, "u_Sampler");
	}
	sprite->data3d = data2D;
	cube = initVertexBuffersForCube();
	cube->sprite = sprite;
	plane = initVertexBuffersForPlane();
	plane->sprite = sprite;
	gles2.glUniform1i(data2D->samplerLoc, 0);
	texture = SDL_GL_LoadTexture(sprite, NULL);
	return 0;
	fbo = initFramebufferObject(data2D);
	gles2.glEnable(GL_DEPTH_TEST);   //  gles2.enable(CULL_FACE);

	Matrix_identity(&g_modelMatrix);
	Matrix_identity(&g_mvpMatrix);
	Matrix_identity(&viewProjMatrix);
	//viewProjMatrix.setPerspective(30, canvas.width/canvas.height, 1.0, 100.0);
	//viewProjMatrix.lookAt(0.0, 0.0, 7.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	Matrix_identity(&viewProjMatrixFBO);
	//viewProjMatrixFBO.setPerspective(30.0, OFFSCREEN_WIDTH/OFFSCREEN_HEIGHT, 1.0, 100.0);
	//viewProjMatrixFBO.lookAt(0.0, 2.0, 7.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);


	Stage_loopEvents();
	return 0;
}
