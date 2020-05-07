#include "LyrePch.h"

#include "Application.h"
#include "Window.h"
#include "WindowLifeTimeEvents.h"
#include "InputEvents.h"

#include "Render/Renderer.h"
#include "Render/Mesh.h"
#include "Render/Camera.h"

using namespace std;

EVENT_MAP_BEGIN(Lyre::CApplication)
	ADD_LISTENER(Lyre::CWindowClosedEvent, OnWindowClosed)
	ADD_LISTENER(Lyre::CMouseMoveEvent, OnMouseMove)
	ADD_LISTENER(Lyre::CMovementEvent, OnMovement)
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


	m_camera = make_shared<CCamera>(
		glm::vec3{ 0.f, 0.f, 5.f },
		glm::vec3{ 0.f, 0.f, -1.f },
		glm::vec3{ 0.f, 1.f, 0.f }
	);
	m_cameraConstants = CRenderer::GetAPI()->CreateConstantBuffer({
		EShaderDataType::Matrix
	});

	m_cameraConstants->UpdateConstant(0, glm::value_ptr(m_camera->GetViewProjection()));
}

void Lyre::CApplication::Run()
{
	string vsSrc = R"(
		cbuffer CAMERA : register(b0)
		{
			matrix viewProj;
		};

		cbuffer MODEL : register(b1)
		{
			matrix model;
		};

		struct VS_Input
		{
			float3 pos : POSITION;
			float3 normal : NORMAL;
		};
		struct VS_Output
		{
			float4 pos : SV_Position;
			float4 normal : NORMAL;
		};

		VS_Output main(VS_Input input)
		{
			VS_Output output = (VS_Output)0;
			matrix modelViewProj = mul(model, viewProj);
			output.pos = mul(float4(input.pos, 1.f), modelViewProj);
			output.normal = mul(float4(normalize(input.normal), 0.f), modelViewProj);
			return output;
		}
	)";

	string psSrc = R"(
		struct PS_Input
		{
			float4 pos : SV_Position;
			float4 normal : NORMAL;
		};

		float4 main(in PS_Input input) : SV_Target
		{
			float3 dirTolight = normalize(float3(1.f, 0.f, -1.f));
			float4 lightColor = float4(0.8f, 0.8f, 0.8f, 1.f);
			float4 diffuseColor = float4(0.3f, 0.5f, 0.45f, 1.f);

			float diffuseCoef = max(0.f, dot(input.normal.xyz, dirTolight).x);
			
			return lightColor * diffuseColor * diffuseCoef;
		}
	)";

	shared_ptr<CMesh> mesh = make_shared<CMesh>("../../data/garg.obj");
	mesh->GetModel() = glm::scale(glm::mat4{ 1.f }, glm::vec3{ 2.f });

	shared_ptr<CShader> shader = CRenderer::GetAPI()->CreateShader(vsSrc, psSrc);
	shader->AddConstantBuffer(m_cameraConstants);
	mesh->SetShader(shader);

	while (m_running)
	{
		m_window->OnUpdate();

		float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		CRenderer::GetAPI()->Clear(clearColor);

		m_cameraConstants->UpdateConstant(0, glm::value_ptr(m_camera->GetViewProjection()));
		CRenderer::Submit(mesh);

		CRenderer::Present();
	}
}

bool Lyre::CApplication::OnWindowClosed(CWindowClosedEvent const& event)
{
	m_running = false;
	return true;
}

bool Lyre::CApplication::OnMouseMove(CMouseMoveEvent const& event)
{
	m_camera->Pan(event.dx);
	m_camera->Tilt(event.dy);
	return true;
}

bool Lyre::CApplication::OnMovement(CMovementEvent const& event)
{
	m_camera->MoveAhead(event.ahead);
	m_camera->MoveAside(event.aside);
	return true;
}
