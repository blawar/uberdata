#ifndef VE_H
#define VE_H

#include "percent.h"
#include "pods.h"
#include "nullable.h"

#define MAX_VE	0xFFFF
#define VE_MULT 100

#define VePercent	Percent<uint16, 0, MAX_VE, uint16, VE_MULT, Ve>

class Ve : public VePercent
{
public:
	Ve() : VePercent()
	{
	}

	Ve(const uint16 val) : VePercent(val)
	{
	}

	Ve(const uint16 dividend, const uint16 divisor) : VePercent(dividend, divisor)
        {
        }

        using VePercent::set;
        using VePercent::get;
        using VePercent::value;
};

#endif

