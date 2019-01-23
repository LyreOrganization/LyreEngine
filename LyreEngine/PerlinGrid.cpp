#include "stdafx.h"

#include "PerlinGrid.h"

using namespace std;
using namespace DirectX;

namespace {

	inline float quinticSmooth(float x) {
		return x*x*x*(x*(6.f*x - 15.f) + 10.f);
	}

	inline float quinticSmoothDerivative(float x) {
		return 30.f * x*x*(x - 1.f)*(x - 1.f);
	}

	inline float gridGrad(int hash, float x, float y, float z) {
		switch (hash & 0xF) {
		case 0xC:
		case 0x0: return  x + y;
		case 0xE:
		case 0x1: return -x + y;
		case 0x2: return  x - y;
		case 0x3: return -x - y;
		case 0x4: return  x + z;
		case 0x5: return -x + z;
		case 0x6: return  x - z;
		case 0x7: return -x - z;
		case 0x8: return  y + z;
		case 0xD:
		case 0x9: return -y + z;
		case 0xA: return  y - z;
		case 0xF:
		case 0xB: return -y - z;
		default: return 0.f; // never happens
		}
	}

}

PerlinGrid::PerlinGrid(unsigned seed)
	:m_seed(seed) {

	m_perlinPermutations.resize(PERLIN_GRID_SIZE * 2);
	for (int i = 0; i < PERLIN_GRID_SIZE; i++) {
		m_perlinPermutations[i] = i;
	}
	shuffle(m_perlinPermutations.begin(), m_perlinPermutations.begin() + PERLIN_GRID_SIZE, default_random_engine(m_seed));
	copy_n(m_perlinPermutations.begin(), PERLIN_GRID_SIZE, m_perlinPermutations.begin() + PERLIN_GRID_SIZE);
}

float PerlinGrid::perlinNoise(DirectX::XMFLOAT3 loc) const {
	XMINT3 locInt {
		static_cast<int>(floor(loc.x)),
		static_cast<int>(floor(loc.y)),
		static_cast<int>(floor(loc.z))
	};
	float x = loc.x - static_cast<float>(locInt.x); float x1 = x - 1.f;
	float y = loc.y - static_cast<float>(locInt.y); float y1 = y - 1.f;
	float z = loc.z - static_cast<float>(locInt.z); float z1 = z - 1.f;
	locInt.x &= PERLIN_GRID_FILTER;
	locInt.y &= PERLIN_GRID_FILTER;
	locInt.z &= PERLIN_GRID_FILTER;

	//hash coordinates
	int A = m_perlinPermutations[locInt.x] + locInt.y,
		AA = m_perlinPermutations[A] + locInt.z,
		AAA = m_perlinPermutations[AA],
		AAB = m_perlinPermutations[AA + 1],
		AB = m_perlinPermutations[A + 1] + locInt.z,
		ABA = m_perlinPermutations[AB],
		ABB = m_perlinPermutations[AB + 1],
		B = m_perlinPermutations[locInt.x + 1] + locInt.y,
		BA = m_perlinPermutations[B] + locInt.z,
		BAA = m_perlinPermutations[BA],
		BAB = m_perlinPermutations[BA + 1],
		BB = m_perlinPermutations[B + 1] + locInt.z,
		BBA = m_perlinPermutations[BB],
		BBB = m_perlinPermutations[BB + 1];

	float sx = quinticSmooth(x);
	float sy = quinticSmooth(y);
	float sz = quinticSmooth(z);

	float g000 = GridGrad(AAA, x, y, z);
	float g100 = GridGrad(BAA, x1, y, z);
	float g010 = GridGrad(ABA, x, y1, z);
	float g110 = GridGrad(BBA, x1, y1, z);
	float g001 = GridGrad(AAB, x, y, z1);
	float g101 = GridGrad(BAB, x1, y, z1);
	float g011 = GridGrad(ABB, x, y1, z1);
	float g111 = GridGrad(BBB, x1, y1, z1);

	float k100 = g100 - g000;
	float k010 = g010 - g000;
	float k001 = g001 - g000;
	float k110 = g110 - g010 - g100 + g000;
	float k101 = g101 - g001 - g100 + g000;
	float k011 = g011 - g001 - g010 + g000;
	float k111 = g111 - g011 - g101 + g001 - k110;

	return g000 + (sx*k100 + sy*k010 + sz*k001 +
				   sx*sy*k110 + sx*sz*k101 + sy*sz*k011 +
				   sx*sy*sz*k111);
}

XMFLOAT4 PerlinGrid::perlinNoiseWithDerivative(XMFLOAT3 loc) const {
	XMINT3 locInt {
		static_cast<int>(floor(loc.x)),
		static_cast<int>(floor(loc.y)),
		static_cast<int>(floor(loc.z))
	};
	float x = loc.x - static_cast<float>(locInt.x); float x1 = x - 1.f;
	float y = loc.y - static_cast<float>(locInt.y); float y1 = y - 1.f;
	float z = loc.z - static_cast<float>(locInt.z); float z1 = z - 1.f;
	locInt.x &= PERLIN_GRID_FILTER;
	locInt.y &= PERLIN_GRID_FILTER;
	locInt.z &= PERLIN_GRID_FILTER;

	//hash coordinates
	int A = m_perlinPermutations[locInt.x] + locInt.y,
		AA = m_perlinPermutations[A] + locInt.z,
		AAA = m_perlinPermutations[AA],
		AAB = m_perlinPermutations[AA + 1],
		AB = m_perlinPermutations[A + 1] + locInt.z,
		ABA = m_perlinPermutations[AB],
		ABB = m_perlinPermutations[AB + 1],
		B = m_perlinPermutations[locInt.x + 1] + locInt.y,
		BA = m_perlinPermutations[B] + locInt.z,
		BAA = m_perlinPermutations[BA],
		BAB = m_perlinPermutations[BA + 1],
		BB = m_perlinPermutations[B + 1] + locInt.z,
		BBA = m_perlinPermutations[BB],
		BBB = m_perlinPermutations[BB + 1];

	float sx = quinticSmooth(x);
	float sy = quinticSmooth(y);
	float sz = quinticSmooth(z);

	float g000 = GridGrad(AAA, x, y, z);
	float g100 = GridGrad(BAA, x1, y, z);
	float g010 = GridGrad(ABA, x, y1, z);
	float g110 = GridGrad(BBA, x1, y1, z);
	float g001 = GridGrad(AAB, x, y, z1);
	float g101 = GridGrad(BAB, x1, y, z1);
	float g011 = GridGrad(ABB, x, y1, z1);
	float g111 = GridGrad(BBB, x1, y1, z1);

	float k100 = g100 - g000;
	float k010 = g010 - g000;
	float k001 = g001 - g000;
	float k110 = g110 - g010 - g100 + g000;
	float k101 = g101 - g001 - g100 + g000;
	float k011 = g011 - g001 - g010 + g000;
	float k111 = g111 - g011 - g101 + g001 - k110;

	XMFLOAT4 result;

	result.x = quinticSmoothDerivative(x) * (k100 + sy*k110 + sz*k101 + sy*sz*k111);

	result.y = quinticSmoothDerivative(y) * (k010 + sx*k110 + sz*k011 + sx*sz*k111);

	result.z = quinticSmoothDerivative(z) * (k001 + sx*k101 + sy*k011 + sx*sy*k111);

	result.w = g000
		+ sx*k100 + sy*k010 + sz*k001
		+ sx*sy*k110
		+ sx*sz*k101
		+ sy*sz*k011
		+ sx*sy*sz*k111;

	return result;
}
