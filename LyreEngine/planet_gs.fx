struct GS_INPUT {
	float3 pos : POSITION;
	float3 color : COLOR;
	float3 normal : NORMAL;
};

struct Geometry {
	float4 pos : POSITION;
	float4 color : COLOR;
};

struct Normal {
	float4 pos0: POSITION0;
	float4 pos1: POSITION1;
};

[maxvertexcount(6)]
void GS(
	triangle GS_INPUT input[3],
	inout PointStream< Geometry > geometry,
	inout PointStream< Normal > normals
) {
	for (uint i = 0; i < 3; i++) {
		Geometry element;
		element.pos = float4(input[i].pos, 1.f);
		element.color = float4(input[i].color, 1.f);
		geometry.Append(element);
	}

	float distance = 0.05f;
	if (distance < 0.1f) {
		Normal normal;
		normal.pos0 = float4(input[0].pos, 1.f);
		normal.pos1 = float4(input[0].pos + 0.05*input[0].normal, 1.f);
		normals.Append(normal);
	}
}