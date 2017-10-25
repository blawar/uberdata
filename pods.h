#ifndef PODS_H
#define PODS_H

typedef signed char	int8;
typedef unsigned char	uint8;
typedef uint8	byte;
typedef signed short	int16;
typedef unsigned short	uint16;
typedef signed int	int32;
typedef unsigned int	uint32;

#define TIMER_CLOCK_RATE      600000

#define DEFAULT_NUMBER	bool

#define Time		uint16
#define AirflowType	uint16	// kg/m3
#define Airflow		Alpha<AirflowType>

#define NOT_FOUND       (Index)-1
typedef uint16  Index;

#define MAX_TIMING_EVENTS 32
#define TimingKey       uint16
#define TimingEventBucket Dict<TimingKey,TimingEvent,MAX_TIMING_EVENTS>

#define Member(T,N) \
public: \
const T& N() const { return m_##N; } \
T& N() { return m_##N; } \
private: \
T m_##N; \
private: \


#define MemberPtr(T,N) \
public: \
T& N() { return m_##N; } \
private: \
T m_##N; \
private: \

#define MemberArr(T,N,SZ) \
public: \
T const (& N() const)[SZ] { return m_##N; } \
T (& N())[SZ] { return m_##N; } \
const T& N(Index i) const { return m_##N[i]; } \
T& N(Index i) { return m_##N[i]; } \
private: \
T m_##N[SZ]; \
private: \


#define MemberArr2(T,N,X,Y) \
public: \
/*T const (& N() const)[X][Y] { return m_##N; }*/ \
T (& N())[X][Y] { return m_##N; } \
/*const T& N(Index a, Index b) const { return m_##N[a][b]; } \
T& N(Index a, Index b) { return m_##N[a][b]; }*/ \
private: \
T m_##N[X][Y]; \
private: \


#define MemberArr6(T,N,X,Y,Z,A,B,C) \
public: \
/*T const (& N() const)[X][Y] { return m_##N; }*/ \
T (& N())[C][B][A][Z][Y][X] { return m_##N; } \
/*const T& N(Index a, Index b) const { return m_##N[a][b]; } \
 * T& N(Index a, Index b) { return m_##N[a][b]; }*/ \
private: \
T m_##N[C][B][A][Z][Y][X];


#define BIT(N,F,D) \
bool is##N() const \
{ \
	return D & F; \
} \
\
void set##N(const bool flag) \
{ \
	if(flag) \
	{ \
		D |= F; \
	} \
	else \
	{ \
		D &= ~(T(F)); \
	} \
} 

#define BITI(N,F,D) \
bool is##N() const \
{ \
        return !(D & F); \
} \
\
void set##N(const bool flag) \
{ \
        if(flag) \
        { \
                D &= ~(T(F)); \
        } \
        else \
        { \
                D |= F; \
        } \
}

#define BIT1	0x1
#define BIT2	0x2
#define BIT3	0x4
#define BIT4	0x8
#define BIT5	0x10
#define BIT6	0x20
#define BIT7	0x40
#define BIT8	0x80

#define ISSIGNED(t) (t(-1) < 0 ? 1 : 0)

#include "log2.h"

#define TE_PARTITION_COUNT	32
#define TE_PARTITION	(0xFFFF / TE_PARTITION_COUNT)

#define TE_ENG	0
#define TE_CYL	1

#define TEID(A,B)		(TE_PARTITION * A + B)
#define TEID_SUB(A,B,C,SZ)	((TE_PARTITION * A + ( B * SZ )) + C)

#define PURE	__attribute__((pure))

#define FAR_TEXTf2_ATTR __attribute__ ((far)) __attribute__ ((section (".textf2")))
#define FAR_TEXTf3_ATTR __attribute__ ((far)) __attribute__ ((section (".textf3")))
#define FAR_TEXTf4_ATTR __attribute__ ((far)) __attribute__ ((section (".textf4")))
#define FAR_TEXTf5_ATTR __attribute__ ((far)) __attribute__ ((section (".textf5")))
#define FAR_TEXTf6_ATTR __attribute__ ((far)) __attribute__ ((section (".textf6")))
#define FAR_TEXTf7_ATTR __attribute__ ((far)) __attribute__ ((section (".textf7")))
#define FAR_TEXTf8_ATTR __attribute__ ((far)) __attribute__ ((section (".textf8")))
#define FAR_TEXTf9_ATTR __attribute__ ((far)) __attribute__ ((section (".textf9")))
#define FAR_TEXTfa_ATTR __attribute__ ((far)) __attribute__ ((section (".textfa")))
#define FAR_TEXTfb_ATTR __attribute__ ((far)) __attribute__ ((section (".textfb")))
#define FAR_TEXTfc_ATTR __attribute__ ((far)) __attribute__ ((section (".textfc")))
#define FAR_TEXTfe_ATTR __attribute__ ((far)) __attribute__ ((section (".textfe")))
#define LOOKUP_ATTR __attribute__ ((section (".lookup")))
#define CNFDATA_ATTR __attribute__ ((section (".cnfdata")))
#define CNFDATA2_ATTR __attribute__ ((section (".cnfdata2")))
#define TEXTfc_ATTR __attribute__ ((section (".textfc")))
#define TEXT3_ATTR __attribute__ ((section (".text3")))
#define TEXT_ATTR __attribute__ ((section (".text")))
#define INTERRUPT
#define POST_INTERRUPT __attribute__((interrupt))
#define POST_INTERRUPT_TEXT3 __attribute__ ((section (".text3"))) __attribute__((interrupt))
#define POST_INTERRUPT_TEXT3d __attribute__ ((section (".text3d"))) __attribute__((interrupt))
#define ENABLE_INTERRUPTS __asm__ __volatile__ ("cli");
#define DISABLE_INTERRUPTS __asm__ __volatile__ ("sei");
#define NEAR
#define VECT_ATTR __attribute__ ((section (".vectors")))
#define DATA1_ATTR __attribute__ ((section (".data1")))
#define DATA2_ATTR __attribute__ ((section (".data2")))
#define XGATE_ALIGN_ATTR __attribute__ ((aligned (2)))
/* gcc insists on using indexed addressing for the following even if I don't, so follow suit */
#define SSEM0 __asm__ __volatile__ ("movw #0x0101, 0,x\n" "brclr 1,x, #0x01, -9\n" ::"x" (&XGSEMM) );
#define CSEM0 __asm__ __volatile__ ("movw #0x0100, %0\n" ::"m" (XGSEMM) );
#define CSEM0X __asm__ __volatile__ ("movw #0x0100, 0,x\n" ::"x" (&XGSEMM) );
#define SSEM0SEI __asm__ __volatile__ ("sei\n ""movw #0x0101, 0,x\n" "brclr 1,x, #0x01, -9\n" ::"x" (&XGSEMM) );
#define CSEM0CLI __asm__ __volatile__ ("movw #0x0100, %0\n" "cli\n" ::"m" (XGSEMM) );
#define SSEM1 __asm__ __volatile__ ("movw #0x0202, 0,x\n" "brclr 1,x, #0x02, -9\n" ::"x" (&XGSEMM) );
#define CSEM1 __asm__ __volatile__ ("movw #0x0200, %0\n" ::"m" (XGSEMM) );
#define SSEM2 __asm__ __volatile__ ("movw #0x0404, 0,x\n" "brclr 1,x, #0x04, -9\n" ::"x" (&XGSEMM) );
#define CSEM2 __asm__ __volatile__ ("movw #0x0400, %0\n" ::"m" (XGSEMM) );
#define FIRE_COIL XGSWTM=0x0101;
#define DWELL_COIL XGSWTM=0x0202;
#define FIRE_COIL_ROTARY XGSWTM=0x0404;
#define DWELL_COIL_ROTARY XGSWTM=0x0808;

#endif
