cbuffer Projection : register(b0) {
	matrix ProjMatrix;
};

struct VS_INPUT {
	float4 pos : POSITION;
	float4 color : COLOR;
};

struct VS_OUTPUT {
	float4 pos : SV_Position;
	float4 color : COLOR;
};

VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = mul(input.pos, ProjMatrix);
	output.color = input.color;
	return output;
}