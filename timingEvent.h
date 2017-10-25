#ifndef TIMING_EVENT_H
#define TIMING_EVENT_H

#include "timingEvents.h"
#include "functor.h"

class TimingEvent
{
public:
	TimingEvent()
	{
		status() = 0;
		lastCycle() = 0;
	}

	TimingEvent(uint16 val)
	{
		status() = 0;
		lastCycle() = 0;
	}

	bool active() const
	{
		return !(status() & BIT1);
	}

	bool active(Rotation& p)
	{
		return lastCycle() < p.cycle();
	}

	void enable()
	{
		status() &= ~BIT1;
	}

	void disable()
	{
		status() |= BIT1;
	}

	void trigger(Rotation p)
	{
		lastCycle() = p.cycle();
		if(callback())
		{
			callback()->call();
		}
	}

	template<class F>
	void set(const Rotation position, F& callback)
	{
		this->position() = position;
                this->callback() = &callback;
		enable();
	}

private:
	Member(byte, status);
	Member(Rotation, position);
	MemberPtr(Functor*, callback);
	MemberPtr(void*, param1);
	MemberPtr(void*, param2);
	Member(RotationT, lastCycle);
};

#endif

