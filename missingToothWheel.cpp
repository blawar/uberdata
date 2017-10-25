#include "missingToothWheel.h"

template<uint16 TEETH, uint16 MISSING_TEETH>
MissingToothWheel<TEETH, MISSING_TEETH>::MissingToothWheel() : EngineWheel<TEETH, MISSING_TEETH>()
{
}

template<uint16 TEETH, uint16 MISSING_TEETH>
void MissingToothWheel<TEETH, MISSING_TEETH>::tick()
{
	Time now = timestamp();

	this->toothPosition() += this->TOOTH_ROTATION;
	this->currentPosition() = this->toothPosition();
	this->update();
	
	if(this->toothMetrics()[this->tooth()].time)
	{
		this->toothMetrics()[this->tooth()].duration = now - this->toothMetrics()[this->tooth()].time;
	}

	this->tooth()++;
	//printf("tooth: %d\n", this->tooth());
	if(this->tooth() >= TEETH) {
		this->tooth() = 0;
		this->reset();
	}

	this->toothMetrics()[this->tooth()].time = now;
}

template<uint16 TEETH, uint16 MISSING_TEETH>
void MissingToothWheel<TEETH, MISSING_TEETH>::tock()
{
	if(!this->tockOffset()) {
		return;
	}

	this->currentPosition() += this->tockOffset();
	this->update();
}

