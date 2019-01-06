cbuffer CubeFaces : register(b0) {
	matrix PlaneRotations[6];
}

cbuffer Planet : register(b1) {
	float3 PlanetPos;
	float Radius;
}

cbuffer Lighting : register(b2) {
	float3 Direction;
	float Power;
	float4 Diffuse;
}

Texture2DArray Terrain : register(t0);

SamplerState PointSampler : register(s0);
SamplerState LinearSampler : register(s1);

struct HS_OUTPUT {
	int2 pos : POSITION;
	int face : FACE;
	int level : LEVEL;
};

struct HSCF_OUTPUT {
	int edgeDivision[4] : EDGES;
	float edge[4]  :  SV_TessFactor;
	float inside[2]  :  SV_InsideTessFactor;
};

struct DS_OUTPUT {
	float3 pos : POSITION;
	float3 color : COLOR;
	float3 normal : NORMAL;
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

float3 UVToGlobal(HS_OUTPUT plane, float2 uv) {
	return MapFaceUVToSphere(
		((float2)plane.pos + uv) / 
		(float)(1 << plane.level), plane.face);
}

float3 UVToGlobalOnLodEdge(HS_OUTPUT plane, float2 uv, float lod) {
	return MapFaceUVToSphere(
		// Round everything, as neighbour plane has different lod, so the
		// computations are different, but we don't wont tiny holes on lod edges.
		round(((float2)plane.pos + uv) * lod) / 
		round((float)(1 << plane.level) * lod), plane.face);
}

#define UV_EPS 0.0001f

[domain("quad")]
DS_OUTPUT main(HSCF_OUTPUT lods,
			   float2 uv : SV_DomainLocation,
			   const OutputPatch<HS_OUTPUT, 1> patch,
			   uint PatchID : SV_PrimitiveID) {
	static const float3 COLOR_OCEAN = { 0.2f,0.3f,0.8f };
	static const float3 COLOR_SAND = { 0.6f,0.7f,0.3f };
	static const float3 COLOR_ROCK = { 0.44f,0.42f,0.4f };
	
	uv.y = 1.f - uv.y; // convert to our uv coord system

	int edgeLod = -1;
	[flatten] if (abs(uv.x) < UV_EPS) //left edge
		edgeLod = lods.edgeDivision[3];
	[flatten] if (abs(uv.x - 1.f) < UV_EPS) //right edge
		edgeLod = lods.edgeDivision[1];
	[flatten] if (abs(uv.y) < UV_EPS) //top edge
		edgeLod = lods.edgeDivision[0];
	[flatten] if (abs(uv.y - 1.f) < UV_EPS) //bottom edge
		edgeLod = lods.edgeDivision[2];

	DS_OUTPUT output;
	float4 terrain;

	[flatten] if (edgeLod > 0) 
		terrain = Terrain.SampleLevel(PointSampler, float3(uv, (float)PatchID), 0.f);
	else
		terrain = Terrain.SampleLevel(LinearSampler, float3(uv, (float)PatchID), 0.f);

	[branch] if (edgeLod > 0) output.normal = UVToGlobalOnLodEdge(patch[0], uv, edgeLod);
	else output.normal = UVToGlobal(patch[0], uv);

	output.pos = PlanetPos + output.normal*(Radius + terrain.w);

	output.normal = normalize(output.normal - (terrain.xyz - dot(terrain.xyz, output.normal) * output.normal));

	if (terrain.w < -0.001f) //ocean
		output.color = COLOR_OCEAN; 
	else if (terrain.w < 0.f) //ocean -> sand
		output.color = lerp(COLOR_OCEAN, COLOR_SAND, (terrain.w + 0.001f) / 0.001f);
	else if (terrain.w < 0.004f) //sand -> rocks
		output.color = lerp(COLOR_SAND, COLOR_ROCK, (terrain.w) / 0.004f);
	else //rocks
		output.color = COLOR_ROCK;

	output.color *= Diffuse.xyz * (Power * clamp(dot(output.normal, Direction), 0.f, 1.f) + (1.f - Power));
	return output;
}
