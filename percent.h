#ifndef PERCENT_H
#define PERCENT_H

#include "fixed.h"

#define PercentFixed Fixed<T, MIN, MAX, STORAGE_T, DERIVED>

template<class T, T MIN, T MAX, class STORAGE_T = byte, uint32 MULTIPLIER = 100, typename DERIVED = DEFAULT_NUMBER>
class Percent : public PercentFixed
{
public:
	Percent() : PercentFixed()
	{
	}

	Percent(const T val) : PercentFixed(val)
	{
	}

	Percent(const uint16 dividend, const uint16 divisor)
	{
		set(dividend, divisor);
	}

	void set(const uint16 dividend, const uint16 divisor)
	{
                if(!divisor)
                {
                        this->value() = 0;
                }
                else
                {
                        this->value() = (uint32)dividend * MAXVALUE(STORAGE_T) / (uint32)divisor / MAX;
                }
	}

	template<class NT>
	NT scale(const NT val) const
	{
		uint32 tmp = val;
		tmp *= this->value();
		if(val >= MAXVALUE(T))
		{
			return tmp / this->containerMax() * this->range() / MULTIPLIER;
		}
		else
		{
			return tmp * this->range() / this->containerMax() /  MULTIPLIER;
		}
	}

	using PercentFixed::set;
	using PercentFixed::get;
	using PercentFixed::value;
private:
};

#endif
