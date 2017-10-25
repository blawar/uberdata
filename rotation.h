#ifndef ROTATION_H
#define ROTATION_H

#define RotationT        uint16
#define RotationTDbl     uint32
#define RotationMax     0xFFFF
#define DEGREES(d) (RotationMax / (CYCLE_DEGREES) * d)
#define CYCLE_DEGREES	720
//#define TODEGREES(r) (r.degrees())

#include "duration.h"

class Rotation
{
public:
	Rotation()
	{
		value() = 0;
		cycle() = 1;
	}

	Rotation(const RotationT v)
	{
		value() = v;
		cycle() = 1;
	}

	operator RotationT() const
	{
		return value();
	}

	Rotation& operator=(const RotationT right)
	{
		if(right < value())
		{
			cycle()++;
		}
		value() = right;
		return *this;
	}

	Rotation& operator+=(const Rotation& right)
	{
		if(RotationMax - right.value() < value())
		{
			cycle()++;
		}

		cycle() += right.cycle() - 1;
	
		value() += right.value();
		return *this;
	}

        Rotation& operator+=(const RotationT right)
        {
                if(RotationMax - right < value())
                {
                        cycle()++;
                }

                value() += right;
                return *this;
        }

        Rotation& operator-=(const Rotation right)
        {
                if(right.value() > value())
                {
                        cycle()--;
                }

		cycle() -= right.cycle() - 1;

                value() -= right.value();
                return *this;
        }

        Rotation& operator-=(const RotationT right)
        {
                if(right > value())
                {
                        cycle()--;
                }

                value() -= right;
                return *this;
        }

	bool operator>=(const Rotation right) const
	{
		if(cycle() >= right.cycle() && value() >= right.value()) 
		{
			return true;
		}
		return false;
	}

	uint16 degrees() const
	{
		return value() * CYCLE_DEGREES / RotationMax;
	}

	Rotation& fuzzySet(RotationT right) // this is really a signed operation, thus it is only accurate if moving within half the cycle
	{
		if(right == value())
		{
			// nothing to do
			return *this;
		}

		RotationT delta = right - value();
		RotationT delta2 = delta & 0x7FFF;//(RotationMax / 2); // strip out MSB
		if(delta == delta2)
		{
			*this += delta;
		}
		else
		{
			*this -= ~delta;
		}
		//value() = right;
		return *this;
	}


private:
	Member(RotationT, cycle);
	Member(RotationT, value);
};

#endif

