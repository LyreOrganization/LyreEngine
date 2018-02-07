#include "stdafx.h"
#include "PipelineConfigDX.h"
#include "LyreEngine.h"

ShaderData& PipelineConfigDX::getShaderData(Shader shader) {
	switch (shader) {
	case Shader::VS:
		return m_VSData;
		break;
	case Shader::HS:
		return m_HSData;
		break;
	case Shader::DS:
		return m_DSData;
		break;
	case Shader::GS:
		return m_GSData;
		break;
	case Shader::PS:
		return m_PSData;
		break;
	}
}

std::vector<char> PipelineConfigDX::loadShader(Shader shader, const WCHAR* fileName) {
	std::vector<char> shaderBytecode;
	HRESULT hr = LyreEngine::readShaderFromFile(fileName, shaderBytecode);
	if (FAILED(hr)) {
		throw std::runtime_error("PipelineConfigDX.loadShader: failed to read shader from file.");
	}
	switch (shader) {
	case Shader::VS:
		hr = m_pDevice->CreateVertexShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iVS);
		break;
	case Shader::HS:
		hr = m_pDevice->CreateHullShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iHS);
		break;
	case Shader::DS:
		hr = m_pDevice->CreateDomainShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iDS);
		break;
	case Shader::GS:
		hr = m_pDevice->CreateGeometryShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iGS);
		break;
	case Shader::PS:
		hr = m_pDevice->CreatePixelShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iPS);
		break;
	}
	if (FAILED(hr)) {
		throw std::runtime_error("PipelineConfigDX.loadShader: failed to create shader.");
	}
	return shaderBytecode;
}

void PipelineConfigDX::setConstantBuffer(Shader shader, ID3D11Buffer* cb, UINT slot) {
	ShaderData& data = getShaderData(shader);
	ID3D11Buffer* currentCbuffer;

	currentCbuffer = data.cBuffers[slot];
	if (currentCbuffer) {
		currentCbuffer->Release();
	}

	if (cb) {
		cb->AddRef();
	}
	data.cBuffers[slot] = cb;
}

void PipelineConfigDX::setSampler(Shader shader, ID3D11SamplerState* sampler, UINT slot) {
	ShaderData& data = getShaderData(shader);
	ID3D11SamplerState* currentSampler;

	currentSampler = data.samplers[slot];
	if (currentSampler) {
		currentSampler->Release();
	}

	if (sampler) {
		sampler->AddRef();
	}
	data.samplers[slot] = sampler;
}

void PipelineConfigDX::setSRV(Shader shader, ID3D11ShaderResourceView* srv, UINT slot) {
	ShaderData& data = getShaderData(shader);
	ID3D11ShaderResourceView* currentSrv;

	currentSrv = data.srvs[slot];
	if (currentSrv) {
		currentSrv->Release();
	}

	if (srv) {
		srv->AddRef();
	}
	data.srvs[slot] = srv;
}

D3D11_SO_DECLARATION_ENTRY& PipelineConfigDX::createSOEntry() {
	m_GSData.SO.entries.emplace_back();
	return m_GSData.SO.entries.back();
}

void PipelineConfigDX::addSOEntry(D3D11_SO_DECLARATION_ENTRY entry) {
	m_GSData.SO.entries.push_back(entry);
}

void PipelineConfigDX::setSOBuffer(ID3D11Buffer* buffer, UINT slot, UINT offset) {
	ID3D11Buffer* currentBuffer = m_GSData.SO.buffers[slot];
	if (currentBuffer) {
		currentBuffer->Release();
	}
	buffer->AddRef();
	m_GSData.SO.buffers[slot] = buffer;
	m_GSData.SO.offsets[slot] = offset;
}

void PipelineConfigDX::loadGSwithSO(const WCHAR* fileName, const std::vector<UINT>& strides, UINT rasterizedStream) {
	HRESULT hr;

	std::vector<char> shaderBytecode;
	hr = LyreEngine::readShaderFromFile(fileName, shaderBytecode);
	if (FAILED(hr)) {
		throw std::runtime_error("PipelineConfigDX.loadGSwithSO: failed to read shader from file.");
	}

	hr = m_pDevice->CreateGeometryShaderWithStreamOutput(
		shaderBytecode.data(), shaderBytecode.size(),
		m_GSData.SO.entries.data(), m_GSData.SO.entries.size(),
		strides.data(), strides.size(),
		rasterizedStream,
		nullptr,
		&m_iGS
	);
	if (FAILED(hr)) {
		throw std::runtime_error("PipelineConfigDX.loadGSwithSO: failed to create shader.");
	}
}

void PipelineConfigDX::bind()
{
	//Vertex Shader
	m_pContext->VSSetShader(m_iVS, nullptr, 0);
	m_pContext->VSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_VSData.cBuffers.data());
	m_pContext->VSSetSamplers(0, MAX_SAMPLERS_AMOUNT, m_VSData.samplers.data());
	m_pContext->VSSetShaderResources(0, MAX_SRVS_AMOUNT, m_VSData.srvs.data());

	//Hull Shader
	m_pContext->HSSetShader(m_iHS, nullptr, 0);
	m_pContext->HSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_HSData.cBuffers.data());
	m_pContext->HSSetSamplers(0, MAX_SAMPLERS_AMOUNT, m_HSData.samplers.data());
	m_pContext->HSSetShaderResources(0, MAX_SRVS_AMOUNT, m_HSData.srvs.data());

	//Domain Shader
	m_pContext->DSSetShader(m_iDS, nullptr, 0);
	m_pContext->DSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_DSData.cBuffers.data());
	m_pContext->DSSetSamplers(0, MAX_SAMPLERS_AMOUNT, m_DSData.samplers.data());
	m_pContext->DSSetShaderResources(0, MAX_SRVS_AMOUNT, m_DSData.srvs.data());

	//Geometry Shader
	m_pContext->GSSetShader(m_iGS, nullptr, 0);
	m_pContext->GSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_GSData.cBuffers.data());
	m_pContext->GSSetSamplers(0, MAX_SAMPLERS_AMOUNT, m_GSData.samplers.data());
	m_pContext->GSSetShaderResources(0, MAX_SRVS_AMOUNT, m_GSData.srvs.data());

	//Stream Output
	m_pContext->SOSetTargets(MAX_SOBUFFERS_AMOUNT, m_GSData.SO.buffers.data(), m_GSData.SO.offsets.data());

	//Pixel Shader
	m_pContext->PSSetShader(m_iPS, nullptr, 0);
	m_pContext->PSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_PSData.cBuffers.data());
	m_pContext->PSSetSamplers(0, MAX_SAMPLERS_AMOUNT, m_PSData.samplers.data());
	m_pContext->PSSetShaderResources(0, MAX_SRVS_AMOUNT, m_PSData.srvs.data());
}

void PipelineConfigDX::unbind() {
	//Stream Output
	m_pContext->SOSetTargets(0, nullptr, nullptr);
}
