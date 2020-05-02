#pragma once

#include "Render/RenderAPI.h"

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

		void DrawIndexed(Ref<CIndexBuffer> indexBuffer) override;

		Ref<CVertexBuffer> CreateVertexBuffer(float* vertices, int size) override;
		Ref<CIndexBuffer> CreateIndexBuffer(unsigned* indices, int size) override;
		Ref<CInputLayout> CreateInputLayout(std::initializer_list<SLayoutEntry> layout) override;
		Ref<CShader> CreateShader(std::string const& vsSrc, std::string const& psSrc) override;

	private:
		SDirectXInterface m_dxInterface;
	};

}
