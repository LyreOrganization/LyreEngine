#pragma once

namespace Lyre
{

	enum class EShaderDataType
	{
		None = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Matrix
	};

	class CShaderDataType
	{
	public:
		static unsigned GetSize(EShaderDataType type);

	private:
		static std::unordered_map<EShaderDataType, unsigned> const s_TypeSizes;
	};

}
