cbuffer View : register(b0) {
	matrix ViewProjMatrix;
}

cbuffer Vectors : register(b1) {
	float3 LightDirection;
	float dummy1;
	float3 CameraPosition;
	float dummy2;
	float3 InvWaveLength;
	float dummy3;
}

cbuffer Scalars : register(b2) {
	float CameraHeight2;
	float Radius;
	float Radius2;
	float PlanetRadius;
	float PlanetRadius2;
	float ScaleFactor;
	int SampleAmount;
	float dummy4;
}

#define PI 3.14159f
#define Kr4Pi 0.314159f //0.0314159f
#define KrESun 0.275f //0.0375f

Texture2D<float4> OpticalDepth : register(t0);
SamplerState Tex2DSampler : register(s0);

struct VS_INPUT {
	float3 pos : VERTEX;
};

struct VS_OUTPUT {
	float4 pos: SV_Position;
	float4 color: COLOR;
	float4 direction: DIRECTION;
};

float isIntersecting(float3 pos, float3 ray, float distance2, float radius2) {
	float B = 2.0 * dot(pos, ray);
	float C = distance2 - radius2;
	float det = B*B - 4.0 * C;
	if (det >= 0.f) {
		return 0.5 * (-B - sqrt(det)) >= 0.f;
	}
	return false;
}

float getNearIntersection(float3 pos, float3 ray, float distance2, float radius2) {
	float B = 2.0 * dot(pos, ray);
	float C = distance2 - radius2;
	float det = max(0.0, B*B - 4.0 * C);
	return 0.5 * (-B - sqrt(det));
}

float3 getAttenuation(float height, float angle) {
	float4 opticalDepth = OpticalDepth.SampleLevel(Tex2DSampler, float2(height, angle), 0);
	return Kr4Pi * opticalDepth.y * InvWaveLength;
}

float getDensity(float height) {
	return OpticalDepth.SampleLevel(Tex2DSampler, float2(height, 0.f), 0).x;
}

VS_OUTPUT VS(VS_INPUT input) {
	float3 ray = input.pos.xyz - CameraPosition;
	float far = length(ray);
	ray /= far;
	
	float near = getNearIntersection(CameraPosition, ray, CameraHeight2, Radius2);
	float3 start = CameraPosition + ray * near;

	far -= near;

	float sampleLength = far / SampleAmount;
	float scaledLength = sampleLength * ScaleFactor;
	float3 sampleRay = ray * sampleLength;
	float3 samplePoint = start + sampleRay * 0.5;

	float3 totalAttenuation = float3(0.0, 0.0, 0.0);
	for (int i = 0; i < SampleAmount; ++i) {
		float samplePointLength = length(samplePoint);
		float scaledSamplePointHeight = (samplePointLength - PlanetRadius) * ScaleFactor;

		float samplePointLength2 = samplePointLength * samplePointLength;

		if (isIntersecting(samplePoint, -LightDirection, samplePointLength2, PlanetRadius2)) {
			float samplePointAngleToSun = acos(dot(samplePoint, LightDirection) / samplePointLength) / PI;
			float samplePointAngleToCamera = acos(dot(samplePoint, ray) / samplePointLength) / PI;

			float3 attenuationToSun = getAttenuation(scaledSamplePointHeight, samplePointAngleToSun);
			float3 attenuationToCamera = getAttenuation(scaledSamplePointHeight, samplePointAngleToCamera);

			totalAttenuation += exp(-attenuationToSun - attenuationToCamera)
				* getDensity(scaledSamplePointHeight) * scaledLength;
		}

		samplePoint += sampleRay;
	}

	VS_OUTPUT output = (VS_OUTPUT)0;

	output.pos = mul(float4(input.pos, 1.f), ViewProjMatrix);
	output.color = float4(totalAttenuation * InvWaveLength * KrESun, 1.f);
	output.direction = float4(CameraPosition - input.pos.xyz, 1.f);

	return output;
}