#include "LyrePch.h"
#include "InputLayout.h"

int GetShadeDataTypeSize(Lyre::EShaderDataType type)
{
	switch (type)
	{
	case Lyre::EShaderDataType::Float:
		return 4;
	case Lyre::EShaderDataType::Float2:
		return 4 * 2;
	case Lyre::EShaderDataType::Float3:
		return 4 * 3;
	case Lyre::EShaderDataType::Float4:
		return 4 * 4;
	case Lyre::EShaderDataType::Matrix:
		return 4 * 4 * 4;
	default:
		LYRE_ASSERT(false, "Unknown shader data type.");
		return 0;
	}
}

Lyre::SLayoutEntry::SLayoutEntry(EShaderDataType _type, std::string _name)
	: type(_type)
	, name(_name)
	, size(GetShadeDataTypeSize(_type))
	, offset(0)
{}

Lyre::CInputLayout::CInputLayout(std::initializer_list<SLayoutEntry> const& layout)
	: m_entries(layout)
	, m_stride(0)
{
	int offset = 0;
	for (auto& entry : m_entries)
	{
		entry.offset = offset;
		offset += entry.size;
		m_stride += entry.size;
	}
}
