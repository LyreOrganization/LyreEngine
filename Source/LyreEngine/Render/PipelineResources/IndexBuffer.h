#pragma once

namespace Lyre
{

	enum class EDrawTopology
	{
		None = 0,
		Triangles
	};

	class CIndexBuffer
	{
	public:
		virtual ~CIndexBuffer() = default;

		virtual void Bind(EDrawTopology topology) = 0;

		inline unsigned GetSize() const { return m_size; }

	protected:
		CIndexBuffer(unsigned size)
			: m_size(size)
		{}

	protected:
		unsigned m_size;
	};

}
