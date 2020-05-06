#include "LyrePch.h"
#include "ShaderDX11.h"
#include "InputLayoutDX11.h"
#include "VertexBufferDX11.h"
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
	{
		m_interface = interface;

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
		m_interface->context->VSSetShader(m_vs, nullptr, 0);
		m_interface->context->PSSetShader(m_ps, nullptr, 0);
	}

}