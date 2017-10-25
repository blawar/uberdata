#ifndef DURATION_H
#define DURATION_H

#define DurationT        uint16
#define DurationTDbl     uint32
#define DurationMax     0xFFFF

class Duration
{
public:
	Duration()
	{
		value() = 0;
	}

	Duration(DurationT v)
	{
		value() = v;
	}

	operator DurationT() const
	{
		return value();
	}

	Duration& operator=(const DurationT right)
	{
		value() = right;
		return *this;
	}

	/*Duration& operator=(const Duration right)
	{
		return operator=(right.value());
	}*/

	Duration& operator+=(const Duration right)
	{
		/*if(DurationMax - right.value() < value())
		{
			// overflow
		}*/
		value() += right.value();
		return *this;
	}

        Duration& operator-=(const Duration right)
        {
                value() -= right.value();
                return *this;
        }

private:
	Member(DurationT, value);
};

#endif

