 #pragma once

#define DIR_UP		true
#define DIR_DOWN	false

#define INVALID_HALF -1

class SpherifiedCube;

typedef std::array<DWORD, 4> DWORD4;

class SpherifiedPlane
{
private:
	friend class SpherifiedCube;

	SpherifiedCube* m_pSphere;
	DWORD4 m_points;
	DirectX::XMFLOAT3 m_normal;

	SpherifiedPlane* m_pParent;
	std::array<SpherifiedPlane*, 4> m_neighbours;
	std::array<std::optional<DWORD>, 4> m_halfs;
	
	bool m_divided;
	std::array<SpherifiedPlane*, 4> m_children;

public:
	SpherifiedPlane(SpherifiedCube* sphere, DWORD4 points, SpherifiedPlane* fatherPlane = nullptr);
	void divide(int depth = 1);
	/*
	void setHalf(int halfInd, SpherifiedPoint * half);
	int getTrianglesAmount();
	DWORD getTrianPointIndex(int pointInd);
	DWORD getHalfIndex(int halfInd);
	*/
};
