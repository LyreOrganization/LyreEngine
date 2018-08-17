#pragma once 

class SpherifiedCube;

class TerrainMap;

class SpherifiedPlane final {
	friend class SpherifiedCube;

public:
	enum NeighbourPatchDivision : UINT {
		EquallyDivided = 0,
		MoreDivided,
		LessDivided
	};

	typedef std::array<DWORD, 4> DWORD4;

private:
	SpherifiedCube* m_pSphere;
	DWORD4 m_points;
	DirectX::XMFLOAT3 m_normal;

	SpherifiedPlane* m_pParent;
	std::array<SpherifiedPlane*, 4> m_neighbours;
	std::array<std::optional<DWORD>, 4> m_halfs;
	DWORD m_middle;

	std::unique_ptr<TerrainMap> m_pTerrainMap;

	bool m_divided;
	int m_level;
	std::array<std::unique_ptr<SpherifiedPlane>, 4> m_children;

	void loadTopology(std::vector<DirectX::XMFLOAT4>& terrain,
					  std::vector<DWORD>& indices,
					  std::vector<NeighbourPatchDivision>& neighboursInfo,
					  std::vector<int>& divisionInfo);

	bool tryDivide(int depth = 1);
	bool tryUndivide();

public:
	SpherifiedPlane(SpherifiedCube* sphere, DWORD4 points, SpherifiedPlane* fatherPlane = nullptr);
};
