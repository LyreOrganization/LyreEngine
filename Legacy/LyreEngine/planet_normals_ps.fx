struct PS_INPUT {
	float4 pos : SV_Position;
	float4 color : COLOR;
};

float4 PS(in PS_INPUT input) : SV_Target
{
	return input.color;
}
