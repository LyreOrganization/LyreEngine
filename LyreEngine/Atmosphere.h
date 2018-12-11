#pragma once

#include "PipelineConfigDX.h"
#include "ConstantBufferDX.h"
#include "UVSphere.h"

class Atmosphere {
public:
	Atmosphere(float planetRadius, float height, int quality);
	void init();
	void render();

private:
	static const float AverageDensityHeight;
	static const int OutScatteringRes;
	static const int SamplesAmount;

	struct ScatteringSample {
		float rayleighDensity;
		float rayleighOpticalDepth;
		float MieDensity;
		float MieOpticalDepth;
	};

	float m_radius;
	float m_planetRadius;
	float m_height;
	int m_quality;

	UVSphere m_skyDome;
	PipelineConfigDX m_renderConfig;

	struct AtmosphereCb {
		DirectX::XMFLOAT4X4 viewProj;
	};
	ConstantBufferDX<AtmosphereCb> m_atmosphereCb;

	float getOpticalDepth(float height, float angle);
	CComPtr<ID3D11Texture2D> precomputeOpticalDepth();

};
