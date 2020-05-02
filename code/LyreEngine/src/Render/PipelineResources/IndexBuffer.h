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

		inline int GetSize() const { return m_size; }

	protected:
		CIndexBuffer(int size)
			: m_size(size)
		{}

	protected:
		int m_size;
	};

}
