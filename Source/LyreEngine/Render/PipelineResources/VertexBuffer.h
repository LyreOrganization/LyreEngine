#pragma once

namespace Lyre
{

	class CInputLayout;

	class CVertexBuffer
	{
	public:
		virtual ~CVertexBuffer() = default;

		virtual void Bind() = 0;

		inline void SetLayout(std::shared_ptr<CInputLayout> const& layout) { m_layout = layout; }
		inline std::shared_ptr<const CInputLayout> GetLayout() const { return m_layout; }
		inline std::shared_ptr<CInputLayout> GetLayout() { return m_layout; }

	protected:
		CVertexBuffer() = default;

	protected:
		std::shared_ptr<CInputLayout> m_layout;
	};

}
