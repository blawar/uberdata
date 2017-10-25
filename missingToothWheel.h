#ifndef MISSINGTOOTHWHEEL_H
#define MISSINGTOOTHWHEEL_H

#include "engineWheel.h"

template<uint16 TEETH, uint16 MISSING_TEETH=0>
class MissingToothWheel : public EngineWheel<TEETH, MISSING_TEETH>
{
public:
	MissingToothWheel();
	void tick();
	void tock();
private:
};


#include "missingToothWheel.cpp"

#endif
