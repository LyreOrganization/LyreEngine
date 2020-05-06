#include "LyrePch.h"
#include "InputLayoutDX11.h"
#include "DirectX11API.h"

#undef interface

namespace Lyre
{

	DXGI_FORMAT ToDXGIFormat(EShaderDataType type)
	{
		switch (type)
		{
		case EShaderDataType::Float:
			return DXGI_FORMAT_R32_FLOAT;
		case EShaderDataType::Float2:
			return DXGI_FORMAT_R32G32_FLOAT;
		case EShaderDataType::Float3:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case EShaderDataType::Float4:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		default:
			LYRE_ASSERT(false, "Unsupported shader data type.");
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	CInputLayoutDX11::CInputLayoutDX11(std::initializer_list<SLayoutEntry> const& layout, SDirectXInterface const* interface)
		: CInputLayout(layout)
	{
		m_interface = interface;

		for (auto const& entry : m_entries)
		{
			D3D11_INPUT_ELEMENT_DESC desc;
			{
				desc.SemanticName = entry.name.c_str();
				desc.SemanticIndex = 0;
				desc.Format = ToDXGIFormat(entry.type);
				desc.InputSlot = 0;
				desc.AlignedByteOffset = entry.offset;
				desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				desc.InstanceDataStepRate = 0;
			}
			m_layoutDesc.push_back(desc);
		}
	}

	void CInputLayoutDX11::Bind()
	{
		m_interface->context->IASetInputLayout(m_layout);
	}

	bool CInputLayoutDX11::CreateDxResource(void const* shaderByteCode, size_t length)
	{
		HRESULT hr = m_interface->device->CreateInputLayout(m_layoutDesc.data(), m_layoutDesc.size(), shaderByteCode, length, &m_layout);
		return SUCCEEDED(hr);
	}

}
