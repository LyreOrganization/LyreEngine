cbuffer Vectors : register(b0) {
	float3 LightDirection;
	float dummy1;
	float3 CameraPosition;
	float dummy2;
	float3 InvWaveLength;
	float dummy3;
}

cbuffer Scalars : register(b1) {
	float CameraHeightSqr;
	float Radius;
	float RadiusSqr;
	float PlanetRadius;
	float PlanetRadiusSqr;
	float ScaleFactor;
	float2 dummy4;
}

#define PI 3.14159f
#define Kr4Pi 0.14159f
#define KrESun 0.175f

#define SAMPLE_AMOUNT 100

Texture2D<float4> OpticalDepth : register(t0);
SamplerState Tex2DSampler : register(s0);

struct VS_OUTPUT {
	float4 pos : SV_Position;
	float3 originalPos : ORIG_POS;
};

float GetNearIntersection(float3 pos, float3 ray, float distanceSqr, float radiusSqr) {
	float b = 2.f * dot(pos, ray);
	float c = distanceSqr - radiusSqr;
	float det = b * b - 4.f * c;
	// we don't use negative values anyway
	return det > 0.f ? -0.5f * (b + sqrt(det)) : -1.f;
}

float3 GetAttenuation(float height, float angle) {
	return Kr4Pi * InvWaveLength * OpticalDepth.SampleLevel(
		Tex2DSampler, float2(height, angle), 0).y;
}

float GetDensity(float height) {
	return OpticalDepth.SampleLevel(Tex2DSampler, float2(height, 0.f), 0).x;
}

float GetRayleighPhase(float cosAngle) {
	return 0.75f * (1.f + cosAngle * cosAngle);
}

float4 PS(VS_OUTPUT input) : SV_Target {
	float3 ray = input.originalPos - CameraPosition;
	float far = length(ray);
	ray /= far;

	float near = GetNearIntersection(CameraPosition, ray, CameraHeightSqr, RadiusSqr);
	float3 start = CameraPosition + ray * near;
	far -= near;

	float sampleLength = far / SAMPLE_AMOUNT;
	float scaledLength = sampleLength * ScaleFactor;
	float3 sampleRay = ray * sampleLength;
	float3 samplePoint = start + sampleRay * 0.5f;

	float3 totalAttenuation = float3(0.f, 0.f, 0.f);
	[unroll(SAMPLE_AMOUNT)] for (int i = 0; i < SAMPLE_AMOUNT; ++i) {
		float samplePointLength = length(samplePoint);
		float scaledSamplePointHeight = (samplePointLength - PlanetRadius) * ScaleFactor;

		float samplePointLengthSqr = samplePointLength * samplePointLength;

		[flatten] if (GetNearIntersection(samplePoint, -LightDirection, samplePointLengthSqr, PlanetRadiusSqr) < 0.f) {
			float samplePointAngleToSun = acos(dot(samplePoint, LightDirection) / samplePointLength) / PI;
			float samplePointAngleToCamera = acos(dot(samplePoint, ray) / samplePointLength) / PI;

			float3 attenuationToSun = GetAttenuation(scaledSamplePointHeight, samplePointAngleToSun);
			float3 attenuationToCamera = GetAttenuation(scaledSamplePointHeight, samplePointAngleToCamera);

			totalAttenuation += exp(-attenuationToSun - attenuationToCamera)
				* GetDensity(scaledSamplePointHeight) * scaledLength;
		}

		samplePoint += sampleRay;
	}

	float lightCos = dot(normalize(CameraPosition - input.originalPos), LightDirection);
	return float4(totalAttenuation * InvWaveLength * KrESun * GetRayleighPhase(lightCos), 1.f);
}
