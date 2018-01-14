cbuffer Lod : register(b0) {
	float minDistance;
	float maxDistance;
	float minLOD;
	float maxLOD;
}

struct HS_INPUT {
	float4 pos : CONTROL_POINT_VIEW_POSITION;
};

struct HS_OUTPUT {
	float4 pos : QUAD_POINT_VIEW_POSITION;
};

struct HSCF_OUTPUT {
	float edge[4]  :  SV_TessFactor;
	float inside[2]  :  SV_InsideTessFactor;
};

float ComputePatchLOD(float3 midPoint) {
	float dist = length(midPoint);
	if (dist > maxDistance) return 0.f;
	float d = 1.f - (clamp(dist, minDistance, maxDistance) - minDistance) / (maxDistance - minDistance);
	return lerp(minLOD, maxLOD, pow(2.f, 7.f * d) / (float)(1 << 7));
}

HSCF_OUTPUT HSCF(InputPatch<HS_INPUT, 9> patch) {
	HSCF_OUTPUT output = (HSCF_OUTPUT)0;

	float lods[] =
	{
		ComputePatchLOD(patch[4].pos),
		ComputePatchLOD(patch[5].pos),
		ComputePatchLOD(patch[6].pos),
		ComputePatchLOD(patch[7].pos),
		ComputePatchLOD(patch[8].pos) //me
	};

	output.inside[0] = output.inside[1] = lods[4];
	output.edge[0] = min(lods[4], lods[2]);
	output.edge[1] = min(lods[4], lods[1]);
	output.edge[2] = min(lods[4], lods[0]);
	output.edge[3] = min(lods[4], lods[3]);

	return output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSCF")]
HS_OUTPUT HS(InputPatch<HS_INPUT, 9> patch, uint i : SV_OutputControlPointID) {
	HS_OUTPUT output = (HS_OUTPUT)0;
	output.pos = patch[i].pos;
	return output;
}