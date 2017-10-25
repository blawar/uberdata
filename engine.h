#ifndef ENGINE_H
#define ENGINE_H

#include "pods.h"
Time timestamp();

#include "alpha.h"
#include "sortedList.h"
#include "enginePosition.h"
#include "camshafts.h"
#include "cylinder.h"
#include "map.h"
#include "pressure.h"
#include "temperature.h"
#include "fixed.h"
#include "sensors.h"
#include "ve.h"
#include "rpm.h"

#define MAX_CYLINDERS	4
#define DEFAULT 0

#define ANMAP	Map<Percent<byte,0,100>,Rpm,32,Tps,32>
#define VEMAP	Map<Ve,Rpm,32,Pressure,32>
#define AFRMAP	Map<uint8,Rpm,8,Ve,8>
#define PPPMAP	Map<uint16,Rpm,8,Ve,8>

class Engine
{
public:
	Engine()
	{
		init();
	}

	void init()
	{
		sensors().map().value().value() = (85);
		sensors().rpm().value().set(3900);
		sensors().iat().value().set(353);
		displacement() = 2000;
                for(uint8 i=0; i < MAX_CYLINDERS; i++)
                {
                        Cylinder& cyl = cylinders(i);
                        cyl.engine() = this;
			cyl.number() = i;
                }
	}

	void loop()
	{
		Airflow sd = sdAirflow();
		Airflow maf = mafAirflow();
		Airflow tps = tpsAirflow();

		AirflowType totalAlpha = sd.alpha() + maf.alpha() + tps.alpha();
		AirflowType airflow;

		if(totalAlpha)
		{
			airflow  = sd.value();//  * sd.alpha()  / totalAlpha;
			/*airflow += maf.value() * maf.alpha() / totalAlpha;
			airflow += tps.value() * tps.alpha() / totalAlpha;
			airflow /= 3;*/
			//printf("airflow: %d\n", airflow);
		}
		else
		{
			// we have a catostrophic failure, try to limp.
		}

		for(Index i=0; i < MAX_CYLINDERS; i++)
		{
			cylinders()[i].tick();
		}
	}

	void calculateFuelIgn()
	{
		//uint16 airMass;
		//airMass = m_ve1.get(rpm(), map()) * DISPLACEMENT * 0.21 * iat();

	}

	Ve airTempCorrection(const Ve ve)
	{
		if(!sensors().iat().value().value())
		{
			return 0;
		}
		//printf("ve.scale(%d / %d) = %d, iat: %d\n", uint16(1.2929 * KELVIN(0)), sensors().iat().value().get(), ve.get(), sensors().iat().value().get());
		return uint16(1.2929 * KELVIN(0)) * ve.value() / sensors().iat().value().get();
	}

	AirflowType sdAirflow()
	{
		Ve ve = ve1().lookup(sensors().rpm().value(), sensors().map().value());
		//printf("RPM: %d, MAP: %d\n", sensors().rpm().value().get(), sensors().map().value().get());
		ve = airTempCorrection(ve);
		//printf("VE TEMP: %d\n", ve.get());
		//return displacement() / MAX_CYLINDERS;
		return ve.scale<AirflowType>(displacement() / MAX_CYLINDERS);
	}

	AirflowType mafAirflow()
	{
		return 0;
	}

	AirflowType tpsAirflow()
	{
		return 0;
		//const Ve& map = ve1().lookup(sensors().rpm().value(), sensors().tps().value().value());
                //return airDensity(Ve(map.value().value(), sensors().baro().value().value())) / MAX_CYLINDERS * displacement();
	}

private:
	Member(uint16, displacement);
	Member(Sensors, sensors);
	Member(VEMAP, ve1);
	Member(ANMAP, an1);
	Member(AFRMAP, afr1);
	Member(PPPMAP, ppp1);
	MemberArr(Cylinder, cylinders,MAX_CYLINDERS);
	Member(Camshafts, camshafts);
	Member(EnginePosition, position);
};

#endif
