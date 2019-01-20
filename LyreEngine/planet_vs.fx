#include <sphere_mapping_b0.fx>

cbuffer Planet : register(b1) {
	float3 PlanetPos;
	float Radius;
}

cbuffer Camera : register(b2) {
	float3 Position;
	float _dummy_;
}

cbuffer Lod : register(b3) {
	float MinDistance;
	float MaxDistance;
	float MinLOD;
	float MaxLOD;
}

struct VS_INPUT {
	int2 pos : POSITION;
	int faceAndFlags : FACE_AND_FLAGS;
	int level : LEVEL;
};

struct VS_OUTPUT {
	int2 pos : POSITION;
	int faceAndFlags : FACE_AND_FLAGS;
	int level : LEVEL;
	float2 lod : LOD_CLAMPED;
	float lodDiff : LOD_DIFF;
};

// N nodes per 1 radian angle
#define GRID_NODES_PER_RAD 200.f

// returns if plane is visible
bool GetMaxUVViewAngles(VS_INPUT plane, out float2 angles) {
	float3 cornerViewNorms[4];
	int visibleCorners = 0;
	for (int i = 0; i < 4; i++) {
		float3 cornerNorm = MapFaceUVToSphere(
			(float2)(plane.pos + int2(
				(i == 0 || i == 3),
				(i == 0 || i == 1)))
			/ (float)(1 << plane.level),
			// take only first byte from faceAndFlags to extract face index
			plane.faceAndFlags & 0xff);
		cornerViewNorms[i] = normalize(PlanetPos + cornerNorm*Radius - Position);
		visibleCorners += (dot(cornerNorm, -cornerViewNorms[i]) > -0.1f);
	}
	angles = acos(float2(min(dot(cornerViewNorms[0], cornerViewNorms[1]),
							 dot(cornerViewNorms[3], cornerViewNorms[2])),
						 min(dot(cornerViewNorms[0], cornerViewNorms[3]),
							 dot(cornerViewNorms[1], cornerViewNorms[2]))));
	return visibleCorners > 0;
}

VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = input.pos;
	output.faceAndFlags = input.faceAndFlags;
	output.level = input.level;

	float2 angles;
	bool visible = GetMaxUVViewAngles(input, angles);
	float2 lod = angles*GRID_NODES_PER_RAD;

	output.lod = clamp(lod, MinLOD, MaxLOD);
	output.lodDiff = visible ? (MinLOD / max(lod.x, lod.y)) : -1.f;

	return output;
}