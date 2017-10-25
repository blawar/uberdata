#ifndef SENSORS_H
#define SENSORS_H

#include "engine.h"
#include "sensor.h"
#include "tps.h"
#include "rpm.h"

enum
{
	MAP,
	MAF,
	ECT,
	O2,
	IAT,
	TPS,
	CMP,
	CKP,
	INJ1,
	INJ2,
	INJ3,
	INJ4,
        INJ5,
        INJ6,
        INJ7,
        INJ8,
        INJ9,
        INJ10,
        INJ11,
        INJ12,
        INJ13,
        INJ14,
        INJ15,
        INJ16,
        INJ17,
        INJ18,
        INJ19,
        INJ20,
        INJ21,
        INJ22,
        INJ23,
        INJ24,
        INJ25,
        INJ26,
        INJ27,
        INJ28,
        INJ29,
        INJ30,
        INJ31,
        INJ32,
        IGN1,
        IGN2,
        IGN3,
        IGN4,
        IGN5,
        IGN6,
        IGN7,
        IGN8,
        IGN9,
        IGN10,
        IGN11,
        IGN12,
        IGN13,
        IGN14,
        IGN15,
        IGN16,
        IGN17,
        IGN18,
        IGN19,
        IGN20,
        IGN21,
        IGN22,
        IGN23,
        IGN24,
        IGN25,
        IGN26,
        IGN27,
        IGN28,
        IGN29,
        IGN30,
        IGN31,
        IGN32
};

enum
{
	NOTSET,
	IN,
	OUT,
	INOUT
};

class Sensors
{
public:
	Member(Sensor<Pressure>, baro);
	Member(Sensor<Pressure>, map);
	Member(Sensor<Temperature>, iat);
	Member(Sensor<Temperature>, clt);
	Member(Sensor<Rpm>, rpm);
	Member(Sensor<Tps>, tps);
};

#endif

