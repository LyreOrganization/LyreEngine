#pragma once

#include <d3d11.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <windows.h>
#include <atlbase.h>
#include <stdexcept>
#include <functional>
#include <memory>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <vector>
#include <string>
#include <array>
#include <map>
#include <unordered_map>
#include <queue>
#include <optional>
#include <algorithm>
#include <random>
#include <mutex>
#include <atomic>
#include <shared_mutex>
#include <chrono>
#include <time.h>

// Custom headers
#include "Utils.h"


#define ZeroStruct(structure)		(ZeroMemory(&structure, sizeof(structure)))
#define VecElementSize(vec)			(vec.size() > 0 ? sizeof(vec[0]) : 0)
#define VecBufferSize(vec)			(vec.size() > 0 ? vec.size()*sizeof(vec[0]) : 0)

#define WindowsLetterIdx(letter)	(0x41 + letter - 'A')
#define WindowsDigitIdx(digit)		(0x30 + digit - '0')

#define WND_POS_X -7
#define WND_POS_Y 0

#define WND_WIDTH   1280
#define WND_HEIGHT  700
