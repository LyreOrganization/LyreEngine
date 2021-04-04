#include "stdafx.h"

using namespace std;

namespace
{
	mutex g_stdTimeLock;
}

tm localtime_threadsafe(const time_t* _Time)
{
	lock_guard<mutex> guard(g_stdTimeLock);
	return *localtime(_Time);
}

const string CurrentDateTimeStr()
{
	time_t     now = time(NULL);
	struct tm  tmLocal;
	char       buf[80];
	tmLocal = localtime_threadsafe(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tmLocal);
	return buf;
}

