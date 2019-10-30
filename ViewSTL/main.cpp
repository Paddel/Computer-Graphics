// Teile aus https://gist.github.com/kopiro/4414350
// und http://www.sgh1.net/posts/read-stl-file.md
// und http://users.polytech.unice.fr/~buffa/cours/synthese_image/DOCS/Tutoriaux/Nehe/opengl.htm
// und https://www.opengl.org/archives/resources/code/samples/glut_examples/examples/highlight.c

#include <map>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

using namespace std;


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

	bool operator< (const CVector& Other) const
	{
		return m_x < Other.m_x;
	}

	bool operator> (const CVector& Other) const
	{
		return m_x > Other.m_x;
	}

	bool operator== (const CVector& Other) const
	{
		return m_x == Other.m_x && m_y == Other.m_y && m_z == Other.m_z;
	}
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

	CTriangle();
	CTriangle(const CVector& p1, const CVector& p2, const CVector& p3, const CVector& normal):
		m_faceNormal(normal)
	{
		m_p[0] = p1;
		m_p[1] = p2;
		m_p[2] = p3;
	}
	~CTriangle()
	{		
	}

	CVector m_p[3], m_faceNormal;
	CVector *m_vertexNormals[3];


};
// Nothing special here, so I won't waste your bandwidth with the implementations. Note that we won't store the triangle's normal, so we don't need to read that from the STL file.
// 
// Reading the STL File
// Once we understand the STL file format, and have a vector class which can handle most of the binary data reading, it's not much work to put together a function to read a whole STL file. Here, we'll read an STL file into a std::vector of tri's

void calc_vertexNormals(vector <CTriangle>&triangles, map<CVector, CVector>& vertices)
{
	cout << "Calculating vertex normals for " << triangles.size() << " triangles" << endl;

	for (int t = 0; t < triangles.size(); t++)
	{
		for (int v = 0; v < 3; v++)
		{
			triangles[t].m_vertexNormals[v]->m_x += triangles[t].m_faceNormal.m_x;
			triangles[t].m_vertexNormals[v]->m_y += triangles[t].m_faceNormal.m_y;
			triangles[t].m_vertexNormals[v]->m_z += triangles[t].m_faceNormal.m_z;
		}
	}

	for(auto it = vertices.begin(); it != vertices.end(); it++)
	{
		CVector *vertex = &it->second;
		GLfloat w = sqrt(vertex->m_x * vertex->m_x + vertex->m_y * vertex->m_y + vertex->m_z * vertex->m_z);
		vertex->m_x /= w;
		vertex->m_y /= w;
		vertex->m_z /= w;
	}

	//for (int ta = 0; ta < triangles.size(); ta++)
	//{
	//	if(ta % 100 == 0)
	//	cout << "Triangle " << ta << "/" << triangles.size() << "=" << ta/(float)triangles.size() << endl;
	//	for (int va = 0; va < 3; va++)
	//	{
	//		for (int tb = 0; tb < triangles.size(); tb++)
	//		{
	//			for (int vb = 0; vb < 3; vb++)
	//			{
	//				if (memcmp(&triangles[ta].m_p[va], &triangles[tb].m_p[vb], sizeof(CVector)) == 0)
	//				{
	//					//cout << "Identical Vertex found: Triangle " << ta << " vertex " << va << " & Triangle " << tb << " vertex " << vb << endl;
	//					triangles[ta].m_vertexNormals[va].m_x += triangles[tb].m_faceNormal.m_x;
	//					triangles[ta].m_vertexNormals[va].m_y += triangles[tb].m_faceNormal.m_y;
	//					triangles[ta].m_vertexNormals[va].m_z += triangles[tb].m_faceNormal.m_z;
	//				}
	//			}
	//		}

	//		//normalisieren
	//		float w = sqrt(triangles[ta].m_vertexNormals[va].m_x * triangles[ta].m_vertexNormals[va].m_x + triangles[ta].m_vertexNormals[va].m_y * triangles[ta].m_vertexNormals[va].m_y + triangles[ta].m_vertexNormals[va].m_z * triangles[ta].m_vertexNormals[va].m_z);
	//		triangles[ta].m_vertexNormals[va].m_x /= w;
	//		triangles[ta].m_vertexNormals[va].m_y /= w;
	//		triangles[ta].m_vertexNormals[va].m_z /= w;
	//	}
	//}
}

void read_stl(string fname, vector <CTriangle>&v, map<CVector, CVector>& vertices) {

	//!!
	//don't forget ios::binary
	//!!
	ifstream myFile(
		fname.c_str(), ios::in | ios::binary);

	char header_info[80] = "";
	char nTri[4];
	unsigned long nTriLong;

	//read 80 byte header
	if (myFile) {
		myFile.read(header_info, 80);
		cout << "header: " << header_info << endl;
	}
	else {
		cout << "error" << endl;
	}

	//read 4-byte ulong
	if (myFile) {
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
			CTriangle triangle(p1, p2, p3, normal);

			char *tmpFace = nullptr;
			for (int i = 0; i < 3; i++)
			{
				CVector p = i == 0 ? p1 : (i == 1 ? p2 : p3);
				tmpFace = new char[VECTOR_BINARY_LEN];
				memcpy(tmpFace, facet + VECTOR_BINARY_LEN * i, sizeof(char) * VECTOR_BINARY_LEN);

				/*map<CVector, CVector>::iterator it = vertices.find(p);
				if (it == vertices.end())*/
				map<CVector, CVector>::iterator it = vertices.insert(make_pair(p, CVector())).first;
				triangle.m_vertexNormals[i] = &it->second;
			}

			v.push_back(triangle);
		}
	}

	return;

}

// hier haben wir alle unsere Dreiecke reinkopiert:
vector <CTriangle> allTriangles;


#if 1
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
//float invViewMatrix[12];
// -----------------------------------------for navigation 

void setProjectionMatrix(int width, int height, double zoomFactor)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	gluPerspective(20.0*zoomFactor, (float)width / (float)height, zNear, zFar);
}

void renderScene() // this function is called when you need to redraw the scene 
{
	GLenum err;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the scene

	setProjectionMatrix((int)gWidth, (int)gHeight, zoomFactor);


	// navigation ---------------------------------------------------
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	

	// set the observer 
	// he's in position in (0,0,-10)
	// he's looking on (0,0,0)
	// he's in (0,1,0) direction
	gluLookAt(0, -60.0, 0, 0, 0, 0, 0, 0, 1.0);

	glRotatef(viewRotation[0], 1.0, 0.0, 0.0);
	glRotatef(viewRotation[2], 0.0, 0.0, 1.0);

	// NOW HERE YOU CAN DRAW WHAT YOU WANT! :) --------------------------------------------------------------

 	glColorMaterial(GL_FRONT, GL_AMBIENT);
 	glColorMaterial(GL_FRONT, GL_DIFFUSE);
 	glColorMaterial(GL_FRONT, GL_SPECULAR);
 
  	glMaterialfv(GL_FRONT, GL_AMBIENT , mat_ambient);
 	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

	glBegin( GL_TRIANGLES );
	
	for (size_t i = 0; i < allTriangles.size(); i++)
	{
		glNormal3f(allTriangles[i].m_vertexNormals[0]->m_x, allTriangles[i].m_vertexNormals[0]->m_y, allTriangles[i].m_vertexNormals[0]->m_z);
		glVertex3f(allTriangles[i].m_p[0].m_x, allTriangles[i].m_p[0].m_y, allTriangles[i].m_p[0].m_z);
		glNormal3f(allTriangles[i].m_vertexNormals[1]->m_x, allTriangles[i].m_vertexNormals[1]->m_y, allTriangles[i].m_vertexNormals[1]->m_z);
		glVertex3f(allTriangles[i].m_p[1].m_x, allTriangles[i].m_p[1].m_y, allTriangles[i].m_p[1].m_z);
		glNormal3f(allTriangles[i].m_vertexNormals[2]->m_x, allTriangles[i].m_vertexNormals[2]->m_y, allTriangles[i].m_vertexNormals[2]->m_z);
		glVertex3f(allTriangles[i].m_p[2].m_x, allTriangles[i].m_p[2].m_y, allTriangles[i].m_p[2].m_z);
	}
	glEnd();

	//glDisable(GL_COLOR_MATERIAL);

	glFlush();

	// -------------------------------------------------------------------------------------------------------

	// swap the buffer on the screen (real draw)
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

// void selectObject(int x, int y)
// {
// 	GLuint buff[64] = { 0 }; // the buffer
// 	GLint hits, view[4];
// 
// 	glSelectBuffer(64, buff); // This choose the buffer where store the values for the selection data
// 	glGetIntegerv(GL_VIEWPORT, view); // This retrieve info about the viewport
// 	glRenderMode(GL_SELECT); // Switching in selecton mode
// 	glInitNames(); // This stack contains all the info about the objects
// 	glPushName(0); // Now fill the stack with one element (or glLoadName will generate an error)
// 
// 	// Now modify the vieving volume, restricting selection area around the cursor
// 	glMatrixMode(GL_PROJECTION);
// 	glPushMatrix();
// 
// 	glLoadIdentity();
// 	gluPickMatrix(x, y, 1.0, 1.0, view); // restrict the draw to an area around the cursor
// 
// 	// [IMPORTANT] apply the same perspective in the reshapeFunc
// 	gluPerspective(20, view[2] * 1.0f / view[3], 0.1f, 100);
// 	glMatrixMode(GL_MODELVIEW); // Draw the objects onto the screen
// 
// 	glutSwapBuffers(); // draw only the names in the stack, and fill the array
// 	renderScene(); // render
// 
// 	glMatrixMode(GL_PROJECTION); // Do you remember? We do pushMatrix in PROJECTION mode
// 	glPopMatrix();
// 
// 	hits = glRenderMode(GL_RENDER); // get number of objects drawed in that area and return to render mode
// 
// 	// NOW 
// 	// hits: number of hits
// 	// buff[ (hits*x)+3] the ID
// 	std::cout << "Hits: " << hits << "\n" << "ID: " << buff[3] << "\n";
// 
// 	glMatrixMode(GL_MODELVIEW); // and revert again
// }

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

#endif

int main(int argc, char** argv)
{
	glutInit(&argc, argv); // init OpenGL
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA | GLUT_DEPTH); // set the display mode
	glutInitWindowSize((int)gWidth, (int)gHeight); // set the window size
	glutInitWindowPosition(100, 100);               // window location
	int handle = glutCreateWindow("ViewSTL"); // set the window title

	// glutMotionFunc(glutMotion);        // called when the mouse moves over the screen with one of this button pressed
	glutReshapeFunc(reshapeScene); // set the callback to reshape
	glutDisplayFunc(renderScene); // set the callback to render

	//glutMouseFunc(mouseHandler);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	init(); // init

	map<CVector, CVector> vertices;


	read_stl(".\\Upper.stl", allTriangles, vertices);
	read_stl(".\\Lower.stl", allTriangles, vertices);
	calc_vertexNormals(allTriangles, vertices);

	glutMainLoop(); // enter in a loop
}
