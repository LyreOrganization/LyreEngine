#pragma once

#include "DeviceReference.h"

struct ShaderData {
	std::array<ID3D11Buffer*, MAX_CBUFFERS_AMOUNT>			cBuffers;
	std::array<ID3D11SamplerState*, MAX_SAMPLERS_AMOUNT>	samplers;
	std::array<ID3D11ShaderResourceView*, MAX_SRVS_AMOUNT>	srvs;

	ShaderData() {
		cBuffers.fill(nullptr);
		samplers.fill(nullptr);
		srvs.fill(nullptr);
	}

	~ShaderData() {
		for (auto& buffer : cBuffers) {
			if (buffer) {
				buffer->Release();
			}
		}
		for (auto& sampler : samplers) {
			if (sampler) {
				sampler->Release();
			}
		}
		for (auto& srv : srvs) {
			if (srv) {
				srv->Release();
			}
		}
	}
};

struct GShaderData :public ShaderData {
	struct {
		std::array<ID3D11Buffer*, MAX_SOBUFFERS_AMOUNT> buffers;
		std::array<UINT, MAX_SOBUFFERS_AMOUNT>			offsets;
		std::vector<D3D11_SO_DECLARATION_ENTRY>			entries;
	} SO;

	GShaderData() {
		SO.buffers.fill(nullptr);
		SO.offsets.fill(0);
	}
	~GShaderData() {
		for (auto& buffer : SO.buffers) {
			if (buffer) {
				buffer->Release();
			}
		}
	}
};

enum class Shader {
	VS = 0,
	HS,
	DS,
	GS,
	PS
};

class PipelineConfigDX :public DeviceReference {

	CComPtr<ID3D11VertexShader>		m_iVS = nullptr;
	ShaderData						m_VSData;
	CComPtr<ID3D11HullShader>		m_iHS = nullptr;
	ShaderData						m_HSData;
	CComPtr<ID3D11DomainShader>		m_iDS = nullptr;
	ShaderData						m_DSData;
	CComPtr<ID3D11GeometryShader>	m_iGS = nullptr;
	GShaderData						m_GSData;
	CComPtr<ID3D11PixelShader>		m_iPS = nullptr;
	ShaderData						m_PSData;

	ShaderData& getShaderData(Shader);

public:
	std::vector<char> loadShader(Shader shader, const WCHAR* fileName);

	void setConstantBuffer(Shader shader, ID3D11Buffer* cb, UINT slot);
	void setSampler(Shader shader, ID3D11SamplerState* sampler, UINT slot);
	void setSRV(Shader shader, ID3D11ShaderResourceView* srv,UINT slot);

	D3D11_SO_DECLARATION_ENTRY& createSOEntry();
	void addSOEntry(D3D11_SO_DECLARATION_ENTRY);
	void setSOBuffer(ID3D11Buffer* buffer, UINT slot, UINT offset = 0);
	void loadGSwithSO(const WCHAR* fileName, const std::vector<UINT>& strides, UINT rasterizedStream);

	void bind();
	void unbind();
};
