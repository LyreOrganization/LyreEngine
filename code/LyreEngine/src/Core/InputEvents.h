#pragma once

#include "Event.h"

namespace Lyre
{

	class CMouseMoveEvent : public Event
	{
	public:
		EVENT(CMouseMoveEvent);

		float dx;
		float dy;
	};

	class CMovementEvent : public Event
	{
	public:
		EVENT(CMovementEvent);

		float ahead;
		float aside;
	};

}
