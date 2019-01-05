cbuffer CubeFaces : register(b0) {
	matrix PlaneRotations[6];
}

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

float3 MapFaceUVToSphere(float2 uv, int face) {
	uv = uv * 2.f - 1.f;
	float2 uvSqr = uv*uv;
	// rotate plane to its real position
	return mul(PlaneRotations[face], float4(
		uv.x*sqrt(1.f - (uvSqr.y + 1.f) / 2.f + uvSqr.y / 3.f),
		uv.y*sqrt(1.f - (uvSqr.x + 1.f) / 2.f + uvSqr.x / 3.f),
		sqrt(1.f - (uvSqr.x + uvSqr.y) / 2.f + uvSqr.x * uvSqr.y / 3.f),
		1.f)).xyz;
}

float3 GetPlaneMidpointPos(VS_INPUT plane) {
	return MapFaceUVToSphere(
		// multiply by 2 and add 1 to get midpoint of plane
		(float2)(2 * plane.pos + 1) /
		(float)(2 << plane.level),
		// take only first byte from faceAndFlags to extract face index
		plane.faceAndFlags & 0xff);
}

float ComputePatchLOD(float3 midPoint) {
	float dist = length(PlanetPos + midPoint*Radius - Position);
	[flatten] if (dist > MaxDistance) return 0.f;
	float d = 1.f - (clamp(dist, MinDistance, MaxDistance) - MinDistance) / (MaxDistance - MinDistance);
	return lerp(MinLOD, MaxLOD, pow(2.f, 7.f * d) / (float)(1 << 7));
}

VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = input.pos;
	output.faceAndFlags = input.faceAndFlags;
	output.level = input.level;
	output.lod = ComputePatchLOD(GetPlaneMidpointPos(input));
	return output;
}