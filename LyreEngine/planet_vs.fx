cbuffer Viewer : register(b0) {
	matrix ViewMatrix;
}

struct VS_INPUT {
	float3 pos : CONTROL_POINT_WORLD_POSITION;
};

struct VS_OUTPUT {
	float3 pos : CONTROL_POINT_VIEW_POSITION;
};

VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = mul(float4(input.pos, 1.f), ViewMatrix).xyz;
	return output;
}