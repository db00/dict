/**
 *
 gcc matrix.c -I"../SDL2/include/" -lm -D test_matrix && ./a.out
 */
#ifndef matrix_h
#define matrix_h

#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
//#include "SDL_opengles2.h"
#include "SDL.h"

#ifndef min
#define min(x,y) ((x)>(y))?(y):(x)
#endif
#ifndef max
#define max(x,y) ((x)<(y))?(y):(x)
#endif

typedef struct {
	float   rawData[4][4]; //float rawData[16];// 一个由 16 个数字组成的矢量，其中，每四个元素可以是 4x4 矩阵的一行或一列。 
} Matrix3D;

typedef struct {
	float position[4];// position : Vector3D// 一个保存显示对象在转换参照帧中的 3D 坐标 (x,y,z) 位置的 Vector3D 对象。 
} Vector3D;


void esScale(Matrix3D *result,float scaleX,float scaleY,float scaleZ);
void esPerspective(Matrix3D *result, float fovy, float aspect, float nearZ, float farZ);
void esFrustum(Matrix3D *result, float left, float right, float bottom, float top, float nearZ, float farZ);
void esRotate(Matrix3D *result, float angle, float x, float y, float z);
void esTranslate(Matrix3D *result, float tx, float ty, float tz);
void esMatrixMultiply(Matrix3D *result, Matrix3D *srcA, Matrix3D *srcB);
void esOrtho(Matrix3D *result, float left, float right, float bottom, float top, float nearZ, float farZ);
void esVectorXmatrix(Vector3D* result, Vector3D * v, Matrix3D * m);

/*
   void Matrix_pointAt(Matrix3D* m,Vector3D * position, Vector3D * at, Vector3D * up);// 旋转显示对象以使其朝向指定的位置。
   */

void Matrix_lookAt(Matrix3D *matrix, Vector3D* pos,Vector3D * dir,Vector3D* up);// ???????。
void Matrix_append(Matrix3D* m, Matrix3D * right,Matrix3D * result);// 左乘一个矩阵。 
void Matrix_prepend(Matrix3D* m, Matrix3D * left,Matrix3D * result);// 右乘一个矩阵。
void Matrix_prependRotation(Matrix3D * m,float degrees, float rotationX,float rotationY,float rotationZ);// 在 Matrix3D 对象上后置一个增量旋转。
void Matrix_appendRotation(Matrix3D * m,float degrees, float rotationX,float rotationY,float rotationZ);// 在 Matrix3D 对象上后置一个增量旋转。
void Matrix_prependTranslation(Matrix3D *m ,int x, int y,int z);// 在 Matrix3D 对象上后置一个增量平移，沿 x、y 和 z 轴重新定位。 
void Matrix_appendTranslation(Matrix3D *m ,int x, int y,int z);// 在 Matrix3D 对象上后置一个增量平移，沿 x、y 和 z 轴重新定位。 
void Matrix_prependScale(Matrix3D * m,float xScale, float yScale,float zScale);// 在 Matrix3D 对象上后置一个增量缩放，沿 x、y 和 z 轴改变位置。 
void Matrix_appendScale(Matrix3D * m,float xScale, float yScale,float zScale);// 在 Matrix3D 对象上后置一个增量缩放，沿 x、y 和 z 轴改变位置。

void Matrix_transpose(Matrix3D* m,Matrix3D *result);// 将当前 Matrix3D 对象转换为一个矩阵，并将互换其中的行和列。
void Matrix_transformVector(Matrix3D * m, Vector3D * position, Vector3D * result);// 使用转换矩阵将 Vector3D 对象从一个空间坐标转换到另一个空间坐标。 
int Matrix_invert(Matrix3D * m,Matrix3D * result);//逆矩阵
float Matrix_determinant(Matrix3D *m,int n); //float determinant;// [只读] 一个用于确定矩阵是否可逆的数字。矩阵的模 n=4;
void Matrix_identity(Matrix3D *m); //identity();// 将当前矩阵转换为恒等或单位矩阵

int esGenSphere ( int numSlices, float radius, float **vertices, float **normals, float **texCoords, unsigned **indices );
int esGenCube ( float scale, float **vertices, float **normals, float **texCoords, unsigned **indices );

#endif
