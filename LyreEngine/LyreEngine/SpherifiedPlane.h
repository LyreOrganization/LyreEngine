 #pragma once 

class SpherifiedCube;

typedef std::array<DWORD, 4> DWORD4;

class SpherifiedPlane
{
public:
	static constexpr DWORD nextIdx(DWORD index) { return ((index + 1) % 4); }		// next clockwise element index (of 4)
	static constexpr DWORD previousIdx(DWORD index) { return ((index + 3) % 4); }	// previous clockwise element index (of 4)
	static constexpr DWORD oppositeIdx(DWORD index) { return ((index + 2) % 4); }	// opposite element index (of 4)

private:
	friend class SpherifiedCube;

	SpherifiedCube* m_pSphere;
	DWORD4 m_points;
	DirectX::XMFLOAT3 m_normal;

	SpherifiedPlane* m_pParent;
	std::array<SpherifiedPlane*, 4> m_neighbours;
	std::array<std::optional<DWORD>, 4> m_halfs;
	
	bool m_divided;
	std::array<std::unique_ptr<SpherifiedPlane>, 4> m_children;

public:
	SpherifiedPlane(SpherifiedCube* sphere, DWORD4 points, SpherifiedPlane* fatherPlane = nullptr);
	void divide(int depth = 1);
	std::vector<DWORD> getIndicesBuffer();
	/*
	int getTrianglesAmount();
	DWORD getTrianPointIndex(int pointInd);
	DWORD getHalfIndex(int halfInd);
	*/
};
