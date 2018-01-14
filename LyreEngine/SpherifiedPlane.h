#pragma once 

class SpherifiedCube;

#define HEIGHTMAP_RESOLUTION 64

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

	std::optional<
		std::array<DirectX::XMFLOAT4, HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION>
	> m_terrainMap;
	
	bool m_divided;
	std::array<std::unique_ptr<SpherifiedPlane>, 4> m_children;

	DirectX::XMFLOAT3 uv2pos(float u, float v) const;
	void loadTopology(std::vector<DirectX::XMFLOAT4>& terrain, std::vector<DWORD>& indices);

public:
	SpherifiedPlane(SpherifiedCube* sphere, DWORD4 points, SpherifiedPlane* fatherPlane = nullptr);
	void divide(int depth = 1);
	void generateTerrain();
};
