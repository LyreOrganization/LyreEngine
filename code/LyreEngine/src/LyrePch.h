#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

#ifdef LYRE_PLATFORM_WINDOWS
#define GLM_FORCE_RIGHT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <Core/Core.h>
#include <Core/LyreObject.h>
#include <Core/Event.h>

#ifdef LYRE_PLATFORM_WINDOWS
#include <Windows.h>
#include <d3d11.h>
#include <atlbase.h>
#endif
