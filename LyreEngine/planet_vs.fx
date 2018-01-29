struct VS_INPUT {
	float3 pos : POSITION;
};

struct VS_OUTPUT {
	float3 pos : POSITION;
};

VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = input.pos;
	return output;
}