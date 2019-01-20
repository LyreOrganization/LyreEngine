struct HS_INPUT {
	int2 pos : POSITION;
	int faceAndFlags : FACE_AND_FLAGS;
	int level : LEVEL;
	float2 lod : LOD_CLAMPED;
	float lodDiff : LOD_DIFF;
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

RWBuffer<float> LodDifferenceOutput : register(u8); // 0-7 - RTVs

int NearestPowerOfTwo_4to64(float value) {
	[flatten] if (value < 2.f) return 0.f;
	return 1 << (int)round(log2(min(max(value, 4.f), 64.f)));
}

float ComputeEdgeLod(HS_INPUT patchPlane, HS_INPUT neighbourPlane, int edgeIndex, out int edgeLod) {
	float2 patchLODSelector = (edgeIndex % 2) ? float2(0.f, 1.f) : float2(1.f, 0.f);
	// check if neighbour is on the different face and it's uv is rotated (for this patch it`s "vu")
	float2 neighbourLODSelector =
		(((patchPlane.faceAndFlags ^ neighbourPlane.faceAndFlags) & 0xff) &&
		(edgeIndex == 0 || edgeIndex == 3)) ?
		 ((float2)1.f - patchLODSelector) : patchLODSelector;
	float patchLOD = dot(patchPlane.lod, patchLODSelector);
	float neighbourLOD = dot(neighbourPlane.lod, neighbourLODSelector);
	// if neighbour is less divided
	[flatten] if (neighbourPlane.level < patchPlane.level) {
		edgeLod = NearestPowerOfTwo_4to64(neighbourLOD) >> 1;
		return (float)edgeLod;
	}
	// if neighbour is more divided (check 2nd byte flags)
	[flatten] if (patchPlane.faceAndFlags & (1 << (8 + edgeIndex))) {
		edgeLod = NearestPowerOfTwo_4to64(patchLOD);
		return (float)edgeLod;
	}
	edgeLod = -1.f;
	return min(patchLOD, neighbourLOD);
}

HSCF_OUTPUT HSCF(InputPatch<HS_INPUT, 5> patch) {
	HSCF_OUTPUT output = (HSCF_OUTPUT)0;
	[flatten] if (patch[0].lodDiff > 0.f) {
		output.inside[0] = patch[0].lod.x;
		output.inside[1] = patch[0].lod.y;
		// tesselator enumerates edges ccw, starts from left
		output.edge[3] = ComputeEdgeLod(patch[0], patch[1], 0, output.edgeDivision[0]);
		output.edge[2] = ComputeEdgeLod(patch[0], patch[2], 1, output.edgeDivision[1]);
		output.edge[1] = ComputeEdgeLod(patch[0], patch[3], 2, output.edgeDivision[2]);
		output.edge[0] = ComputeEdgeLod(patch[0], patch[4], 3, output.edgeDivision[3]);
	}
	return output;
}

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(1)]
[patchconstantfunc("HSCF")]
HS_OUTPUT HS(InputPatch<HS_INPUT, 5> patch, uint id : SV_PrimitiveID) {
	HS_OUTPUT output = (HS_OUTPUT)0;
	output.pos = patch[0].pos;
	output.face = patch[0].faceAndFlags & 0xff; // remove flags
	output.level = patch[0].level;
	LodDifferenceOutput[id] = patch[0].lodDiff;
	return output;
}