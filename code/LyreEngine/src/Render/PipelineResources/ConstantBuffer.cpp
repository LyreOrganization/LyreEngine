#include "LyrePch.h"
#include "ConstantBuffer.h"

namespace Lyre
{

	CConstantBuffer::CConstantBuffer(std::initializer_list<SEntry> const& layout)
		: m_layout(layout)
		, m_size(0)
	{
		for (auto& entry : m_layout)
		{
			entry.offset = m_size;
			m_size += entry.size;
		}

		m_data = new byte[m_size];
	}

	CConstantBuffer::~CConstantBuffer()
	{
		delete[] m_data;
	}

}
