#pragma once

#include <D3DX11.h>
#include <D3D11.h>
#include <DirectXMath.h>
#include <windows.h>
#include <atlbase.h>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <math.h>
#include <vector>
#include <array>
#include <forward_list>
#include <optional>


#define ZeroStruct(structure)		(ZeroMemory(&structure, sizeof(structure)))
#define VecElementSize(vec)			(vec.size() > 0 ? sizeof(vec[0]) : 0)
#define VecBufferSize(vec)			(vec.size() > 0 ? vec.size()*sizeof(vec[0]) : 0)

#define WindowsLetterIdx(letter)	(0x41 + letter - 'A')

#define WND_POS_X -7
#define WND_POS_Y 0

#define WND_WIDTH   1280
#define WND_HEIGHT  700
