#include "stdafx.h"

#include "LyreEngine.h"

#include "resource.h"

#include "Planet.h"
#include "FreeCamera.h"
#include "TargetCamera.h"
#include "Controls.h"

using namespace std;
using namespace DirectX;

namespace {

	struct ViewConstantBuffer {
		XMFLOAT4X4 view;
	};

	struct ProjectionConstantBuffer {
		XMFLOAT4X4 projection;
	};

	struct LightingConstantBuffer {
		XMFLOAT3 direction;
		float power;
		XMFLOAT4 diffuse;
	};

	struct LodConstantBuffer {
		float minDistance;
		float maxDistance;
		float minLOD;
		float maxLOD;
	};

	HINSTANCE							g_hInstance = nullptr;
	HWND								g_hWindow = nullptr;

	D3D_DRIVER_TYPE						g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL					g_featureLevel = D3D_FEATURE_LEVEL_11_0;

	CComPtr<ID3D11Device>				g_iDevice = nullptr;
	CComPtr<ID3D11DeviceContext>		g_iContext = nullptr;

	CComPtr<IDXGISwapChain>				g_iSwapChain = nullptr;

	CComPtr<ID3D11RenderTargetView>		g_iRTV = nullptr;

	CComPtr<ID3D11DepthStencilState>	g_iDSState = nullptr;
	CComPtr<ID3D11DepthStencilView>		g_iDSV = nullptr;

	CComPtr<ID3D11BlendState>			g_iBlendState = nullptr;

	CComPtr<ID3D11RasterizerState>		g_iRasterizerStateWireframe = nullptr;
	CComPtr<ID3D11RasterizerState>		g_iRasterizerStateSolid = nullptr;

	CComPtr<ID3D11Buffer>				g_iViewConstantBuffer = nullptr;
	CComPtr<ID3D11Buffer>				g_iProjectionConstantBuffer = nullptr;
	CComPtr<ID3D11Buffer>				g_iLightingConstantBuffer = nullptr;
	CComPtr<ID3D11Buffer>				g_iLodConstantBuffer = nullptr;

	CComPtr<ID3D11SamplerState>			g_iTex2DSampler;

	std::unique_ptr<Planet>				g_pPlanet;

	std::unique_ptr<Camera>				g_pCamera;

	float								g_lightAngle;

	HRESULT init() {
		HRESULT hr;

		UINT width, height;
		LyreEngine::getClientWH(width, height);

		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0
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
			swapChainDesc.OutputWindow = g_hWindow;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.Windowed = TRUE;
		}
		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
			g_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDeviceAndSwapChain(nullptr, g_driverType, nullptr, D3D11_CREATE_DEVICE_DEBUG, featureLevels,
											   numFeatureLevels, D3D11_SDK_VERSION, &swapChainDesc,
											   &g_iSwapChain, &g_iDevice, &g_featureLevel, &g_iContext);
			if (SUCCEEDED(hr))
				break;
		}
		if (FAILED(hr))
			return hr;

		//Creating RTV
		CComPtr<ID3D11Texture2D> pSwapChainBuffer = nullptr;
		hr = g_iSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pSwapChainBuffer);
		if (FAILED(hr))
			return hr;
		hr = g_iDevice->CreateRenderTargetView(pSwapChainBuffer, nullptr, &g_iRTV);
		if (FAILED(hr))
			return hr;

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
		hr = g_iDevice->CreateTexture2D(&depthSurface, nullptr, &pDSBuffer);
		if (FAILED(hr))
			return hr;
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
		hr = g_iDevice->CreateDepthStencilState(&depthStencil, &g_iDSState);
		if (FAILED(hr))
			return hr;
		g_iContext->OMSetDepthStencilState(g_iDSState, 1);
		//Creting DSV
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		{
			ZeroStruct(depthStencilViewDesc);
			depthStencilViewDesc.Format = depthSurface.Format;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDesc.Texture2D.MipSlice = 0;
		}
		hr = g_iDevice->CreateDepthStencilView(pDSBuffer, &depthStencilViewDesc, &g_iDSV);
		if (FAILED(hr))
			return hr;
		g_iContext->OMSetRenderTargets(1, &g_iRTV.p, g_iDSV);

		D3D11_BLEND_DESC blendStateDesc;
		{
			ZeroStruct(blendStateDesc);
			blendStateDesc.AlphaToCoverageEnable = 0;
			blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		hr = g_iDevice->CreateBlendState(&blendStateDesc, &g_iBlendState);
		if (FAILED(hr))
			return hr;
		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		g_iContext->OMSetBlendState(g_iBlendState, blendFactor, 0xffffffff);

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
		hr = g_iDevice->CreateRasterizerState(&rasterizerState, &g_iRasterizerStateSolid);
		if (FAILED(hr))
			return hr;
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
		hr = g_iDevice->CreateRasterizerState(&rasterizerState, &g_iRasterizerStateWireframe);
		if (FAILED(hr))
			return hr;
		g_iContext->RSSetState(g_iRasterizerStateSolid);
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
		g_iContext->RSSetViewports(1, &vp);

		D3D11_BUFFER_DESC bufferDesc;

		//View constant buffer
		{
			ZeroStruct(bufferDesc);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = sizeof(ViewConstantBuffer);
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		}
		hr = g_iDevice->CreateBuffer(&bufferDesc, nullptr, &g_iViewConstantBuffer);
		if (FAILED(hr))
			return hr;

		//Projection constant buffer
		{
			ZeroStruct(bufferDesc);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = sizeof(ProjectionConstantBuffer);
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		}
		hr = g_iDevice->CreateBuffer(&bufferDesc, nullptr, &g_iProjectionConstantBuffer);
		if (FAILED(hr))
			return hr;

		//Lighting constant buffer
		{
			ZeroStruct(bufferDesc);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = sizeof(LightingConstantBuffer);
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		}
		hr = g_iDevice->CreateBuffer(&bufferDesc, nullptr, &g_iLightingConstantBuffer);
		if (FAILED(hr))
			return hr;

		//LOD constant buffer
		{
			ZeroStruct(bufferDesc);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = sizeof(LodConstantBuffer);
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		}
		hr = g_iDevice->CreateBuffer(&bufferDesc, nullptr, &g_iLodConstantBuffer);
		if (FAILED(hr))
			return hr;

		//Texture sampler
		D3D11_SAMPLER_DESC tex2DSamplerDesc;
		{
			ZeroStruct(tex2DSamplerDesc);
			tex2DSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			tex2DSamplerDesc.AddressU = tex2DSamplerDesc.AddressV = tex2DSamplerDesc.AddressW =
				D3D11_TEXTURE_ADDRESS_CLAMP;
			tex2DSamplerDesc.MinLOD = 0.f;
			tex2DSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		}
		hr = LyreEngine::getDevice()->CreateSamplerState(&tex2DSamplerDesc, &g_iTex2DSampler);
		if (FAILED(hr))
			return hr;

		//Planet
		g_pPlanet = make_unique<Planet>(sqrt(3.f));
		hr = g_pPlanet->init();
		if (FAILED(hr))
			throw runtime_error("Planet init failed!");

		//Camera
		g_pCamera = make_unique<TargetCamera>(XMFLOAT3 { -4.f, 0.f, 0.f },
											  XMFLOAT3 { 0.f, 0.f, 0.f }, sqrt(3.f));
		//Setup common Camera actions
		{
			Controls::ActionGroup camera("Camera");

			camera.action("SwitchToFreeCamera").onTriggered([]() {
				FreeCamera* pFreeCamera = dynamic_cast<FreeCamera*>(g_pCamera.get());
				if (pFreeCamera == nullptr) {
					g_pCamera = make_unique<FreeCamera>(*g_pCamera);
				}
			});
			camera.action("SwitchToTargetCamera").onTriggered([]() {
				TargetCamera* pTargetCamera = dynamic_cast<TargetCamera*>(g_pCamera.get());
				if (pTargetCamera == nullptr) {
					g_pCamera = make_unique<TargetCamera>(*g_pCamera, XMFLOAT3{ 0.f, 0.f, 0.f }, sqrt(3.f));
				}
			});
			camera.action("ToggleWireframe").onTriggered([]() {
				static bool s_bWireframe = false;
				if (s_bWireframe ^= true) g_iContext->RSSetState(g_iRasterizerStateWireframe);
				else g_iContext->RSSetState(g_iRasterizerStateSolid);
			}, false);

			Controls::addActionGroup(camera);
		}
		//Setup Free Camera actions
		{
			Controls::ActionGroup camera("FreeCamera");

			camera.action("RollCW").on([](DWORD ticksPerFrame) {
				FreeCamera* pFreeCamera = dynamic_cast<FreeCamera*>(g_pCamera.get());
				if (pFreeCamera != nullptr) {
					pFreeCamera->roll(-0.001f*ticksPerFrame);
				}
			});
			camera.action("RollCCW").on([](DWORD ticksPerFrame) {
				FreeCamera* pFreeCamera = dynamic_cast<FreeCamera*>(g_pCamera.get());
				if (pFreeCamera != nullptr) {
					pFreeCamera->roll(0.001f*ticksPerFrame);
				}
			});
			camera.action("MoveForward").on([](DWORD ticksPerFrame) {
				FreeCamera* pFreeCamera = dynamic_cast<FreeCamera*>(g_pCamera.get());
				if (pFreeCamera != nullptr) {
					pFreeCamera->moveAhead(0.001f*ticksPerFrame);
				}
			});
			camera.action("MoveBackward").on([](DWORD ticksPerFrame) {
				FreeCamera* pFreeCamera = dynamic_cast<FreeCamera*>(g_pCamera.get());
				if (pFreeCamera != nullptr) {
					pFreeCamera->moveAhead(-0.001f*ticksPerFrame);
				}
			});
			camera.action("MoveRight").on([](DWORD ticksPerFrame) {
				FreeCamera* pFreeCamera = dynamic_cast<FreeCamera*>(g_pCamera.get());
				if (pFreeCamera != nullptr) {
					pFreeCamera->moveAside(0.001f*ticksPerFrame);
				}
			});
			camera.action("MoveLeft").on([](DWORD ticksPerFrame) {
				FreeCamera* pFreeCamera = dynamic_cast<FreeCamera*>(g_pCamera.get());
				if (pFreeCamera != nullptr) {
					pFreeCamera->moveAside(-0.001f*ticksPerFrame);
				}
			});

			Controls::addActionGroup(camera);
		}
		//Setup Target Camera actions
		{
			Controls::ActionGroup camera("TargetCamera");

			camera.action("RotateUp").on([](DWORD ticksPerFrame) {
				TargetCamera* pTargetCamera = dynamic_cast<TargetCamera*>(g_pCamera.get());
				if (pTargetCamera != nullptr) {
					pTargetCamera->rotateAroundHorizontally(0.0002f*ticksPerFrame);
				}
			});
			camera.action("RotateDown").on([](DWORD ticksPerFrame) {
				TargetCamera* pTargetCamera = dynamic_cast<TargetCamera*>(g_pCamera.get());
				if (pTargetCamera != nullptr) {
					pTargetCamera->rotateAroundHorizontally(-0.0002f*ticksPerFrame);
				}
			});
			camera.action("RotateRight").on([](DWORD ticksPerFrame) {
				TargetCamera* pTargetCamera = dynamic_cast<TargetCamera*>(g_pCamera.get());
				if (pTargetCamera != nullptr) {
					pTargetCamera->rotateAroundVertically(0.0002f*ticksPerFrame);
				}
			});
			camera.action("RotateLeft").on([](DWORD ticksPerFrame) {
				TargetCamera* pTargetCamera = dynamic_cast<TargetCamera*>(g_pCamera.get());
				if (pTargetCamera != nullptr) {
					pTargetCamera->rotateAroundVertically(-0.0002f*ticksPerFrame);
				}
			});
			camera.action("Approach").on([](DWORD ticksPerFrame) {
				TargetCamera* pTargetCamera = dynamic_cast<TargetCamera*>(g_pCamera.get());
				if (pTargetCamera != nullptr) {
					pTargetCamera->approach(0.05f*ticksPerFrame);
				}
			});
			camera.action("MoveFurther").on([](DWORD ticksPerFrame) {
				TargetCamera* pTargetCamera = dynamic_cast<TargetCamera*>(g_pCamera.get());
				if (pTargetCamera != nullptr) {
					pTargetCamera->approach(-0.05f*ticksPerFrame);
				}
			});
			camera.action("SpinCW").on([](DWORD ticksPerFrame) {
				TargetCamera* pTargetCamera = dynamic_cast<TargetCamera*>(g_pCamera.get());
				if (pTargetCamera != nullptr) {
					pTargetCamera->spin(0.001f*ticksPerFrame);
				}
			});
			camera.action("SpinCCW").on([](DWORD ticksPerFrame) {
				TargetCamera* pTargetCamera = dynamic_cast<TargetCamera*>(g_pCamera.get());
				if (pTargetCamera != nullptr) {
					pTargetCamera->spin(-0.001f*ticksPerFrame);
				}
			});
			camera.action("TiltUp").on([](DWORD ticksPerFrame) {
				TargetCamera* pTargetCamera = dynamic_cast<TargetCamera*>(g_pCamera.get());
				if (pTargetCamera != nullptr) {
					pTargetCamera->tilt(0.001f*ticksPerFrame);
				}
			});
			camera.action("TiltDown").on([](DWORD ticksPerFrame) {
				TargetCamera* pTargetCamera = dynamic_cast<TargetCamera*>(g_pCamera.get());
				if (pTargetCamera != nullptr) {
					pTargetCamera->tilt(-0.001f*ticksPerFrame);
				}
			});

			Controls::addActionGroup(camera);
		}

		//Lighting
		g_lightAngle = 0.f;
		//Setup Lighting actions
		{
			Controls::ActionGroup lighting("Lighting");

			lighting.action("RotateCW").on([](DWORD ticksPerFrame) {
				g_lightAngle += ticksPerFrame / 500.f;
				if (g_lightAngle > XM_PI*2.f) g_lightAngle = 0.f;
			});
			lighting.action("RotateCCW").on([](DWORD ticksPerFrame) {
				g_lightAngle -= ticksPerFrame / 500.f;
				if (g_lightAngle < -XM_PI*2.f) g_lightAngle = 0.f;
			});

			Controls::addActionGroup(lighting);
		}

		return S_OK;
	}

}

void LyreEngine::render(DWORD ticksPerFrame) {
	ViewConstantBuffer cbView;
	cbView.view = g_pCamera->calculateViewMatrix();
	g_iContext->UpdateSubresource(g_iViewConstantBuffer, 0, nullptr, &cbView, 0, 0);

	ProjectionConstantBuffer cbProjection;
	cbProjection.projection = g_pCamera->calculateProjectionMatrix(WND_WIDTH / static_cast<FLOAT>(WND_HEIGHT));
	g_iContext->UpdateSubresource(g_iProjectionConstantBuffer, 0, nullptr, &cbProjection, 0, 0);

	LightingConstantBuffer cbLight;
	cbLight.diffuse = { 1.0f, 0.9f, 0.7f, 1.f };
	cbLight.direction = { sin(g_lightAngle), 0.f, cos(g_lightAngle) };
	cbLight.power = 0.8f;
	g_iContext->UpdateSubresource(g_iLightingConstantBuffer, 0, nullptr, &cbLight, 0, 0);

	LodConstantBuffer cbLod;
	cbLod.minDistance = 1.f;
	cbLod.maxDistance = 32.f;
	cbLod.minLOD = 1.f;
	cbLod.maxLOD = 63.f;
	g_iContext->UpdateSubresource(g_iLodConstantBuffer, 0, nullptr, &cbLod, 0, 0);

	float sky = 0;
	XMFLOAT3 eye = g_pCamera->getPosition();
	//XMStoreFloat(&sky, XMVector3Dot(XMVector3Normalize(XMLoadFloat3(&eye)), XMLoadFloat3(&cbLight.direction)));
	float clearColor[4] = { 0.3f*sky, 0.5f*sky, 0.9f*sky, 1.0f };
	g_iContext->ClearRenderTargetView(g_iRTV, clearColor);
	g_iContext->ClearDepthStencilView(g_iDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	g_pPlanet->render();

	g_iSwapChain->Present(0, 0);
}

void LyreEngine::getClientWH(UINT &width, UINT &height) {
	RECT rc;
	GetClientRect(g_hWindow, &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
}

HRESULT LyreEngine::initWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc) {
	WNDCLASSEX wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = LoadIcon(hInst, reinterpret_cast<LPCTSTR>(IDI_ICON));
		wcex.hCursor = LoadCursor(nullptr, reinterpret_cast<LPCTSTR>IDC_ARROW);
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = L"LyreEngine";
		wcex.hIconSm = LoadIcon(wcex.hInstance, reinterpret_cast<LPCTSTR>(IDI_ICON));
	}
	if (!RegisterClassEx(&wcex))
		return E_FAIL;
	g_hInstance = hInst;
	RECT rc = { 0, 0, WND_WIDTH, WND_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWindow = CreateWindow(L"LyreEngine", L"LyreEngine", WS_OVERLAPPEDWINDOW, WND_POS_X,
							 WND_POS_Y, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);
	if (!g_hWindow)
		return E_FAIL;
	ShowCursor(FALSE);
	SetCursorPos(WND_WIDTH / 2, WND_HEIGHT / 2);
	ShowWindow(g_hWindow, nCmdShow);

	return S_OK;
}

ID3D11Device* LyreEngine::getDevice() {
	if (g_iDevice == nullptr)
		if (FAILED(init())) {
			throw runtime_error("D3D11 init failed!");
		}

	return g_iDevice;
}

ID3D11DeviceContext* LyreEngine::getContext() {
	if (g_iContext == nullptr)
		if (FAILED(init())) {
			throw runtime_error("D3D11 init failed!");
		}

	return g_iContext;
}

ID3D11Buffer* LyreEngine::getViewCB() {
	return g_iViewConstantBuffer;
}

ID3D11Buffer* LyreEngine::getProjectionCB() {
	return g_iProjectionConstantBuffer;
}

ID3D11Buffer* LyreEngine::getLightingCB() {
	return g_iLightingConstantBuffer;
}

ID3D11Buffer* LyreEngine::getLodCB() {
	return g_iLodConstantBuffer;
}

ID3D11SamplerState * LyreEngine::getSampler2D() {
	return g_iTex2DSampler;
}

Camera* LyreEngine::getCamera() {
	return g_pCamera.get();
}

HRESULT LyreEngine::readShaderFromFile(const WCHAR* szFileName, std::vector<char> &shaderBytecode) {
	std::ifstream input(szFileName, std::ios::binary);
	shaderBytecode = std::vector<char>(std::istreambuf_iterator<char>(input),
									   std::istreambuf_iterator<char>());
	return (shaderBytecode.size() == 0 ? E_FAIL : S_OK);
}
