#include <sphere_mapping_b0.fx>

cbuffer Planet : register(b0) {
	float3 PlanetPos;
	float Radius;
}

cbuffer TerrainMapConfig : register(b1) {
	int2 Pos;
	int Face;
	int Level;
	int Region;
	float StartOctave;
	float StartAmplitude;
	float StartShift;
}

Buffer<int> PerlinPermutations : register(t0);

Texture1D<float> QuinticSmoothLT : register(t1);

SamplerState PointSampler : register(s0);
SamplerState LinearSampler : register(s1);

#define DEPTH 6
#define RESOLUTION ((1 << DEPTH) + 1)
#define HALFRES (1 << (DEPTH - 1))

// Parent
// RESOLUTION x RESOLUTION x 4 (only height)
RWTexture2DArray<float> ParentBaseRegions : register(u8); // start from 8
// RESOLUTION x 4 x 4
RWTexture2DArray<float4> ParentEdgesBackup : register(u9);

// This map
// RESOLUTION x RESOLUTION
RWTexture2D<float4> Map : register(u10);
// RESOLUTION x 4
RWTexture2D<float4> EdgesBackup : register(u11);

// Temporary positions storage
// RESOLUTION x RESOLUTION
RWTexture2D<float4> Positions : register(u12);

// For children
// RESOLUTION x RESOLUTION x 4 (only height)
RWTexture2DArray<float> BaseRegions : register(u13);
// RESOLUTION x 4 x 4
RWTexture2DArray<float4> RegionsEdgesBackup : register(u);


float GridGrad(int hash, float x, float y, float z) {
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
	default: return 0; //never happens
	}
}

float PerlinNoise(float3 loc) {
	int3 locInt = (int3)floor(loc);
	float x = loc.x - (float)locInt.x; float x1 = x - 1.f;
	float y = loc.y - (float)locInt.y; float y1 = y - 1.f;
	float z = loc.z - (float)locInt.z; float z1 = z - 1.f;

	locInt &= 255;

	//hash coordinates
	int A = PerlinPermutations[locInt.x] + locInt.y,
		AA = PerlinPermutations[A] + locInt.z,
		AAA = PerlinPermutations[AA],
		AAB = PerlinPermutations[AA + 1],
		AB = PerlinPermutations[A + 1] + locInt.z,
		ABA = PerlinPermutations[AB],
		ABB = PerlinPermutations[AB + 1],
		B = PerlinPermutations[locInt.x + 1] + locInt.y,
		BA = PerlinPermutations[B] + locInt.z,
		BAA = PerlinPermutations[BA],
		BAB = PerlinPermutations[BA + 1],
		BB = PerlinPermutations[B + 1] + locInt.z,
		BBA = PerlinPermutations[BB],
		BBB = PerlinPermutations[BB + 1];

	float sx = QuinticSmoothLT.SampleLevel(LinearSampler, x, 0);
	float sy = QuinticSmoothLT.SampleLevel(LinearSampler, y, 0);
	float sz = QuinticSmoothLT.SampleLevel(LinearSampler, z, 0);

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
	float k111 = g111 - g011 - g101 + g001 - g110 + g010 + g100 - g000;

	return g000
		+ sx*k100 + sy*k010 + sz*k001
		+ sx*sy*k110
		+ sx*sz*k101
		+ sy*sz*k011
		+ sx*sy*sz*k111;
}
