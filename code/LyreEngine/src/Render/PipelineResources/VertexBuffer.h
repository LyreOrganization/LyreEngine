#pragma once

namespace Lyre
{

	class CInputLayout;

	class CVertexBuffer
	{
	public:
		virtual ~CVertexBuffer() = default;

		virtual void Bind() = 0;

		inline void SetLayout(Ref<CInputLayout> layout) { m_layout = layout; }
		inline Ref<CInputLayout> GetLayout() const { return m_layout; }

	protected:
		CVertexBuffer() = default;

	protected:
		Ref<CInputLayout> m_layout;
	};

}
