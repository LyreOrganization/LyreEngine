#pragma once

#include "DeviceReference.h"

class ComputePipelineConfigDX :public DeviceReference {
	CComPtr<ID3D11ComputeShader> m_iCS;

	std::array<ID3D11Buffer*, MAX_CBUFFERS_AMOUNT>			m_cbuffers;
	std::array<ID3D11SamplerState*, MAX_SAMPLERS_AMOUNT>	m_samplers;
	std::array<ID3D11ShaderResourceView*, MAX_SRVS_AMOUNT>	m_srvs;
	std::array<ID3D11UnorderedAccessView*, MAX_UAVS_AMOUNT>	m_uavs;

public:
	ComputePipelineConfigDX();
	~ComputePipelineConfigDX();

	std::vector<char> loadShader(const WCHAR* fileName);

	void setConstantBuffer(ID3D11Buffer* cb, UINT slot);
	void setSampler(ID3D11SamplerState* sampler, UINT slot);
	void setSRV(ID3D11ShaderResourceView* srv, UINT slot);
	void setUAV(ID3D11UnorderedAccessView* uav, UINT slot);

	void bind();
	void unbind();
};
