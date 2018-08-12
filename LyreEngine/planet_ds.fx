cbuffer Planet : register(b0) {
	float3 PlanetPos;
	float Radius;
}

cbuffer Lighting : register(b1) {
	float3 Direction;
	float Power;
	float4 Diffuse;
}

Texture2DArray Terrain : register(t0);

// TODO
// Texture2D SlerpLookup : register(t1);
SamplerState Tex2DSampler : register(s0);

struct DS_OUTPUT {
	float3 pos : POSITION;
	float3 color : COLOR;
	float3 normal : NORMAL;
};

struct HS_OUTPUT {
	float3 pos : POSITION;
};

struct HSCF_OUTPUT {
	float edge[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

float3 slerp(float3 a, float3 b, float x) {
	float angle = acos(dot(normalize(a), normalize(b)));
	return a * sin((1.f - x) * angle) / sin(angle) + b * sin(x * angle) / sin(angle);
}

[domain("quad")]
DS_OUTPUT main(HSCF_OUTPUT input,
			   float2 uv : SV_DomainLocation,
			   const OutputPatch<HS_OUTPUT, 4> patch,
			   uint PatchID : SV_PrimitiveID) {
	static const float3 COLOR_OCEAN = { 0.2f,0.3f,0.8f };
	static const float3 COLOR_SAND = { 0.6f,0.7f,0.3f };
	static const float3 COLOR_ROCK = { 0.44f,0.42f,0.4f };

	DS_OUTPUT output;
	float4 terrain = Terrain.SampleLevel(Tex2DSampler, float3(uv, (float)PatchID), 0.f);
	output.normal = normalize(slerp(slerp(patch[0].pos, patch[1].pos, uv.x),
									slerp(patch[3].pos, patch[2].pos, uv.x),
									uv.y));
	output.pos = PlanetPos + output.normal*(Radius/* + terrain.w*/);

	output.normal = normalize(output.normal - (terrain.xyz - dot(terrain.xyz, output.normal) * output.normal));

	if (terrain.w < -0.02f) {
		output.color = COLOR_OCEAN; //ocean
	}
	else if (terrain.w < 0.f) {
		output.color = lerp(COLOR_OCEAN, COLOR_SAND, (terrain.w + 0.02f) / 0.02f); //ocean -> sand
	}
	else if (terrain.w < 0.04f) {
		output.color = lerp(COLOR_SAND, COLOR_ROCK, (terrain.w) / 0.04f); //sand -> rocks
	}
	else {
		output.color = COLOR_ROCK; //rocks
	}

	output.color *= Diffuse.xyz * (Power * clamp(dot(output.normal, Direction), 0.f, 1.f) + (1.f - Power));
	return output;
}
