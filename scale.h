#ifndef SCALE_H
#define SCALE_H

#include "pods.h"

template<class T>
class Scalar
{
private:
	Member(T, value);
};

/*
 *  *  *         a = (1<<8)*aH + aL;
 *   *   *                 b = (1<<8)*bH + bL;
 *    *    *                 (a*b) = (1<<16)*aH*bH
 *     *     *                       + (1<<8)*aH*bL
 *      *      *                             + (1<<8)*aL*bH
 *       *       *                                   + aL*bL
 *        *        *                                   */

template<class T, class S>
T blend(const T a, const T b, const S scalar)
{
	if(b.value() > a.value())
	{
		return T(a.value() + ((int32)(b.value() - a.value()) * scalar / (MAXVALUE(scalar) >> ISSIGNED(S))));
	}
	else
	{
		return T(a.value() - ((int32)(a.value() - b.value()) * scalar / (MAXVALUE(scalar) >> ISSIGNED(S))));
	}
}

template<class T, class S>
T scale2(const T num, const S scalar)
{
	//printf("MAXVALUE = %d, ISSIGNED = %d\n", MAXVALUE(scalar), ISSIGNED(S));
        return num+(num * scalar / ((MAXVALUE(scalar) >> ISSIGNED(S)) + 1));
}

#endif

