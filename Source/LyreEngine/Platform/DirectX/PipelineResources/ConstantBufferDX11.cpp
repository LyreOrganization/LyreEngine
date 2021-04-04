#include "LyrePch.h"
#include "ConstantBufferDX11.h"
#include "DirectX11API.h"

#undef interface

namespace Lyre
{

	CConstantBufferDX11::CConstantBufferDX11(std::initializer_list<SEntry> const& layout, SDirectXInterface const* interface)
		: CConstantBuffer(layout)
		, CPipelineResourceDX11(interface)
		, m_dirty(false)
	{
		D3D11_BUFFER_DESC desc;
		{
			ZeroStruct(desc);
			desc.ByteWidth = m_size;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}

		HRESULT hr = GetDxInterface()->device->CreateBuffer(&desc, nullptr, &m_buffer);
		LYRE_ASSERT(SUCCEEDED(hr), "Failed to create constant buffer.");
	}

	void CConstantBufferDX11::UpdateConstant(unsigned position, void const* data)
	{
		auto const& entry = m_layout[position];

		if (entry.type == EShaderDataType::Matrix)
		{
			glm::mat4 matrix = glm::make_mat4((float const*)data);
			data = glm::value_ptr(glm::transpose(matrix));
		}

		memcpy((byte*)m_data + entry.offset, data, entry.size);
		m_dirty = true;
	}

	void CConstantBufferDX11::Update()
	{
		if (m_dirty)
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroStruct(mappedResource);

			GetDxInterface()->context->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, m_data, m_size);
			GetDxInterface()->context->Unmap(m_buffer, 0);

			m_dirty = false;
		}
	}

}
