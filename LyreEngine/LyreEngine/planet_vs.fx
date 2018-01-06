cbuffer VSConstantBuffer : register(b0)
{
	matrix view;
	matrix projection;
}

struct VS_INPUT
{
	float3 pos : POSITION;
};

struct VS_OUTPUT
{
	float4 pos : SV_Position;
	float3 color : COLOR;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.color = (input.pos + 1.)/2.;
	output.pos = mul(float4(input.pos, 1.f), view);
	output.pos = mul(output.pos, projection);
	return output;
}