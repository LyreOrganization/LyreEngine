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

	struct SLayoutEntry
	{
		EShaderDataType type;
		std::string name;
		int size;
		int offset;

		SLayoutEntry(EShaderDataType _type, std::string _name);
	};

	class CInputLayout
	{
	public:
		virtual ~CInputLayout() = default;

		virtual void Bind() = 0;

		inline int GetStride() const { return m_stride; }

	protected:
		CInputLayout(std::initializer_list<SLayoutEntry> const& layout);

	protected:
		std::vector<SLayoutEntry> m_entries;
		int m_stride;
	};

}
