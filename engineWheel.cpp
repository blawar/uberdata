#include "engineWheel.h"

template <uint16 TEETH, uint16 MISSING_TEETH>
void EngineWheel<TEETH, MISSING_TEETH>::reset()
{
	Time now = timestamp();
	tooth() = 0;

	cycleDuration() = now - cycleEndTime();
	if(cycleDuration())
	{
		RPM() = TIMER_CLOCK_RATE * 60 / cycleDuration(); //1000000 / cycleDuration() * 60;
	}
	else
	{
		RPM() = 0;
	}

	if(!synced() && cycleEndTime()) {
		Time totalTeethDuration = 0;
		for(uint16 i=0; i < TEETH; i++)
		{
			totalTeethDuration += toothMetrics()[i].duration;
		}

		if(cycleDuration())
		{
			tockOffset() = RotationMax / cycleDuration() * totalTeethDuration / TEETH;
		}
	}
	cycleEndTime() = now;
}

