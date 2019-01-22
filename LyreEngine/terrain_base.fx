#include "terrain_common.fx"

float3 SampleSphere(int2 uv) {
	return MapFaceUVToSphere(
		(float2)(uv + Pos * (1 << DEPTH)) / 
		// Face is in the first byte
		(float)((1 << DEPTH) << Level), FaceAndRegion & 0xff);
}

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID) {
	if (id.x > RESOLUTION || id.y > RESOLUTION) return;
	float3 pos = SampleSphere((int2)id.xy);
	Positions[id.xy] = float4(pos, 0.f);
	// Region is in the second byte
	Map[id.xy] = ParentBaseRegions[uint3(id.xy, FaceAndRegion >> 8)] + PerlinNoise(pos);
}