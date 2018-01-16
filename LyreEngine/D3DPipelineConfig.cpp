#include "stdafx.h"
#include "D3DPipelineConfig.h"
#include "LyreEngine.h"

D3DPipelineConfig::D3DPipelineConfig():
	m_pDevice(LyreEngine::getDevice()),
	m_pContext(LyreEngine::getContext())
{}

std::vector<char> loadByteCode(const WCHAR* fileName, const std::string& shaderName) {
	HRESULT hr;
	std::vector<char> byteCode;

	hr = LyreEngine::readShaderFromFile(fileName, byteCode);
	if (FAILED(hr)) {
		throw std::runtime_error("D3DPipelineConfig.load" + shaderName + ": failed to read shader from file.");
	}

	return byteCode;
}

void D3DPipelineConfig::loadVertexShader(const WCHAR* fileName) {
	std::vector<char> byteCode = loadByteCode(fileName, "VertexShader");

	m_pDevice->CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, &m_iVS);
}

void D3DPipelineConfig::loadHullShader(const WCHAR* fileName) {
	std::vector<char> byteCode = loadByteCode(fileName, "HullShader");

	m_pDevice->CreateHullShader(byteCode.data(), byteCode.size(), nullptr, &m_iHS);
}

void D3DPipelineConfig::loadDomainShader(const WCHAR* fileName) {
	std::vector<char> byteCode = loadByteCode(fileName, "DomainShader");

	m_pDevice->CreateDomainShader(byteCode.data(), byteCode.size(), nullptr, &m_iDS);
}

void D3DPipelineConfig::loadGeometryShader(const WCHAR* fileName) {
	std::vector<char> byteCode = loadByteCode(fileName, "GeometryShader");

	m_pDevice->CreateGeometryShader(byteCode.data(), byteCode.size(), nullptr, &m_iGS);
}

void D3DPipelineConfig::loadPixelShader(const WCHAR* fileName) {
	std::vector<char> byteCode = loadByteCode(fileName, "PixelShader");

	m_pDevice->CreatePixelShader(byteCode.data(), byteCode.size(), nullptr, &m_iPS);
}
