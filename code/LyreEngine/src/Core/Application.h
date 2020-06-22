#pragma once

#include "Event.h"

namespace Lyre
{
	class CWindow;
	class CCamera;
	class CConstantBuffer;
	class CMesh;

	class CWindowClosedEvent;
	class CMouseMoveEvent;
	class CMovementEvent;

	class CApplication : public EventListener
	{
	public:
		CApplication();
		CApplication(CApplication const&) = delete;
		CApplication(CApplication &&) = delete;

		virtual ~CApplication();

		void Init();
		void Run();

		CWindow* GetWindow() { return m_window; }
		CWindow const* GetWindow() const { return m_window; }

		EVENT_MAP(CApplication)
		bool OnWindowClosed(CWindowClosedEvent const& event);
		bool OnMouseMove(CMouseMoveEvent const& event);
		bool OnMovement(CMovementEvent const& event);

	private:
		bool m_running;
		CWindow* m_window;

		std::shared_ptr<CCamera> m_camera;
		std::shared_ptr<CConstantBuffer> m_cameraConstants;

		std::shared_ptr<CMesh> m_mesh;
	};

}
