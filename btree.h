#ifndef BTREE_H
#define BTREE_H

#define SCALAR_MAX 0xFF

template<class T, unsigned int LENGTH>
class BTree
{
public:
        void set(unsigned int ord, T value)
        {
                m_data[ord] = value;
        }

        T& get(unsigned int ord)
        {
                return m_data[ord];
        }

        unsigned int interpolate(T value, unsigned int& ord1, unsigned int& ord2)
        {
                ord1 = find(value);
                T val1 = get(ord1);

                ord2 = ord1+1;
                T val2 = get(ord2);

                T range = val2 - val1;
                T offset = val2 - value;
                return SCALAR_MAX * offset / range;
        }

        unsigned int find(T value)
        {
                unsigned int i=LENGTH-1;
                while(value < m_data[i] && i >= 0)
                {
                        i--;
                }
                return i;
        }
private:
        T m_data[LENGTH];
};

#endif

