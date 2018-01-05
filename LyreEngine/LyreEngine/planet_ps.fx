struct PS_INPUT
{
	float4 Pos : SV_Position;
};

float4 PS(PS_INPUT input) : SV_Target
{
	float4 outputColor = float4(1., 0., 0., 1.);
	return outputColor;
}
