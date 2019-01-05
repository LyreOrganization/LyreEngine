#pragma once 

class TerrainMap;

class SpherifiedPlane final {
	friend class SpherifiedCube;
	friend class TerrainMap;

public:
	enum NeighbourPatchDivision : int {
		EquallyDivided = 0,
		MoreDivided,
		LessDivided
	};

	struct GPUDesc {
		DirectX::XMINT2 position;
		// Face index is in first byte.
		// First 4 bits of second byte are set if 
		// corresponding neighbour is more divided.
		__int32 faceAndFlags;
		__int32 level;
	};

private:
	SpherifiedCube* m_pSphere;
	// one of 6 cube faces
	int m_faceIdx;

	int m_level;
	DirectX::XMINT2 m_planeIndex;

	SpherifiedPlane* m_pParent;
	std::array<SpherifiedPlane*, 4> m_neighbours;

	std::unique_ptr<TerrainMap> m_pTerrainMap;

	bool m_divided;
	std::array<std::unique_ptr<SpherifiedPlane>, 4> m_children;

	// index of plane in loaded planes list
	DWORD indexToLoad;
	// load true or backuped edges?
	std::array<bool, 4> terrainEdgesToLoad;
	// load plane, set index and edges to load
	void loadPlane(std::vector<GPUDesc>& planes,
				   DWORD& nextIndex);
	void loadIndicesAndTerrain(std::vector<DWORD>& indices,
							   std::vector<DirectX::XMFLOAT4>& terrain);

	bool tryDivide(int depth = 1);

public:
	SpherifiedPlane(SpherifiedCube* sphere, SpherifiedPlane* fatherPlane,
					DirectX::XMINT2 childPlaneIndex);
	SpherifiedPlane(SpherifiedCube* sphere, int faceIdx);
};
