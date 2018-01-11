cbuffer ViewProj : register(b0) {
	matrix viewProj;
}

cbuffer Lighting : register(b1) {
	float3 direction;
	float power;
	float4 diffuse;
}

struct VS_INPUT {
	float3 pos : POSITION;
	float3 normal : NORMAL;
};

struct VS_OUTPUT {
	float4 pos : SV_Position;
	float3 color : COLOR;
};

VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.color = diffuse * power * clamp(dot(input.normal, direction), 0.f, 1.f) + (float3)1.f - power;
	output.pos = mul(float4(input.pos, 1.f), viewProj);
	return output;
}