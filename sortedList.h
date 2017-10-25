#ifndef SORTEDLISTTREE_H
#define SORTEDLISTTREE_H

#include "list.h"

template<class T, Index LENGTH>
class SortedList : public List<T, LENGTH>
{
public:
	SortedList() : List<T,LENGTH>()
	{
	}

	T& insert(T value)
	{
		if(this->size() == LENGTH)
		{
			return this->get(0);
		}

		Index i = getInsertLocation(value);
		this->rshift(i);
		this->get(i) = value;
		return this->get(i);
	}

	T& insert()
	{
		// should explicitly throw an error here
		return this->get(0);
	}

	Index getInsertLocation(const T value, bool alloc = false)
	{
		if(!this->size()) 
		{
			if(alloc) this->size()++;
			return 0;
		}

                Index left = 0;
                Index right = this->size();
                Index i = 0;
                do
                {
                        i = (left+right)/2;
                        if(this->get(i) == value)
                        {
                                return i;
                        }
                        else if(value < this->get(i))
                        {
				if(i == right)
				{
					if(alloc) this->rshift(left);
					return left;
				}
                                right = i;
                        }
                        else
                        {
                                if(i == left)
                                {
					if(alloc) this->rshift(right);
					return right;
                                }
                                left = i;
                        }
                }
                while(left < right);
		if(alloc) this->rshift(left);
		return left;
	}

	Index find(T value)
	{
                if(!this->size())
                {
                        return 0;
                }
		Index left = 0;
		Index right = this->size();
                Index i = 0;
                do
                {
			i = (left+right) / 2;
                        if(this->get(i) == value)
                        {
                                return i;
                        }
                        else if(value < this->get(i))
                        {
				right = i;
                        }
                        else
                        {
				if(i == left)
				{
					return right;
				}
                                left = i;
                        }
                }
                while(left < right);
		return NOT_FOUND;
	}

	/*const Index find(T value, Index& a, Index& b)
	{
	}*/

private:
};

#endif
