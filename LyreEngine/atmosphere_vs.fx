cbuffer Viewer : register(b0) {
	matrix ViewProjMatrix;
}

Texture2D<float4> opticalDepth : register(t0);

struct VS_INPUT {
	float3 pos : VERTEX;
};

float4 VS(VS_INPUT input) : SV_Position {
	return mul(float4(input.pos, 1.f), ViewProjMatrix);
}