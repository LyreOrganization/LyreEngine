#pragma once

#include "SpherifiedPlane.h"

class SpherifiedCube
{
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
	};

private:
	float m_radius;

	std::array<std::unique_ptr<SpherifiedPlane>, 6> m_cube;

	std::vector<Vertex> m_vertices;
	std::vector<DWORD> m_indices;

	void buildCube();

	//void buildTrianVIndsR(SpherifiedPlane* trian);
	//void buildTrianCPIndsR(SpherifiedPlane* trian);
	//Vertex* getVertexByIndex(DWORD index);

public:
	SpherifiedCube(float radius);
	void divide(unsigned depth);
	/*
	void rebuildVertexIndicesBuffer();
	void rebuildCPIndicesBuffer();
	//creates a point between two others
	SpherifiedPoint* createHalf(SpherifiedPoint* point1, SpherifiedPoint* point2);
	Vertex* getVertexByTrianPoint(SpherifiedPoint *point);
	const std::vector<Vertex>& getVertices();
	void fillAllTriangleVertices(std::vector<Vertex>& allTriangleVertices);
	const std::vector<DWORD>& getIndices();
	void updatePlanesAmount();
	int getTrianPlanesAmount();
	int getTrianPointsAmount();
	int getIndicesAmount();
	*/
};
