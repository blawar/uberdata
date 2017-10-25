#ifndef LOG_H
#define LOG_H

#include "pods.h"

#ifndef LINUX
#include <stdio.h>

class Log
{
public:

	Log& operator<<(const char* right)
        {
		//printf("%s", right);
                return *this;
        }

        Log& operator<<(const uint16 right)
        {
                //printf("%x", right);
                return *this;
        }

	template<class T>
	Log& operator<<(const T right)
	{
		//printf("*");
		return *this;
	}
};

#define endl	"\n";

extern Log log;

#else
#include <stdio.h>
#include <iostream>

#define endl	"\n"
#define log	std::cout

#endif

#endif
