#include "terrain_common.fx"

[numthreads(1, 16, 4)]
void main(uint3 id : SV_DispatchThreadID) {
	uint2 idx = uint2(id.x, id.y * 2 + 1);
	BaseRegions[uint3(idx, id.z)] = 
		(BaseRegions[uint3(idx + uint2(0, 1), id.z)] +
		 BaseRegions[uint3(idx - uint2(0, 1), id.z)]) / 2.f;
}
