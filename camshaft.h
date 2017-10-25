#ifndef CAMSHAFT_H
#define CAMSHAFT_H

#include "rotation.h"

class Camshaft
{
public:
	Camshaft& open(Rotation r)
	{
        	open() = r;
        	return *this;
	}

	Camshaft& close(Rotation r)
	{
        	close() = r;
        	return *this;
	}

	Camshaft& offset(Rotation r)
	{
        	offset() = r;
        	return *this;
	}

private:
	Member(Rotation, offset);
	Member(Rotation, open);
	Member(Rotation, close);
};


#endif
