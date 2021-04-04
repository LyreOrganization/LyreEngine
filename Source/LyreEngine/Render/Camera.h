#pragma once

namespace Lyre
{

	class CCamera
	{
	public:
		CCamera(glm::vec3 const& position,
				glm::vec3 const& view,
				glm::vec3 const& up);

		void SetPosition(glm::vec3 const& position);
		void SetView(glm::vec3 const& view);
		void SetUp(glm::vec3 const& up);

		glm::mat4 GetView() const;
		glm::mat4 GetProjection() const;
		glm::mat4 GetViewProjection() const;

		void Tilt(float angle);
		void Pan(float angle);
		void Roll(float angle);

		void MoveAhead(float distance);
		void MoveAside(float distance);

	private:
		void updateRight();

	private:
		glm::vec3 m_position;
		glm::vec3 m_view;
		glm::vec3 m_up;
		glm::vec3 m_right;
	};

}
