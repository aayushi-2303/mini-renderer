/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"
#include	<string>
#include	<iostream>
using namespace std;

#define PI (float) 3.14159265358979323846

class DDA {
public:
	GzCoord start;
	GzCoord end;
	GzCoord current;
	float slopeX;
	float slopeZ;
	bool left;

public:
	DDA() {
	}

	DDA(GzCoord v1, GzCoord v2) {
		start[X] = v1[X];
		start[Y] = v1[Y];
		start[Z] = v1[Z];

		end[X] = v2[X];
		end[Y] = v2[Y];
		end[Z] = v2[Z];

		current[X] = v1[X];
		current[Y] = v1[Y];
		current[Z] = v1[Z];

		slopeX = (v2[X] - v1[X]) / (v2[Y] - v1[Y]);
		slopeZ = (v2[Z] - v1[Z]) / (v2[Y] - v1[Y]);
		left = false;
	}

	void AdvanceCurrent(float deltaY) {
		current[X] += (slopeX * deltaY);
		current[Y] += deltaY;
		current[Z] += (slopeZ * deltaY);
	}
};

class Span {
public:
	GzCoord start;
	GzCoord end;
	GzCoord current;
	float slopeZ;

public:
	Span() {
	}

	Span(DDA leftDDA, DDA rightDDA) {
		start[X] = leftDDA.current[X];
		start[Y] = leftDDA.current[Y];
		start[Z] = leftDDA.current[Z];
		end[X] = rightDDA.current[X];
		end[Y] = rightDDA.current[Y];
		end[Z] = rightDDA.current[Z];
		current[X] = leftDDA.current[X];
		current[Y] = leftDDA.current[Y];
		current[Z] = leftDDA.current[Z];
		slopeZ = (end[Z] - start[Z]) / (end[X] - start[X]);
	}

	void AdvanceCurrent(float deltaX) {
		current[X] += deltaX;
		current[Z] += (deltaX * slopeZ);
	}
};

typedef float GzUV[2];
DDA ddas[3];
GzCoord* triCoords;
GzCoord* norms;
GzUV* uvs;
GzUV* p_UVs;
int SetDDAs(GzCoord* triCoords);
int SortVertices(GzCoord* triCoords);
void FillSpan(Span* span, GzRender* r, GzColor* c);
void RemoveScale(GzMatrix mat);
void RemoveTranslation(GzMatrix mat);
void Rasterize(GzRender* r);
void EvaluateLighting(GzRender* r, GzCoord point, GzCoord N, GzColor C);
void GetInterpolatedColor(float abcd[3][4], GzColor out, int x, int y);
void GetInterpolatedNorm(float abcd[3][4], GzCoord out, int x, int y);
void GetInterpolatedUV(float abcd[2][4], GzUV out, int x, int y);
void GetABCD(GzCoord in[], GzColor colors[], float out[3][4]);
void GetNormalABCD(float out[3][4]);
void GetUVABCD(GzCoord in[], float out[3][4]);
int CLAMP(int color);
void ENABLEDEBUG();
void SELECTIONSORT(GzCoord* coords, int n);
void NORMALIZE(GzCoord vec, GzCoord normVect);
void printVertices(float* tri);
void MATMULT(GzMatrix A, GzMatrix B, GzMatrix res);
void CROSS(GzCoord A, GzCoord B, GzCoord res);
void SUBTRACT(GzCoord A, GzCoord B, GzCoord res);
void SCALARMULT(float scalar, GzCoord vec, GzCoord res);
float DOT(GzCoord A, GzCoord B);
void NORMALIZE(GzCoord vec, GzCoord normVect);
void printMatrix(GzMatrix mat);
void MATCOPY(GzMatrix to, GzMatrix from);
void VERTEXTRANSFORM(GzMatrix transform, float verts[], float res[]);
void IDENTITY(GzMatrix mat);

int GzRender::GzRotXMat(float degree, GzMatrix mat)
{
/* HW 3.1
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
*/
	float theta = degree * (PI / 180.00);
	mat[1][1] = cos(theta);
	mat[1][2] = -1 * sin(theta);
	mat[2][1] = sin(theta);
	mat[2][2] = cos(theta);
	return GZ_SUCCESS;
}

int GzRender::GzRotYMat(float degree, GzMatrix mat)
{
/* HW 3.2
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value
*/
	float theta = degree * (PI / 180.00);
	mat[0][0] = cos(theta);
	mat[0][2] = sin(theta);
	mat[2][0] = -1 * sin(theta);
	mat[2][2] = cos(theta);
	return GZ_SUCCESS;
}

int GzRender::GzRotZMat(float degree, GzMatrix mat)
{
/* HW 3.3
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value
*/
	float theta = degree * (PI / 180.00);
	mat[0][0] = cos(theta);
	mat[0][1] = -1 * sin(theta);
	mat[1][0] = sin(theta);
	mat[1][1] = cos(theta);
	return GZ_SUCCESS;
}

int GzRender::GzTrxMat(GzCoord translate, GzMatrix mat)
{
/* HW 3.4
// Create translation matrix
// Pass back the matrix using mat value
*/
	mat[0][3] = translate[X];
	mat[1][3] = translate[Y];
	mat[2][3] = translate[Z];
	return GZ_SUCCESS;
}


int GzRender::GzScaleMat(GzCoord scale, GzMatrix mat)
{
/* HW 3.5
// Create scaling matrix
// Pass back the matrix using mat value
*/
	mat[0][0] = scale[X];
	mat[1][1] = scale[Y];
	mat[2][2] = scale[Z];
	return GZ_SUCCESS;
}


GzRender::GzRender(int xRes, int yRes)
{
/* HW1.1 create a framebuffer for MS Windows display:
 -- set display resolution
 -- allocate memory for framebuffer : 3 bytes(b, g, r) x width x height
 -- allocate memory for pixel buffer
 */
	if (xRes > MAXXRES)
		xRes = MAXXRES;
	if (yRes > MAXYRES)
		yRes = MAXYRES;

	this->xres = xRes;
	this->yres = yRes;

	this->pixelbuffer = new GzPixel[xRes * yRes];
	this->framebuffer = new char[3 * xRes * yRes];
	this->xshift = new float;
	this->yshift = new float;
	this->weight = new float;

/* HW 3.6
- setup Xsp and anything only done once 
- init default camera 
*/ 
	IDENTITY(Xsp);
	this->Xsp[0][0] = xres / 2.0;
	this->Xsp[1][1] = -1 * yres / 2.0;
	this->Xsp[0][3] = xres / 2.0;
	this->Xsp[1][3] = yres / 2.0;
	this->Xsp[2][2] = MAXINT;

	//Default camera settings
	m_camera.FOV = DEFAULT_FOV;
	m_camera.position[X] = DEFAULT_IM_X;
	m_camera.position[Y] = DEFAULT_IM_Y;
	m_camera.position[Z] = DEFAULT_IM_Z;
	m_camera.lookat[X] = 0.0;
	m_camera.lookat[Y] = 0.0;
	m_camera.lookat[Z] = 0.0;
	m_camera.worldup[X] = 0.0f;
	m_camera.worldup[Y] = 1.0f;
	m_camera.worldup[Z] = 0.0f;

	/*HW 4
	- Initialize light count
	*/
	numlights = 0;
}

GzRender::~GzRender()
{
/* HW1.2 clean up, free buffer memory */
	if (pixelbuffer != NULL)
		delete this->pixelbuffer;
	if (framebuffer != NULL)
		delete this->framebuffer;
}

int GzRender::GzDefault()
{
/* HW1.3 set pixel buffer to some default values - start a new frame */
	int n = xres * yres;

	for (int i = 0; i < n; i++) {
		pixelbuffer[i].alpha = 4095;
		pixelbuffer[i].blue = 1500;
		pixelbuffer[i].green = 1040;
		pixelbuffer[i].red = 750;
		pixelbuffer[i].z = MAXINT;
	}
	return GZ_SUCCESS;
}

int GzRender::GzBeginRender()
{
/* HW 3.7 
- setup for start of each frame - init frame buffer color,alpha,z
- compute Xiw and projection xform Xpi from camera definition 
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
- now stack contains Xsw and app can push model Xforms when needed 
*/ 
	IDENTITY(m_camera.Xpi);
	IDENTITY(m_camera.Xiw);
	/**************************XPI CALCULATIONS***********************************/
	float radFOV = m_camera.FOV * (PI / 180.0);
	float projectionTerm = tan(radFOV / 2);
	m_camera.Xpi[2][2] = projectionTerm;
	m_camera.Xpi[3][2] = projectionTerm;

	/**************************XIW CALCULATIONS***********************************/
	//Get axes
	GzCoord zAxis, yAxis, xAxis;
	SUBTRACT(m_camera.lookat, m_camera.position, zAxis);
	//Z
	NORMALIZE(zAxis, zAxis);
	//Y
	float s = DOT(m_camera.worldup, zAxis);
	GzCoord temp;
	SCALARMULT(s, zAxis, temp);
	SUBTRACT(m_camera.worldup, temp, yAxis);
	NORMALIZE(yAxis, yAxis);

	//X
	CROSS(yAxis, zAxis, xAxis);

	//create matrix
	//rotation
	m_camera.Xiw[0][0] = xAxis[X]; m_camera.Xiw[0][1] = xAxis[Y]; m_camera.Xiw[0][2] = xAxis[Z];
	m_camera.Xiw[1][0] = yAxis[X]; m_camera.Xiw[1][1] = yAxis[Y]; m_camera.Xiw[1][2] = yAxis[Z];
	m_camera.Xiw[2][0] = zAxis[X]; m_camera.Xiw[2][1] = zAxis[Y]; m_camera.Xiw[2][2] = zAxis[Z];

	//camera position
	SCALARMULT(-1, xAxis, temp);
	m_camera.Xiw[X][3] = DOT(temp, m_camera.position);
	SCALARMULT(-1, yAxis, temp);
	m_camera.Xiw[Y][3] = DOT(temp, m_camera.position);
	SCALARMULT(-1, zAxis, temp);
	m_camera.Xiw[Z][3] = DOT(temp, m_camera.position);

	/**************************XIMAGE INIT***********************************/
	matlevel = -1;
	GzPushMatrix(Xsp);
	GzPushMatrix(m_camera.Xpi);
	GzPushMatrix(m_camera.Xiw);
	return GZ_SUCCESS;
	return GZ_SUCCESS;
}

int GzRender::GzPutCamera(GzCamera camera)
{
/* HW 3.8 
/*- overwrite renderer camera structure with new camera definition
*/
	m_camera.FOV = camera.FOV;

	m_camera.lookat[X] = camera.lookat[X];
	m_camera.lookat[Y] = camera.lookat[Y];
	m_camera.lookat[Z] = camera.lookat[Z];

	m_camera.position[X] = camera.position[X];
	m_camera.position[Y] = camera.position[Y];
	m_camera.position[Z] = camera.position[Z];

	m_camera.worldup[X] = camera.worldup[X];
	m_camera.worldup[Y] = camera.worldup[Y];
	m_camera.worldup[Z] = camera.worldup[Z];
	return GZ_SUCCESS;	
}

int GzRender::GzPushMatrix(GzMatrix	matrix)
{
/* HW 3.9 
- push a matrix onto the Ximage stack
- check for stack overflow
*/
	if (matlevel < MATLEVELS) {
		GzMatrix result;
		//Push to Ximage
		switch (matlevel) {
		case -1:
			MATCOPY(Ximage[++matlevel], matrix);
			IDENTITY(result);
			MATCOPY(Xnorm[matlevel], result);
			break;
		case 0:
			MATMULT(Ximage[matlevel++], matrix, result);
			MATCOPY(Ximage[matlevel], result);
			IDENTITY(result);
			MATCOPY(Xnorm[matlevel], result);
			break;
		case 1:
			MATMULT(Ximage[matlevel++], matrix, result);
			MATCOPY(Ximage[matlevel], result);
			IDENTITY(result);
			MATCOPY(result, m_camera.Xiw);
			RemoveTranslation(result);
			MATCOPY(Xnorm[matlevel], result);
			break;
		default:
			MATMULT(Ximage[matlevel++], matrix, result);
			MATCOPY(Ximage[matlevel], result);
			IDENTITY(result);
			RemoveTranslation(matrix);
			RemoveScale(matrix);
			MATMULT(Xnorm[matlevel - 1], matrix, result);
			MATCOPY(Xnorm[matlevel], result);
			break;
		}

		return GZ_SUCCESS;
	}

	return GZ_FAILURE;
}

int GzRender::GzPopMatrix()
{
/* HW 3.10
- pop a matrix off the Ximage stack
- check for stack underflow
*/
	if (matlevel > 0) {
		IDENTITY(Ximage[matlevel]);
		matlevel--;
		return GZ_SUCCESS;
	}
	return GZ_SUCCESS;
}

int GzRender::GzPut(int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* HW1.4 write pixel values into the buffer */
	if (i >= 0 && j >= 0 && i < xres && j < yres) {
		int index = ARRAY(i, j);
		pixelbuffer[index].red = CLAMP(r);
		pixelbuffer[index].green = CLAMP(g);
		pixelbuffer[index].blue = CLAMP(b);
		pixelbuffer[index].alpha = a;
		pixelbuffer[index].z = z;

	}

	return GZ_SUCCESS;
}


int GzRender::GzGet(int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
/* HW1.5 retrieve a pixel information from the pixel buffer */
	int index = ARRAY(i, j);
	*r = pixelbuffer[index].red;
	*g = pixelbuffer[index].green;
	*b = pixelbuffer[index].blue;
	*a = pixelbuffer[index].alpha;
	*z = pixelbuffer[index].z;

	return GZ_SUCCESS;
}


int GzRender::GzFlushDisplay2File(FILE* outfile)
{
/* HW1.6 write image to ppm file -- "P6 %d %d 255\r" */
	string colorvals;
	int n = xres * yres;
	string tempheader;

	//write header
	tempheader = "P6 " + to_string(xres) + " " + to_string(yres) + " 255\r";
	char* header = new char[tempheader.length() + 1];
	strcpy(header, tempheader.c_str());
	fwrite(header, sizeof(char), strlen(header), outfile);

	//write rgb
	for (int i = 0; i < n; i++) {
		unsigned char r = pixelbuffer[i].red >> 4;
		unsigned char g = pixelbuffer[i].green >> 4;
		unsigned char b = pixelbuffer[i].blue >> 4;
		fputc(r, outfile);
		fputc(g, outfile);
		fputc(b, outfile);
	}
	return GZ_SUCCESS;
}

int GzRender::GzFlushDisplay2FrameBuffer()
{
/* HW1.7 write pixels to framebuffer: 
	- put the pixels into the frame buffer
	- CAUTION: when storing the pixels into the frame buffer, the order is blue, green, and red 
	- NOT red, green, and blue !!!
*/
	int n = xres * yres;

	for (int i = 0; i < n; i++) {
		unsigned char r = pixelbuffer[i].red >> 4;
		unsigned char g = pixelbuffer[i].green >> 4;
		unsigned char b = pixelbuffer[i].blue >> 4;

		framebuffer[i * 3] = b;
		framebuffer[i * 3 + 1] = g;
		framebuffer[i * 3 + 2] = r;
	}
	return GZ_SUCCESS;
}


/***********************************************/
/* HW2 methods: implement from here */

int GzRender::GzPutAttribute(int numAttributes, GzToken	*nameList, GzPointer *valueList) 
{
/* HW 2.1
-- Set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
-- In later homeworks set shaders, interpolaters, texture maps, and lights
*/

/*
- GzPutAttribute() must accept the following tokens/values:

- GZ_RGB_COLOR					GzColor		default flat-shade color
- GZ_INTERPOLATE				int			shader interpolation mode
- GZ_DIRECTIONAL_LIGHT			GzLight
- GZ_AMBIENT_LIGHT            	GzLight		(ignore direction)
- GZ_AMBIENT_COEFFICIENT		GzColor		Ka reflectance
- GZ_DIFFUSE_COEFFICIENT		GzColor		Kd reflectance
- GZ_SPECULAR_COEFFICIENT       GzColor		Ks coef's
- GZ_DISTRIBUTION_COEFFICIENT   float		spec power
*/
	for (int i = 0; i < numAttributes; i++) {

		if (nameList[i] == GZ_RGB_COLOR) {
			float* colorPtr = static_cast<float*>(valueList[i]);

			flatcolor[RED] = *colorPtr;
			flatcolor[GREEN] = *(colorPtr + 1);
			flatcolor[BLUE] = *(colorPtr + 2);
		}

		/* HW 4
		-- Update light variables
		*/
		if (nameList[i] == GZ_DIRECTIONAL_LIGHT && numlights < 10) {
			GzLight* light = static_cast<GzLight*>(valueList[i]);
			lights[numlights] = *light;
			numlights++;
		}

		if (nameList[i] == GZ_AMBIENT_LIGHT) {
			GzLight* light = static_cast<GzLight*>(valueList[i]);
			ambientlight = *light;
		}

		if (nameList[i] == GZ_INTERPOLATE) {
			int* mode = static_cast<int*>(valueList[i]);
			interp_mode = *mode;
		}

		if (nameList[i] == GZ_DIFFUSE_COEFFICIENT) {
			float* diffuse = static_cast<float*>(valueList[i]);
			Kd[RED] = diffuse[RED];
			Kd[GREEN] = diffuse[GREEN];
			Kd[BLUE] = diffuse[BLUE];
		}

		if (nameList[i] == GZ_AMBIENT_COEFFICIENT) {
			float* ambient = static_cast<float*>(valueList[i]);
			Ka[RED] = ambient[RED];
			Ka[GREEN] = ambient[GREEN];
			Ka[BLUE] = ambient[BLUE];
		}

		if (nameList[i] == GZ_SPECULAR_COEFFICIENT) {
			float* spec = static_cast<float*>(valueList[i]);
			Ks[RED] = spec[RED];
			Ks[GREEN] = spec[GREEN];
			Ks[BLUE] = spec[BLUE];
		}

		if (nameList[i] == GZ_DISTRIBUTION_COEFFICIENT) {
			float* pow = static_cast<float*>(valueList[i]);
			spec = *pow;
		}

		/* HW 5
		-- Get function pointer for tex function
		*/
		if (nameList[i] == GZ_TEXTURE_MAP) {
			tex_fun = static_cast<GzTexture>(valueList[i]);
		}

		//HW6: Shifting
		if (nameList[i] == GZ_AASHIFT) {
			float *shifts = static_cast<float*>(valueList[i]);
			*xshift = shifts[0];
			*yshift = shifts[1];
			*weight = shifts[2];
		}
		/*if (nameList[i] == GZ_AASHIFTX) {
			xshift = static_cast<float*>(valueList[i]);
		}

		if (nameList[i] == GZ_AASHIFTY) {
			yshift = static_cast<float*>(valueList[i]);
		}

		if (nameList[i] == GZ_WEIGHT) {
			weight = static_cast<float*>(valueList[i]);
		}*/

	}
	return GZ_SUCCESS;
}

int GzRender::GzPutTriangle(int numParts, GzToken *nameList, GzPointer *valueList)
/* numParts - how many names and values */
{
	/* HW 2.2
	-- Pass in a triangle description with tokens and values corresponding to
		  GZ_NULL_TOKEN:		do nothing - no values
		  GZ_POSITION:		3 vert positions in model space
	-- Return error code
	*/
	/*
	-- Xform positions of verts using matrix on top of stack
	-- Clip - just discard any triangle with any vert(s) behind view plane
			- optional: test for triangles with all three verts off-screen (trivial frustum cull)
	-- invoke triangle rasterizer
	*/  //ENABLEDEBUG();
 	bool cull = false;
	for (int i = 0; i < numParts; i++) {
		if (nameList[i] == GZ_NORMAL) {
			norms = static_cast<GzCoord*>(valueList[i]);
			GzMatrix normX, result; IDENTITY(normX); IDENTITY(result);
			//Put normals into a 4x4 matrix
			for (int i = 0; i < 3; i++) {
				normX[0][i] = norms[i][X];
				normX[1][i] = norms[i][Y];
				normX[2][i] = norms[i][Z];
				normX[3][i] = 0;
			}
			//Multiply the matrix by the Xnorm
			MATMULT(Xnorm[matlevel], normX, result);

			//convert back to array
			for (int i = 0; i < 3; i++) {
				norms[i][X] = result[0][i];
				norms[i][Y] = result[1][i];
				norms[i][Z] = result[2][i];
			}
		}
		else if (nameList[i] == GZ_POSITION) {
			triCoords = static_cast<GzCoord*>(valueList[i]);
			//Transform coords
			GzMatrix verts;
			IDENTITY(verts);
			//Get all three sets of vertices into a 4x4 matrix
			for (int i = 0; i < 3; i++) {
				verts[0][i] = triCoords[i][X];
				verts[1][i] = triCoords[i][Y];
				verts[2][i] = triCoords[i][Z];
				verts[3][i] = 1;
			}
			//multiply the verts matrix by the Ximage
			this->GzPushMatrix(verts);
			//Assign the transformed matrices back to the array
			for (int i = 0; i < 3; i++) {
				triCoords[i][X] = Ximage[matlevel][0][i] / Ximage[matlevel][3][i];
				triCoords[i][Y] = Ximage[matlevel][1][i] / Ximage[matlevel][3][i];
				triCoords[i][Z] = Ximage[ matlevel][2][i] / Ximage[matlevel][3][i];

				//HW6: Shift
				triCoords[i][X] -= *xshift;
				triCoords[i][Y] -= *yshift;

				if (triCoords[i][Z] <= 0)
					cull = true;
			}


			//calculate face normal
			this->GzPopMatrix();
		}
		else if (nameList[i] == GZ_TEXTURE_INDEX) {
			uvs = static_cast<GzUV*>(valueList[i]);
			p_UVs = new GzUV[3];
		}

	}

	Rasterize(this);
	return GZ_SUCCESS;
}

void Rasterize(GzRender* r) {
	
	//Transform uv to perspective UV
	//float Vz;

	for (int i = 0; i < 3; i++) {
		float Vz = (float)triCoords[i][Z] / (float)(MAXINT - triCoords[i][Z]);
		p_UVs[i][U] = uvs[i][U] / (Vz + 1);
		p_UVs[i][V] = uvs[i][V] / (Vz + 1);
	}

	SetDDAs(triCoords);

	//Sort verts into L and R
	SortVertices(triCoords);
	//Create spans
	Span spanTop, spanBottom;
	DDA* currentLeft;
	DDA* currentRight;
	GzColor* c = new GzColor[3];
	//GzColor vertColors[3];

	if (r->interp_mode == GZ_FLAT)
	{
		c[0][RED] = r->ctoi(r->flatcolor[RED]); c[0][GREEN] = r->ctoi(r->flatcolor[GREEN]); c[0][BLUE] = r->ctoi(r->flatcolor[BLUE]);
	}
	else if (r->interp_mode == GZ_COLOR) {
	/*	for (int i = 0; i < 3; i++) {
			r->tex_fun(uvs[i][U], uvs[i][V], vertColors[i]);
			r->Kd[RED] = vertColors[i][RED]; r->Kd[GREEN] = vertColors[i][GREEN]; r->Kd[BLUE] = vertColors[i][BLUE];
			r->Ks[RED] = vertColors[i][RED]; r->Ks[GREEN] = vertColors[i][GREEN]; r->Ks[BLUE] = vertColors[i][BLUE];
			r->Ka[RED] = vertColors[i][RED]; r->Ka[GREEN] = vertColors[i][GREEN]; r->Ka[BLUE] = vertColors[i][BLUE];
			EvaluateLighting(r, triCoords[i], norms[i], vertColors[i]);
		}*/
	}

	//get current left and right
	if (ddas[0].left) {
		currentLeft = &ddas[0];
		currentRight = &ddas[2];
	}
	else {
		currentLeft = &ddas[2];
		currentRight = &ddas[0];
	}

	//Scan through the triangle
	float dY;
	dY = ceil(ddas[0].current[Y]) - ddas[0].current[Y];
	if (dY > -0.001 && dY < 0.001) dY = 1;
	ddas[0].AdvanceCurrent(dY);
	ddas[2].AdvanceCurrent(dY);

	//span across the top part of the triangle
	while (ddas[0].current[Y] <= ddas[0].end[Y]) {
		spanTop = Span(*currentLeft, *currentRight);

		FillSpan(&spanTop, r, c);
		ddas[0].AdvanceCurrent(1);
		ddas[2].AdvanceCurrent(1);

	}

	//get current left and right
	if (ddas[1].left) {
		currentLeft = &ddas[1];
		currentRight = &ddas[2];
	}
	else {
		currentLeft = &ddas[2];
		currentRight = &ddas[1];
	}

	dY = ceil(ddas[1].current[Y]) - ddas[1].current[Y];
	ddas[1].AdvanceCurrent(dY);

	//span across the bottom part of the triangle
	while (ddas[1].current[Y] <= ddas[1].end[Y]) {
		spanBottom = Span(*currentLeft, *currentRight);
		FillSpan(&spanBottom, r, c);
		ddas[1].AdvanceCurrent(1);
		ddas[2].AdvanceCurrent(1);
	}
}

void FillSpan(Span* span, GzRender* r, GzColor* c) {
	//get number of pixels between start and end
	int numPix = 1;
	float dX;
	Span spanCounter = *span;
	float abcd[3][4], abcd2[2][4];
	GzColor interpColor;
	GzCoord interpNorm;

	dX = ceil(spanCounter.current[X]) - spanCounter.current[X];
	if (dX == 0) dX = 1;
	spanCounter.AdvanceCurrent(dX);

	//pixel counter
	while (spanCounter.current[X] < spanCounter.end[X]) {
		spanCounter.AdvanceCurrent(1);
		numPix++;
	}

	dX = ceil(span->current[X]) - span->current[X];
	if (dX == 0) dX = 1;
	span->AdvanceCurrent(dX);

	for (int i = 0; i < numPix - 1; i++) {
		dX = ceil(span->current[X]) - span->current[X];
		if (dX > -0.001 && dX < 0.001) dX = 1;
		if (span->current[X] >= 0 && span->current[X] < r->xres && span->current[Y] >= 0 && span->current[Y] < r->yres) {
			int i = r->ARRAY(span->current[X], span->current[Y]);

			//HW5: INTERPOLATE UV
			GzUV interpUV, interpuv;
			GetUVABCD(triCoords, abcd2);
			GetInterpolatedUV(abcd2, interpUV, span->current[X], span->current[Y]);

			//Place colors based on shading mode
			if (span->current[Z] < r->pixelbuffer[i].z) {

				//HW5: TRANSFORM INTERPOLATED UV BACK TO uv
				float Vz = (float)span->current[Z] / (float)(MAXINT - span->current[Z]);
				interpuv[U] = interpUV[U] * (Vz + 1);
				interpuv[V] = interpUV[V] * (Vz + 1);				

				//change parameters for gz color and gz normals
				switch (r->interp_mode) {
				case GZ_FLAT:
					r->GzPut(span->current[X], span->current[Y], c[0][RED], c[0][GREEN], c[0][BLUE], 100000, span->current[Z]);
					break;
				case GZ_COLOR:
					if (r->tex_fun != NULL) {
						r->tex_fun(interpuv[U], interpuv[V], interpColor);
						//change Ks
						r->Ka[RED] = interpColor[RED]; r->Ka[GREEN] = interpColor[GREEN]; r->Ka[BLUE] = interpColor[BLUE];
						r->Kd[RED] = interpColor[RED]; r->Kd[GREEN] = interpColor[GREEN]; r->Kd[BLUE] = interpColor[BLUE];
						r->Ks[RED] = interpColor[RED]; r->Ks[GREEN] = interpColor[GREEN]; r->Ks[BLUE] = interpColor[BLUE];

						//get color at the 3 vertices

						GzColor vertColors[3];
						for (int i = 0; i < 3; i++) {
							EvaluateLighting(r, triCoords[i], norms[i], vertColors[i]);
						}
						GetABCD(triCoords, vertColors, abcd);
						GzColor fCol; GetInterpolatedColor(abcd, fCol, span->current[X], span->current[Y]);
						r->GzPut(span->current[X], span->current[Y], r->ctoi(fCol[RED]), r->ctoi(fCol[GREEN]), r->ctoi(fCol[BLUE]), 10000, span->current[Z]);
					}
					else {
						GetABCD(triCoords, c, abcd);
						GetInterpolatedColor(abcd, interpColor, span->current[X], span->current[Y]);
						r->GzPut(span->current[X], span->current[Y], r->ctoi(interpColor[RED]), r->ctoi(interpColor[GREEN]), r->ctoi(interpColor[BLUE]), 100000, span->current[Z]);
					}
					break;
				case GZ_NORMALS:
					GzColor col;
					if (r->tex_fun != NULL) {
						r->tex_fun(interpuv[U], interpuv[V], col);
						r->Kd[RED] = col[RED];
						r->Kd[GREEN] = col[GREEN];
						r->Kd[BLUE] = col[BLUE];
						r->Ka[RED] = col[RED];
						r->Ka[GREEN] = col[GREEN];
						r->Ka[BLUE] = col[BLUE];
						//get interpolated norm, call EvaluateLighting, and then GzPut.
						GzCoord currentPoint = { span->current[X], span->current[Y], span->current[Z] };
						GetNormalABCD(abcd);
						GetInterpolatedNorm(abcd, interpNorm, span->current[X], span->current[Y]);
						EvaluateLighting(r, currentPoint, interpNorm, col);
						r->GzPut(span->current[X], span->current[Y], r->ctoi(col[RED]), r->ctoi(col[GREEN]), r->ctoi(col[BLUE]), 100000, span->current[Z]);
					}
					else {
						GzCoord currentPoint = { span->current[X], span->current[Y], span->current[Z] };
						GetNormalABCD(abcd);
						GetInterpolatedNorm(abcd, interpNorm, span->current[X], span->current[Y]);
						EvaluateLighting(r, currentPoint, interpNorm, interpColor);
						r->GzPut(span->current[X], span->current[Y], r->ctoi(interpColor[RED]), r->ctoi(interpColor[GREEN]), r->ctoi(interpColor[BLUE]), 100000, span->current[Z]);
					}
					break;
				}
			}
		}
		span->AdvanceCurrent(dX);
	}
}


void GetInterpolatedColor(float abcd[3][4], GzColor out, int x, int y) {
	out[RED] = -1 * (abcd[RED][0] * x + abcd[RED][1] * y + abcd[RED][3]);
	out[RED] /= abcd[RED][2];

	out[GREEN] = -1 * (abcd[GREEN][0] * x + abcd[GREEN][1] * y + abcd[GREEN][3]);
	out[GREEN] /= abcd[GREEN][2];

	out[BLUE] = -1 * (abcd[BLUE][0] * x + abcd[BLUE][1] * y + abcd[BLUE][3]);
	out[BLUE] /= abcd[BLUE][2];
}

void GetInterpolatedNorm(float abcd[3][4], GzCoord out, int x, int y) {
	out[X] = -1 * (abcd[X][0] * x + abcd[X][1] * y + abcd[X][3]);
	out[X] /= abcd[X][2];

	out[Y] = -1 * (abcd[Y][0] * x + abcd[Y][1] * y + abcd[Y][3]);
	out[Y] /= abcd[Y][2];

	out[Z] = -1 * (abcd[Z][0] * x + abcd[Z][1] * y + abcd[Z][3]);
	out[Z] /= abcd[Z][2];

	NORMALIZE(out, out);
}

void GetInterpolatedUV(float abcd[2][4], GzUV out, int x, int y) {

	out[U] = -1 * (abcd[U][0] * x + abcd[U][1] * y + abcd[U][3]);
	out[U] /= abcd[U][2];

	out[V] = -1 * (abcd[V][0] * x + abcd[V][1] * y + abcd[V][3]);
	out[V] /= abcd[V][2];
}

void GetABCD(GzCoord in[], GzColor colors[], float out[3][4]) {
	GzCoord temp1, temp2, tempRes;

	//get ABCD for red
	GzCoord vR1 = { in[0][X], in[0][Y], colors[0][RED] };
	GzCoord vR2 = { in[1][X], in[1][Y], colors[1][RED] };
	GzCoord vR3 = { in[2][X], in[2][Y], colors[2][RED] };
	SUBTRACT(vR1, vR2, temp1);
	SUBTRACT(vR3, vR1, temp2);
	CROSS(temp1, temp2, tempRes);
	//ABC
	out[RED][0] = tempRes[0];
	out[RED][1] = tempRes[1];
	out[RED][2] = tempRes[2];
	//D
	out[RED][3] = -1 * (out[RED][0] * vR1[X] + out[RED][1] * vR1[Y] + out[RED][2] * vR1[2]);

	//get ABCD for green
	GzCoord vG1 = { in[0][X], in[0][Y], colors[0][GREEN] };
	GzCoord vG2 = { in[1][X], in[1][Y], colors[1][GREEN] };
	GzCoord vG3 = { in[2][X], in[2][Y], colors[2][GREEN] };
	SUBTRACT(vG1, vG2, temp1);
	SUBTRACT(vG3, vG1, temp2);
	CROSS(temp1, temp2, tempRes);
	//ABC
	out[GREEN][0] = tempRes[0];
	out[GREEN][1] = tempRes[1];
	out[GREEN][2] = tempRes[2];
	//D
	out[GREEN][3] = -1 * (out[GREEN][0] * vG1[X] + out[GREEN][1] * vG1[Y] + out[GREEN][2] * vG1[2]);

	//get ABCD for blue
	GzCoord vB1 = { in[0][X], in[0][Y], colors[0][BLUE] };
	GzCoord vB2 = { in[1][X], in[1][Y], colors[1][BLUE] };
	GzCoord vB3 = { in[2][X], in[2][Y], colors[2][BLUE] };
	SUBTRACT(vB1, vB2, temp1);
	SUBTRACT(vB3, vB1, temp2);
	CROSS(temp1, temp2, tempRes);
	//ABC
	out[BLUE][0] = tempRes[0];
	out[BLUE][1] = tempRes[1];
	out[BLUE][2] = tempRes[2];
	//D
	out[BLUE][3] = -1 * (out[BLUE][0] * vB1[X] + out[BLUE][1] * vB1[Y] + out[BLUE][2] * vB1[2]);
}

void GetNormalABCD(float out[3][4]) {
	GzCoord temp1, temp2, tempRes;

	//get ABCD for Nx
	GzCoord vNx1 = { triCoords[0][X], triCoords[0][Y], norms[0][X] };
	GzCoord vNx2 = { triCoords[1][X], triCoords[1][Y], norms[1][X] };
	GzCoord vNx3 = { triCoords[2][X], triCoords[2][Y], norms[2][X] };
	SUBTRACT(vNx1, vNx2, temp1);
	SUBTRACT(vNx3, vNx1, temp2);
	CROSS(temp1, temp2, tempRes);
	//ABC
	out[X][0] = tempRes[0];
	out[X][1] = tempRes[1];
	out[X][2] = tempRes[2];
	//D
	out[X][3] = -1 * (out[X][0] * vNx1[X] + out[X][1] * vNx1[Y] + out[X][2] * vNx1[2]);

	//get ABCD for Ny
	GzCoord vNy1 = { triCoords[0][X], triCoords[0][Y], norms[0][Y] };
	GzCoord vNy2 = { triCoords[1][X], triCoords[1][Y], norms[1][Y] };
	GzCoord vNy3 = { triCoords[2][X], triCoords[2][Y], norms[2][Y] };
	SUBTRACT(vNy1, vNy2, temp1);
	SUBTRACT(vNy3, vNy1, temp2);
	CROSS(temp1, temp2, tempRes);
	//ABC
	out[Y][0] = tempRes[0];
	out[Y][1] = tempRes[1];
	out[Y][2] = tempRes[2];
	//D
	out[Y][3] = -1 * (out[Y][0] * vNy1[X] + out[Y][1] * vNy1[Y] + out[Y][2] * vNy1[2]);

	//get ABCD for NZ
	GzCoord vNz1 = { triCoords[0][X], triCoords[0][Y], norms[0][Z] };
	GzCoord vNz2 = { triCoords[1][X], triCoords[1][Y], norms[1][Z] };
	GzCoord vNz3 = { triCoords[2][X], triCoords[2][Y], norms[2][Z] };
	SUBTRACT(vNz1, vNz2, temp1);
	SUBTRACT(vNz3, vNz1, temp2);
	CROSS(temp1, temp2, tempRes);
	//ABC
	out[Z][0] = tempRes[0];
	out[Z][1] = tempRes[1];
	out[Z][2] = tempRes[2];
	//D
	out[Z][3] = -1 * (out[Z][0] * vNz1[X] + out[Z][1] * vNz1[Y] + out[Z][2] * vNz1[2]);

}

void GetUVABCD(GzCoord in[], float out[3][4]) {
	GzCoord temp1, temp2, tempRes;

	//get ABCD for U

	GzCoord vU1 = { triCoords[0][X], triCoords[0][Y], p_UVs[0][U] };
	GzCoord vU2 = { triCoords[1][X], triCoords[1][Y], p_UVs[1][U] };
	GzCoord vU3 = { triCoords[2][X], triCoords[2][Y], p_UVs[2][U] };

	
	SUBTRACT(vU2, vU1, temp1);
	SUBTRACT(vU3, vU2, temp2);
	CROSS(temp1, temp2, tempRes);

	//ABC
	out[U][0] = tempRes[0];
	out[U][1] = tempRes[1];
	out[U][2] = tempRes[2];
	//D
	out[U][3] = -1 * (out[U][0] * vU1[X] + out[U][1] * vU1[Y] + out[U][2] * vU1[2]);

	//get ABCD for V

	GzCoord vV1 = { triCoords[0][X], triCoords[0][Y], p_UVs[0][V] };
	GzCoord vV2 = { triCoords[1][X], triCoords[1][Y], p_UVs[1][V] };
	GzCoord vV3 = { triCoords[2][X], triCoords[2][Y], p_UVs[2][V] };

	SUBTRACT(vV1, vV2, temp1);
	SUBTRACT(vV3, vV1, temp2);
	CROSS(temp1, temp2, tempRes);

	//ABC
	out[V][0] = tempRes[0];
	out[V][1] = tempRes[1];
	out[V][2] = tempRes[2];
	//D
	out[V][3] = -1 * (out[V][0] * vV1[X] + out[V][1] * vV1[Y] + out[V][2] * vV1[2]);

}

void EvaluateLighting(GzRender* r, GzCoord point, GzCoord N, GzColor C) {
	GzCoord specular = { 0,0,0 };
	GzCoord diffuse = { 0,0,0 };
	GzCoord ambient = { 0,0,0 };
	bool skip = false;

	GzCoord E = { 0, 0, -1 };
	GzCoord R = { 0, 0, 0 };
	GzCoord L = { 0, 0, 0 };

	for (int i = 0; i < r->numlights; i++) {
		//get L
		L[X] = r->lights[i].direction[X];
		L[Y] = r->lights[i].direction[Y];
		L[Z] = r->lights[i].direction[Z];

		//sign test
		float s1 = DOT(N, L);
		float s2 = DOT(N, E);

		if ((s1 < 0 && s2 > 0) || (s1 > 0 && s2 < 0))
			skip = true;

		if (!skip) {
			if (s1 < 0 && s2 < 0)
				SCALARMULT(-1, N, N);

			//get R
			SCALARMULT((2 * DOT(N, L)), N, R);
			SUBTRACT(R, L, R);

			//clamp
			float RdotE = DOT(R, E);
			if (RdotE < 0)
				RdotE = 0;
			else if (RdotE > 1)
				RdotE = 1;

			//get summations
			specular[RED] += r->lights[i].color[RED] * pow(RdotE, r->spec);
			specular[GREEN] += r->lights[i].color[GREEN] * pow(RdotE, r->spec);
			specular[BLUE] += r->lights[i].color[BLUE] * pow(RdotE, r->spec);

			diffuse[RED] += r->lights[i].color[RED] * DOT(N, L);
			diffuse[GREEN] += r->lights[i].color[GREEN] * DOT(N, L);
			diffuse[BLUE] += r->lights[i].color[BLUE] * DOT(N, L);
		}
		skip = false;
	}

	//calculate final
	for (int i = RED; i < BLUE + 1; i++) {
		specular[i] *= r->Ks[i];
		diffuse[i] *= r->Kd[i];
		ambient[i] = r->ambientlight.color[i] * r->Ka[i];
		C[i] = specular[i] + diffuse[i] + ambient[i];
	}
	//printVertices(C);
}

int SetDDAs(GzCoord* triCoords) {

	//sort the triangle vertices by Y
	SELECTIONSORT(triCoords, 3);

	//create DDAs from the sorted vertex list
	DDA d1(triCoords[0], triCoords[1]);
	DDA d2(triCoords[1], triCoords[2]);
	DDA d3(triCoords[0], triCoords[2]);

	/*check for equal y*/
	if (triCoords[0][Y] == triCoords[1][Y]) {
		d1.slopeX = NULL;
	}

	if (triCoords[0][Y] == triCoords[2][Y]) {
		d3.slopeX = NULL;
	}
	if (triCoords[1][Y] == triCoords[2][Y]) {
		d2.slopeX = NULL;
	}

	//assign them to the global DDA array in increasing order
	ddas[0] = d1; ddas[1] = d2; ddas[2] = d3;

	return GZ_SUCCESS;
}

int SortVertices(GzCoord* triCoords) {
	//Get mid-Y vert (triCoords[1])
	GzCoord mid = { triCoords[1][X], triCoords[1][Y], triCoords[1][Z] };
	//Get non-midY edge (ddas[2])
	DDA nonMid = ddas[2];

	//formulate line eq for nonMid edge, y = mx + c
	float m = (nonMid.start[Y] - nonMid.end[Y]) / (nonMid.start[X] - nonMid.end[X]);
	float c = nonMid.start[Y] - (m * nonMid.start[X]);

	//Plug in y = midY[Y] and solve for x
	float nonMidX = (mid[Y] - c) / m;

	//compare nonMidX to the X of the mid  
	if (nonMidX < mid[X]) {
		//edges that include the mid are right edges
		ddas[0].left = false;
		ddas[1].left = false;
		ddas[2].left = true;

	}
	else if (nonMidX > mid[X]) {
		//edges that include the mid are left edges
		ddas[0].left = true;
		ddas[1].left = true;
		ddas[2].left = false;
	}

	return GZ_SUCCESS;

}

void RemoveTranslation(GzMatrix mat) {
	for (int i = 0; i < 3; i++) {
		mat[i][3] = 0;
	}
	mat[3][3] = 1;
}

void RemoveScale(GzMatrix mat) {
	float K = sqrt(mat[0][0] * mat[0][0] + mat[0][1] * mat[0][1] + mat[0][2] * mat[0][2]);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mat[i][j] /= K;
}


/*Helper methods*/

int CLAMP(int color) { if (color < 0) return 0; if (color > 4095) return 4095; return color; }

void ENABLEDEBUG() {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
}

void SELECTIONSORT(GzCoord* coords, int n) {
	int i, j, min_idx;

	// One by one move boundary of unsorted subarray
	for (i = 0; i < n - 1; i++)
	{
		// Find the minimum element in unsorted array
		min_idx = i;
		for (j = i + 1; j < n; j++)
			if (coords[j][Y] <= coords[min_idx][Y])
				min_idx = j;

		// Swap the found minimum element with the first element
		GzCoord tempPoint, tempNorm;
		GzUV tempUV;
		tempPoint[X] = coords[min_idx][X]; tempNorm[X] = norms[min_idx][X]; tempUV[U] = p_UVs[min_idx][U];
		tempPoint[Y] = coords[min_idx][Y]; tempNorm[Y] = norms[min_idx][Y]; tempUV[V] = p_UVs[min_idx][V];
		tempPoint[Z] = coords[min_idx][Z]; tempNorm[Z] = norms[min_idx][Z]; 

		coords[min_idx][X] = coords[i][X];  norms[min_idx][X] = norms[i][X]; p_UVs[min_idx][U] = p_UVs[i][U];
		coords[min_idx][Y] = coords[i][Y];  norms[min_idx][Y] = norms[i][Y]; p_UVs[min_idx][V] = p_UVs[i][V];
		coords[min_idx][Z] = coords[i][Z]; norms[min_idx][Z] = norms[i][Z];

		coords[i][X] = tempPoint[X]; norms[i][X] = tempNorm[X]; p_UVs[i][U] = tempUV[U];
		coords[i][Y] = tempPoint[Y]; norms[i][Y] = tempNorm[Y]; p_UVs[i][V] = tempUV[V];
		coords[i][Z] = tempPoint[Z]; norms[i][Z] = tempNorm[Z];
	}
}

void printVertices(float* tri) {
	cout << tri[X] << " " << tri[Y] << " " << tri[Z] << endl;
}

void printMatrix(GzMatrix mat) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cout << mat[i][j] << "  ";
		}
		cout << endl;
	}
	cout << endl;
}

void NORMALIZE(GzCoord vec, GzCoord normVect) {
	float magnitude = sqrt((vec[X] * vec[X]) + (vec[Y] * vec[Y]) + (vec[Z] * vec[Z]));
	normVect[X] = vec[X] / magnitude;
	normVect[Y] = vec[Y] / magnitude;
	normVect[Z] = vec[Z] / magnitude;
}

float DOT(GzCoord A, GzCoord B) {
	return ((A[X] * B[X]) + (A[Y] * B[Y]) + (A[Z] * B[Z]));
}

void SCALARMULT(float scalar, GzCoord vec, GzCoord res) {
	res[X] = vec[X] * scalar;
	res[Y] = vec[Y] * scalar;
	res[Z] = vec[Z] * scalar;
}

void SUBTRACT(GzCoord A, GzCoord B, GzCoord res) {
	res[X] = A[X] - B[X];
	res[Y] = A[Y] - B[Y];
	res[Z] = A[Z] - B[Z];
}

void CROSS(GzCoord A, GzCoord B, GzCoord res) {
	res[0] = (A[1] * B[2]) - (A[2] * B[1]);
	res[1] = (A[2] * B[0]) - (A[0] * B[2]);
	res[2] = (A[0] * B[1]) - (A[1] * B[0]);
}


void MATMULT(GzMatrix A, GzMatrix B, GzMatrix res) {

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			res[i][j] = 0;
			for (int k = 0; k < 4; k++) {
				res[i][j] += A[i][k] * B[k][j];
			}
		}
	}
}

void MATCOPY(GzMatrix to, GzMatrix from) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			to[i][j] = from[i][j];
		}
	}
}

void VERTEXTRANSFORM(GzMatrix transform, float verts[], float res[]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			/*cout << "MATRIX IJ " << transform[i][j] << endl;
			cout << "VERTEX J " << verts[j] << endl;
			cout << "MULT " << transform[i][j] * verts[j];*/
			res[i] += transform[i][j] * verts[j];
			//cout << "RESULT " << i << " " << res[i] << endl;
			//system("pause");
		}
	}
}

void IDENTITY(GzMatrix mat) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (i == j)
				mat[i][j] = 1;
			else
				mat[i][j] = 0;
		}
	}
}

