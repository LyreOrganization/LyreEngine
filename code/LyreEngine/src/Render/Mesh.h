#pragma once

namespace Lyre
{

	class CVertexBuffer;
	class CIndexBuffer;
	class CShader;

	class CMesh
	{
		friend class CRenderer;
	public:
		CMesh(std::string const& filename);

		void SetShader(std::shared_ptr<CShader> shader);

		glm::mat4& GetModel() { return m_model; }
		glm::mat4 const& GetModel() const { return m_model; }

	private:
		std::shared_ptr<CVertexBuffer> m_vertexBuffer;
		std::shared_ptr<CIndexBuffer> m_indexBuffer;
		glm::mat4 m_model;

		std::shared_ptr<CShader> m_shader;
	};

}
