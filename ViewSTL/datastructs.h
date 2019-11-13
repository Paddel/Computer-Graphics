#pragma once
#include <vector>

class CVertex;

class CVector
{
public:

	//Notice the constructor v3(char* bin); This constructor will help us create an instance of v3 from binary data 
	// found in the STL file.Here's the implementation:
	CVector(char* facet)
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

	CVector() :
		m_x(0.f),
		m_y(0.f),
		m_z(0.f)
	{

	}

	CVector(GLfloat x, GLfloat y, GLfloat z) :
		m_x(x),
		m_y(y),
		m_z(z)
	{

	}

	~CVector()
	{
	}

	GLfloat m_x, m_y, m_z;

	CVector operator+= (const CVector  &obj)
	{
		m_x += obj.m_x;
		m_y += obj.m_y;
		m_z += obj.m_z;
		return *this;
	}

	CVector operator-= (const CVector  &obj)
	{
		m_x -= obj.m_x;
		m_y -= obj.m_y;
		m_z -= obj.m_z;
		return *this;
	}

	CVector operator/= (const float &num)
	{
		m_x /= num;
		m_y /= num;
		m_z /= num;
		return *this;
	}

	CVector operator+ (CVector const &obj) const
	{
		CVector result;
		result.m_x = m_x + obj.m_x;
		result.m_y = m_y + obj.m_y;
		result.m_z = m_z + obj.m_z;
		return result;
	}

	CVector operator- (CVector const &obj) const
	{
		CVector result;
		result.m_x = m_x - obj.m_x;
		result.m_y = m_y - obj.m_y;
		result.m_z = m_z - obj.m_z;
		return result;
	}

	CVector operator/ (float num) const
	{
		CVector result;
		result.m_x = m_x / num;
		result.m_y = m_y / num;
		result.m_z = m_z / num;
		return result;
	}
};



class CTriangle
{
public:
	CTriangle(const CVector& p1, const CVector& p2, const CVector& p3, const CVector& normal) :
		m_faceNormal(normal)
	{
		m_p[0] = p1;
		m_p[1] = p2;
		m_p[2] = p3;

		for (int i = 0; i < 3; i++)
			m_crinkled[i] = false;
	}
	~CTriangle()
	{
	}

	CVector m_p[3], m_faceNormal;
	CVertex *m_vertices[3];
	CVector m_vertexNormals[3];

	//Visualisation
	bool m_crinkled[3];
	CVector m_vertexNormalsNoEdgeDetection[3];
};

class CVertex
{
public:
	CVertex() {};
	CVertex(CTriangle *pFirst) { m_adjacentTriangles.push_back(pFirst); };

	std::vector<CTriangle *> m_adjacentTriangles;
};