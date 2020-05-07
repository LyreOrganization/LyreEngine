#include "LyrePch.h"
#include "Camera.h"

namespace Lyre
{

	CCamera::CCamera(glm::vec3 const& position,
					 glm::vec3 const& view,
					 glm::vec3 const& up)
		: m_position(position)
		, m_view(glm::normalize(view))
		, m_up(glm::normalize(up))
	{
		updateRight();
	}

	void CCamera::SetPosition(glm::vec3 const& position)
	{
		m_position = position;
	}

	void CCamera::SetView(glm::vec3 const& view)
	{
		m_view = glm::normalize(view);
		updateRight();
	}

	void CCamera::SetUp(glm::vec3 const& up)
	{
		m_up = glm::normalize(up);
		updateRight();
	}

	glm::mat4 CCamera::GetView() const
	{
		return glm::lookAt(m_position, m_position + m_view, m_up);
	}

	glm::mat4 CCamera::GetProjection() const
	{
		return glm::perspectiveFov(glm::pi<float>() / 3.f, 1280.f, 720.f, 0.1f, 100.f);
	}

	glm::mat4 CCamera::GetViewProjection() const
	{
		return GetProjection() * GetView();
	}

	void CCamera::Tilt(float angle)
	{
		m_view = glm::rotate(m_view, angle, m_right);
	}

	void CCamera::Pan(float angle)
	{
		m_view = glm::rotate(m_view, angle, m_up);
		updateRight();
	}

	void CCamera::Roll(float angle)
	{
		m_up = glm::rotate(m_up, angle, m_view);
		updateRight();
	}

	void CCamera::MoveAhead(float distance)
	{
		m_position += m_view * distance;
	}

	void CCamera::MoveAside(float distance)
	{
		m_position += m_right * distance;
	}

	void CCamera::updateRight()
	{
		m_right = glm::normalize(glm::cross(m_view, m_up));
	}

}
