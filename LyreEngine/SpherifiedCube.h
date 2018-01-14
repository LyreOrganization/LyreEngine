#pragma once

#include "SpherifiedPlane.h"

class SpherifiedCube final {
public:
	struct Vertex {
		DirectX::XMFLOAT3 position;
	};

	std::vector<DWORD> indices;
	std::vector<DirectX::XMFLOAT4> terrain;

private:
	float m_radius;
	std::array<std::unique_ptr<SpherifiedPlane>, 6> m_cube;
	std::vector<Vertex> m_vertices;

	void buildCube();

public:
	SpherifiedCube(float radius);
	~SpherifiedCube();
	void divide(unsigned depth);
	DWORD createHalf(DWORD point1, DWORD point2);
	DWORD createMidpoint(const SpherifiedPlane::DWORD4& points);
	const std::vector<Vertex>& vertices();
	void distort();
	void applyTopology();
	float getRadius() const;
};
