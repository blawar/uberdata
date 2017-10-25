#ifndef SENSOR_H
#define SENSOR_H

#include "port.h"

template<class T=uint16>
class Sensor
{
public:
	Sensor()
	{
	}

	operator T() const
	{
		return value();
	}

private:
	Member(T, value);
	Member(Port, port);
};

#endif

