#ifndef PRESSURE_H
#define PRESSURE_H

#include "pods.h"
#include "percent.h"

#define BAR(t) (t*1000)
#define pressureType	uint16
#define PressurePercent	Percent<uint16, 0, 3000, uint16, 100, Pressure>

class Pressure : public PressurePercent
{
public:
	Pressure() : PressurePercent()
	{
	}

	Pressure(const pressureType v) : PressurePercent(v)
	{
	}
	
	Pressure(const PressurePercent v) : PressurePercent(v)
	{
	}

};

#endif
