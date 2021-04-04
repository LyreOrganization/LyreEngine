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
	// which neighbour is this plane to its neighbour?
	// is direction of indexing the edge same for both?
	std::pair<int, bool> getNeighboursNeighbourIndexOfThis(int neighbour);

	std::unique_ptr<TerrainMap> m_pTerrainMap;


	//////////// Loading to GPU /////////////

	bool bLoad;
	bool bLoadChildren;
	// index of plane in loaded planes list
	DWORD indexToLoad;
	// load true or backuped edges?
	std::array<bool, 4> terrainEdgesToLoad;

	void prepareToLoad();
	void loadPlane(std::vector<GPUDesc>& planes,
				   DWORD& nextIndex);
	void loadIndicesAndTerrain(std::vector<DWORD>& indices,
							   std::vector<DirectX::XMFLOAT4>& terrain);


	//////////// State ////////////

	enum class State : int {
		Making = -1,	//-1 -> 0 (generate)
		Ready = 0,		// 0 -> 0 (undivide, restore), 
						//		1 (divide), 
						//		3 (erase)
		Divided = 1,	// 1 -> 2 (undivide)
		Undivided = 2,	// 2 -> 0 (clear), 
						//		1 (divide),
						//		2 (undivide)
		Deleted = 3		// 3 -> 0 (restore), 3 (erase)
	};

	class StateTransitions {
		State m_state = State::Making;
	public:
		const State& state = m_state;

		bool hasChildren();

		const State& generate();
		const State& divide();
		const State& undivide();
		const State& clear();
		const State& erase();
		const State& restore();
	} m_state;


	//////////// Children ////////////
	
	void setChildrenNeighbours();
	void resetChildrenNeighbours();
	std::array<std::unique_ptr<SpherifiedPlane>, 4> m_children;

	SpherifiedPlane(SpherifiedCube* sphere, SpherifiedPlane* fatherPlane,
					DirectX::XMINT2 childPlaneIndex);


public:
	SpherifiedPlane(SpherifiedCube* sphere, int faceIdx);

	bool tryDivide();
	bool tryUndivide();

	SpherifiedPlane(const SpherifiedPlane& another) = delete;
	SpherifiedPlane& operator=(const SpherifiedPlane& another) = delete;
	SpherifiedPlane(SpherifiedPlane&& moved) = delete;
	SpherifiedPlane& operator=(SpherifiedPlane&& moved) = delete;
	SpherifiedPlane* operator&() = delete;
};
