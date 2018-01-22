#pragma once 

class SpherifiedCube;

class TerrainMap;

class SpherifiedPlane final {
	friend class SpherifiedCube;

	typedef std::array<DWORD, 4> DWORD4;

	SpherifiedCube* m_pSphere;
	DWORD4 m_points;
	DirectX::XMFLOAT3 m_normal;

	SpherifiedPlane* m_pParent;
	std::array<SpherifiedPlane*, 4> m_neighbours;
	std::array<std::optional<DWORD>, 4> m_halfs;
	DWORD m_middle;

	std::unique_ptr<TerrainMap> m_pTerrainMap;
	
	bool m_divided;
	std::array<std::unique_ptr<SpherifiedPlane>, 4> m_children;

	void loadTopology(std::vector<DirectX::XMFLOAT4>& terrain, std::vector<DWORD>& indices);

	void divide(int depth = 1);

public:
	SpherifiedPlane(SpherifiedCube* sphere, DWORD4 points, SpherifiedPlane* fatherPlane = nullptr);
};
