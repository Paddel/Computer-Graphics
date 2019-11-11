// Teile aus https://gist.github.com/kopiro/4414350
// und http://www.sgh1.net/posts/read-stl-file.md
// und http://users.polytech.unice.fr/~buffa/cours/synthese_image/DOCS/Tutoriaux/Nehe/opengl.htm
// und https://www.opengl.org/archives/resources/code/samples/glut_examples/examples/highlight.c

#define _USE_MATH_DEFINES

#include <map>
#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>
#include <math.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

using namespace std;

class CVertex;

class CVector
{
public:

	CVector():
		m_x(0.f),
		m_y(0.f),
		m_z(0.f)
	{

	}
	CVector(char* facet);
	CVector(GLfloat x, GLfloat y, GLfloat z):
		m_x(x),
		m_y(y),
		m_z(z)
	{

	}

	~CVector()
	{
	}

	GLfloat m_x, m_y, m_z;
};

//Notice the constructor v3(char* bin); This constructor will help us create an instance of v3 from binary data 
// found in the STL file.Here's the implementation:

CVector::CVector(char* facet)
{
	char f1[4] = { facet[0],
		facet[1],facet[2],facet[3] };

	char f2[4] = { facet[4],
		facet[5],facet[6],facet[7] };

	char f3[4] = { facet[8],
		facet[9],facet[10],facet[11] };

	GLfloat xx = *((GLfloat*)f1);
	GLfloat yy = *((GLfloat*)f2);
	GLfloat zz = *((GLfloat*)f3);

	m_x = xx;
	m_y = yy;
	m_z = zz;
}

//We assume that char* facet is really a pointer to a contiguous block of at least nine single precision floats.We then cast the binary data 4 bytes at a time.Note that since our v3 class uses double precision, we must cast the single precision values found in the STL file to double precision.Also, it's not necessary to copy the binary data into three separate char arrays. The first line of the method could have been simply:
// float xx = *((float*)facet);

// Okay, now we need a simple triangle class :
class CTriangle
{
public:
	CTriangle(const CVector& p1, const CVector& p2, const CVector& p3, const CVector& normal):
		m_faceNormal(normal)
	{
		m_p[0] = p1;
		m_p[1] = p2;
		m_p[2] = p3;

		for (int i = 0; i < 3; i++)
			m_Crinkled[i] = false;
	}
	~CTriangle()
	{		
	}

	CVector m_p[3], m_faceNormal;
	CVertex *m_vertices[3];
	CVector m_vertexNormals[3];

	//Visualisation
	bool m_Crinkled[3];
};

class CVertex
{
public:
	CVertex() {};
	CVertex(CTriangle *pFirst) { m_adjacentTriangles.push_back(pFirst); };

	vector<CTriangle *> m_adjacentTriangles;
};

enum
{
	RENDERMODE_HEATMAP = 0,
	RENDERMODE_CRINKLES,
	RENDERMODE_WIREFRAME,
	RENDERMODE_SHADING,
	RENDERMODE_FACENORMALS,
	RENDERMODE_VERTEXNORMALS,
	NUM_RENDERMODES,
};

bool renderModes[NUM_RENDERMODES] = { false };

void calcVertexNormals(vector <CTriangle *>&triangles)
{
	cout << "Calculating vertex normals for " << triangles.size() << " triangles" << endl;
	const GLfloat limit = (float)(M_PI / 180.0 * 70.0);

	for (int t = 0; t < triangles.size(); t++)
	{
		for (int v = 0; v < 3; v++)
		{
			for (int j = 0; j < triangles[t]->m_vertices[v]->m_adjacentTriangles.size(); j++)
			{
				CTriangle *pAdjacentTriangle = triangles[t]->m_vertices[v]->m_adjacentTriangles[j];

				CVector v1 = triangles[t]->m_faceNormal;
				CVector v2 = pAdjacentTriangle->m_faceNormal;
				GLfloat dot = v1.m_x * v2.m_x + v1.m_y * v2.m_y + v1.m_z * v2.m_z;
				GLfloat lenSq1 = v1.m_x*v1.m_x + v1.m_y*v1.m_y + v1.m_z * v1.m_z;
				GLfloat lenSq2 = v2.m_x*v2.m_x + v2.m_y*v2.m_y + v2.m_z * v2.m_z;
				GLfloat angle = acos(dot / sqrt(lenSq1 * lenSq2));
				triangles[t]->m_Crinkled[v] = angle >= limit;

				if (angle < limit)
				{
					triangles[t]->m_vertexNormals[v].m_y += pAdjacentTriangle->m_faceNormal.m_y;
					triangles[t]->m_vertexNormals[v].m_x += pAdjacentTriangle->m_faceNormal.m_x;
					triangles[t]->m_vertexNormals[v].m_z += pAdjacentTriangle->m_faceNormal.m_z;
				}
			}
		}
	}
}

void normalizeVertices(vector <CTriangle *>&triangles)
{
	cout << "Normalizing vertices" << endl;
	for (int t = 0; t < triangles.size(); t++)
	{
		for (int v = 0; v < 3; v++)
		{
			CVector *vertex = &triangles[t]->m_vertexNormals[v];
			GLfloat w = sqrt(vertex->m_x * vertex->m_x + vertex->m_y * vertex->m_y + vertex->m_z * vertex->m_z);
			vertex->m_x /= w;
			vertex->m_y /= w;
			vertex->m_z /= w;
		}
	}
}

void read_stl(string fname, vector <CTriangle *>&v, map<string, CVertex *>& vertices)
{
	ifstream myFile(fname.c_str(), ios::in | ios::binary);

	char header_info[80] = "";
	char nTri[4];
	unsigned long nTriLong;

	//read 80 byte header
	if (myFile) 
	{
		myFile.read(header_info, 80);
		cout << "header: " << header_info << endl;
	}
	else {
		cout << "error" << endl;
	}

	//read 4-byte ulong
	if (myFile)
	{
		myFile.read(nTri, 4);
		nTriLong = *((unsigned long*)nTri);
		cout << "n Tri: " << nTriLong << endl;
	}
	else {
		cout << "error" << endl;
	}

	const int VECTOR_BINARY_LEN = 12;

	//now read in all the triangles
	for (unsigned long int i = 0; i < nTriLong; i++)
	{

		char facet[50];

		if (myFile) {

			//read one 50-byte triangle
			myFile.read(facet, 50);

			//populate each point of the triangle
			//using v3::v3(char* bin);
				//facet + 12 skips the triangle's unit normal
			CVector normal( facet );
			CVector p1(facet + VECTOR_BINARY_LEN);
			CVector p2(facet + VECTOR_BINARY_LEN * 2);
			CVector p3(facet + VECTOR_BINARY_LEN * 3);

			//add a new triangle to the array
			CTriangle *pTriangle = new CTriangle(p1, p2, p3, normal);
		
			for (int v = 0; v < 3; v++)
			{
				CVector p = v == 0 ? p1 : (v == 1 ? p2 : p3);
				char vecStr[128];
				sprintf_s(vecStr, sizeof(vecStr), "%f:%f:%f", p.m_x, p.m_y, p.m_z);

				map<string, CVertex *>::iterator it = vertices.find(vecStr);
				if (it != vertices.end())
					it->second->m_adjacentTriangles.push_back(pTriangle);
				else
					it = vertices.insert(make_pair(vecStr, new CVertex(pTriangle))).first;

				pTriangle->m_vertices[v] = it->second;
			}

			v.push_back(pTriangle);
		}
	}
}

// hier haben wir alle unsere Dreiecke reinkopiert:
vector <CTriangle *> allTriangles;

GLfloat gWidth = 1000.0f;
GLfloat gHeight = 800.0f;
GLfloat zNear = 0.01f;
GLfloat zFar = 1000.0f;
GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[] = { 0.0f, 0.0f, 200.0f, 1.0f };

float mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
float mat_ambient_color[] = { 0.48f, 0.48f, 0.2f, 1.0f };
float mat_diffuse[] = { 0.7f, 0.7f, 0.3f, 1.0f };
float mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };

float model_ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f };

// for navigation --------------------------------
int ox, oy;
int buttonState = 0;
float viewRotation[] = { 0.0f, 0.0f, 0.0f };
const float originalEyeZ = -60.0f;
float viewTranslation[] = { 0.0f, 0.0f, originalEyeZ };
double zoomFactor = 1.0;
const double zoomFactorDelta = 0.01;
const double zoomFactorMax = 30.0;
const double zoomFactorMin = 0.1;
// -----------------------------------------for navigation 

void setProjectionMatrix(int width, int height, double zoomFactor)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	gluPerspective(20.0*zoomFactor, (float)width / (float)height, zNear, zFar);
}

void setMaterialForHeatmap(int neighbours)
{
	float color_green[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float color_yellow[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	float color_red[] = { 1.0f, 0.0f, 0.0f, 1.0f };

	if (neighbours >= 6)
	{
		glMaterialfv(GL_FRONT, GL_DIFFUSE, color_red);
	}
	else if (neighbours >= 5)
	{
		glMaterialfv(GL_FRONT, GL_DIFFUSE, color_yellow);
	}
	else if (neighbours >= 4)
	{
		glMaterialfv(GL_FRONT, GL_DIFFUSE, color_green);
	}
	else
	{
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	}
}

void drawModel()
{

	glColorMaterial(GL_FRONT, GL_AMBIENT);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	glColorMaterial(GL_FRONT, GL_SPECULAR);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

	glBegin(GL_TRIANGLES);

	for (size_t i = 0; i < allTriangles.size(); i++)
	{
		CTriangle *pTriangle = allTriangles[i];
		for (int j = 0; j < 3; j++)
		{
			if (renderModes[RENDERMODE_HEATMAP])
			{
				int neighbours = (int)pTriangle->m_vertices[j]->m_adjacentTriangles.size();
				setMaterialForHeatmap(neighbours);
			}

			if (renderModes[RENDERMODE_SHADING])
				glNormal3f(pTriangle->m_faceNormal.m_x, pTriangle->m_faceNormal.m_y, pTriangle->m_faceNormal.m_z);
			else
				glNormal3f(pTriangle->m_vertexNormals[j].m_x, pTriangle->m_vertexNormals[j].m_y, pTriangle->m_vertexNormals[j].m_z);

			glVertex3f(pTriangle->m_p[j].m_x, pTriangle->m_p[j].m_y, pTriangle->m_p[j].m_z);
		}
	}

	glEnd();
}

const GLfloat markerLength = 1.0;

void drawFaceNormals()
{
	if (renderModes[RENDERMODE_FACENORMALS])
	{
		const float color_white[] = { 1.0, 1.0, 1.0, 1.0 };

		glBegin(GL_LINES);
		glMaterialfv(GL_FRONT, GL_AMBIENT, color_white);
		for (size_t i = 0; i < allTriangles.size(); i++)
		{
			CTriangle *pTriangle = allTriangles[i];
			CVector start;
			CVector end;

			start.m_x = (pTriangle->m_p[0].m_x + pTriangle->m_p[1].m_x + pTriangle->m_p[2].m_x) / 3;
			start.m_y = (pTriangle->m_p[0].m_y + pTriangle->m_p[1].m_y + pTriangle->m_p[2].m_y) / 3;
			start.m_z = (pTriangle->m_p[0].m_z + pTriangle->m_p[1].m_z + pTriangle->m_p[2].m_z) / 3;

			end.m_x = start.m_x + pTriangle->m_faceNormal.m_x * markerLength;
			end.m_y = start.m_y + pTriangle->m_faceNormal.m_y * markerLength;
			end.m_z = start.m_z + pTriangle->m_faceNormal.m_z * markerLength;

			glVertex3f(start.m_x, start.m_y, start.m_z);
			glVertex3f(end.m_x, end.m_y, end.m_z);
		}

		glEnd();
	}
}

void drawVertexNormals()
{
	if (renderModes[RENDERMODE_VERTEXNORMALS])
	{
		const float color_black[] = { 0.0, 0.0, 0.0, 0.0 };

		glBegin(GL_LINES);
		glMaterialfv(GL_FRONT, GL_AMBIENT, color_black);
		for (size_t i = 0; i < allTriangles.size(); i++)
		{
			CTriangle *pTriangle = allTriangles[i];
			for (int v = 0; v < 3; v++)
			{
				CVector start;
				CVector end;

				start.m_x = pTriangle->m_p[v].m_x;
				start.m_y = pTriangle->m_p[v].m_y;
				start.m_z = pTriangle->m_p[v].m_z;

				end.m_x = pTriangle->m_p[v].m_x + pTriangle->m_vertexNormals[v].m_x * markerLength;
				end.m_y = pTriangle->m_p[v].m_y + pTriangle->m_vertexNormals[v].m_y * markerLength;
				end.m_z = pTriangle->m_p[v].m_z + pTriangle->m_vertexNormals[v].m_z * markerLength;

				glVertex3f(start.m_x, start.m_y, start.m_z);
				glVertex3f(end.m_x, end.m_y, end.m_z);
			}
		}
		glEnd();
	}
}

void drawCrinkles()
{
	const GLfloat cubeSize = 0.1f;
	if (renderModes[RENDERMODE_CRINKLES])
	{
		const float color_pink[] = { 1.0, 0.0, 0.0, 1.0 };

		glBegin(GL_LINES);
		glMaterialfv(GL_FRONT, GL_AMBIENT, color_pink);
		for (size_t i = 0; i < allTriangles.size(); i++)
		{
			CTriangle *pTriangle = allTriangles[i];
			for (int v = 0; v < 3; v++)
			{
				if (pTriangle->m_Crinkled[v])
				{
					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z + cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z + cubeSize);

					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z + cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z + cubeSize);

					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z + cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z + cubeSize);

					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z + cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z + cubeSize);

					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z + cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x - cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z + cubeSize);

					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z - cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y - cubeSize, pTriangle->m_p[v].m_z + cubeSize);
					glVertex3f(pTriangle->m_p[v].m_x + cubeSize, pTriangle->m_p[v].m_y + cubeSize, pTriangle->m_p[v].m_z + cubeSize);
				}
			}
		}
		glEnd();
	}
}

void setObserver()
{	
	// set the observer 
	// he's in position in (0,0,-10)
	// he's looking on (0,0,0)
	// he's in (0,1,0) direction
	gluLookAt(0, -60.0, 0, 0, 0, 0, 0, 0, 1.0);

	glRotatef(viewRotation[0], 1.0, 0.0, 0.0);
	glRotatef(viewRotation[2], 0.0, 0.0, 1.0);	
}

void renderScene() // this function is called when you need to redraw the scene 
{
	GLenum err;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the scene

	setProjectionMatrix((int)gWidth, (int)gHeight, zoomFactor);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	

	setObserver();

	drawModel();
	drawFaceNormals();
	drawVertexNormals();
	drawCrinkles();
	
	glFlush();
	glutSwapBuffers();

	while ((err = glGetError()) != GL_NO_ERROR)
	{
		cout << "GL ERROR: " << err << endl;
	}
}

// this function is called when the window is resized
void reshapeScene(int w, int h)
{
	glMatrixMode(GL_PROJECTION); // enter in projection matrix mode
	glLoadIdentity(); // load the identity, to reset transformations
	glViewport(0, 0, w, h); // set the viewport (window) size
	// apply a perspective
	// 1: the "visual angle"
	// 2: the viewport ratio
	// 3: the minimum distance of view
	// 4: the maximum distance of view
	gluPerspective(20, (double)w / (double)h, 0.01, 1000.0);
	glMatrixMode(GL_MODELVIEW); // revert to model view
	gWidth = (GLfloat)w;
	gHeight = (GLfloat)h;
}

void setRenderMode(int mode, bool value)
{
	renderModes[mode] = value;
}

void toggleRenderMode(int mode)
{
	setRenderMode(mode, !renderModes[mode]);
}

void printRenderModes()
{
	cout << "------- RENDERMODES -------" << endl;
	cout << "[H]eatmap: " << (renderModes[RENDERMODE_HEATMAP] ? "ON" : "OFF") << endl;
	cout << "[W]ireframe: " << (renderModes[RENDERMODE_WIREFRAME] ? "ON" : "OFF") << endl;
	cout << "[S]hading: " << (renderModes[RENDERMODE_SHADING] ? "ON" : "OFF") << endl;
	cout << "[F]acenormals: " << (renderModes[RENDERMODE_FACENORMALS] ? "ON" : "OFF") << endl;
	cout << "[V]ertexnormals: " << (renderModes[RENDERMODE_VERTEXNORMALS] ? "ON" : "OFF") << endl;
	cout << "[C]rinkles: " << (renderModes[RENDERMODE_CRINKLES] ? "ON" : "OFF") << endl;
	cout << "---------------------------" << endl;
}

void keyboard(unsigned char key, int x, int y)
{
	bool updateRenderScene = true;
	switch (key)
	{
	case 'h': toggleRenderMode(RENDERMODE_HEATMAP); break;
	case 'w':
	{
		toggleRenderMode(RENDERMODE_WIREFRAME);
		glPolygonMode(GL_FRONT_AND_BACK, renderModes[RENDERMODE_WIREFRAME] ? GL_LINE : GL_FILL);
	} break;
	case 's': toggleRenderMode(RENDERMODE_SHADING); break;
	case 'f': toggleRenderMode(RENDERMODE_FACENORMALS); break;
	case 'v': toggleRenderMode(RENDERMODE_VERTEXNORMALS); break;
	case 'c': toggleRenderMode(RENDERMODE_CRINKLES); break;
	default:
		updateRenderScene = false;
	}

	if (updateRenderScene)
	{
		printRenderModes();
		renderScene();
	}
}

void mouse(int button, int state, int x, int y)
{
	const float deltaWheel = 1.0;
	if (button == 3) // wheel 
	{
		zoomFactor += zoomFactorDelta;
		buttonState = 0;
		if (zoomFactor > zoomFactorMax)
		{
			zoomFactor = zoomFactorMax;
		}
	}
	else if (button == 4)
	{
		zoomFactor -= zoomFactorDelta;
		buttonState = 0;
		if (zoomFactor < zoomFactorMin)
		{
			zoomFactor = zoomFactorMin;
		}

	}
	else
	{
		float dx, dy;
		dx = (float)(x - ox);
		dy = (float)(y - oy);
		if (state == GLUT_DOWN)
		{
			buttonState |= 1 << button;
		}
		else if (state == GLUT_UP)
		{
			buttonState = 0;
		}
		ox = x;
		oy = y;
	}

	glutPostRedisplay();
}

void motion(int x, int y)
{
	float dx, dy;
	dx = (float)(x - ox);
	dy = (float)(y - oy);

	if (buttonState == 2)
	{
		// middle = translate
		viewTranslation[0] += dx / 100.0f;
		viewTranslation[1] -= dy / 100.0f;
	}
	else if (buttonState == 1)
	{
		// left = rotate
		viewRotation[0] += dy / 5.0f;
		viewRotation[2] += dx / 5.0f;
	}

	ox = x;
	oy = y;
	glutPostRedisplay();
}


void init() // called to glEnable features or to init display lists
{
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST); // to enable when you draw in 3D
	glEnable(GL_CULL_FACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
 	glEnable(GL_LIGHTING);
 	glEnable(GL_LIGHT0);
	glClearColor( 0.0f,0.0f,0.3f,0.0f );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glDisable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv); // init OpenGL
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA | GLUT_DEPTH); // set the display mode
	glutInitWindowSize((int)gWidth, (int)gHeight); // set the window size
	glutInitWindowPosition(100, 100);               // window location
	int handle = glutCreateWindow("ViewSTL"); // set the window title

	glutReshapeFunc(reshapeScene); // set the callback to reshape
	glutDisplayFunc(renderScene); // set the callback to render

	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(motion);
	init();

	map<string, CVertex *> vertices;

	read_stl(".\\Upper.stl", allTriangles, vertices);
	read_stl(".\\Lower.stl", allTriangles, vertices);
	calcVertexNormals(allTriangles);
	normalizeVertices(allTriangles);

	printRenderModes();

	glutMainLoop();
}
