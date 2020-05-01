#pragma once

namespace Lyre
{

	template<class T>
	using NotOwn = T*;

	template<class T>
	using Scope = std::unique_ptr<T>;

	template<class T>
	using Ref = std::shared_ptr<T>;


	template<class T, class... Args>
	inline Scope<T> MakeScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<class T, class... Args>
	inline Ref<T> MakeRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

#ifdef LYRE_DEBUG
	#define LYRE_ASSERT(condition) if (!condition) __debugbreak()
#else
	#define LYRE_ASSERT(condition)
#endif

}
