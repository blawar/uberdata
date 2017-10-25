#ifndef ALPHA_H
#define ALPHA_H

#include "log.h"
#include "nullable.h"

template<class T, class A = byte, class B = byte>
class Alpha : public Nullable<T, B>
{
public:
	Alpha() :  Nullable<T,B>()
	{
	}

	Alpha(const T& val)
	{
		this->operator=(val);
	}

	operator T() const
	{
		if(this->isNull()) return 0;
		return this->value();
	}

	Alpha& operator=(const T val)
	{
		this->value() = val;
		this->setNull(false);
		return *this;
	}

private:
	Member(A, alpha);
};

#endif
