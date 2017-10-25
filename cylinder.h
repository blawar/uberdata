#ifndef CYLINDER_H
#define CYLINDER_H
#include "map.h"
#include "rotation.h"
#include "engine.h"
#include "enginePosition.h"
#include "camshafts.h"
#include "watchdog.h"

enum
{
	INJ_CLOSE = 0,
	INJ_OPEN = 1,
	IGN_CLOSE = 2,
	IGN_OPEN = 3
};

class Engine;
class Cylinder;

#define MAX_FUEL 0xFFFF

#define VETRIM	Map<int8,uint16,8,uint16,8>

class Cylinder
{
public:
	Cylinder();

	Cylinder& spark(Rotation r)
	{
	        spark() = r;
	        return *this;
	}

	Cylinder& tdc(Rotation r)
	{
        	tdc() = r;
        	return *this;
	}

	Rotation calculateInjClose();
	Rotation calculateIgnClose();

	void init();
	void injOpen(TimingEvent* event);
	void injClose();

        void ignOpen(TimingEvent* event);
        void ignClose();

	void close()
	{
		log << "closing!!!!!" << endl;
	}

	PURE TimingKey id(unsigned int cat)
	{
		return TEID_SUB(TE_CYL,number(),cat,8);
	}

	void tick()
	{
		injWatchdog.tick();
		ignWatchdog.tick();
	}

	uint16 degrees();

	Watchdog<Cylinder, &Cylinder::injClose> injWatchdog;
	Watchdog<Cylinder, &Cylinder::ignClose> ignWatchdog;
	Functor1<Cylinder, TimingEvent*, &Cylinder::injOpen> injOpenCallback;
	Functor1<Cylinder, TimingEvent*, &Cylinder::ignOpen> ignOpenCallback;
private:
	Member(uint16, number);
	MemberPtr(Engine*, engine);
	Member(VETRIM, vetrim);
	Member(Rotation, spark);
	Member(Rotation, sparkAdvance);
	Member(Rotation, tdc);
	Member(Rotation, ign);
	Member(Duration, fuel);
	Member(Duration, dwell);

};



#endif
