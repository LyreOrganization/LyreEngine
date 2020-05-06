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
			0.5f, -0.5f, 0.0f,			0.f, 1.f, 0.f, 1.f,
			0.0f, 0.5f, 0.0f,			1.f, 0.f, 0.f, 1.f,
			-0.5f, -0.5f, 0.0f,			0.f, 0.f, 1.f, 1.f
		};

		unsigned indices[] = { 0, 1, 2 };

		std::string vsSrc = R"(
			struct VS_Input
			{
				float3 pos : POSITION;
				float4 color : COLOR;
			};
			struct VS_Output
			{
				float4 pos : SV_Position;
				float4 color : COLOR;
			};

			VS_Output main(VS_Input input)
			{
				VS_Output output = (VS_Output)0;
				output.pos = float4(input.pos, 1.f);
				output.color = input.color;
				return output;
			}
		)";

		std::string psSrc = R"(
			struct PS_Input
			{
				float4 pos : SV_Position;
				float4 color : COLOR;
			};
			float4 main(in PS_Input input) : SV_Target
			{
				return input.color;
			}
		)";

		std::shared_ptr<CVertexBuffer> vertexBuffer = CRenderer::GetAPI()->CreateVertexBuffer(vertices, sizeof(vertices) / sizeof(float));
		std::shared_ptr<CIndexBuffer> indexBuffer = CRenderer::GetAPI()->CreateIndexBuffer(indices, sizeof(indices) / sizeof(int));

		std::shared_ptr<CInputLayout> layout = CRenderer::GetAPI()->CreateInputLayout({
			{ EShaderDataType::Float3, "POSITION" },
			{ EShaderDataType::Float4, "COLOR" }
		});

		vertexBuffer->SetLayout(layout);

		std::shared_ptr<CShader> shader = CRenderer::GetAPI()->CreateShader(vsSrc, psSrc);

		float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		CRenderer::GetAPI()->Clear(clearColor);
		CRenderer::Submit(vertexBuffer, indexBuffer, shader);

		CRenderer::Present();
	}
}

bool Lyre::CApplication::OnWindowClosed(CWindowClosedEvent const& event)
{
	m_running = false;
	return true;
}
