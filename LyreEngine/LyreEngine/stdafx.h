#pragma once

#include <D3DX11.h>
#include <D3D11.h>
#include <DirectXMath.h>
#include <windows.h>
#include <atlbase.h>
#include <memory>
#include <vector>
#include <fstream>

#include "resource.h"


#define ZeroStruct(structure) (ZeroMemory(&structure, sizeof(structure)))
#define VecElementSize(vec)			(vec.size() > 0 ? sizeof(vec[0]) : 0)
#define VecBufferSize(vec)			(vec.size() > 0 ? vec.size()*sizeof(vec[0]) : 0)

#define WND_WIDTH   1900
#define WND_HEIGHT  1000
