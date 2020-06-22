#include "LyrePch.h"
#include "ShaderDX11.h"
#include "InputLayoutDX11.h"
#include "VertexBufferDX11.h"
#include "ConstantBufferDX11.h"
#include "TextureDX11.h"
#include "DirectX11API.h"

#include <d3dcompiler.h>

#pragma comment(lib,"d3dcompiler.lib")

#undef interface

namespace
{

	HRESULT CompileShader(std::string const& srcFile, LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob)
	{
		*blob = nullptr;

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef LYRE_DEBUG
		flags |= D3DCOMPILE_DEBUG;
#endif

		const D3D_SHADER_MACRO defines[] =
		{
			"EXAMPLE_DEFINE", "1",
			NULL, NULL
		};

		ID3DBlob* shaderBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompile(srcFile.c_str(), srcFile.size(), nullptr, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint, profile,
			flags, 0, &shaderBlob, &errorBlob);
		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}

			if (shaderBlob)
				shaderBlob->Release();

			return hr;
		}

		*blob = shaderBlob;

		return hr;
	}

}

namespace Lyre
{

	CShaderDX11::CShaderDX11(std::string const& vsSrc, std::string const& psSrc, SDirectXInterface const* interface)
		: CPipelineResourceDX11(interface)
	{
		HRESULT hr = E_FAIL;

		hr = CompileShader(vsSrc, "main", "vs_5_0", &m_vsBlob);
		LYRE_ASSERT(SUCCEEDED(hr), "Vertex shader compilation failed.");

		hr = interface->device->CreateVertexShader(
			m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(),
			nullptr,
			&m_vs
		);
		LYRE_ASSERT(SUCCEEDED(hr), "Cannot create vertex shader.");

		hr = CompileShader(psSrc, "main", "ps_5_0", &m_psBlob);
		LYRE_ASSERT(SUCCEEDED(hr), "Pixel shader compilation failed.");

		hr = interface->device->CreatePixelShader(
			m_psBlob->GetBufferPointer(),
			m_psBlob->GetBufferSize(),
			nullptr,
			&m_ps
		);
		LYRE_ASSERT(SUCCEEDED(hr), "Cannot create pixel shader.");

		D3D11_SAMPLER_DESC samplerDesc;
		{
			ZeroStruct(samplerDesc);
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW =
				D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.MinLOD = 0.f;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		}

		hr = interface->device->CreateSamplerState(&samplerDesc, &m_defaultSampler);

		LYRE_ASSERT(SUCCEEDED(hr), "Cannot create default sampler state.");
	}

	bool CShaderDX11::BindInputLayout(CVertexBuffer* vertexBuffer)
	{
		CInputLayoutDX11* inputLayout = static_cast<CInputLayoutDX11*>(vertexBuffer->GetLayout().get());

		if (inputLayout->CreateDxResource(m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize()))
		{
			inputLayout->Bind();
			return true;
		}

		return false;
	}

	void CShaderDX11::Bind()
	{
		std::vector<ID3D11Buffer*> buffers;
		for (auto const& constantBuffer : m_constantBuffers)
		{
			constantBuffer->Update();

			auto constantBufferDx = static_cast<CConstantBufferDX11*>(constantBuffer.get());
			buffers.push_back(constantBufferDx->m_buffer.p);
		}
		GetDxInterface()->context->VSSetConstantBuffers(0, buffers.size(), buffers.data());

		std::vector<ID3D11ShaderResourceView*> textures;
		for (auto const& texture : m_textures)
		{
			auto textureDx = static_cast<CTextureDX11*>(texture.get());
			textures.push_back(textureDx->m_view.p);
		}
		GetDxInterface()->context->PSSetShaderResources(0, textures.size(), textures.data());
		GetDxInterface()->context->PSSetSamplers(0, 1, &m_defaultSampler.p);

		GetDxInterface()->context->VSSetShader(m_vs, nullptr, 0);
		GetDxInterface()->context->PSSetShader(m_ps, nullptr, 0);
	}

}