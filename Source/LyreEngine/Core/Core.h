#pragma once

namespace Lyre
{

	// This alias is a temporary decision, it's not a replacement for std::string
	// In future, CStringID class supposed to be an id to unique strings hash table
	using CStringId = std::string;

#ifdef LYRE_DEBUG
	#define LYRE_ASSERT(condition, ...) if (!(condition)) __debugbreak()
#else
	#define LYRE_ASSERT(condition, ...)
#endif

}
