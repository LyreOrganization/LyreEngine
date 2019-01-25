cbuffer View : register(b0) {
	matrix ViewProjMatrix;
}

struct VS_OUTPUT {
	float4 pos : SV_Position;
	float3 originalPos : ORIG_POS;
};

VS_OUTPUT VS(float3 pos : POSITION) {
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.originalPos = pos;
	output.pos = mul(float4(pos, 1.f), ViewProjMatrix);
	return output;
}