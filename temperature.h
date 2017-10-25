#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "pods.h"
#include "fixed.h"

#define temperatureType	uint16
#define KELVIN(t) (t+273.15)

#define TemperatureFixed Fixed<temperatureType, 0, 500, uint16, Temperature>

class Temperature : public TemperatureFixed
{
public:
	Temperature() : TemperatureFixed()
	{
	}

	Temperature(const temperatureType v) : TemperatureFixed(v)
	{
	}
};

#endif
