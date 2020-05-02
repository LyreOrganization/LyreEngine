#include "LyrePch.h"
#include "Application.h"
#include "Window.h"
#include "Render/Renderer.h"

EVENT_MAP_BEGIN(Lyre::CApplication)
	ADD_LISTENER(Lyre::CWindowClosedEvent, OnWindowClosed)
EVENT_MAP_END()

Lyre::CApplication::CApplication()
	: m_running(true)
{
	m_window = CWindow::Create(this);
}

Lyre::CApplication::~CApplication()
{
	delete m_window;
}

void Lyre::CApplication::Init()
{
	CRenderer::CreateAPIIntance(ERenderAPIType::DirectX_11);
	bool success = CRenderer::GetAPI()->Init(*this);
	LYRE_ASSERT(success);
}

void Lyre::CApplication::Run()
{
	while (m_running)
	{
		m_window->OnUpdate();


		float vertices[] = {
			0.0f, 0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f
		};

		unsigned indices[] = { 0, 1, 2 };

		std::string vsSrc = R"(
			float4 main(float3 pos : POSITION) : SV_Position
			{
				return float4(pos, 1.f);
			}
		)";

			std::string psSrc = R"(
			float4 main() : SV_Target
			{
				return float4(1.f, 0.f, 0.f, 1.f);
			}
		)";

		Ref<CVertexBuffer> vertexBuffer = CRenderer::GetAPI()->CreateVertexBuffer(vertices, sizeof(vertices) / sizeof(float));
		Ref<CIndexBuffer> indexBuffer = CRenderer::GetAPI()->CreateIndexBuffer(indices, sizeof(indices) / sizeof(int));

		Ref<CInputLayout> layout = CRenderer::GetAPI()->CreateInputLayout({
			{ EShaderDataType::Float3, "POSITION" }
		});

		vertexBuffer->SetLayout(layout);

		Ref<CShader> shader = CRenderer::GetAPI()->CreateShader(vsSrc, psSrc);

		float clearColor[] = { 0.3f, 0.5f, 0.9f, 1.0f };
		CRenderer::GetAPI()->Clear(clearColor);
		CRenderer::Submit(vertexBuffer, indexBuffer, shader);

		CRenderer::Present();
	}
}

bool Lyre::CApplication::OnWindowClosed(CWindowClosedEvent const & event)
{
	m_running = false;
	return true;
}
