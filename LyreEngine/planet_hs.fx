struct HS_INPUT {
	int2 pos : POSITION;
	int faceAndFlags : FACE_AND_FLAGS;
	int level : LEVEL;
	float lod : LOD;
};

struct HS_OUTPUT {
	int2 pos : POSITION;
	int face : FACE;
	int level : LEVEL;
};

struct HSCF_OUTPUT {
	int edgeDivision[4] : EDGES;
	float edge[4]  :  SV_TessFactor;
	float inside[2]  :  SV_InsideTessFactor;
};

// TODO: profile and debug different versions of this
float ComputeEdgeLod(HS_INPUT patchPlane, HS_INPUT neighbourPlane, int edgeIndex, out int edgeLod) {
	// if neighbour is less divided
	[flatten] if (neighbourPlane.level < patchPlane.level) {
		edgeLod = (((int)round(neighbourPlane.lod) >> 2) << 1);
		return (float)edgeLod;
	}
	// if neighbour is more divided (check 2nd byte flags)
	if (patchPlane.faceAndFlags & (1 << (8 + edgeIndex))) {
		edgeLod = (((int)round(patchPlane.lod) >> 2) << 2);
		return (float)edgeLod;
	}
	edgeLod = -1.f;
	return min(patchPlane.lod, neighbourPlane.lod);
}

HSCF_OUTPUT HSCF(InputPatch<HS_INPUT, 5> patch) {
	HSCF_OUTPUT output = (HSCF_OUTPUT)0;
	output.inside[0] = output.inside[1] = patch[0].lod;
	// tesselator enumerates edges ccw, starts from left
	output.edge[3] = ComputeEdgeLod(patch[0], patch[1], 0, output.edgeDivision[0]);
	output.edge[2] = ComputeEdgeLod(patch[0], patch[2], 1, output.edgeDivision[1]);
	output.edge[1] = ComputeEdgeLod(patch[0], patch[3], 2, output.edgeDivision[2]);
	output.edge[0] = ComputeEdgeLod(patch[0], patch[4], 3, output.edgeDivision[3]);
	return output;
}

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(1)]
[patchconstantfunc("HSCF")]
HS_OUTPUT HS(InputPatch<HS_INPUT, 5> patch) {
	HS_OUTPUT output = (HS_OUTPUT)0;
	output.pos = patch[0].pos;
	output.face = patch[0].faceAndFlags & 0xff; // remove flags
	output.level = patch[0].level;
	return output;
}