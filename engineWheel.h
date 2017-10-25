#ifndef ENGINEWHEEL_H
#define ENGINEWHEEL_H

#include <limits.h>
#include "pods.h"
#include "rotation.h"

class EnginePosition;

typedef void (*WheelEventCallback)(EnginePosition*,Rotation,bool);

struct ToothMetric
{
	Time time;
	Time duration;
};

class Wheel
{
};

template<uint16 TEETH, uint16 MISSING_TEETH=0>
class EngineWheel : public Wheel
{
public:
	EngineWheel()
	{
		TOOTH_ROTATION = RotationMax / TEETH;
		id() = 0;
		synced() = false;
		cycleEndTime() = 0;
	}

	void heartbeat()
	{
                if(callback())
                {
                        callback()(parent(), estimate(), true);
                }
	}

	void tick();
	void tock();
	void setPosition(Rotation r);
	void reset();
	void update()
	{
		if(callback())
		{
			callback()(parent(), currentPosition(), false);
		}
		//parent()->set(*this, currentPosition());
	}

	RotationT calcRotation(const Duration d) const
	{
		return 0x1000;
		if(d > cycleDuration())
		{
			//printf("max duration\n");
			return RotationMax;
		}
		//printf("Rotations: %d, Cycle Dureation: %d, dureation: %d\n", d * RotationMax / cycleDuration().value(), cycleDuration().value(), d.value());
		return d * RotationMax / cycleDuration().value();
	}

	Rotation estimate()
	{
		Index previousTooth = tooth();
		if(previousTooth == 0)
		{
			previousTooth = TEETH - 1;
		}
		else
		{
			--previousTooth;
		}

		if(!toothMetrics()[previousTooth].duration)
		{
			return currentPosition();
		}

		Rotation e = toothPosition();
		e += (timestamp() - toothMetrics()[previousTooth].time) * TOOTH_ROTATION / toothMetrics()[previousTooth].duration;
		return e;
	}

	Rotation TOOTH_ROTATION;
private:
	Member(uint16, tooth);
	Member(uint16, RPM);
	Member(uint8, id);
	Member(bool, synced);
	Member(Time, cycleEndTime);
	Member(Duration, cycleDuration);
	Member(Rotation, currentPosition);
	Member(Rotation, toothPosition);
	Member(Rotation, tockOffset);
	MemberPtr(EnginePosition*, parent);
	Member(WheelEventCallback, callback);
	MemberArr(ToothMetric, toothMetrics, TEETH);
};

#include "engineWheel.cpp"

#endif
