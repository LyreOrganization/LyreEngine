#pragma once

#include "Render/RenderAPI.h"

#include <d3d11.h>
#include <atlbase.h>

namespace Lyre
{

	class CDirectX11API final : public CRenderAPI
	{
	public:
		CDirectX11API();

		bool Init(CApplication const& app) override;
		void Present() override;

	private:
		CComPtr<IDXGISwapChain>						m_swapChain;
		CComPtr<ID3D11Device>						m_device;
		CComPtr<ID3D11DeviceContext>				m_context;
		D3D_FEATURE_LEVEL							m_featureLevel;

		CComPtr<ID3D11RenderTargetView>				m_renderTargetView;
		CComPtr<ID3D11DepthStencilView>				m_depthStencilView;

		CComPtr<ID3D11RasterizerState>				m_rasterizerStateSolid;
		CComPtr<ID3D11RasterizerState>				m_rasterizerStateWireframe;
	};

}
