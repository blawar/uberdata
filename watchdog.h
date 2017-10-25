#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "pods.h"
#include "functor.h"

template<class C, void (C::*F)()>
class Watchdog : public Functor0<C,F>
{
public:
	Watchdog()
	{
		active() = true;
		value() = 0;
	}

	Watchdog(C* obj)
	{
		active() = true;
		value() = 0;
		this->object() = obj;
	}

	/*Watchdog(C* obj, void (C::*funcPtr)()) //: callback(C, funcPtr)
	{
		value() = 0;
	}*/

	void tick()
	{
		if(active() && value())
		{
			if(!--value())
			{
				this->call();
			}
		}
	}

	void stop()
	{
		active() = false;
	}

	void start()
	{
		active() = true;
	}

private:
	Member(bool, active);
	Member(int16, value);
};

#endif
