cbuffer CubeFaces : register(b0) {
	matrix PlaneRotations[6];
}

float3 MapFaceUVToSphere(float2 uv, int face) {
	uv = uv * 2.f - 1.f;
	float2 uvSqr = uv*uv;
	// rotate plane to its real position
	return mul(PlaneRotations[face], float4(
		uv.x*sqrt(1.f - (uvSqr.y + 1.f) / 2.f + uvSqr.y / 3.f),
		uv.y*sqrt(1.f - (uvSqr.x + 1.f) / 2.f + uvSqr.x / 3.f),
		sqrt(1.f - (uvSqr.x + uvSqr.y) / 2.f + uvSqr.x * uvSqr.y / 3.f),
		1.f)).xyz;
}
