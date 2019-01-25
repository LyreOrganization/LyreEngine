#pragma once

#include "PipelineConfigDX.h"
#include "ConstantBufferDX.h"
#include "UVSphere.h"

class Atmosphere {
public:
	Atmosphere(float planetRadius, float height, int quality);
	void init();
	void render();

	bool visible = true;

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
	float m_scaleFactor;
	int m_quality;

	UVSphere m_skyDome;
	PipelineConfigDX m_renderConfig;

	struct ViewCb {
		DirectX::XMFLOAT4X4 viewProj;
	};
	struct VectorsCb {
		DirectX::XMFLOAT3 lightDirection;
		float dummy1;
		DirectX::XMFLOAT3 cameraPosition;
		float dummy2;
		DirectX::XMFLOAT3 invWaveLength;
		float dummy3;
	};
	struct ScalarsCb {
		float cameraHeight2;
		float radius;
		float radius2;
		float planetRadius;
		float planetRadius2;
		float scaleFactor;
		DirectX::XMFLOAT2 dummy;
	};
	ConstantBufferDX<ViewCb> m_viewCb;
	ConstantBufferDX<VectorsCb> m_vectorsCb;
	ConstantBufferDX<ScalarsCb> m_scalarsCb;

	float getOpticalDepth(float height, float angle);
	CComPtr<ID3D11Texture2D> precomputeOpticalDepth();

};
