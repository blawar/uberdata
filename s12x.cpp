#include "engine.h"

Engine engine TEXT3_ATTR;

uint16 timestamp()
{
	return 0;
}

int main()
{
	engine.camshafts().intake().open(DEGREES(-10)).close(DEGREES(222));
	engine.camshafts().exhaust().open(DEGREES(492)).close(DEGREES(4));

	engine.camshafts().intake().offset(DEGREES(2));
	engine.camshafts().exhaust().offset(DEGREES(-8));

	engine.cylinders(DEFAULT).spark(DEGREES(360)).init();

	engine.cylinders(1).tdc(DEGREES(0)).init();
	engine.cylinders(2).tdc(DEGREES(270)).init();
	engine.cylinders(3).tdc(DEGREES(90)).init();
	engine.cylinders(4).tdc(DEGREES(180)).init();

	for (;;)
	{
		engine.loop();
	}
}

