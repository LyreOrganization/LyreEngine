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
	float lod : LOD;
};

float3 GetPlaneMidpointPos(VS_INPUT plane) {
	return MapFaceUVToSphere(
		// multiply by 2 and add 1 to get midpoint of plane
		(float2)(2 * plane.pos + 1) /
		(float)(2 << plane.level),
		// take only first byte from faceAndFlags to extract face index
		plane.faceAndFlags & 0xff);
}

float ComputePatchLOD(float3 midPoint, int level) {
	float dist = length(PlanetPos + midPoint*Radius - Position);
	[flatten] if (dist > MaxDistance) return 0.f;
	dist = clamp(dist, MinDistance, MaxDistance);
	return lerp(MinLOD, MaxLOD, 500.f / (dist * (float)(1 << level)));
}

VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = input.pos;
	output.faceAndFlags = input.faceAndFlags;
	output.level = input.level;
	output.lod = ComputePatchLOD(GetPlaneMidpointPos(input), input.level);
	return output;
}