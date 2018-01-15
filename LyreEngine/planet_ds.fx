cbuffer Planet : register(b0) {
	float3 planetViewPos;
	float radius;
}

cbuffer Projection : register(b1) {
	matrix ProjMatrix;
};

cbuffer Lighting : register(b2) {
	float3 Direction;
	float Power;
	float4 Diffuse;
}

SamplerState Tex2DSampler : register(s0);

Texture2DArray Terrain : register(t0);

struct DS_OUTPUT {
	float4 pos : SV_Position;
	float3 color : COLOR;
};

struct HS_OUTPUT {
	float4 pos : QUAD_POINT_VIEW_POSITION;
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
	output.pos = float4(planetViewPos +
						normalize(lerp(
							lerp(patch[2].pos, patch[1].pos, uv.x),
							lerp(patch[3].pos, patch[0].pos, uv.x),
							uv.y
						).xyz - planetViewPos)*(radius + terrain.w),
						1.f);
	output.pos = mul(output.pos, ProjMatrix);
	terrain.w = (terrain.w + 1.f)*1.5f;
	output.color = float3(clamp(terrain.w - 0.2f, 0.f, 1.f),
						  clamp(terrain.w - 0.3f, 0.f, 1.f),
						  clamp(terrain.w - 1.f, 0.f, 1.f)) *
						  Diffuse * Power * clamp(dot(terrain.xyz, Direction), 0.f, 1.f);
	return output;
}
