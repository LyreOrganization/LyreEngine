struct PS_INPUT
{
	float4 pos : SV_Position;
	float3 color : COLOR;
};

float4 PS(in PS_INPUT input) : SV_Target
{
	return float4(input.color, 1.);
}
