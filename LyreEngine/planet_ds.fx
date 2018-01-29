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

[domain("quad")]
DS_OUTPUT main(HSCF_OUTPUT input,
			   float2 uv : SV_DomainLocation,
			   const OutputPatch<HS_OUTPUT, 4> patch,
			   uint PatchID : SV_PrimitiveID) {
	DS_OUTPUT output;
	float4 terrain = Terrain.SampleLevel(Tex2DSampler, float3(uv, (float)PatchID), 0.f);
	output.normal = normalize(lerp(lerp(patch[0].pos, patch[1].pos, uv.x),
								   lerp(patch[3].pos, patch[2].pos, uv.x),
								   uv.y));
	output.pos = PlanetPos + output.normal*(Radius + terrain.w);

	output.normal = normalize(output.normal - (terrain.xyz - dot(terrain.xyz, output.normal) * output.normal));

	output.color = Diffuse * Power * clamp(dot(output.normal, Direction), 0.f, 1.f);
	return output;
}
