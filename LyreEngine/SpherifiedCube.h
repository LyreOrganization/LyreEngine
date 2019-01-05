#pragma once

#include "SpherifiedPlane.h"
#include "MapLoader.h"

class SpherifiedCube final {
public:
	std::vector<DirectX::XMFLOAT4> terrain;
	std::vector<SpherifiedPlane::GPUDesc> planes;
	std::vector<DWORD> indices;

private:
	float m_radius;
	std::array<std::unique_ptr<SpherifiedPlane>, 6> m_cube;

	MapLoader m_mapLoader;

	void buildCube();

public:
	SpherifiedCube(float radius, unsigned seed);
	~SpherifiedCube();
	void applyTopology();
	float getRadius() const;

	static const DirectX::XMFLOAT4X4& getFaceRotation(int faceIdx);
};
