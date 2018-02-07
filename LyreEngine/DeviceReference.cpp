#include "stdafx.h"
#include "DeviceReference.h"
#include "LyreEngine.h"

DeviceReference::DeviceReference():
	m_pDevice(LyreEngine::getDevice()),
	m_pContext(LyreEngine::getContext())
{}
