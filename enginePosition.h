#ifndef ENGINEPOSITION_H
#define ENGINEPOSITION_H

#include "pods.h"

#include "engineWheel.h"
#include "timingEvents.h"
#include "engineWheel.h"
#include "missingToothWheel.h"

#define CKP_TYPE	MissingToothWheel <60,2>
#define CMP_TYPE	MissingToothWheel <2,1>

class EnginePosition
{
public:
	EnginePosition()
	{
		ckp().parent() = this;
		ckp().callback() = (WheelEventCallback)&EnginePosition::set;
		cmp().parent() = this;
	}

	static void set(EnginePosition* engine, const Rotation position, const bool isEstimate)
	{
	        engine->get() = position;
		engine->events().process(position);
	}

	const Rotation& get() const { return position(); }
        Rotation& get() { return position(); }

	void heartbeat()
	{
        	ckp().heartbeat();
        	cmp().heartbeat();
	}
private:
	Member(CKP_TYPE, ckp);
	Member(CMP_TYPE, cmp);
	Member(Rotation, position);
	Member(TimingEvents, events);
};

#endif
