#include "stdafx.h"

#include "helper.h"

#define DEFAULT_LOG_FILE "/etc/raspberrycamera/chs/CHS.log"

#ifdef ERROR
	#undef ERROR
#endif

using namespace std;

namespace
{
	mutex g_logLock;
	ofstream g_log;
	bool initialized = false;
	
	typedef unsigned ThreadName;
	ThreadName threadUniqueName = 0;
	map<thread::id, ThreadName> threadDictionary;

	ThreadName get_thread_cached_name(const thread::id& threadId)
	{
		auto it = threadDictionary.find(threadId);
		if (it == threadDictionary.end())
		{
			g_log << "Thread " << threadId << " named " << threadUniqueName << ".\n";
			threadDictionary[threadId] = threadUniqueName;
			return threadUniqueName++;
		}
		else
			return it->second;
	}
}

void start_logging()
{
	unique_lock<mutex> locker(g_logLock);
	if (!initialized){
		g_log.open(DEFAULT_LOG_FILE, ios::out | ios::app);
		initialized = true;
		g_log << "......................CHS......................\n" 
			<< CurrentDateTimeStr() << ":\t" << "CHS logging started.\n";
	}
}

void stop_logging()
{
	unique_lock<mutex> locker(g_logLock);
	if (initialized){
		g_log << CurrentDateTimeStr() << ":\t" << "CHS logging stopped.\n\n\n";
		initialized = false;
		g_log.close();
	}
}

void log_msg(LogLevel level, const std::string& msg)
{
	unique_lock<mutex> locker(g_logLock);
	if (initialized)
	{
		ThreadName threadName = get_thread_cached_name(this_thread::get_id());
		g_log << setw(4) << threadName << ")";
		switch (level)
		{
		case LogLevel::FATAL:
			g_log << " FATAL "; break;
		case LogLevel::ERROR:
			g_log << " ERROR "; break;
		case LogLevel::WARNING:
			g_log << " WARN  "; break;
		default:
		case LogLevel::INFO:
			g_log << " INFO  "; break;
		}
		g_log << setw(4) << CurrentDateTimeStr() << ": " << msg << "\n";
		g_log.flush();
	}
}

void log_msg(const std::string& msg)
{
	log_msg(LogLevel::INFO, msg);
}