/**
 *
 gcc matrix.c -I"../SDL2/include/" -lm -D test_matrix && ./a.out
 */
#include "matrix.h"

int esGenCube ( float scale, float **vertices, float **normals, float **texCoords, unsigned **indices )
{/*{{{*/
	int i;
	int numVertices = 24;
	int numIndices = 36;

	float cubeVerts[] =
	{
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f,  0.5f, 0.5f,
		0.5f,  0.5f, 0.5f, 
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
	};

	float cubeNormals[] =
	{
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
	};

	float cubeTex[] =
	{
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
	};

	// Allocate memory for buffers
	if ( vertices != NULL )
	{
		*vertices = malloc ( sizeof(float) * 3 * numVertices );
		memcpy( *vertices, cubeVerts, sizeof( cubeVerts ) );
		for ( i = 0; i < numVertices * 3; i++ )
		{
			(*vertices)[i] *= scale;
		}
	}

	if ( normals != NULL )
	{
		*normals = malloc ( sizeof(float) * 3 * numVertices );
		memcpy( *normals, cubeNormals, sizeof( cubeNormals ) );
	}

	if ( texCoords != NULL )
	{
		*texCoords = malloc ( sizeof(float) * 2 * numVertices );
		memcpy( *texCoords, cubeTex, sizeof( cubeTex ) ) ;
	}


	// Generate the indices
	if ( indices != NULL )
	{
		unsigned cubeIndices[] =
		{
			0, 2, 1,
			0, 3, 2, 
			4, 5, 6,
			4, 6, 7,
			8, 9, 10,
			8, 10, 11, 
			12, 15, 14,
			12, 14, 13, 
			16, 17, 18,
			16, 18, 19, 
			20, 23, 22,
			20, 22, 21
		};

		*indices = malloc ( sizeof(unsigned) * numIndices );
		memcpy( *indices, cubeIndices, sizeof( cubeIndices ) );
	}

	return numIndices;
}/*}}}*/
int esGenSphere ( int numSlices, float radius, float **vertices, float **normals, float **texCoords, unsigned **indices )
{/*{{{*/
	int i;
	int j;
	int numParallels = numSlices / 2;
	int numVertices = ( numParallels + 1 ) * ( numSlices + 1 );
	int numIndices = numParallels * numSlices * 6;
	float angleStep = (2.0f * M_PI) / ((float) numSlices);

	// Allocate memory for buffers
	if ( vertices != NULL )
		*vertices = malloc ( sizeof(float) * 3 * numVertices );

	if ( normals != NULL )
		*normals = malloc ( sizeof(float) * 3 * numVertices );

	if ( texCoords != NULL )
		*texCoords = malloc ( sizeof(float) * 2 * numVertices );

	if ( indices != NULL )
		*indices = malloc ( sizeof(unsigned) * numIndices );

	for ( i = 0; i < numParallels + 1; i++ )
	{
		for ( j = 0; j < numSlices + 1; j++ )
		{
			int vertex = ( i * (numSlices + 1) + j ) * 3; 

			if ( vertices )
			{
				(*vertices)[vertex + 0] = radius * sinf ( angleStep * (float)i ) *
					sinf ( angleStep * (float)j );
				(*vertices)[vertex + 1] = radius * cosf ( angleStep * (float)i );
				(*vertices)[vertex + 2] = radius * sinf ( angleStep * (float)i ) *
					cosf ( angleStep * (float)j );
			}

			if ( normals )
			{
				(*normals)[vertex + 0] = (*vertices)[vertex + 0] / radius;
				(*normals)[vertex + 1] = (*vertices)[vertex + 1] / radius;
				(*normals)[vertex + 2] = (*vertices)[vertex + 2] / radius;
			}

			if ( texCoords )
			{
				int texIndex = ( i * (numSlices + 1) + j ) * 2;
				(*texCoords)[texIndex + 0] = (float) j / (float) numSlices;
				(*texCoords)[texIndex + 1] = -( 1.0f - (float) i ) / (float) (numParallels - 1 );
			}
		}
	}

	// Generate the indices
	if ( indices != NULL )
	{
		unsigned *indexBuf = (*indices);
		for ( i = 0; i < numParallels ; i++ ) 
		{
			for ( j = 0; j < numSlices; j++ )
			{
				*indexBuf++  = i * ( numSlices + 1 ) + j;
				*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + j;
				*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + ( j + 1 );

				*indexBuf++ = i * ( numSlices + 1 ) + j;
				*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + ( j + 1 );
				*indexBuf++ = i * ( numSlices + 1 ) + ( j + 1 );
			}
		}
	}
	return numIndices;
}/*}}}*/

void esMatrixMultiply(Matrix3D *result, Matrix3D *srcA, Matrix3D *srcB)
{/*{{{*/
	Matrix3D    tmp;
	int         i;

	for (i=0; i<4; i++)
	{
		tmp.rawData[i][0] =	(srcA->rawData[i][0] * srcB->rawData[0][0]) +
			(srcA->rawData[i][1] * srcB->rawData[1][0]) +
			(srcA->rawData[i][2] * srcB->rawData[2][0]) +
			(srcA->rawData[i][3] * srcB->rawData[3][0]) ;

		tmp.rawData[i][1] =	(srcA->rawData[i][0] * srcB->rawData[0][1]) + 
			(srcA->rawData[i][1] * srcB->rawData[1][1]) +
			(srcA->rawData[i][2] * srcB->rawData[2][1]) +
			(srcA->rawData[i][3] * srcB->rawData[3][1]) ;

		tmp.rawData[i][2] =	(srcA->rawData[i][0] * srcB->rawData[0][2]) + 
			(srcA->rawData[i][1] * srcB->rawData[1][2]) +
			(srcA->rawData[i][2] * srcB->rawData[2][2]) +
			(srcA->rawData[i][3] * srcB->rawData[3][2]) ;

		tmp.rawData[i][3] =	(srcA->rawData[i][0] * srcB->rawData[0][3]) + 
			(srcA->rawData[i][1] * srcB->rawData[1][3]) +
			(srcA->rawData[i][2] * srcB->rawData[2][3]) +
			(srcA->rawData[i][3] * srcB->rawData[3][3]) ;
	}
	memcpy(result, &tmp, sizeof(Matrix3D));
}/*}}}*/

void esFrustum(Matrix3D *result, float left, float right, float bottom, float top, float nearZ, float farZ)
{/*{{{*/
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farZ - nearZ;
	Matrix3D    frust;

	if ( (nearZ <= 0.0f) || (farZ <= 0.0f) ||
			(deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f) )
		return;

	frust.rawData[0][0] = 2.0f * nearZ / deltaX;
	frust.rawData[0][1] = frust.rawData[0][2] = frust.rawData[0][3] = 0.0f;

	frust.rawData[1][1] = 2.0f * nearZ / deltaY;
	frust.rawData[1][0] = frust.rawData[1][2] = frust.rawData[1][3] = 0.0f;

	frust.rawData[2][0] = (right + left) / deltaX;
	frust.rawData[2][1] = (top + bottom) / deltaY;
	frust.rawData[2][2] = -(nearZ + farZ) / deltaZ;
	frust.rawData[2][3] = -1.0f;

	frust.rawData[3][2] = -2.0f * nearZ * farZ / deltaZ;
	frust.rawData[3][0] = frust.rawData[3][1] = frust.rawData[3][3] = 0.0f;

	esMatrixMultiply(result, &frust, result);
}/*}}}*/
void esPerspective(Matrix3D *result, float fovy, float aspect, float nearZ, float farZ)
{/*{{{*/
	float frustumW, frustumH;

	frustumH = tanf( fovy / 360.0f * M_PI ) * nearZ;
	frustumW = frustumH * aspect;

	esFrustum( result, -frustumW, frustumW, -frustumH, frustumH, nearZ, farZ );
}/*}}}*/

void Matrix_identity(Matrix3D *result)
{/*{{{*/
	memset(result, 0x0, sizeof(Matrix3D));
	result->rawData[0][0] = 1.0f;
	result->rawData[1][1] = 1.0f;
	result->rawData[2][2] = 1.0f;
	result->rawData[3][3] = 1.0f;
}/*}}}*/

void esScale(Matrix3D *result, float sx, float sy, float sz)
{/*{{{*/
	result->rawData[0][0] *= sx;
	result->rawData[0][1] *= sx;
	result->rawData[0][2] *= sx;
	result->rawData[0][3] *= sx;

	result->rawData[1][0] *= sy;
	result->rawData[1][1] *= sy;
	result->rawData[1][2] *= sy;
	result->rawData[1][3] *= sy;

	result->rawData[2][0] *= sz;
	result->rawData[2][1] *= sz;
	result->rawData[2][2] *= sz;
	result->rawData[2][3] *= sz;
}/*}}}*/



void esTranslate(Matrix3D *result, float tx, float ty, float tz)
{/*{{{*/
	result->rawData[3][0] += (result->rawData[0][0] * tx + result->rawData[1][0] * ty + result->rawData[2][0] * tz);
	result->rawData[3][1] += (result->rawData[0][1] * tx + result->rawData[1][1] * ty + result->rawData[2][1] * tz);
	result->rawData[3][2] += (result->rawData[0][2] * tx + result->rawData[1][2] * ty + result->rawData[2][2] * tz);
	result->rawData[3][3] += (result->rawData[0][3] * tx + result->rawData[1][3] * ty + result->rawData[2][3] * tz);
}/*}}}*/

void esRotate(Matrix3D *result, float angle, float x, float y, float z)
{/*{{{*/
	float sinAngle, cosAngle;
	float mag = sqrtf(x * x + y * y + z * z);

	sinAngle = sinf ( angle * M_PI / 180.0f );
	cosAngle = cosf ( angle * M_PI / 180.0f );
	if ( mag > 0.0f )
	{
		float xx, yy, zz, xy, yz, zx, xs, ys, zs;
		float oneMinusCos;
		Matrix3D rotMat;

		x /= mag;
		y /= mag;
		z /= mag;

		xx = x * x;
		yy = y * y;
		zz = z * z;
		xy = x * y;
		yz = y * z;
		zx = z * x;
		xs = x * sinAngle;
		ys = y * sinAngle;
		zs = z * sinAngle;
		oneMinusCos = 1.0f - cosAngle;

		rotMat.rawData[0][0] = (oneMinusCos * xx) + cosAngle;
		rotMat.rawData[0][1] = (oneMinusCos * xy) - zs;
		rotMat.rawData[0][2] = (oneMinusCos * zx) + ys;
		rotMat.rawData[0][3] = 0.0F; 

		rotMat.rawData[1][0] = (oneMinusCos * xy) + zs;
		rotMat.rawData[1][1] = (oneMinusCos * yy) + cosAngle;
		rotMat.rawData[1][2] = (oneMinusCos * yz) - xs;
		rotMat.rawData[1][3] = 0.0F;

		rotMat.rawData[2][0] = (oneMinusCos * zx) - ys;
		rotMat.rawData[2][1] = (oneMinusCos * yz) + xs;
		rotMat.rawData[2][2] = (oneMinusCos * zz) + cosAngle;
		rotMat.rawData[2][3] = 0.0F; 

		rotMat.rawData[3][0] = 0.0F;
		rotMat.rawData[3][1] = 0.0F;
		rotMat.rawData[3][2] = 0.0F;
		rotMat.rawData[3][3] = 1.0F;

		esMatrixMultiply( result, &rotMat, result );
	}
}/*}}}*/


void esOrtho(Matrix3D *result, float left, float right, float bottom, float top, float nearZ, float farZ)
{/*{{{*/
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farZ - nearZ;
	Matrix3D    ortho;

	if ( (deltaX == 0.0f) || (deltaY == 0.0f) || (deltaZ == 0.0f) )
		return;

	Matrix_identity(&ortho);
	ortho.rawData[0][0] = 2.0f / deltaX;
	ortho.rawData[3][0] = -(right + left) / deltaX;
	ortho.rawData[1][1] = 2.0f / deltaY;
	ortho.rawData[3][1] = -(top + bottom) / deltaY;
	ortho.rawData[2][2] = -2.0f / deltaZ;
	ortho.rawData[3][2] = -(nearZ + farZ) / deltaZ;

	esMatrixMultiply(result, &ortho, result);
}/*}}}*/

/*
   void esMatrixXvector(Vector3D* result,Matrix3D * rawData , Vector3D * position)
   {
   position->position[3] = 1.0;
   int i = 0;
   for(i=0;i<4;++i)
   result->position[i] = position->position[0]*rawData->rawData[i][0] + position->position[1]*rawData->rawData[i][1] + position->position[2]*rawData->rawData[i][2] + position->position[3]*rawData->rawData[i][3];
   }
   */
void esVectorXmatrix(Vector3D* result, Vector3D * position, Matrix3D * m)
{
	//position->position[3] = 1.0;
	int i = 0;
	for(i=0;i<4;++i)
		result->position[i] = position->position[0]*m->rawData[0][i] + position->position[1]*m->rawData[1][i] + position->position[2]*m->rawData[2][i] + 1.0*m->rawData[3][i] ;
}

void Matrix_transformVector(Matrix3D * rawData, Vector3D * position, Vector3D * result)
{
	esVectorXmatrix(result,position,rawData);
}

/** 
 * 计算矩阵src的模 
 */ 
float Matrix_determinant( Matrix3D * src, int n )  
{  
	int i,j,k,x,y;  
	Matrix3D tmp;  
	float result = 0.0;  
	double t;

	if( n == 1 ) {
		return src->rawData[0][0];  
	}  

	for( i = 0; i < n; ++i )  
	{  
		for( j = 0; j < n - 1; ++j )  
		{  
			for( k = 0; k < n - 1; ++k )  
			{  
				x = j + 1;  
				y = k >= i ? k + 1 : k;  

				tmp.rawData[j][k] = src->rawData[x][y];  
			}  
		}  

		t = Matrix_determinant( &tmp, n - 1 );  

		if( i % 2 == 0 )  
		{  
			result += src->rawData[0][i] * t;  
		}  
		else 
		{  
			result -= src->rawData[0][i] * t;  
		}  
	}  
	return result;  
}  

/** 
 * 计算伴随矩阵 
 */ 
void calculate_A_adjoint( Matrix3D * src, Matrix3D * dst, int n )  
{  
	int i, j, k, t, x, y;  
	Matrix3D tmp;  

	if( n == 1 )  
	{  
		dst->rawData[0][0] = 1;  
		return;  
	}  

	for( i = 0; i < n; ++i )  
	{  
		for( j = 0; j < n; ++j )  
		{  
			for( k = 0; k < n - 1; ++k )  
			{  
				for( t = 0; t < n - 1; ++t )  
				{  
					x = k >= i ? k + 1 : k ;  
					y = t >= j ? t + 1 : t;  

					tmp.rawData[k][t] = src->rawData[x][y];  
				}  
			}  

			dst->rawData[j][i]  =  Matrix_determinant( &tmp, n - 1 );  

			if( ( i + j ) % 2 == 1 )  
			{  
				dst->rawData[j][i] = -1*dst->rawData[j][i];  
			}  
		}  
	}  
}  

/** 
 * 得到逆矩阵 
 */ 
int calculate_A_invert( Matrix3D * src, Matrix3D * dst, int n )  
{  
	double A = Matrix_determinant( src, n );  
	Matrix3D tmp;  
	int i, j;  

	if ( fabs( A - 0 ) <= .000000001 )  
	{  
		printf("不可能有逆矩阵！\n");  
		return 0;  
	}  

	calculate_A_adjoint( src, &tmp, n );    

	for( i = 0; i < n; ++i )    
	{    
		for( j = 0; j < n; ++j )    
		{    
			dst->rawData[i][j] = (double)( tmp.rawData[i][j] / A );  
		}    
	}  

	return 1;  
}  
void Matrix_transpose(Matrix3D* m,Matrix3D *result)// 将当前 Matrix3D 对象转换为一个矩阵，并将互换其中的行和列。
{
	if(m)
	{
		int i=0,j=0;
		float tmp;
		for(;i<4;i++)
		{
			for(j=0;i<4;i++)
			{
				tmp = m->rawData[i][j];
				m->rawData[i][j] =  m->rawData[j][i];
				m->rawData[j][i] =  tmp;
			}
		}
	}
}

void Matrix_prependRotation(Matrix3D * m,float degrees, float rotationX,float rotationY,float rotationZ)// 在 Matrix3D 对象上后置一个增量旋转。
{
	esRotate(m,-degrees,rotationX,rotationY,rotationZ);
}
void Matrix_appendRotation(Matrix3D * m,float degrees, float rotationX,float rotationY,float rotationZ)// 在 Matrix3D 对象上后置一个增量旋转。
{
	esRotate(m,degrees,rotationX,rotationY,rotationZ);
}
void Matrix_prependTranslation(Matrix3D *m ,int x, int y,int z)// 在 Matrix3D 对象上后置一个增量平移，沿 x、y 和 z 轴重新定位。 
{
	esTranslate(m,x,y,z);
}
void Matrix_appendTranslation(Matrix3D *m ,int x, int y,int z)// 在 Matrix3D 对象上后置一个增量平移，沿 x、y 和 z 轴重新定位。 
{
	esTranslate(m,-x,-y,-z);
}
void Matrix_prependScale(Matrix3D * m,float xScale, float yScale,float zScale)// 在 Matrix3D 对象上后置一个增量缩放，沿 x、y 和 z 轴改变位置。 
{
	esTranslate(m,1.0/xScale,1.0/yScale,1.0/zScale);
}
void Matrix_appendScale(Matrix3D * m,float xScale, float yScale,float zScale)// 在 Matrix3D 对象上后置一个增量缩放，沿 x、y 和 z 轴改变位置。
{
	esTranslate(m,xScale,yScale,zScale);
}

void Matrix_append(Matrix3D* m, Matrix3D * right,Matrix3D * result)// 左乘一个矩阵。 
{
	esMatrixMultiply(result,m,right);
}
void Matrix_prepend(Matrix3D* m, Matrix3D * left,Matrix3D * result)// 右乘一个矩阵。
{
	esMatrixMultiply(result,left,m);
}

int Matrix_invert(Matrix3D * m,Matrix3D * result)
{
	int r= calculate_A_invert( m, result, 4);  
	if(r) {
		printf("get invert success!\n");
	}else{
		printf("can not invert !\n");
	}
	return r;
}

void Vector3DxNumber(Vector3D * result,Vector3D * position,float f)
{
	result->position[0] = position->position[0]*f;
	result->position[1] = position->position[1]*f;
	result->position[2] = position->position[2]*f;
}
void Vector3DMinus(Vector3D * r,Vector3D * v1,Vector3D * v2)
{
	r->position[0] = v1->position[0] - v2->position[0];
	r->position[1] = v1->position[1] - v2->position[1];
	r->position[2] = v1->position[2] - v2->position[2];
}
void crossProduct(Vector3D* result,Vector3D * v1,Vector3D * v2)
{
	result->position[0]= v1->position[1]*v2->position[2] - v2->position[1]*v1->position[2];
	result->position[1]= v1->position[2]*v2->position[0] - v2->position[2]*v1->position[0];
	result->position[2]= v1->position[0]*v2->position[1] - v2->position[0]*v1->position[1];
}
float dotProduct(Vector3D * v1 , Vector3D * v2)
{
	return v1->position[0]*v2->position[0] + v1->position[1]*v2->position[1] +  v1->position[2]*v2->position[2];// +  v1->position[3]*v2->position[4];
}
float esLength(Vector3D * position)
{
	return sqrtf(dotProduct(position,position));
}
float esDistance(Vector3D * v1,Vector3D * v2)
{
	Vector3D r;
	Vector3DMinus(&r,v2,v1);
	return esLength(&r);
}
void normalize(Vector3D*result,Vector3D* position)
{
	//Vector3D * result = malloc(sizeof(float)*4);
	Vector3DxNumber(result,position,1.0/esLength(position));
	//return result;
}
void esFaceward(Vector3D*result,Vector3D * normal,Vector3D *in,Vector3D * nref)
{
	//Vector3D * result = malloc(sizeof(float)*4);
	int f = -1;
	if(dotProduct(nref,in)<0)
	{
		f=1;
	}
	Vector3DxNumber(result,normal,f);
	//return result;
}
void esReflect(Vector3D*result,Vector3D* in,Vector3D * normal)
{
	//return in - 2*dot(normal,in)*normal;
	//Vector3D * result = malloc(sizeof(float)*4);
	Vector3DxNumber(result,normal,dotProduct(normal,in)*2);
	Vector3DMinus(result,in,result);
}
void esRefract(Vector3D*result,Vector3D* in,Vector3D * normal,float eta)
{
	float k = 1.0 - eta*eta*(1.0-pow(dotProduct(normal,in),2));
	if(k<0){
		memset(result,0,4*sizeof(float));
	}else{
		Vector3DxNumber(result,in,eta);
		Vector3D position;
		Vector3DxNumber(&position,normal,(eta*dotProduct(normal,in)+sqrtf(k)));
		Vector3DMinus(result,result,&position);
	}
}


void Matrix_lookAt(Matrix3D *matrix, Vector3D* pos,Vector3D * dir,Vector3D* up){
	Vector3D dirN;
	Vector3D upN;
	Vector3D lftN;
	float raw[16];


	crossProduct(&lftN,dir,up);
	normalize(&lftN,&lftN);

	crossProduct(&upN,&lftN,dir);
	normalize(&upN,&upN);
	memcpy(&dirN,dir,sizeof(Vector3D));
	normalize(&dirN,&dirN);

	raw[0] = lftN.position[0];
	raw[1] = upN.position[0];
	raw[2] = -dirN.position[0];
	raw[3] = 0.0;

	raw[4] = lftN.position[1];
	raw[5] = upN.position[1];
	raw[6] = -dirN.position[1];
	raw[7] = 0.0;

	raw[8] = lftN.position[2];
	raw[9] = upN.position[2];
	raw[10] = -dirN.position[2];
	raw[11] = 0.0;

	raw[12] = -dotProduct(&lftN,pos);
	raw[13] = -dotProduct(&upN,pos);
	raw[14] = dotProduct(&dirN,pos);
	raw[15] = 1.0;

	memcpy(matrix->rawData,raw,sizeof(raw));
}

typedef struct Plane3D{
	float a;
	float b;
	float c;
	float d;
}Plane3D;
void reflection(Plane3D * plane,Matrix3D * target){
	float a = plane->a, b= plane->b, c= plane->c, d= plane->d;
	float rawData[16];
	float ab2 = -2 * a * b;
	float ac2 = -2 * a * c;
	float bc2 = -2 * b * c;
	// reflection matrix
	rawData[0] = 1 - 2 * a * a;
	rawData[4] = ab2;
	rawData[8] = ac2;
	rawData[12] = -2 * a * d;
	rawData[1] = ab2;
	rawData[5] = 1 - 2 * b * b;
	rawData[9] = bc2;
	rawData[13] = -2 * b * d;
	rawData[2] = ac2;
	rawData[6] = bc2;
	rawData[10] = 1 - 2 * c * c;
	rawData[14] = -2 * c * d;
	rawData[3] = 0;
	rawData[7] = 0;
	rawData[11] = 0;
	rawData[15] = 1;
	memcpy(target,rawData,sizeof(Matrix3D));
}

void decompose(Matrix3D * sourceMatrix,int orientationStyle,Matrix3D * result){
	float raw[16];
	memcpy(raw,sourceMatrix,sizeof(Matrix3D));

	float a = raw[0];
	float e = raw[1];
	float i = raw[2];
	float b = raw[4];
	float f = raw[5];
	float j = raw[6];
	float c = raw[8];
	float g = raw[9];
	float k = raw[10];

	float x = raw[12];
	float y = raw[13];
	float z = raw[14];

	float tx = sqrt(a * a + e * e + i * i);
	float ty = sqrt(b * b + f * f + j * j);
	float tz = sqrt(c * c + g * g + k * k);
	float tw = 0;

	float scaleX = tx;
	float scaleY = ty;
	float scaleZ = tz;

	if (a*(f*k - j*g) - e*(b*k - j*c) + i*(b*g - f*c) < 0) {
		scaleZ = -scaleZ;
	}

	a = a / scaleX;
	e = e / scaleX;
	i = i / scaleX;
	b = b / scaleY;
	f = f / scaleY;
	j = j / scaleY;
	c = c / scaleZ;
	g = g / scaleZ;
	k = k / scaleZ;

	//from away3d-ts
	int EULER_ANGLES = 1;
	int AXIS_ANGLE = 2;
	//int QUATERNION = 3;
	if (orientationStyle == EULER_ANGLES) {
		tx = atan2(j, k);
		ty = atan2(-i, sqrt(a * a + e * e));
		float s1 = sin(tx);
		float c1 = cos(tx);
		tz = atan2(s1*c-c1*b, c1*f - s1*g);
	} else if (orientationStyle == AXIS_ANGLE) {
		tw = acos((a + f + k - 1) / 2);
		float len = sqrt((j - g) * (j - g) + (c - i) * (c - i) + (e - b) * (e - b));
		tx = (j - g) / len;
		ty = (c - i) / len;
		tz = (e - b) / len;
	} else {//QUATERNION
		float tr = a + f + k;
		if (tr > 0) {
			tw = sqrt(1 + tr) / 2;
			tx = (j - g) / (4 * tw);
			ty = (c - i) / (4 * tw);
			tz = (e - b) / (4 * tw);
		} else if ((a > f) && (a > k)) {
			tx = sqrt(1 + a - f - k) / 2;
			tw = (j - g) / (4 * tx);
			ty = (e + b) / (4 * tx);
			tz = (c + i) / (4 * tx);
		} else if (f > k) {
			ty = sqrt(1 + f - a - k) / 2;
			tx = (e + b) / (4 * ty);
			tw = (c - i) / (4 * ty);
			tz = (j + g) / (4 * ty);
		} else {
			tz = sqrt(1 + k - a - f) / 2;
			tx = (c + i) / (4 * tz);
			ty = (j + g) / (4 * tz);
			tw = (e - b) / (4 * tz);
		}
	}

	memset(result,0,sizeof(*result));
	result->rawData[0][0] = x;
	result->rawData[0][1] = y;
	result->rawData[0][2] = z;
	result->rawData[1][0] = tx;
	result->rawData[1][1] = ty;
	result->rawData[1][2] = tz;
	result->rawData[1][3] = tw;
	result->rawData[2][0] = scaleX;
	result->rawData[2][1] = scaleY;
	result->rawData[2][2] = scaleZ;
}

void transformVector(Matrix3D * matrix,Vector3D * vector,Vector3D * result){
	float raw[16];
	memcpy(raw,matrix,sizeof(Matrix3D));
	float a = raw[0];
	float e = raw[1];
	float i = raw[2];
	float m = raw[3];
	float b = raw[4];
	float f = raw[5];
	float j = raw[6];
	float n = raw[7];
	float c = raw[8];
	float g = raw[9];
	float k = raw[10];
	float o = raw[11];
	float d = raw[12];
	float h = raw[13];
	float l = raw[14];
	float p = raw[15];

	float x = vector->position[0];
	float y = vector->position[1];
	float z = vector->position[2];
	result->position[0] = a * x + b * y + c * z + d;
	result->position[1] = e * x + f * y + g * z + h;
	result->position[2] = i * x + j * y + k * z + l;
	result->position[3] = m * x + n * y + o * z + p;
}

void deltaTransformVector(Matrix3D * matrix,Vector3D * vector,Vector3D * result){
	float raw[16];
	memcpy(raw,matrix,sizeof(Matrix3D));
	float a = raw[0];
	float e = raw[1];
	float i = raw[2];
	float m = raw[3];
	float b = raw[4];
	float f = raw[5];
	float j = raw[6];
	float n = raw[7];
	float c = raw[8];
	float g = raw[9];
	float k = raw[10];
	float o = raw[11];
	float x = vector->position[0];
	float y = vector->position[1];
	float z = vector->position[2];
	result->position[0] = a * x + b * y + c * z;
	result->position[1] = e * x + f * y + g * z;
	result->position[2] = i * x + j * y + k * z;
	result->position[3] = m * x + n * y + o * z;
}

void getTranslation(Matrix3D *transform,Vector3D* result){
	int i;
	for(i=0;i<4;++i)
		result->position[i] = transform->rawData[i][3];
	//transform.copyColumnTo(3, result);
}

void deltaTransformVectors(Matrix3D * matrix,float * vin,float * vout){
	float raw[16];
	memcpy(raw,matrix,sizeof(Matrix3D));
	float a = raw[0];
	float e = raw[1];
	float i = raw[2];
	//float m = raw[3];
	float b = raw[4];
	float f = raw[5];
	float j = raw[6];
	//float n = raw[7];
	float c = raw[8];
	float g = raw[9];
	float k = raw[10];
	//float o = raw[11];
	int outIndex= 0;
	int length = 16;
	int index = 0;
	for(index= 0; index<length; index+=3) {
		float x = vin[index];
		float y = vin[index+1];
		float z = vin[index+2];
		vout[outIndex++] = a * x + b * y + c * z;
		vout[outIndex++] = e * x + f * y + g * z;
		vout[outIndex++] = i * x + j * y + k * z;
	}
}



/*

   方向						平移
   scaleX	0		0		tx
   0		scaleY	0		ty
   0		0		scaleZ	tz
   0		0		0		tw

identity:
1	0	0	0
0	1	0	0
0	0	1	0
0	0	0	1


2d:
a	b	tx
c	d	ty
u	position	w


a	b	tx
c	d	ty
0	0	w

translate:
1	0	tx
0	1	ty
0	0	1

scale:
sx	0	tx
0	sy	ty
0	0	1

rotation(q):
cos(q)	sin(q)	0
-sin(q)	cos(q)	0
0		0		1

倾斜:
0			tan(skewY)	0
tan(skewX)	0			0
0			0			1

*/






#ifdef test_matrix
void printfV(Vector3D * position)
{
	printf("%f,\t%f,\t%f"
			,position->position[0]
			,position->position[1]
			,position->position[2]
		  );
}
void printfM(Matrix3D * m)
{
	int i = 0;
	for(;i<4;i++){
		printfV((Vector3D*)m + i);
		printf(",\t%f\n",(float)*((float*)((Vector3D*)m+i)+3));
	}
	printf("\n");
}

int main()
{
	Matrix3D m;
	Matrix_identity(&m);
	esTranslate(&m,1.0,2.0,3.0);
	esScale(&m,2.0,3.0,4.0);
	esRotate(&m,60,2.0,3.0,4.0);
	printfM(&m);


	Matrix3D rm;
	Matrix_invert(&m,&rm);
	printfM(&rm);
	printf("\n");
	Matrix_invert(&rm,&rm);
	printfM(&rm);
	printf("\n");

	Vector3D v;
	memset(&v,0,sizeof(Vector3D));
	v.position[0]= 0;
	v.position[1]= 0;
	v.position[2]= 0;
	printfV(&v);
	printf("\n");

	Vector3D r;
	Matrix_transformVector(&m,&v,&r);
	printfV(&r);
	printf("\n");

	Matrix_identity(&m);
	esRotate(&m,-60,2.0,3.0,4.0);
	//esRotate(&m,60,-2.0,-3.0,-4.0);
	esScale(&m,1/2.0,1/3.0,1/4.0);
	esTranslate(&m,-1.0,-2.0,-3.0);
	printfM(&m);

	Vector3D r2;
	//esMatrixXvector(&r2,&m,&r);
	Matrix_transformVector(&m,&r,&r2);
	printfV(&r2);
	printf("\n");


	return 0;

}
#endif
