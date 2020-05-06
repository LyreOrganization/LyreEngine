#pragma once

#include "Render/ShaderDataType.h"

namespace Lyre
{

	class CInputLayout
	{
	public:
		struct SEntry
		{
			EShaderDataType type;
			std::string name;
			int size;
			int offset;

			SEntry(EShaderDataType _type, std::string _name);
		};

	public:
		virtual ~CInputLayout() = default;

		virtual void Bind() = 0;

		inline int GetStride() const { return m_stride; }

	protected:
		CInputLayout(std::initializer_list<SEntry> const& layout);

	protected:
		std::vector<SEntry> m_entries;
		int m_stride;
	};

}
