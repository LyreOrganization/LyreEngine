#include "LyrePch.h"
#include "DirectX11API.h"

#include "Core/Application.h"
#include "WindowsWnd.h"

#define ZeroStruct(structure) ZeroMemory(&structure, sizeof(structure))

Lyre::CDirectX11API::CDirectX11API()
	: CRenderAPI(ERenderAPIType::DirectX_11)
{}

bool Lyre::CDirectX11API::Init(CApplication const& app)
{
	HRESULT hr;

	CWindowsWnd const* wnd = static_cast<CWindowsWnd const*>(app.GetWindow());
	UINT width = wnd->GetWidth();
	UINT height = wnd->GetHeight();

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	{
		ZeroStruct(swapChainDesc);
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = wnd->getHandle();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;
	}

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr, 
			driverType, 
			nullptr,
			D3D11_CREATE_DEVICE_DEBUGGABLE & D3D11_CREATE_DEVICE_DEBUG,
			featureLevels, 
			numFeatureLevels, 
			D3D11_SDK_VERSION,
			&swapChainDesc, 
			&m_swapChain, 
			&m_device, 
			&m_featureLevel, 
			&m_context);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return false;

	//Creating RTV
	CComPtr<ID3D11Texture2D> swapChainBuffer = nullptr;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&swapChainBuffer);

	if (FAILED(hr))
		return false;

	hr = m_device->CreateRenderTargetView(swapChainBuffer, nullptr, &m_renderTargetView);

	if (FAILED(hr))
		return false;

	//Creating DSBuffer
	CComPtr<ID3D11Texture2D> pDSBuffer = nullptr;
	D3D11_TEXTURE2D_DESC depthSurface;
	{
		ZeroStruct(depthSurface);
		depthSurface.Width = width;
		depthSurface.Height = height;
		depthSurface.MipLevels = 1;
		depthSurface.ArraySize = 1;
		depthSurface.Format = DXGI_FORMAT_D32_FLOAT;
		depthSurface.SampleDesc.Count = 1;
		depthSurface.SampleDesc.Quality = 0;
		depthSurface.Usage = D3D11_USAGE_DEFAULT;
		depthSurface.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	}
	hr = m_device->CreateTexture2D(&depthSurface, nullptr, &pDSBuffer);

	if (FAILED(hr))
		return false;

	//Creating DSState
	D3D11_DEPTH_STENCIL_DESC depthStencil;
	{
		ZeroStruct(depthStencil);
		depthStencil.DepthEnable = true;
		depthStencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencil.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		depthStencil.StencilEnable = true;
		depthStencil.StencilReadMask = 0xFF;
		depthStencil.StencilWriteMask = 0xFF;
		depthStencil.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencil.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencil.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencil.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencil.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencil.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencil.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencil.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	}
	CComPtr<ID3D11DepthStencilState> depthStencilState;
	hr = m_device->CreateDepthStencilState(&depthStencil, &depthStencilState);

	if (FAILED(hr))
		return false;

	m_context->OMSetDepthStencilState(depthStencilState, 1);
	//Creting DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	{
		ZeroStruct(depthStencilViewDesc);
		depthStencilViewDesc.Format = depthSurface.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
	}
	hr = m_device->CreateDepthStencilView(pDSBuffer, &depthStencilViewDesc, &m_depthStencilView);

	if (FAILED(hr))
		return false;

	m_context->OMSetRenderTargets(1, &m_renderTargetView.p, m_depthStencilView);

	D3D11_BLEND_DESC blendStateDesc;
	{
		ZeroStruct(blendStateDesc);
		blendStateDesc.AlphaToCoverageEnable = 0;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	CComPtr<ID3D11BlendState> blendState;
	hr = m_device->CreateBlendState(&blendStateDesc, &blendState);

	if (FAILED(hr))
		return false;

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_context->OMSetBlendState(blendState, blendFactor, 0xffffffff);

	//Setting rasterizer states
	///solid
	D3D11_RASTERIZER_DESC rasterizerState;
	{
		ZeroStruct(rasterizerState);
		rasterizerState.FillMode = D3D11_FILL_SOLID;
		rasterizerState.CullMode = D3D11_CULL_BACK;
		rasterizerState.FrontCounterClockwise = false;
		rasterizerState.DepthBias = 0;
		rasterizerState.SlopeScaledDepthBias = 0.0f;
		rasterizerState.DepthBiasClamp = 0.0f;
		rasterizerState.DepthClipEnable = true;
		rasterizerState.ScissorEnable = false;
		rasterizerState.MultisampleEnable = false;
		rasterizerState.AntialiasedLineEnable = false;
	}
	hr = m_device->CreateRasterizerState(&rasterizerState, &m_rasterizerStateSolid);

	if (FAILED(hr))
		return false;

	///wireframe
	{
		ZeroStruct(rasterizerState);
		rasterizerState.FillMode = D3D11_FILL_WIREFRAME;
		rasterizerState.CullMode = D3D11_CULL_BACK;
		rasterizerState.FrontCounterClockwise = false;
		rasterizerState.DepthBias = 0;
		rasterizerState.SlopeScaledDepthBias = 0.0f;
		rasterizerState.DepthBiasClamp = 0.0f;
		rasterizerState.DepthClipEnable = true;
		rasterizerState.ScissorEnable = false;
		rasterizerState.MultisampleEnable = false;
		rasterizerState.AntialiasedLineEnable = false;
	}
	hr = m_device->CreateRasterizerState(&rasterizerState, &m_rasterizerStateWireframe);

	if (FAILED(hr))
		return false;

	m_context->RSSetState(m_rasterizerStateSolid);
	//Setup the viewport
	D3D11_VIEWPORT vp;
	{
		vp.Width = static_cast<FLOAT>(width);
		vp.Height = static_cast<FLOAT>(height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
	}
	m_context->RSSetViewports(1, &vp);

	return true;
}

void Lyre::CDirectX11API::Present()
{
	float clearColor[4] = { 0.3f, 0.5f, 0.9f, 1.0f };
	m_context->ClearRenderTargetView(m_renderTargetView, clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_context->RSSetState(m_rasterizerStateSolid);

	m_swapChain->Present(0, 0);
}
