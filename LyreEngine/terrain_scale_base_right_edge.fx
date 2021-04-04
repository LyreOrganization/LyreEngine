#include "terrain_common.fx"

[numthreads(1, 16, 4)]
void main(uint3 id : SV_DispatchThreadID) {
	if (id.y > HALFRES) return;
	BaseRegions[uint3(RESOLUTION - 1, id.y * 2, id.z)] =
		// Pos & 1 - region position in parent level
		Map[uint2(HALFRES + (Pos.x & 1) * HALFRES, id.y)].w;
}