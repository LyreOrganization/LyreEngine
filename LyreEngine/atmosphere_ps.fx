struct PS_INPUT {
	float4 pos : SV_Position;
};

float4 PS(in PS_INPUT input) : SV_Target
{
	return float4(0.0f, 1.0f, 1.0f, 1.0f);
}
