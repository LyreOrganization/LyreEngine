#pragma once

#include <functional>
#include "LyreObject.h"

namespace Lyre
{
	/**
	 * The base class for events.
	 * Usage:
	 *
	 *		class SomeEvent final : public Event
	 *		{
	 *		public:
	 *			EVENT(SomeEvent)
	 *			
	 *			... your event impl here ...
	 *		};
	 *		
	 *	or you can use these macros:
	 *		
	 *		EVENT_CLASS(SomeEvent)
	 *		... class defenition ...
	 *		}; / EVENT_CLASS_END()
	 */
	class Event : public Object
	{
	public:
		LYRE_OBJECT(Event, Object)
	};

	class EventListener
	{
	public:
		virtual bool OnEvent(Event const&) { return false; }
	};
}


#define EVENT(Class) LYRE_OBJECT(Class, Event)

#define EVENT_CLASS(Class) 		\
	class Class final : public Event	\
	{									\
	public:								\
		EVENT(Class)					\

#define EVENT_CLASS_END()	\
	};						\

#define EVENT_MAP(Class)														\
private:																		\
	using EventCallback = std::function<bool(Class* instance, Event const&)>;	\
	using EventMap = std::unordered_map<TypeID, EventCallback>;					\
																				\
	static EventMap const& InitEventMap();										\
																				\
public:																			\
	bool OnEvent(Event const& event) override									\
	{																			\
		static EventMap const& eventMap = InitEventMap();						\
																				\
		auto eventHandlerIt = eventMap.find(event.GetType());					\
		if (eventHandlerIt != eventMap.end())									\
		{																		\
			return eventHandlerIt->second(this, event);							\
		}																		\
																				\
		return false;															\
	}																			\

#define EVENT_MAP_BEGIN(Class)					\
Class::EventMap const& Class::InitEventMap()	\
	{											\
		using ThisClass = Class;				\
		static EventMap eventMap;				\

#define ADD_LISTENER(eventType, fn)																	\
		eventMap.emplace(eventType::ToTypeID(), [](ThisClass* instance, Event const& event) -> bool	\
		{																							\
			return instance->fn(static_cast<eventType const&>(event));								\
		});																							\

#define EVENT_MAP_END()		\
		return eventMap;	\
	}						\
