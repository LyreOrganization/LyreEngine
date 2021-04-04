#include "LyrePch.h"
#include "ShaderDataType.h"

namespace Lyre
{

	std::unordered_map<EShaderDataType, unsigned> const CShaderDataType::s_TypeSizes =
	{
		{ EShaderDataType::None,					0							},
		{ EShaderDataType::Float,					sizeof(float)				},
		{ EShaderDataType::Float2,					sizeof(float) * 2			},
		{ EShaderDataType::Float3,					sizeof(float) * 3			},
		{ EShaderDataType::Float4,					sizeof(float) * 4			},
		{ EShaderDataType::Matrix,					sizeof(float) * 4 * 4		}
	};

	unsigned CShaderDataType::GetSize(EShaderDataType type)
	{
		auto sizeIt = s_TypeSizes.find(type);
		LYRE_ASSERT(sizeIt != s_TypeSizes.end(), "Unknown shader data type.");
		return sizeIt->second;
	}

}
