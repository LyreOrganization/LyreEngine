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

	m_mesh = make_shared<CMesh>("../../data/sphere.obj");
	m_mesh->GetModel() = glm::scale(glm::mat4{ 1.f }, glm::vec3{ 2.f });

	shared_ptr<CTexture> albedo = CRenderer::GetAPI()->CreateTextureFromFile("Metal003_2K_Color.jpg");

	shared_ptr<CShader> shader = CRenderer::GetAPI()->CreateShaderFromFiles("test_vertex_shader", "test_pixel_shader");
	shader->AddConstantBuffer(m_cameraConstants);
	shader->AddTexture(albedo);
	m_mesh->SetShader(shader);
}

void Lyre::CApplication::Run()
{
	while (m_running)
	{
		m_window->OnUpdate();

		float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		CRenderer::GetAPI()->Clear(clearColor);

		m_cameraConstants->UpdateConstant(0, glm::value_ptr(m_camera->GetViewProjection()));
		CRenderer::Submit(m_mesh);

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
