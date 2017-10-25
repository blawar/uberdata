#include "engine.h"
#include "cylinder.h"

Cylinder::Cylinder() : injWatchdog(this), ignWatchdog(this)
{
	//watchdog.callback.callback() = &Cylinder::close;
	//watchdog.call();
	engine() = 0;
	spark() = DEGREES((360+10));
	fuel() = 5000;
	sparkAdvance() = DEGREES(10);
	dwell() = 1000;
}

uint16 Cylinder::degrees()
{
	return engine()->position().position().degrees();
}

Rotation Cylinder::calculateInjClose()
{
	Rotation p = engine()->camshafts().intake().close();
	p += tdc();
	return p;
}

Rotation Cylinder::calculateIgnClose()
{
	Rotation p = spark();
	p += tdc();
	p -= sparkAdvance();
	return p;
}

void Cylinder::init()
{
	TimingEvents& events = engine()->position().events();
	TimingEvent& injEvent = events[id(INJ_OPEN)];

	injOpenCallback.object() = this;
	injOpenCallback.p1() = &injEvent;
	injEvent.set(Rotation(0), injOpenCallback);
	injWatchdog.start();


	TimingEvent& ignEvent = events[id(IGN_OPEN)];

        ignOpenCallback.object() = this;
        ignOpenCallback.p1() = &ignEvent;
        ignEvent.set(Rotation(0), ignOpenCallback);
        ignWatchdog.start();
}

void Cylinder::injClose()
{
	log << "injClose() @ " << degrees() << endl;
        TimingEvents& events = engine()->position().events();

        TimingEvent& injOpen  = events[id(INJ_OPEN)];
	injOpen.position().cycle()++;
	injOpen.position().fuzzySet(calculateInjClose().value());
        injOpen.position() -= engine()->position().ckp().calcRotation(fuel());
	injOpen.enable();
}

void Cylinder::injOpen(TimingEvent* event)
{
	injWatchdog.value() = fuel();
	log << "injOpen() @ " << degrees() << endl;
	//log << p.cycle() << "/" << event->position().cycle() << " " << timestamp() << "> inj " << number() << " open @ " << event->position().degrees() << endl;
}

void Cylinder::ignClose()
{
	log << "ignClose() @ " << degrees() << endl;
        TimingEvents& events = engine()->position().events();

        TimingEvent& ignOpen  = events[id(IGN_OPEN)];
	ignOpen.position().cycle()++;
        ignOpen.position().fuzzySet(calculateIgnClose().value());
        ignOpen.position() -= engine()->position().ckp().calcRotation(dwell());
	ignOpen.enable();
}

void Cylinder::ignOpen(TimingEvent* event)
{
	ignWatchdog.value() = dwell();
	log << "ignOpen() @ " << degrees() << endl;
	//log << p.cycle() << " " << timestamp() << "> ign " << number() << " open @ " << event->position().degrees() << endl;
}
