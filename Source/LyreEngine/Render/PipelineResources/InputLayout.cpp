#include "LyrePch.h"
#include "InputLayout.h"

namespace Lyre
{

	CInputLayout::SEntry::SEntry(EShaderDataType _type, std::string _name)
		: type(_type)
		, name(_name)
		, size(CShaderDataType::GetSize(_type))
		, offset(0)
	{}

	CInputLayout::CInputLayout(std::initializer_list<SEntry> const& layout)
		: m_entries(layout)
		, m_stride(0)
	{
		for (auto& entry : m_entries)
		{
			entry.offset = m_stride;
			m_stride += entry.size;
		}
	}

}

