#include "stdafx.h"
#include "Atmosphere.h"
#include "LyreEngine.h"

using namespace std;
using namespace DirectX;

const float Atmosphere::AverageDensityHeight	= 0.1f;
const int	Atmosphere::OutScatteringRes		= 512;
const int	Atmosphere::SamplesAmount			= 50;

Atmosphere::Atmosphere(float planetRadius, float height, int quality) :
	m_radius(planetRadius + height),
	m_height(height),
	m_planetRadius(planetRadius),
	m_scaleFactor(1.f / height),
	m_quality(quality),
	m_skyDome(m_radius, 150, 150, true)
{}

float Atmosphere::getOpticalDepth(float height, float angle) {
	float rayLength;
	
	// some magic happening here
	float b = m_radius * m_scaleFactor;
	float scaledPlanetRadius = m_planetRadius * m_scaleFactor;
	float c = scaledPlanetRadius + height;
	float beta = angle * XM_PI;
	float h_a = c * sinf(beta);

	if (!Utils::solveTriangleBySSA(&rayLength, b, c, beta)) {
		return 0.f;
	}

	// computing the INTEGRAL
	float cosBeta = cosf(beta);
	float sampleLength = rayLength / SamplesAmount; // Change to m_quality
	float opticalDepth = 0.f;
	for (float sample = sampleLength; sample < rayLength; sample += sampleLength) {
		float sampleHeight = Utils::solveTriangleBySAS(c, sample, beta) - scaledPlanetRadius;
		// actual integral sum
		opticalDepth += expf(-sampleHeight / AverageDensityHeight) * sampleLength;
	}

	return opticalDepth;
}

CComPtr<ID3D11Texture2D> Atmosphere::precomputeOpticalDepth() {
	vector<ScatteringSample> outScattering(OutScatteringRes*OutScatteringRes);

	for (int x = 0; x < OutScatteringRes; ++x) {
		for (int y = 0; y < OutScatteringRes; ++y) {
			float h = static_cast<float>(x) / (OutScatteringRes - 1);
			float angle = static_cast<float>(y) / (OutScatteringRes - 1);

			float density = expf(-h / AverageDensityHeight);
			float opticalDepth = getOpticalDepth(h, angle);

			outScattering[y*OutScatteringRes + x].rayleighDensity = density;
			outScattering[y*OutScatteringRes + x].rayleighOpticalDepth = opticalDepth;
			outScattering[y*OutScatteringRes + x].MieDensity = density;
			outScattering[y*OutScatteringRes + x].MieOpticalDepth = opticalDepth;
		}
	}

	D3D11_TEXTURE2D_DESC desc;
	{
		desc.Width = desc.Height = OutScatteringRes;
		desc.MipLevels = desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
	}
	D3D11_SUBRESOURCE_DATA initData;
	{
		initData.pSysMem = outScattering.data();
		initData.SysMemPitch = sizeof(ScatteringSample) * OutScatteringRes;
	}
	CComPtr<ID3D11Texture2D> opticalDepth;
	LyreEngine::getDevice()->CreateTexture2D(&desc, &initData, &opticalDepth);

	return opticalDepth;
}

void Atmosphere::init() {
	try {
		vector<char> byteCode = m_renderConfig.loadShader(Shader::VS, L"atmosphere_vs.cso");
		m_skyDome.loadLayout(byteCode.data(), byteCode.size());

		CComPtr<ID3D11Texture2D> opticalDepth = precomputeOpticalDepth();
		CComPtr<ID3D11ShaderResourceView> srv;
		LyreEngine::getDevice()->CreateShaderResourceView(opticalDepth, nullptr, &srv);

		m_renderConfig.setSRV(Shader::VS, srv, 0);

		m_renderConfig.loadShader(Shader::PS, L"atmosphere_ps.cso");
	}
	catch (runtime_error) {
		throw runtime_error("Atmosphere init failed!");
	}
}

void Atmosphere::render() {
	if (!visible) return;

	UINT w, h;
	LyreEngine::getClientWH(w, h);

	m_viewCb.data.viewProj = LyreEngine::getCamera()->calculateViewProjMatrix(static_cast<float>(w) / h);
	m_vectorsCb.data.lightDirection = LyreEngine::getLightingDirection();
	
	XMFLOAT3 height;
	XMStoreFloat3(&height, XMVector3LengthSq(XMLoadFloat3(&LyreEngine::getCamera()->getPosition())));
	m_vectorsCb.data.cameraPosition = LyreEngine::getCamera()->getPosition();
	m_vectorsCb.data.invWaveLength = XMFLOAT3(1.f / powf(0.650f, 4.0), 1.f / powf(0.570f, 4.0), 1.f / powf(0.475f, 4.0));

	m_scalarsCb.data.cameraHeight2 = height.x;
	m_scalarsCb.data.radius = m_radius;
	m_scalarsCb.data.radius2 = m_radius * m_radius;
	m_scalarsCb.data.planetRadius = m_planetRadius;
	m_scalarsCb.data.planetRadius2 = m_planetRadius * m_planetRadius;
	m_scalarsCb.data.scaleFactor = m_scaleFactor;
	m_scalarsCb.data.sampleAmount = 30;

	m_viewCb.update();
	m_vectorsCb.update();
	m_scalarsCb.update();

	m_lightCb.data.lightDirection = LyreEngine::getLightingDirection();
	m_lightCb.update();

	m_renderConfig.setConstantBuffer(Shader::VS, m_viewCb, 0);
	m_renderConfig.setConstantBuffer(Shader::VS, m_vectorsCb, 1);
	m_renderConfig.setConstantBuffer(Shader::VS, m_scalarsCb, 2);
	m_renderConfig.setSampler(Shader::VS, LyreEngine::getSampler2D(), 0);

	m_renderConfig.setConstantBuffer(Shader::PS, m_lightCb, 0);

	m_skyDome.bind();
	m_renderConfig.bind();

	LyreEngine::getContext()->DrawIndexed(m_skyDome.getIndicesSize(), 0, 0);
}
