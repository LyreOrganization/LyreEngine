cbuffer CAMERA : register(b0)
{
	matrix viewProj;
};

cbuffer MODEL : register(b1)
{
	matrix model;
};

struct VS_Input
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
};
struct VS_Output
{
	float4 pos : SV_Position;
	float4 normal : NORMAL;
};

VS_Output main(VS_Input input)
{
	VS_Output output = (VS_Output)0;
	matrix modelViewProj = mul(model, viewProj);
	output.pos = mul(float4(input.pos, 1.f), modelViewProj);
	output.normal = float4(normalize(input.normal), 0.f);
	return output;
}
