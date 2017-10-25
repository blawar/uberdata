#ifndef MAP_H
#define MAP_H

#include "engine.h"
#include "btree.h"
#include "log.h"

#define SortedListXT	SortedList<XTYPE,X_SZ>
#define SortedListYT    SortedList<YTYPE,Y_SZ>
#define SortedListZT    SortedList<ZTYPE,Z_SZ>
#define SortedListAT    SortedList<ATYPE,A_SZ>
#define SortedListBT    SortedList<BTYPE,B_SZ>
#define SortedListCT    SortedList<YTYPE,C_SZ>

struct Point
{
	Index a,b;
};

template<class T, class XTYPE, Index X_SZ, class YTYPE, Index Y_SZ, class ZTYPE=uint8, Index Z_SZ=1, class ATYPE=uint8, Index A_SZ=1, class BTYPE=uint8, Index B_SZ=1, class CTYPE=uint8, Index C_SZ=1>
class Map
{
public:
	Map()
	{
		reset();
	}

	template<class AT>
	Map(const AT& m)
	{
		load(m);
	}

	template<class AT>
	void load(const AT& m, bool rawValues = true)
	{
                reset();
                for(Index y=0; y < sizeof(m) / sizeof(*m); y++)
                {
                        for(Index x=0; x < sizeof(*m); x++)
                        {
				if(rawValues)
				{
                                	set(T(m[y][x]), x, y);
				}
				else
				{
					data()[0][0][0][0][y][x].set(T(m[y][x]));
				}
                        }
                }
	}

	void reset()
	{
		for(Index c=0; c < C_SZ; c++)
		{
			for(Index b=0; b < B_SZ; b++)
			{
				for(Index a=0; a < A_SZ; a++)
				{
					for(Index z=0; z < Z_SZ; z++)
					{
						for(Index y=0; y < Y_SZ; y++)
						{
							for(Index x=0; x < X_SZ; x++)
							{
								data()[c][b][a][z][y][x] = T(0);
							}
						}
					}
				}
			}
		}
	}

	void set(T value, const XTYPE x=0, const YTYPE y=0, const ZTYPE z=0, const ATYPE a=0, const BTYPE b=0, const CTYPE c=0)
	{
		data()[c][b][a][z][y][x] = value;
	}

	const T& get(const XTYPE x=0, const YTYPE y=0, const ZTYPE z=0, const ATYPE a=0, const BTYPE b=0, const CTYPE c=0) const
	{
		return data()[c][b][a][z][y][x];
	}

	T& get(const XTYPE x=0, const YTYPE y=0, const ZTYPE z=0, const ATYPE a=0, const BTYPE b=0, const CTYPE c=0)
	{
		return data()[c][b][a][z][y][x];
	}

	void processValues(uint16 (&scalars)[6], Index (&pts)[6][2], const Index x, const Index y, const Index z, const Index a, const Index b, const Index c)
	{
		/*while(const T& tmp = get(pts[0][x],pts[1][y],pts[2][z],pts[3][a],pts[4][b],pts[5][c]))
		{
			if(pts[i][0] == 0)
			{
				pts[i][0].setNull(true);
			}
		}*/
	}

	T lookup(const XTYPE xValue, const YTYPE yValue, const ZTYPE zValue=0, const ATYPE aValue=0, const BTYPE bValue=0, const CTYPE cValue=0)
	{
		Index pts[6][2];
		uint16 scalars[6];

		T results[6];
		const byte DEPTH = 6;
		scalars[DEPTH-1] = c().SortedListCT::template interpolate<uint16,SortedListCT>(cValue, pts[DEPTH-1][0], pts[DEPTH-1][1]);
		for(Index c=0; c < 2 && c < C_SZ; c++)
		{
			const byte DEPTH = 5;
			scalars[DEPTH-1] = b().SortedListBT::template interpolate<uint16,SortedListBT>(bValue, pts[DEPTH-1][0], pts[DEPTH-1][1]);
			for(Index b=0; b < 2 && b < B_SZ; b++)
			{
				const byte DEPTH = 4;
				scalars[DEPTH-1] = a().SortedListAT::template interpolate<uint16,SortedListAT>(aValue, pts[DEPTH-1][0], pts[DEPTH-1][1]);
				for(Index a=0; a < 2 && a < A_SZ; a++)
				{
					const byte DEPTH = 3;
					scalars[DEPTH-1] = z().SortedListZT::template interpolate<uint16,SortedListZT>(zValue, pts[DEPTH-1][0], pts[DEPTH-1][1]);
					for(Index z=0; z < 2 && z < Z_SZ; z++)
					{
						const byte DEPTH = 2;
						scalars[DEPTH-1] = y().SortedListYT::template interpolate<uint16,SortedListYT>(yValue, pts[DEPTH-1][0], pts[DEPTH-1][1]);
						for(Index y=0; y < 2 && y < Y_SZ; y++)
						{
							const byte DEPTH = 1;
							scalars[DEPTH-1] = x().SortedListXT::template interpolate<uint16,SortedListXT>(xValue, pts[DEPTH-1][0], pts[DEPTH-1][1]);
							for(Index x=0; x < 2 && x < X_SZ; x++)
							{
								processValues(scalars, pts, x, y, z, a, b, c);
								const byte DEPTH = 0;
								const T& tmp = get(pts[0][x],pts[1][y],pts[2][z],pts[3][a],pts[4][b],pts[5][c]);
								if(x) {
									/*if(tmp.isNull())
									{
										// leave the old value in
									}
									else if(results[DEPTH].isNull())
									{
										results[DEPTH] = tmp;
									}
									else*/
									{
										//printf("%d, %d * %u = ", (int)results[DEPTH], (int)tmp, scalars[DEPTH]);
										//printf("x: %d, y: %d\n", pts[0][x], pts[1][y]);
										//printf("%d, %d * %d = ", results[DEPTH].value(), tmp.value(), scalars[DEPTH]);
										results[DEPTH] = blend(results[DEPTH], tmp, scalars[DEPTH]);
										//printf("%d\n", results[DEPTH].value());
									}
								} else {
									results[DEPTH] = tmp;
								}
							}
							if(y) {
								results[DEPTH] = blend(results[DEPTH],  results[DEPTH-1], scalars[DEPTH]);
							} else {
								results[DEPTH] = results[DEPTH-1];
							}
						}
						if(z) {
							results[DEPTH] = blend(results[DEPTH],  results[DEPTH-1], scalars[DEPTH]);
						} else {
							results[DEPTH] = results[DEPTH-1];
						}
					}
					if(a) {
						results[DEPTH] = blend(results[DEPTH],  results[DEPTH-1], scalars[DEPTH]);
					} else {
						results[DEPTH] = results[DEPTH-1];
					}
				}
				if(b) {
					results[DEPTH] = blend(results[DEPTH],  results[DEPTH-1], scalars[DEPTH]);
				} else {
					results[DEPTH] = results[DEPTH-1];
				}
			}
			if(c) {
				results[DEPTH] = blend(results[DEPTH],  results[DEPTH-1], scalars[DEPTH]);
			} else {
				results[DEPTH] = results[DEPTH-1];
			}
		}
		return results[5];

		/*const T& av1 = data()[y1][x1];
		const T& av2 = data()[y1][x2];
		const T& bv1 = data()[y2][x1];
		const T& bv2 = data()[y2][x2];
		return blend(
			blend(av1, av2, xscalar),
			blend(bv1, bv2, xscalar),
			yscalar);*/
	}

	void dump()
	{
#ifdef STDOUT

		printf("     ");
		for(unsigned int x=0; x < X_SZ; x++)
		{
			printf("%4.4d ", this->x().get(x));
		}
		printf("\n");

		for(Index y=0; y < Y_SZ; y++)
		{
			printf("%4.4d ", this->y().get(y));
			for(Index x=0; x < X_SZ; x++)
			{
				printf("%3.3d  ", (int)this->get(x,y).get());
			}
			printf("\n");
		}
#endif
	}

private:
	Member(SortedListXT, x);
	Member(SortedListYT, y);
	Member(SortedListZT, z);
	Member(SortedListAT, a);
	Member(SortedListBT, b);
	Member(SortedListCT, c);
	MemberArr6(T, data, X_SZ, Y_SZ, Z_SZ, A_SZ, B_SZ, C_SZ);
};

#endif
