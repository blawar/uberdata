#ifndef TPS_H
#define TPS_H

#define TpsPercent Percent<byte,0,100,uint16,100,Tps>

class Tps : public TpsPercent
{
public:
	Tps() : TpsPercent()
	{
	}

	Tps(uint16 val) : TpsPercent(val)
	{
	}
};

#endif
