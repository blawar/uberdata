#ifndef DICT_H
#define DICT_H

#include "pods.h"
#include "sortedList.h"

#define DictBase	SortedList<KT,LENGTH>
#define ListType	List<VT,LENGTH>

template<class KT, class VT, unsigned int LENGTH>
class Dict
{
public:
	/*Dict() : DictBase()
	{
	}*/

	VT& operator[](KT key)
	{
		Index i = keys().getInsertLocation(key, true);
		keys()[i] = key;

		if(keys().size() != values().size())
		{
			values().rshift(i);
		}

		return values()[i];
	}

        void dump()
        {
/*
                for(Index i=0; i < keys().size(); i++)
                {
                        printf("key: %d, value: %d\n", keys()[i], values()[i]);
                }
*/
        }

	VT& ord(Index i) { return values()[i]; }
	KT& key(Index i) { return keys()[i]; }
	VT& value(Index i) { return values()[i]; }

	Index size() { return keys().size(); }

private:
	Member(DictBase, keys);
	Member(ListType,  values);
};

#endif
