struct PS_Input
{
	float4 pos : SV_Position;
	float4 normal : NORMAL;
};

float4 main(in PS_Input input) : SV_Target
{
	float3 dirTolight = normalize(float3(1.f, 0.f, -1.f));
	float4 lightColor = float4(1.f, 1.f, 1.f, 1.f);
	float4 diffuseColor = float4(0.5f, 0.7f, 0.45f, 1.f);

	float diffuseCoef = max(0.f, dot(input.normal.xyz, dirTolight).x);

	return lightColor * diffuseColor * diffuseCoef;
	//return float4(abs(input.normal.xyz), 1.f);
}
