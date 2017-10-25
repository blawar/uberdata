#ifndef NULLABLE_H
#define NULLABLE_H

template<class T, class NT = byte>
class Nullable
{
public:
	Nullable()
	{
	}

	Nullable(const T& val)
	{
		operator=(val);
	}

	operator T() const
	{
		if(isNull()) return 0;
		return value();
	}

        operator T()
        {
                return value();
        }

	Nullable& operator=(const T val)
	{
		value() = val;
		setNull(false);
		return *this;
	}

/*
	bool operator>(const Nullable<T, NT> right) const
	{
		if(isNull())
		{
			return false;
		}
		if(right.isNull())
		{
			return true;
		}
		return value() > right.value();
	}

	T operator-(const Nullable<T, NT> right) const
	{
		return value() - right.value();
	}

        T operator-(const T right) const
        {
                return value() - right;
        }

        T operator+(const Nullable<T, NT> right) const
        {
                return value() + right.value();
        }

        T operator+(const T right) const
        {
                return value() + right;
        }
*/

	BITI(Null, BIT8, null());
	BIT(Locked, BIT7, null());
private:
	Member(NT, null);
	Member(T, value);
};

#endif
