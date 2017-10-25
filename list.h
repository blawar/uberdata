#ifndef LISTTREE_H
#define LISTTREE_H

#include "scale.h"

typedef uint16	Index;

template<class T, Index LENGTH>
class List
{
public:
	List()
	{
		reset();
	}

	void reset()
	{
		size() = 0;
		for(Index i=0; i < LENGTH; i++)
		{
			nodes()[i] = T(0);
		}
	}

	template<class IT, class DERIVED>
        IT interpolate(T value, Index& ord1, Index& ord2)
        {
		// have to use this hackish method to ensure it calls derived class methods
		DERIVED& obj = *static_cast<DERIVED*>(this);
                ord2 = obj.getInsertLocation(value);
                T val1;
		T val2 = obj.get(ord2);

		if(ord2 == 0 || val2 == value)
		{
			ord1 = ord2;
			return 0;
		}
		else
		{
			ord1 = ord2 - 1;
                	val1 = obj.get(ord1);
		}

                T range = val2 - val1;
                T offset = val2 - value;
		if(!range)
		{
			return ~0;
		}
                return ~(MAXVALUE(IT) * offset / range);
        }

        const T& operator[](Index i) const
        {
                return nodes()[i];
        }

	T& operator[](Index i)
	{
                if(i >= size())
                {
                        size() = i+1;
                }
		return nodes()[i];
	}

	const T& get(Index i) const
	{
		return nodes()[i];
	}

	T& get(Index i)
	{
		if(i >= size())
		{
			size() = i+1;
		}
		return nodes()[i];
	}

	T& insert(T value)
	{
		if(size() == LENGTH)
		{
			return nodes()[0];
		}

		nodes()[size()++] = value;
		return nodes()[size()-1];
	}

	T& insert()
        {
                if(size() == LENGTH)
                {
                        return nodes()[0];
                }
		size()++;

                return nodes()[size()-1];
        }

	Index getInsertLocation(const T value, bool alloc = false)
	{
		if(alloc)
		{
			return size()++;
		}
		return size();
	}

	template<class IT>
	void set(const IT& data)
	{
		reset();
		for(Index i=0; i < sizeof(data); i++)
		{
			insert(data[i]);
		}
	}

	bool lshift(Index offset)
	{
		if(!size())
		{
			return false;
		}
		--size();
		for(Index i=offset; i < size(); i++)
		{
			nodes()[i] = nodes()[i+1];
		}
		return true;
	}

	bool rshift(Index offset)
	{
		if(size()+1 >= LENGTH)
		{
			return false;
		}
		if(size() && size() != offset)
		{
			Index i=size();
			do
			{
				--i;
				nodes()[i+1] = nodes()[i];
			}
			while(i && i >= offset);
		}
		size()++;
		return true;
	}

	const Index find(T value) const
	{
		for(Index i=0; i < size(); i++)
		{
			if(get(i) == value)
			{
				return i;
			}
		}
		return NOT_FOUND;
	}

private:
	Member(Index, size);
	MemberArr(T, nodes, LENGTH);
};

#endif
