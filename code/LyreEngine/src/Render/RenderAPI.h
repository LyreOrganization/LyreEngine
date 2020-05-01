#pragma once

namespace Lyre
{
	class CApplication;

	enum class ERenderAPIType
	{
		None = 0,
		DirectX_11
	};

	class CRenderAPI
	{
		friend class CRenderer;
	public:
		virtual ~CRenderAPI() = default;

		virtual bool Init(CApplication const& app) = 0;
		virtual void Present() = 0;

		ERenderAPIType GetApiType() const { return m_apiType; }

	protected:
		CRenderAPI(ERenderAPIType apiType = ERenderAPIType::None);

	private:
		static Scope<CRenderAPI> Create(ERenderAPIType apiType);

	private:
		ERenderAPIType m_apiType;
	};

}
