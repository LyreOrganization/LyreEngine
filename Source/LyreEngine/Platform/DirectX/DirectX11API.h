#pragma once

#include <Render/RenderAPI.h>

namespace Lyre
{

	struct SDirectXInterface
	{
		CComPtr<IDXGISwapChain>						swapChain;
		CComPtr<ID3D11Device>						device;
		CComPtr<ID3D11DeviceContext>				context;
		D3D_FEATURE_LEVEL							featureLevel;

		CComPtr<ID3D11RenderTargetView>				renderTargetView;
		CComPtr<ID3D11DepthStencilView>				depthStencilView;

		CComPtr<ID3D11RasterizerState>				rasterizerStateSolid;
		CComPtr<ID3D11RasterizerState>				rasterizerStateWireframe;
	};

	class CDirectX11API final : public CRenderAPI
	{
	public:
		CDirectX11API();

		bool Init(CApplication const& app) override;

		void Clear(float color[4]) override;
		void Present() override;

		void DrawIndexed(std::shared_ptr<CIndexBuffer> indexBuffer) override;

		std::shared_ptr<CVertexBuffer> CreateVertexBuffer(float* vertices, unsigned size) override;
		std::shared_ptr<CIndexBuffer> CreateIndexBuffer(unsigned* indices, unsigned size) override;
		std::shared_ptr<CInputLayout> CreateInputLayout(std::initializer_list<CInputLayout::SEntry> layout) override;
		
		std::shared_ptr<CConstantBuffer> CreateConstantBuffer(std::initializer_list<CConstantBuffer::SEntry> layout) override;
		
		std::shared_ptr<CShader> CreateShaderFromSources(std::string const& vsSrc, std::string const& psSrc) override;
		std::shared_ptr<CShader> CreateShaderFromFiles(std::string const& vsFile, std::string const& psFile) override;

		std::shared_ptr<CTexture> CreateTextureFromFile(std::string const& filename) override;

	private:
		SDirectXInterface m_dxInterface;
	};


#define ZeroStruct(structure) ZeroMemory(&structure, sizeof(structure))

}
