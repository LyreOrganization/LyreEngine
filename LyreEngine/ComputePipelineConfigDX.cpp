#include "stdafx.h"

#include "ComputePipelineConfigDX.h"
#include "LyreEngine.h"

ComputePipelineConfigDX::ComputePipelineConfigDX() {
	m_cbuffers.fill(nullptr);
	m_samplers.fill(nullptr);
	m_srvs.fill(nullptr);
	m_uavs.fill(nullptr);
}

ComputePipelineConfigDX::~ComputePipelineConfigDX() {
	for (auto& buffer : m_cbuffers) {
		if (buffer) {
			buffer->Release();
		}
	}
	for (auto& sampler : m_samplers) {
		if (sampler) {
			sampler->Release();
		}
	}
	for (auto& srv : m_srvs) {
		if (srv) {
			srv->Release();
		}
	}
	for (auto& uav : m_uavs) {
		if (uav) {
			uav->Release();
		}
	}
}

std::vector<char> ComputePipelineConfigDX::loadShader(const WCHAR* fileName) {
	std::vector<char> shaderBytecode;

	HRESULT hr = LyreEngine::readShaderFromFile(fileName, shaderBytecode);
	if (FAILED(hr)) {
		throw std::runtime_error("ComputePipelineConfigDX.loadShader: failed to read shader from file.");
	}

	hr = m_pDevice->CreateComputeShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iCS);
	if (FAILED(hr)) {
		throw std::runtime_error("ComputePipelineConfigDX.loadShader: failed to create shader.");
	}
	return shaderBytecode;
}

void ComputePipelineConfigDX::setConstantBuffer(ID3D11Buffer* cb, UINT slot) {
	ID3D11Buffer* currentCbuffer;

	currentCbuffer = m_cbuffers[slot];
	if (currentCbuffer) {
		currentCbuffer->Release();
	}
	
	if (cb) {
		cb->AddRef();
	}
	m_cbuffers[slot] = cb;
}

void ComputePipelineConfigDX::setSampler(ID3D11SamplerState* sampler, UINT slot) {
	ID3D11SamplerState* currentSampler;

	currentSampler = m_samplers[slot];
	if (currentSampler) {
		currentSampler->Release();
	}

	if (sampler) { 
		sampler->AddRef(); 
	}
	m_samplers[slot] = sampler;
}

void ComputePipelineConfigDX::setSRV(ID3D11ShaderResourceView* srv, UINT slot) {
	ID3D11ShaderResourceView* currentSrv;

	currentSrv = m_srvs[slot];
	if (currentSrv) {
		currentSrv->Release();
	}

	if (srv) {
		srv->AddRef();
	}
	m_srvs[slot] = srv;
}

void ComputePipelineConfigDX::setUAV(ID3D11UnorderedAccessView* uav, UINT slot) {
	ID3D11UnorderedAccessView* currentUav;

	currentUav = m_uavs[slot];
	if (currentUav) {
		currentUav->Release();
	}

	if (uav) {
		uav->AddRef();
	}
	m_uavs[slot] = uav;
}

void ComputePipelineConfigDX::bind() {
	m_pContext->CSSetShader(m_iCS, nullptr, 0);
	m_pContext->CSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_cbuffers.data());
	m_pContext->CSSetSamplers(0, MAX_SAMPLERS_AMOUNT, m_samplers.data());
	m_pContext->CSSetShaderResources(0, MAX_SRVS_AMOUNT, m_srvs.data());
	m_pContext->CSSetUnorderedAccessViews(0, MAX_UAVS_AMOUNT, m_uavs.data(), nullptr);
}

void ComputePipelineConfigDX::unbind() {
	std::array<ID3D11Buffer*, MAX_CBUFFERS_AMOUNT> cbuffers;
	std::array<ID3D11SamplerState*, MAX_SAMPLERS_AMOUNT> samplers;
	std::array<ID3D11ShaderResourceView*, MAX_SRVS_AMOUNT> srvs;
	std::array<ID3D11UnorderedAccessView*, MAX_UAVS_AMOUNT> uavs;

	cbuffers.fill(nullptr);
	samplers.fill(nullptr);
	srvs.fill(nullptr);
	uavs.fill(nullptr);

	m_pContext->CSSetShader(nullptr, nullptr, 0);
	m_pContext->CSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, cbuffers.data());
	m_pContext->CSSetSamplers(0, MAX_SAMPLERS_AMOUNT, samplers.data());
	m_pContext->CSSetShaderResources(0, MAX_SRVS_AMOUNT, srvs.data());
	m_pContext->CSSetUnorderedAccessViews(0, MAX_UAVS_AMOUNT, uavs.data(), nullptr);
}
