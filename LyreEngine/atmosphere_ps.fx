cbuffer Light : register(b0) {
	float3 LightDirection;
	float dummy;
}
	
struct PS_INPUT {
	float4 pos : SV_Position;
	float4 color: COLOR;
	float4 direction: DIRECTION;
};

float getRayleighPhase(float cosAngle) {
	float cos2 = cosAngle * cosAngle;
	return 0.75f * (1 + cos2);
}

float4 PS(in PS_INPUT input) : SV_Target {
	float cos = dot(input.direction.xyz, LightDirection) / length(input.direction.xyz);

	return float4(input.color.xyz * getRayleighPhase(cos), 1.0f);
}
