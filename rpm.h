#ifndef RPM_H
#define RPM_H

#include "number.h"

#define RpmNumber Number<uint16, Rpm>

class Rpm : public RpmNumber
{
public:
	Rpm() : RpmNumber()
	{
	}

	Rpm(uint16 val) : RpmNumber(val)
	{
	}

	void set(uint16 val)
	{
		this->value() = val;
	}

	uint16 get() const
	{
		return this->value();
	}

	operator uint16() const
	{
		return this->value();
	}

	Rpm operator++(int)
	{
		return Rpm(value()++);
	}

	//using Number::value;
};

#endif

