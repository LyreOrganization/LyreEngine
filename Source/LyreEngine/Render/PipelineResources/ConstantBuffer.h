#pragma once

#include "Render/ShaderDataType.h"

namespace Lyre
{

	class CConstantBuffer
	{
	public:
		struct SEntry
		{
			EShaderDataType type;
			unsigned offset;
			unsigned size;

			SEntry(EShaderDataType _type)
				: type(_type)
				, size(CShaderDataType::GetSize(_type))
				, offset(0)
			{}
		};

	public:
		virtual ~CConstantBuffer();

		virtual void UpdateConstant(unsigned position, void const* data) = 0;
		virtual void Update() = 0;

	protected:
		CConstantBuffer(std::initializer_list<SEntry> const& layout);

	protected:
		std::vector<SEntry> m_layout;
		void* m_data;
		unsigned m_size;
	};

}
