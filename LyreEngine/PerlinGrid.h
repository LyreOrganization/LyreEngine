#pragma once

#define PERLIN_GRID_SIZE_RATIO 8
#define PERLIN_GRID_SIZE (1 << PERLIN_GRID_SIZE_RATIO)
#define PERLIN_GRID_FILTER (PERLIN_GRID_SIZE - 1)

class PerlinGrid final {
	unsigned m_seed;
	std::vector<int> m_perlinPermutations;

public:
	PerlinGrid(unsigned seed);
	float perlinNoise(DirectX::XMFLOAT3 loc) const;
	DirectX::XMFLOAT4 perlinNoiseWithDerivative(DirectX::XMFLOAT3 loc) const;
};

