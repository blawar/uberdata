#include "log.h"
#include <stdlib.h> 
#include <sys/time.h>
#include <unistd.h>
#include "pods.h"
#include <stdint.h>

//600khz clock, 16bit timer will overflow every ~109 ms

Time CONVERT_CLOCK(const Time val, const uint32 oldClock, const uint32 newClock)
{
	if((val*newClock) < val) // overflowed
	{
		return val / oldClock * newClock;
	}
	return val * newClock / oldClock;
}

Time counter=0, lastTs=0;;

Time timestamp()
{
	/*timeval  tv;
	gettimeofday(&tv, NULL);
	Time ts = CONVERT_CLOCK(tv.tv_usec, 10000, TIMER_CLOCK_RATE);
	if(ts != lastTs)
	{
		counter++;
		lastTs = ts;
	}*/
	return counter;
	
}

#include "engine.h"

Engine engine;

int main()
{
	#include "vetable.h"

	/*printf("TEID(TE_CYL,0) = %x\n", TEID(TE_CYL,0));
	printf("TEID(TE_CYL,1) = %x\n", TEID(TE_CYL,1));
	printf("TEID(TE_CYL,2) = %x\n", TEID(TE_CYL,2));

        printf("TEID_SUB(TE_CYL,0,0) = %x\n", TEID_SUB(TE_CYL,0,0,8));
        printf("TEID_SUB(TE_CYL,0,1) = %x\n", TEID_SUB(TE_CYL,0,1,8));
        printf("TEID_SUB(TE_CYL,0,2) = %x\n", TEID_SUB(TE_CYL,0,2,8));

        printf("TEID_SUB(TE_CYL,1,0) = %x\n", TEID_SUB(TE_CYL,1,0,8));
        printf("TEID_SUB(TE_CYL,1,1) = %x\n", TEID_SUB(TE_CYL,1,1,8));
        printf("TEID_SUB(TE_CYL,1,2) = %x\n", TEID_SUB(TE_CYL,1,2,8));
	return 0;*/
	/*Map<byte,10,5,byte,byte> m;
	for(byte y=0; y < 5; y++)
	{
		m.y().get(y) = y * 0x10;

		byte c = y * 0x20;
		for(byte x=0; x < 10; x++)
		{
			if(y==0)m.x().get(x) = c;
			m.set(x,y, c);
			c += 0x10;
		}
	}*/

	log << "sizeof(Engine) = " << sizeof(Engine) << endl;

/*
	printf("sizeof(Engine) = %d\n", sizeof(Engine));
	printf("sizeof(Cylinder) = %d\n", sizeof(Cylinder));
	printf("sizeof(Camshaft) = %d\n", sizeof(Camshaft));
	printf("sizeof(VEMAP) = %d\n", sizeof(VEMAP));
	printf("sizeof(EnginePosition) = %d\n", sizeof(EnginePosition));
	printf("sizeof(TimingEvents) = %d\n", sizeof(TimingEvents));
	printf("sizeof(CKP) = %d\n", sizeof(CKP_TYPE));
	printf("sizeof(VETRIM) = %d\n", sizeof(VETRIM));
*/

        engine.camshafts().intake().open(DEGREES(-10)).close(DEGREES(222));
        engine.camshafts().exhaust().open(DEGREES(492)).close(DEGREES(4));

        engine.camshafts().intake().offset(DEGREES(2));
        engine.camshafts().exhaust().offset(DEGREES(-8));

        engine.cylinders(1).tdc(DEGREES(0)).init();

        /*engine.cylinders(2).tdc(DEGREES(540)).init();
        engine.cylinders(3).tdc(DEGREES(180)).init();
        engine.cylinders(4).tdc(DEGREES(360)).init();*/

	Time now, last=timestamp();
	Rotation lastRotation;
	unsigned long lastMs = 0;
	for(;;)
	{
		counter++;
		now = timestamp();
		unsigned long ms = CONVERT_CLOCK(now, TIMER_CLOCK_RATE, 1000);
		if(ms != lastMs)
		{
			lastMs = ms;
			printf("timestamp: %u ms, RPM = %d\n", ms, engine.position().ckp().RPM());
		}
		//printf("tick: %u\n", now);
		if(now % 100 == 0)
		{
			//printf("tock!\n");
			engine.position().ckp().tick();
		}
		engine.position().heartbeat();
		Rotation est = engine.position().ckp().estimate();
		if(lastRotation.degrees() != est.degrees())
		{
			lastRotation = est;
			//printf("\tangle: %d\n", est.degrees());
		}
		engine.loop();
	}

	/*m.dump();
	for(uint16 x=0x0; x <= 0x40; x+= 8)
	for(uint16 y=0x0; y <= 0x40; y+= 8)
	{
		printf("x[0x%2.2x][0x%2.2x] = 0x%2.2x\n", x, y, m.get(x, y));
	}
*/
	return 0;
}
