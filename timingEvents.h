#ifndef TIMINGEVENTS_H
#define TIMINGEVENTS_H

class TimingEvent;

#include "engineWheel.h"
#include "timingEvent.h"
#include "dict.h"
#include "functor.h"

class TimingEvents
{
public:
	TimingEvents()
	{
		count() = 0;
	}

	TimingEvent& set(TimingKey key, Rotation position, Functor* callback, void* param1 = 0, void* param2 = 0)
	{
		TimingEvent& event = events()[key];
		event.position() = position;
		event.callback() = callback;
		event.param1() = param1;
		event.param2() = param2;
		return event;
	}

	TimingEvent& operator[](TimingKey key)
	{
		return events()[key];
	}

	void process(Rotation position)
	{
                for(Index i=0; i < events().size(); i++)
                {
			TimingEvent& event = events().value(i);
			if(!event.active())
			{
				continue;
			}

			if(position >= event.position())
			{
				event.trigger(position);
				event.disable();
			}
                }
	}

	void dump()
	{
/*
		for(Index i=0; i < events().keys().size(); i++)
		{
			printf("key: %d, ord: %d\n", events().keys()[i], i);
		}
*/
	}

	TimingEvent& next(Rotation position)
	{
        	Index left = 0;
        	Index right = count()-1;
        	Index i = 0;
        	while(left != right)
        	{
                	i = (left + right) / 2;
                	TimingEvent& event = events()[i];
                	if(position < event.position())
                	{
                        	right = i;
                	}
                	else if(position > event.position())
                	{
                        	left = i;
                	}
                	else
                	{
                        	return event;
                	}
        	}
	        return events()[i];
	}
private:
	Member(TimingEventBucket, events);
	Member(Index, count);
};

#endif
