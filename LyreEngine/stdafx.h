#pragma once

#include <D3DX11.h>
#include <D3D11.h>
#include <DirectXMath.h>
#include <windows.h>
#include <atlbase.h>
#include <stdexcept>
#include <functional>
#include <memory>
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <random>


#define ZeroStruct(structure)		(ZeroMemory(&structure, sizeof(structure)))
#define VecElementSize(vec)			(vec.size() > 0 ? sizeof(vec[0]) : 0)
#define VecBufferSize(vec)			(vec.size() > 0 ? vec.size()*sizeof(vec[0]) : 0)

#define WindowsLetterIdx(letter)	(0x41 + letter - 'A')
#define WindowsDigitIdx(digit)		(0x30 + digit - '0')

#define WND_POS_X -7
#define WND_POS_Y 0

#define WND_WIDTH   1280
#define WND_HEIGHT  700
