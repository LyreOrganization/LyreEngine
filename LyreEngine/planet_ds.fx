cbuffer Planet : register(b0) {
	float3 planetViewPos;
	float radius;
}

cbuffer Lighting : register(b1) {
	float3 Direction;
	float Power;
	float4 Diffuse;
}

Texture2DArray<float4> Terrain : register(t0);
SamplerState Tex2DSampler : register(s0);

struct DS_OUTPUT {
	float3 pos : POSITION;
	float3 color : COLOR;
	float3 normal : NORMAL;
};

struct HS_OUTPUT {
	float3 pos : QUAD_POINT_VIEW_POSITION;
};

struct HSCF_OUTPUT {
	float edge[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

[domain("quad")]
DS_OUTPUT main(HSCF_OUTPUT input,
			   float2 uv : SV_DomainLocation,
			   const OutputPatch<HS_OUTPUT, 4> patch,
			   uint PatchID : SV_PrimitiveID) {
	DS_OUTPUT output;
	float4 terrain = Terrain.SampleLevel(Tex2DSampler, float3(uv, (float)PatchID), 0.f);
	output.pos = planetViewPos +
		normalize(lerp(
			lerp(patch[0].pos, patch[1].pos, uv.x),
			lerp(patch[3].pos, patch[2].pos, uv.x),
			uv.y
		).xyz - planetViewPos)*(radius/* + terrain.w*/);
	terrain.w = (terrain.w + 1.f)*1.5f;
	output.color = float3(clamp(terrain.w - 0.2f, 0.f, 1.f),
						  clamp(terrain.w - 0.3f, 0.f, 1.f),
						  clamp(terrain.w - 1.f, 0.f, 1.f)) *
		Diffuse * Power * clamp(dot(terrain.xyz, -Direction), 0.f, 1.f);
	output.normal = terrain.xyz;
	return output;
}
