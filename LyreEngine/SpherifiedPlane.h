#pragma once 

class SpherifiedCube;

typedef std::array<DWORD, 4> DWORD4;

class SpherifiedPlane final {
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
};
