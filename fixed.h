#ifndef FIXED_H
#define FIXED_H

#include "number.h"

#define FixedNumber	Number<STORAGE_T, DERIVED>

template<class T, T MIN=0, T MAX=MAXVALUE(T), class STORAGE_T=uint16, typename DERIVED=DEFAULT_NUMBER >
class Fixed : public FixedNumber
{
public:
	Fixed() : FixedNumber()
	{
	}

	Fixed(const STORAGE_T val) : FixedNumber(val)
	{
		value() = val;
	}

	template<class CT>
	Fixed(const CT val)
        {
                value() = (STORAGE_T)val;
        }

	template<class CT>
	void set(const CT val)
	{
        	value() = (uint32)(val + min()) * containerMax() / range();
	}

	T get() const
	{
		return (uint32)value() * range() / containerMax() - min();
	}

	STORAGE_T containerMax() const
	{
		return MAXVALUE(STORAGE_T);
	}

	T max() const
	{
		return MAX;
	}

	T min() const
	{
		return MIN;
	}

	T range() const
	{
		return max() - min();
	}
/*
        Fixed<T,MIN,MAX,STORAGE_T>&
        operator=(const T right)
        {
                value() = right;
                return *this;
        }

        operator STORAGE_T() const
        {
                return value();
        }

        operator STORAGE_T()
        {
                return value();
        }

        bool operator<(const Fixed right) const
        {
                return value() < right.value();
        }

        bool operator==(const Fixed right) const
        {
                return value() == right.value();
        }

        Fixed operator-(const Fixed<T,MIN,MAX,STORAGE_T> right) const
        {
                Fixed left = *this;
                left -= right;
                return left;
        }

        Fixed operator-=(const Fixed<T,MIN,MAX,STORAGE_T> right)
        {
                value() -= right.value();
        }

        Fixed operator+(const Fixed<T,MIN,MAX,STORAGE_T> right) const
        {
                Fixed left = *this;
                left -= right;
                return left;
        }

        Fixed operator+=(const Fixed<T,MIN,MAX,STORAGE_T> right)
        {
                value() += right.value();
        }

        bool operator>(const Fixed<T,MIN,MAX,STORAGE_T> right) const
        {
                return value() > right.value();
        }
*/
	using FixedNumber::value;
};

#endif
