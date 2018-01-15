cbuffer Projection : register(b0) {
	matrix ProjMatrix;
};

struct VS_INPUT {
	float4 pos : POSITION;
};

struct VS_OUTPUT {
	float4 pos : SV_Position;
	float4 color : COLOR;
};

VS_OUTPUT VS(VS_INPUT input, uint id : SV_VertexID) {
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = mul(input.pos, ProjMatrix);
	if (id & 1) output.color = float4(1.f, 0.f, 0.f, 1.f);
	else output.color = float4(0.f, 0.f, 1.f, 1.f);
	return output;
}