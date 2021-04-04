#include "terrain_common.fx"

[numthreads(16, 1, 4)]
void main(uint3 id : SV_DispatchThreadID) {
	// Pos & 1 - region position in parent level
	uint2 leftIdx = uint2(id.xy + (Pos & 1) * HALFRES);
	uint3 leftIdxBase = uint3(id.xy * 2, id.z);
	float left = Map[leftIdx].w;
	BaseRegions[leftIdxBase] = left;
	BaseRegions[leftIdxBase + uint3(1, 0, 0)] = 
		(left + Map[leftIdx + uint2(1, 0)].w) / 2.f;
}
