#ifndef NUMBER_H
#define NUMBER_H
#include "pods.h"

template<class T, typename DERIVED=DEFAULT_NUMBER>
class Number
{
public:
        Number()
        {
		value() = 0;
        }

        Number(const T val)
        {
                value() = val;
        }

/*
        template<class CT>
        Number(const CT val)
        {
                value() = (T)val;
        }
*/

        T&
        operator=(const T right)
        {
                value() = right;
                return *this;
        }

	operator DERIVED() const
	{
		return DERIVED(*this);
	}

	operator T() const
	{
		return value();
	}

        bool operator<(const T right) const
        {
                return value() < (T)right;
        }

        bool operator==(const T right) const
        {
                return value() == (T)right;
        }

        bool operator==(const DERIVED right) const
        {
                return value() == (T)right;
        }

        T operator-(const T right) const
        {
                T left = *this;
                left -= right;
                return left;
        }

        DERIVED operator-(const DERIVED right) const
        {
                T left = *this;
                left -= right;
                return left;
        }

        T operator-=(const T right)
        {
                value() -= right.value();
        }

        T operator+(const T right) const
        {
                T left = *this;
                left -= right;
                return left;
        }

        T operator+=(const T right)
        {
                value() += right.value();
        }

	bool operator!() const
	{
		return !value();
	}

        bool operator>(const T right) const
        {
                return value() > right.value();
        }

private:
	Member(T, value);
};

#endif
