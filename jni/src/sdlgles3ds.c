/**
 *
 gcc sdlgles3ds.c  -I"../SDL2/include/" -I"../SDL2_ttf/"  -I"../3ds/" -I"../SDL2_image/" -L. -lmingw32 -lSDL2_test -lSDL2main -lSDL2 -l3ds -lSDL2_ttf && a.exe
 gcc sdlgles3ds.c  lib3ds_atmosphere.c  lib3ds_background.c  lib3ds_camera.c  lib3ds_chunk.c  lib3ds_chunktable.c  lib3ds_file.c  lib3ds_io.c  lib3ds_light.c  lib3ds_material.c  lib3ds_math.c  lib3ds_matrix.c  lib3ds_mesh.c  lib3ds_node.c  lib3ds_quat.c  lib3ds_shadow.c  lib3ds_track.c  lib3ds_util.c  lib3ds_vector.c  lib3ds_viewport.c -lSDL2_test -lSDL2 -lSDL2_ttf && ./a.out
 gcc -Wall sdlgles3ds.c -lm -lSDL2_test -lSDL2 -lSDL2_ttf -l3ds && chmod +x a.out  && ./a.out --info all &&
 gcc -Wall sdlgles3ds.c -L. -l3ds20 -lmingw32 -lSDL2_test -lSDL2main -lSDL2 -lSDL2_ttf  && a --info all
 gcc sdlgles3ds.c  lib3ds_atmosphere.c  lib3ds_background.c  lib3ds_camera.c  lib3ds_chunk.c  lib3ds_chunktable.c  lib3ds_file.c  lib3ds_io.c  lib3ds_light.c  lib3ds_material.c  lib3ds_math.c  lib3ds_matrix.c  lib3ds_mesh.c  lib3ds_node.c  lib3ds_quat.c  lib3ds_shadow.c  lib3ds_track.c  lib3ds_util.c  lib3ds_vector.c  lib3ds_viewport.c -L. -lopengl32 -lmingw32 -lSDL2_test -lSDL2main -lSDL2 -lSDL2_ttf && a 
 export LD_LIBRARY_PATH=/usr/local/lib
 ls /usr/local/lib
 */
#include "SDL.h"
#include "gles2base.h"
#include "lib3ds.h"
#include <assert.h>
#ifndef max
#  define max(x,y)  ((x)>(y)?(x):(y))
#endif

static int Rendered = 0;
static Lib3dsFile *model=0;
static const char* camera=0;
//static float current_frame=0.0;
//static int halt=0;
static int nfaces=0;
float      sizes[4];		// 模型尺寸坐标
float      center[3];		// 模型目标中心坐标
int      startDrag;
int      startx;
int      starty;
int      anglex;
int      angley;
//static const char* filename="Crow.3ds";
//static const char* filename="plane.3DS";
//static const char* filename="cube.3ds";
#ifdef __ANDROID__
static const char* filename="/sdcard/sdl2opengles/man.3DS";
#else
static const char* filename="../../a/man.3DS";
#endif
//static const char* filename="../Dennis_HR.3ds";
//static const char* filename="../Monkey_HR.3ds";
//static const char* filename="../motom.3DS";
//static const char* filename="../bmw.3ds";
//static const char* filename="../bed.3ds";
//static const char* filename="../horse.3DS";
//static const char* filename="../bird.3DS";
//static const char* filename="../dog.3DS";
//static const char* filename="../horse2.3DS";
//static const char* filename="../rabbit.3DS";
//static const char* filename="d:/work/j/gl/opengles-book-samples-master/Windows/Chapter_10/RM_ClipPlane/Sphere.3ds";
//static const char* filename="d:/work/j/gl/opengles-book-samples-master/Windows/Chapter_10/RM_AlphaTest/Torus.3ds";
//static const char* filename="d:/work/j/gl/opengles-book-samples-master/Windows/Chapter_10/RM_LinearFog/Terrain.3ds";
//static const char* filename="d:/work/j/gl/Source/lib3ds_2.0/glut_test/man/man.3DS";
//static const char* filename="d:/work/j/gl/Source/lib3ds_2.0/glut_test/cube.3ds";
//static const char* filename="d:/work/j/gl/Source/lib3ds_2.0/data/ABOM.3DS";
typedef struct UserData
{
	// Uniform locations
	GLint  mvpLoc;
	GLint  lightPosLoc;
	// Vertex daata
	// Rotation angle
	GLfloat   angle;
	// MVP matrix
	ESMatrix  mvpMatrix;

	GLuint shader_frag, shader_vert;
	int      numIndices;
	GLfloat *vertices;
	GLfloat *normals;
	GLfloat *texCoords;
	GLuint  *indices;
	// Attribute locations
	GLint baseMapLoc;
	GLint lightMapLoc;
	GLint  normalLoc;
	// Texture handle
	GLuint baseMapTexId;
	GLuint lightMapTexId;
	// Handle to a program object
	GLuint programObject;
	// Attribute locations
	GLint  positionLoc;
	GLint  texCoordLoc;
	// Sampler location
	GLint samplerLoc;
	// Attribute locations
	GLint  lifetimeLoc;
	GLint  startPositionLoc;
	GLint  endPositionLoc;
	// Offset location
	GLint offsetLoc;
	GLint timeLoc;
	GLint colorLoc;
	GLint centerPositionLoc;
	// Texture handle
	GLuint textureId;
	float particleData[ NUM_PARTICLES * PARTICLE_SIZE ];
	float time;
} UserData;
void Draw ( UserData*userData);
int Init ( UserData*userData);
void Update ( UserData *userData , float deltaTime );
static void render_node(Lib3dsNode *node,UserData*userData)
{
	Lib3dsNode		*p;
	Lib3dsMesh		*mesh;
	Lib3dsFace		*face;
	//Lib3dsMaterial  *mat;
	//Lib3dsMeshInstanceNode *meshData;

	Lib3dsVector	*norm_verts;
	Lib3dsVector	*norm_faces;
	unsigned	fi;
	float		M[4][4];
	assert(model);
	// 递归
	for (p=node->childs; p!=0; p=p->next){
		render_node(p,userData);
	}

	if (node->type==LIB3DS_NODE_MESH_INSTANCE)
	{
		if (strcmp(node->name,"$$$DUMMY")==0) {
			return;
		}
		if (!node->user_id) 
		{
			mesh = lib3ds_file_mesh_for_node(model, node);
			assert(mesh);
			if (!mesh) {
				return;
			}
			//node->user_id = glGenLists(1);
			//glNewList(node->user_id, GL_COMPILE);
			norm_verts = (Lib3dsVector*) malloc(3*sizeof(Lib3dsVector)*mesh->nfaces);
			norm_faces = (Lib3dsVector*) malloc(sizeof(Lib3dsVector)*mesh->nfaces);				
			lib3ds_matrix_copy(M, mesh->matrix);
			lib3ds_matrix_inv(M);
			//glMultMatrixf(&M[0][0]);
			lib3ds_mesh_calculate_face_normals(mesh, (float (*)[3])norm_faces);
			lib3ds_mesh_calculate_vertex_normals(mesh, (float (*)[3])norm_verts);

			/*
			   for (fi=0; fi<mesh->nfaces; ++fi) {
			   face = &(mesh->faces[fi]);
			   int i;
			   mat = 0;
			   if (face->material>=0 && face->material<model->nmaterials)
			   mat=model->materials[face->material];

			   if (mat) 
			   {
			// 使用材质
			float s = pow(2, 10.0*mat->shininess);
			if (s>128.0) s = 128.0f;
			glMaterialfv(GL_FRONT, GL_AMBIENT, mat->ambient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR, mat->specular);
			glMaterialf(GL_FRONT, GL_SHININESS, s);
			} else {
			// 使用贴图
			float a[]={0.2, 0.2, 0.2, 1.0};
			float d[]={0.8, 0.8, 0.8, 1.0};
			float s[]={0.0, 0.0, 0.0, 1.0};
			glMaterialfv(GL_FRONT, GL_AMBIENT, a);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, d);
			glMaterialfv(GL_FRONT, GL_SPECULAR, s);
			}

			// Draw tri-face
			//gles2.glBegin(GL_TRIANGLES);
			gles2.glBegin(GL_LINE_LOOP);
			//face normal
			//gles2.glNormal3fv(norm_faces[fi].v);	 

			for (i=0; i<3; ++i) {
			//vertex normal
			//gles2.glNormal3fv(norm_verts[3*fi+i].v);	 
			gles2.glVertex3fv(mesh->vertices[face->index[i]]);
			}
			gles2.glEnd();

			}

*/
			if(userData->vertices==NULL){
				userData->vertices = (GLfloat*)malloc(9*sizeof(GLfloat)*(nfaces+mesh->nfaces));
				userData->normals = (GLfloat*)malloc(9*sizeof(GLfloat)*(nfaces+mesh->nfaces));
			}else{
				userData->vertices = (GLfloat*)realloc(userData->vertices,9*sizeof(GLfloat)*(nfaces+mesh->nfaces));
				userData->normals = (GLfloat*)realloc(userData->normals,9*sizeof(GLfloat)*(nfaces+mesh->nfaces));
			}

			for (fi=0; fi<mesh->nfaces; ++fi) {
				face = &(mesh->faces[fi]);
				int findex = fi+nfaces;
				int i=0;
				for (i=0; i<9; ++i) {
					userData->vertices[findex*9+i]=(GLfloat)*(mesh->vertices[face->index[i/3]]+i%3);
					userData->vertices[findex*9+i]-=center[i%3];
					userData->vertices[findex*9+i]/=sizes[3]/2;
				}
				for (i=0; i<9; ++i) {
					userData->normals[findex*9+i]=(GLfloat)norm_verts[3*fi+i/3].v[i%3];
				}
			}

			nfaces += mesh->nfaces;

			free(norm_faces);
			free(norm_verts);
		}
		/*
		   if (node->user_id) {
		   glPushMatrix();
		   meshData = (Lib3dsMeshInstanceNode*) node;

		   glMultMatrixf(&node->matrix[0][0]);
		   glTranslatef(-meshData->pivot[0], -meshData->pivot[1], -meshData->pivot[2]);
		   glCallList(node->user_id);

		   glutSolidSphere(50.0, 20,20);
		   glPopMatrix();
		   }
		   */
	}
}


// 增加默认的灯光
static void add_default_lights (Lib3dsFile *model, Lib3dsBBox *bbox, float sizes[], float center[])
{
	int  index=0;
	Lib3dsOmnilightNode     *omniNode;
	Lib3dsLight				*light;

	// light0
	light = lib3ds_light_new("light0");
	assert(light);

	light->spot_light = 0;
	light->see_cone = 0;
	light->color[0]=light->color[1]=light->color[2]=0.6f;
	light->position[0]=center[0]+sizes[3]*0.75;
	light->position[1]=center[1]-sizes[3]*1;
	light->position[2]=center[2]+sizes[3]*1.5;
	light->outer_range = 100;
	light->inner_range = 10;
	light->multiplier=1;

	lib3ds_file_insert_light(model, light, index++);
	omniNode = lib3ds_node_new_omnilight(light);
	assert(omniNode);
	lib3ds_file_append_node(model, (Lib3dsNode*)omniNode, 0);

	// light1
	light = lib3ds_light_new("light1");
	assert(light);

	light->spot_light = 0;
	light->see_cone = 0;
	light->color[0]=light->color[1]=light->color[2]=0.3f;
	light->position[0]=center[0]-sizes[3];
	light->position[1]=center[1]-sizes[3];
	light->position[2]=center[2]+sizes[3]*0.75;
	light->outer_range = 100;
	light->inner_range = 10;
	light->multiplier=1;

	lib3ds_file_insert_light(model, light, index++);
	omniNode = lib3ds_node_new_omnilight(light);
	assert(omniNode);
	lib3ds_file_append_node(model, (Lib3dsNode*)omniNode, 0);

	// light2
	light = lib3ds_light_new("light2");
	assert(light);

	light->spot_light = 0;
	light->see_cone = 0;
	light->color[0]=light->color[1]=light->color[2]=0.3f;
	light->position[0]=center[0];
	light->position[1]=center[1]+sizes[3];
	light->position[2]=center[2]+sizes[3];
	light->outer_range = 100;
	light->inner_range = 10;
	light->multiplier=1;

	lib3ds_file_insert_light(model, light, index++);
	omniNode = lib3ds_node_new_omnilight(light);
	assert(omniNode);
	lib3ds_file_append_node(model, (Lib3dsNode*)omniNode, 0);
}


// 增加默认的相机和目标
static void add_default_cameras (Lib3dsFile *model, Lib3dsBBox *bbox, float sizes[], float center[])
{
	int  index=0;
	Lib3dsCamera     *cam;
	Lib3dsCameraNode *nodCam;

	Lib3dsTargetNode *nodTgt;

	// Camera_X
	cam = lib3ds_camera_new ("Camera_X");
	assert(cam);
	memcpy(cam->target, center, sizeof(Lib3dsVector));
	memcpy(cam->position, center, sizeof(Lib3dsVector));
	cam->position[0] = bbox->bmax[0] + 1.5*max(sizes[1],sizes[2]);
	cam->near_range = (cam->position[0] - bbox->bmax[0])/2;
	cam->far_range = (cam->position[0] - bbox->bmin[0])*2;
	lib3ds_file_insert_camera(model, cam, index++);
	nodCam = lib3ds_node_new_camera(cam);
	assert(nodCam);
	lib3ds_file_append_node(model, (Lib3dsNode*)nodCam, 0);

	nodTgt = lib3ds_node_new_camera_target(cam);
	assert(nodTgt);
	lib3ds_file_append_node(model, (Lib3dsNode*)nodTgt, 0);

	// Camera_Y
	cam = lib3ds_camera_new ("Camera_Y");
	assert(cam);
	memcpy(cam->target, center, sizeof(Lib3dsVector));
	memcpy(cam->position, center, sizeof(Lib3dsVector));
	cam->position[1] = bbox->bmin[1] - 1.5*max(sizes[0],sizes[2]);
	cam->near_range = (bbox->bmin[1]-cam->position[1])/2;
	cam->far_range = (bbox->bmax[1]-cam->position[1])*2;
	lib3ds_file_insert_camera(model, cam, index++);
	nodCam = lib3ds_node_new_camera(cam);
	assert(nodCam);
	lib3ds_file_append_node(model, (Lib3dsNode*)nodCam, 0);

	nodTgt = lib3ds_node_new_camera_target(cam);
	assert(nodTgt);
	lib3ds_file_append_node(model, (Lib3dsNode*)nodTgt, 0);

	// Camera_Z
	cam = lib3ds_camera_new ("Camera_Z");
	assert(cam);
	memcpy(cam->target, center, sizeof(Lib3dsVector));
	memcpy(cam->position, center, sizeof(Lib3dsVector));
	cam->position[2] = bbox->bmax[2] + 1.5*max(sizes[0],sizes[1]);
	cam->near_range = (cam->position[2]-bbox->bmax[2])/2;
	cam->far_range = (cam->position[2]-bbox->bmin[2])*2;
	lib3ds_file_insert_camera(model, cam, index++);
	nodCam = lib3ds_node_new_camera(cam);
	assert(nodCam);
	lib3ds_file_append_node(model, (Lib3dsNode*)nodCam, 0);

	nodTgt = lib3ds_node_new_camera_target(cam);
	assert(nodTgt);
	lib3ds_file_append_node(model, (Lib3dsNode*)nodTgt, 0);

	// Camera_ISO
	cam = lib3ds_camera_new ("Camera_ISO");
	assert(cam);
	memcpy(cam->target, center, sizeof(Lib3dsVector));
	cam->position[0] = bbox->bmax[0] + 0.75f*sizes[3];
	cam->position[1] = bbox->bmin[1] - 0.75f*sizes[3];
	cam->position[2] = bbox->bmax[2] + 0.75f*sizes[3];
	cam->near_range = (cam->position[0]-bbox->bmax[0])/2;
	cam->far_range = (cam->position[0]-bbox->bmin[0])*3;
	lib3ds_file_insert_camera(model, cam, index++);
	nodCam = lib3ds_node_new_camera(cam);
	assert(nodCam);
	lib3ds_file_append_node(model, (Lib3dsNode*)nodCam, 0);

	nodTgt = lib3ds_node_new_camera_target(cam);
	assert(nodTgt);
	lib3ds_file_append_node(model, (Lib3dsNode*)nodTgt, 0);
}



/*
   GLbyte vShaderStr[] =  
//13-1 顶点着色器每个频道的光照 
"uniform mat4 u_matViewInverse; "
"uniform mat4 u_matViewProjection; "
"uniform vec3 u_lightPosition; "
"uniform vec3 u_eyePosition; "
"varying vec2 v_texcoord; "
"varying vec3 v_viewDirection; "
"varying vec3 v_lightDirection; "
"attribute vec4 a_vertex; "
"attribute vec2 a_texcoord0; "
"attribute vec3 a_normal; "
"attribute vec3 a_binormal; "
"attribute vec3 a_tangent; "
""
"void main(void) "
"{ "
"	// Transform eye vector into world space "
"	vec3 eyePositionWorld = (u_matViewInverse * vec4(u_eyePosition, 1.0)).xyz; "
""
"	// Compute world space direction vector "
"	vec3 viewDirectionWorld = eyePositionWorld - a_vertex.xyz; "
""
"	// Transform light position into world space "
"	vec3 lightPositionWorld = (u_matViewInverse * vec4(u_lightPosition, 1.0)).xyz; "
""
"	// Compute world space light direction vector "
"	vec3 lightDirectionWorld = lightPositionWorld - a_vertex.xyz; "
""
"	// Create the tangent matrix "
"	mat3 tangentMat = mat3(a_tangent, a_binormal, a_normal);       "
""
"	// Transform the view and light vectors into tangent space "
"	v_viewDirection = viewDirectionWorld * tangentMat; "
"	v_lightDirection = lightDirectionWorld * tangentMat; "
""
"	// Transform output position "
"	gl_Position = u_matViewProjection * a_vertex; "
"	// Pass through texture coordinate "
"	v_texcoord = a_texcoord0.xy; "
"} "
;
GLbyte fShaderStr[] =  
"//13-2 片段着色器每片段的光照 "
"precision mediump float; "
"uniform vec4 u_ambient; "
"uniform vec4 u_specular; "
"uniform vec4 u_diffuse; "
"uniform float u_specularPower; "
"uniform sampler2D s_baseMap; "
"uniform sampler2D s_bumpMap; "
"varying vec2 v_texcoord; "
"varying vec3 v_viewDirection; "
"varying vec3 v_lightDirection; "
"void main(void) "
"{ "
"	// Fetch basemap color "
"	vec4 baseColor = texture2D(s_baseMap, v_texcoord); "
""
"	// Fetch the tangent-space normal from normal map "
"	vec3 normal = texture2D(s_bumpMap, v_texcoord).xyz; "
""
"	// Scale and bias from [0, 1] to [-1, 1] and normalize "
"	normal = normalize(normal * 2.0 - 1.0); "
""
"	// Normalize the light direction and view direction "
"	vec3 lightDirection = normalize(v_lightDirection); "
"	vec3 viewDirection = normalize(v_viewDirection); "
""
"	// Compute N.L "
"	float nDotL = dot(normal, lightDirection); "
""
"	// Compute reflection vector "
"	vec3 reflection = (2.0 * normal * nDotL) - lightDirection; "
""
"	// Compute R.V "
"	float rDotV = max(0.0, dot(reflection, viewDirection)); "
""
"	// Compute Ambient term "
"	vec4 ambient = u_ambient * baseColor; "
""
"	// Compute Diffuse term "
"	vec4 diffuse = u_diffuse * nDotL * baseColor; "
""
"	// Compute Specular term "
"	vec4 specular = u_specular * pow(rDotV, u_specularPower); "
""
"	// Output final color "
"	gl_FragColor = ambient + diffuse + specular;      "
"} "
;

*/



int Init ( UserData *userData)
{

	GLbyte vShaderStr[] =  
		"uniform mat4 u_matViewInverse; "
		"uniform vec3 u_lightPosition; "
		"uniform mat4 u_mvpMatrix;                   \n"
		"attribute vec2 a_texcoord0; "
		"attribute vec3 a_binormal; "
		"attribute vec3 a_tangent; "
		"attribute vec4 a_position;                  \n"
		"attribute vec3 a_normal;     \n"
		"varying vec3 v_normal;       \n"
		"varying vec2 v_texcoord; "
		"varying vec3 v_lightDirection; "
		"void main()                                 \n"
		"{                                           \n"
		"	vec3 lightPositionWorld = (vec4(u_lightPosition, 1.0)).xyz; "
		"	vec3 lightDirectionWorld = lightPositionWorld - a_position.xyz; "
		//"	mat3 tangentMat = mat3(a_tangent, a_binormal, a_normal);       "
		"	mat3 tangentMat = mat3(vec3(.0), vec3(.0), vec3(5.0));       "
		"	v_lightDirection = lightDirectionWorld * tangentMat; "
		"	v_texcoord = a_texcoord0.xy; "
		"   gl_Position = u_mvpMatrix * a_position;  \n"
		"   v_normal = a_normal;      \n"
		"}                                           \n";

	GLbyte fShaderStr[] =  
		"precision mediump float;                            \n"
		"uniform vec4 u_diffuse; "
		"uniform sampler2D s_baseMap; "
		"varying vec3 v_normal;                              \n"
		"varying vec2 v_texcoord; "
		"varying vec3 v_lightDirection; "
		"void main()                                         \n"
		"{                                                   \n"
		//"  gl_FragColor = vec4( v_normal/8.0, 1.0 );        \n"
		//"	vec4 baseColor = texture2D(s_baseMap, v_texcoord); "
		"	vec3 lightDirection = normalize(v_lightDirection); "
		"	gl_FragColor = dot(v_normal, lightDirection) * vec4(1.0)/4 + vec4(vec3(.2),1.0);      "
		"}                                                   \n";

	// Load the shaders and get a linked program object
	userData->programObject = esLoadProgram ( vShaderStr, fShaderStr );
	// Get the attribute locations
	userData->positionLoc = gles2.glGetAttribLocation ( userData->programObject, "a_position" );
	// Get the uniform locations
	userData->mvpLoc = gles2.glGetUniformLocation( userData->programObject, "u_mvpMatrix" );
	userData->normalLoc = gles2.glGetAttribLocation ( userData->programObject, "a_normal" );

	userData->lightPosLoc = gles2.glGetUniformLocation( userData->programObject, "u_lightPosition" );
	userData->baseMapLoc = gles2.glGetUniformLocation ( userData->programObject, "s_baseMap" );
#ifdef __ANDROID__
	userData->baseMapTexId = LoadTexture ( "/sdcard/basemap.tga" );
#else
	userData->baseMapTexId = LoadTexture ( "../../a/basemap.tga" );
#endif
	if ( userData->baseMapTexId == 0 )
		return 0;

	// Generate the vertex data
	//userData->numIndices = esGenCube( 1.0, &userData->vertices, NULL, NULL, &userData->indices );
	// Starting rotation angle for the cube
	userData->angle = 0.0f;
	gles2.glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
	gles2.glEnable(GL_DEPTH_TEST);

	/**

	  glShadeModel(GL_SMOOTH);
	  glEnable(GL_LIGHTING);
	  glEnable(GL_LIGHT0);
	  glDisable(GL_LIGHT1);
	  glDepthFunc(GL_LEQUAL);
	  glEnable(GL_DEPTH_TEST);
	  glEnable(GL_CULL_FACE);
	  glCullFace(GL_BACK);
	//glDisable(GL_NORMALIZE);
	//glPolygonOffset(1.0, 2);
	*/



	Lib3dsBBox bbox;			// 模型范围
	gles2.glClearColor(0, 0, 0, 1.0);	// 背景色
	gles2.glEnable(GL_CULL_FACE);
	// 打开模型文件，读入模型数据并关闭文件句柄
	model=lib3ds_file_open(filename);
	if (!model) {
		printf("***ERROR*** Loading 3DS file failed.");
		exit(1);
	}
	/**
	 *
	 lib3ds_file_bounding_box_of_nodes(Lib3dsFile *file, 
	 int include_meshes, int include_cameras,int include_lights,
	 float bmin[3], float bmax[3], float matrix[4][4]) 
	 */
	// 读取模型的包围盒
	lib3ds_file_bounding_box_of_nodes (model, 1, 0, 0, bbox.bmin, bbox.bmax, 0);
	int i;
	for (i=0; i<3; i++) {
		sizes[i] = bbox.bmax[i]-bbox.bmin[i];
		center[i]= (bbox.bmax[i]+bbox.bmin[i])/2;
	}
	sizes[3] = max(sizes[0],sizes[1]);
	sizes[3] = max(sizes[3],sizes[2]);

	SDL_Log("center:%f,%f,%f",center[0],center[1],center[2]);
	SDL_Log("sizes:%f,%f,%f,%f",sizes[0],sizes[1],sizes[2],sizes[3]);
	SDL_Log("bbox.bmin:%f,%f,%f",bbox.bmin[0],bbox.bmin[1],bbox.bmin[2]);
	SDL_Log("bbox.bmax:%f,%f,%f",bbox.bmax[0],bbox.bmax[1],bbox.bmax[2]);
	if (!model->cameras) {
		add_default_cameras (model, &bbox, sizes, center);
		add_default_lights (model, &bbox, sizes, center);
	}

	if (!camera) {
		camera = model->cameras[0]->name;
	}
	lib3ds_file_eval(model, 0);
	return 1;
}

void Draw ( UserData *userData)
{
	Update(userData,0);
	int w = state->window_w;
	int h = state->window_h;
	float rate = .0f; 
	gles2.glViewport(-w*rate*1, -h*rate, w*(rate*2+1), h*(rate*2+1));
	gles2.glUseProgram ( userData->programObject );
	gles2.glUniformMatrix4fv( userData->mvpLoc, 1, GL_FALSE, (GLfloat*) &userData->mvpMatrix.m[0][0] );
	gles2.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(!Rendered){
		Lib3dsNode *nodC, *nodT, *nod;
		Lib3dsCameraNode *camNode;
		Lib3dsTargetNode *tgtNode;
		float M[4][4];
		/*
		   int  i, li;
		   GLfloat a[] = {0.0f, 0.0f, 0.0f, 1.0f};
		   GLfloat c[] = {1.0f, 1.0f, 1.0f, 1.0f};
		   GLfloat p[] = {0.0f, 0.0f, 0.0f, 1.0f};
		   Lib3dsLight *l;
		   */
		if (!model) {
			exit(1);
		}
		nodC=lib3ds_file_node_by_name(model, camera, LIB3DS_NODE_CAMERA);
		nodT=lib3ds_file_node_by_name(model, camera, LIB3DS_NODE_CAMERA_TARGET);
		if (!nodC || !nodT) {
			assert(0 && "Camera or Target not found!");
			exit(1);
		}
		camNode = (Lib3dsCameraNode*) nodC;
		tgtNode = (Lib3dsTargetNode*) nodT;
		/*
		   glMatrixMode(GL_PROJECTION);
		   glLoadIdentity();
		   gluPerspective( camNode->fov, gl_width/gl_height, 0.1f, 6000.0);
		   glMatrixMode(GL_MODELVIEW);
		   glLoadIdentity();
		   glRotatef(-90, 1.0, 0,0);

		   li=GL_LIGHT0;

		   for (i=0; i<model->nlights; i++)
		   {
		   l = model->lights[i];
		   glEnable(li);

		   glLightfv(li, GL_AMBIENT, a);
		   glLightfv(li, GL_DIFFUSE, c);
		   glLightfv(li, GL_SPECULAR, c);	// p?

		   p[0] = l->position[0];
		   p[1] = l->position[1];
		   p[2] = l->position[2];
		   glLightfv(li, GL_POSITION, p);

		   if (!l->spot_light) {
		   continue;
		   }

		   p[0] = l->target[0] - l->position[0];
		   p[1] = l->target[1] - l->position[1];
		   p[2] = l->target[2] - l->position[2];      

		   glLightfv(li, GL_SPOT_DIRECTION, p);

		   ++li;
		   }
		   */

		lib3ds_matrix_camera(M, camNode->pos, tgtNode->pos, camNode->roll);
		//gles2.glMultMatrixf(&M[0][0]);
		for (nod=model->nodes; nod!=0; nod=nod->next) {
			render_node(nod,userData);
		}

		gles2.glVertexAttribPointer ( userData->normalLoc, 3, GL_FLOAT, GL_FALSE, 0, userData->normals );
		gles2.glEnableVertexAttribArray ( userData->normalLoc );

		gles2.glVertexAttribPointer ( userData->positionLoc, 3, GL_FLOAT, GL_FALSE, 0, userData->vertices);
		gles2.glEnableVertexAttribArray ( userData->positionLoc );



		GLfloat *lightPos = (GLfloat*)malloc(sizeof(GLfloat)*4);
		lightPos[0]=8.0;
		lightPos[1]=2.0;
		lightPos[2]=4.0;
		gles2.glVertexAttribPointer ( userData->lightPosLoc, 3, GL_FLOAT, GL_FALSE, 0, lightPos);
		gles2.glEnableVertexAttribArray ( userData->lightPosLoc);

		gles2.glBindTexture ( GL_TEXTURE_2D, userData->baseMapTexId );
		gles2.glActiveTexture ( GL_TEXTURE1 );
		gles2.glBindTexture ( GL_TEXTURE_2D, userData->lightMapTexId );

		Rendered = 1;
	}
	gles2.glDrawArrays( GL_TRIANGLES, 0, 3*nfaces);

	//gles2.glDrawElements ( GL_TRIANGLES, 9, GL_UNSIGNED_INT, userData->indices );
	//gles2.glDrawElements ( GL_LINE_STRIP, 9, GL_UNSIGNED_INT, userData->indices);

	/*
	   if (!halt) {
	   current_frame+=1.0;
	   if (current_frame>model->frames) {
	   current_frame=0;
	   }
	   lib3ds_file_eval(model, current_frame);
	//glutSwapBuffers();
	//glutPostRedisplay();
	}
	*/
}
void rotates(UserData*userData,int deltax,int deltay)
{
	ESMatrix perspective;
	ESMatrix modelview;
	float    aspect;

	// Compute the window aspect ratio
	aspect = (GLfloat) state->window_w/ (GLfloat) state->window_h;

	// Generate a perspective matrix with a 60 degree FOV
	esMatrixLoadIdentity( &perspective );
	esPerspective( &perspective, 60.0f, aspect, 1.0f, 20.0f );
	anglex += deltax*30/(20.0+1.0);
	angley += aspect*deltay*30/(20.0+1.0);
	anglex %= 360;
	angley %= 360;
	//printf("-----%d x %d\n",userData->anglex,userData->angley);

	// Generate a model view matrix to rotate/translate the cube
	esMatrixLoadIdentity( &modelview );

	// Translate away from the viewer
	esTranslate( &modelview, 0.0, 0.0, -3.0 );

	// Rotate the cube
	esRotate( &modelview, -anglex, .0, 1.0, .0 );
	esRotate( &modelview, -angley, 1.0, .0, .0 );

	// Compute the final MVP by multiplying the 
	// modevleiw and perspective matrices together
	esMatrixMultiply( &userData->mvpMatrix, &modelview, &perspective );
}

void Update ( UserData *userData , float deltaTime )
{
	rotates(userData,0,0);
}

int main(int argc, char *argv[])
{
	int i;
	int num_vid_drivers= SDL_GetNumVideoDrivers();
	i=0;
	while(i<num_vid_drivers)
	{
		SDL_Log("%d:%s",i,SDL_GetVideoDriver(i));
		++i;
	}
	SDL_DisplayMode mode;
	UserData *userData;
	SDL_setenv("SDL_VIDEODRIVER",SDL_GetVideoDriver(0),1);
	SDL_setenv("DISPLAY",":0.0",1);
	char*driver_name = SDL_getenv("SDL_VIDEODRIVER");
	if(driver_name) SDL_Log(driver_name);
	else SDL_Log("no SDL_VIDEODRIVER");
	/* Initialize test framework */
	state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
	if (!state) {
		return 1;
	}
	/* Set OpenGL parameters */
	state->window_flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS;
	state->gl_red_size = 5;
	state->gl_green_size = 5;
	state->gl_blue_size = 5;
	state->gl_depth_size = depth;
	state->gl_major_version = 2;
	state->gl_minor_version = 0;
	state->window_w = 240;
	state->window_h = 320;
#if defined(linux)    
	state->gl_profile_mask = SDL_GL_CONTEXT_PROFILE_ES;
#endif
	if (!SDLTest_CommonInit(state)) {
		quit(2);
		return 0;
	}
	SDL_Log("SDL_GetCurrentVideoDriver:%s",SDL_GetCurrentVideoDriver());

	context = SDL_calloc(state->num_windows, sizeof(context));
	if (context == NULL) {
		SDL_Log("Out of memory!\n");
		quit(2);
	}

	/* Create OpenGL ES contexts */
	for (i = 0; i < state->num_windows; i++) {
		context[i] = SDL_GL_CreateContext(state->windows[i]);
		if (!context[i]) {
			SDL_Log("SDL_GL_CreateContext(): %s\n", SDL_GetError());
			quit(2);
		}
	}
	/* Important: call this *after* creating the context */
	if(gles2.glGetError==NULL)
		if (LoadContext(&gles2) < 0) {
			SDL_Log("Could not load GLES2 functions\n");
			quit(2);
		}
	if (state->render_flags & SDL_RENDERER_PRESENTVSYNC) {
		SDL_GL_SetSwapInterval(1);
	} else {
		SDL_GL_SetSwapInterval(0);
	}
	SDL_GetCurrentDisplayMode(0, &mode);
#if defined(linux)
	state->window_h = mode.h;
	state->window_w = mode.w;
#endif
	userData = SDL_calloc(state->num_windows, sizeof(UserData));
	int status;
	/* Set rendering settings for each context */
	for (i = 0; i < state->num_windows; ++i) {
		status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
		if (status) {
			SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
			/* Continue for next window */
			continue;
		}
		Init(&userData[i]);
	}
	/* Main render loop */
	Uint32 then, now, frames;
	SDL_Event event;
	frames = 0;
	then = SDL_GetTicks();
	int done;
	done = 0;

	_then = then;
	while (!done) {
		/* Check for events */
		++frames;
		while (SDL_PollEvent(&event) && !done) {
			switch (event.type) {
				case SDL_WINDOWEVENT:
					switch (event.window.event) {
						case SDL_WINDOWEVENT_RESIZED:
							for (i = 0; i < state->num_windows; ++i) {
								if (event.window.windowID == SDL_GetWindowID(state->windows[i])) {
									status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
									if (status) {
										SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
										break;
									}
									/* Change view port to the new window dimensions */
									state->window_w = event.window.data1;
									state->window_h = event.window.data2;
									/* Update window content */
									Draw(&userData[i]); SDL_GL_SwapWindow(state->windows[i]);
									break;
								}
							}
							break;
					}
				case SDL_MOUSEBUTTONDOWN:
					//if(event.button.which==0)
					{
						startDrag = 1;
						startx = event.button.x;
						starty = event.button.y;
					}
					SDL_Log("start mouse: %i,x: %i, y: %i",event.button.which, event.button.x,event.button.y);
					break;
				case SDL_FINGERDOWN:
					//if(event.tfinger.fingerId==0)
					{
						startDrag = 1;
						startx = event.tfinger.x;
						starty = event.tfinger.y;
					}
					SDL_Log("Finger: %i down - x: %i, y: %i", event.tfinger.fingerId,event.tfinger.x,event.tfinger.y);
					break;
				case SDL_MOUSEMOTION:
					if(startDrag)
					{
						int deltax ,deltay;
						deltax = (int)event.button.x - (int)startx;
						deltay = (int)event.button.y - (int)starty;
						rotates(userData,deltax,deltay);
						startx = event.button.x;
						starty = event.button.y;
					}
					SDL_Log("mouse: %i down - x: %i, y: %i", event.button.which,event.button.x,event.button.y);
					break;
				case SDL_FINGERMOTION:
					if(startDrag)
					{
						int deltax ,deltay;
						deltax = (int)event.tfinger.x - (int)startx;
						deltay = (int)event.tfinger.y - (int)starty;
						rotates(userData,deltax,deltay);
						startx = event.tfinger.x;
						starty = event.tfinger.y;
					}
					SDL_Log("Finger: %i,x: %i, y: %i",event.tfinger.fingerId, event.tfinger.x,event.tfinger.y);
					break;
				case SDL_MOUSEBUTTONUP:
					//if(event.button.which==0)
					{
						startDrag = 0;
					}
					SDL_Log("end mouse: %i up - x: %i, y: %i", event.button.which,event.button.x,event.button.y);
					break;
				case SDL_FINGERUP:
					//if(event.tfinger.fingerId==0)
					{
						startDrag = 0;
					}
					break;
			}
			SDLTest_CommonEvent(state, &event, &done);
		}
		if (!done) {
			for (i = 0; i < state->num_windows; ++i) {
				status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
				if (status) {
					SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
					/* Continue for next window */
					continue;
				}
				Draw(&userData[i]);
				SDL_GL_SwapWindow(state->windows[i]);
			}
		}
		wait_per_frame();
	}
	/* Print out some timing information */
	now = SDL_GetTicks();
	if (now > then) {
		SDL_Log("%2.2f frames per second\n",
				((double) frames * 1000) / (now - then));
	}
#if !defined(__ANDROID__)    
	quit(0);
#endif    
	return 0;
}

/**
 *
 Mean过滤 - 用于纹理抗锯齿
// Mean kernel
// 1 1 1
// 1 1 1
// 1 1 1    
const int g_iWeightNumber = 9;
                                    
uniform sampler2D g_FilterTexture;  // 被过滤纹理
uniform float g_aryWeight[g_iWeightNumber]; // Blur权重数组
uniform vec2 g_aryOffset[g_iWeightNumber];  // 横向Blur偏移数组
void main()
{
    vec4 vec4Sum = vec4(0.0);
    if (gl_TexCoord[0].s < 0.495)
    {
        for(int i = 0; i < g_iWeightNumber; ++i)
        {
            vec4Sum += texture2D(g_FilterTexture, gl_TexCoord[0].st + g_aryOffset[i])*g_aryWeight[i];
        }
    }
    else if (gl_TexCoord[0].s > 0.505)
    {
        vec4Sum = texture2D(g_FilterTexture, gl_TexCoord[0].st);
    }
    else
    {
        vec4Sum = vec4(1.0, 0.0, 0.0, 1.0);
    }
    gl_FragColor = vec4Sum;
}
 */
