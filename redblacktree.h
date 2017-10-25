#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

#include "pods.h"

#define BLACK 0
#define RED 1

template<class VALUE>
class RBNode
{
public:
	RBNode()
	{
		m_color = 0;
	}

	bool isNull()
	{
		return m_color & BIT7;
	}

	bool color()
	{
		return m_color & BIT8;
	}

	bool isSet()
	{
		return m_color & BIT6;
	}

	VALUE& value() { return m_value; }

	unsigned char& options() { return m_color; }
private:
	VALUE m_value;
	unsigned char m_color;
};

template<class KEY, class VALUE, unsigned int LENGTH>
class RedBlackTree
{
public:
	RedBlackTree()
	{
		m_count = 0;
	}

	void insert(KEY& key, VALUE& value)
	{
		if(!m_count)
		{
			m_nodes[0].options() = BIT6; // set
			m_nodes[0].value() = value;
			++m_count;
			return;
		}

		const RBNode n = getAvailNode();
	}
private:
	RBNode& getAvailNode()
	{
		for(unsigned int i=0; i < LENGTH; i++)
		{
			if(m_nodes[i].isSet())
			{
				return m_nodes[i];
			}
		}
		return m_nodes[LENGTH-1];
	}
	unsigned int m_count;
	RBNode<VALUE>	m_nodes[LENGTH];
};

#endif
