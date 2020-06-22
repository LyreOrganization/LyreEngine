struct PS_Input
{
	float4 pos : SV_Position;
	float4 normal : NORMAL;
	float2 texUV : TEXCOORD;
};

Texture2D albedo : register(t0);
SamplerState albedoTextureSampler : register(s0);

float4 main(in PS_Input input) : SV_Target
{
	float3 dirTolight = normalize(float3(1.f, 0.f, 1.f));
	float4 lightColor = float4(1.f, 1.f, 1.f, 1.f);
	float4 diffuseColor = albedo.Sample(albedoTextureSampler, input.texUV);

	float diffuseCoef = max(0.f, dot(input.normal.xyz, dirTolight).x);

	return lightColor * diffuseColor * diffuseCoef;
}
