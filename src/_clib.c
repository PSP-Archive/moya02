// clib-mimic

// *******************************************************************

#include "_clib.h"

// *******************************************************************

#define SIN   1
#define COS   2
#define TAN   3
#define EXP   4
#define SINH  5
#define COSH  6
#define TANH  7
#define PI	  3.1415926536	// or use atan(1)*4.0

// *******************************************************************

unsigned int sqrtu(unsigned int x)
{
	unsigned int r = 0;
	int n = 15;

	for(n = 15; n >= 0; n--)
	{
		unsigned int t = r + (1<<n);
		if(x >= t << n)
		{
			x -= t << n;
			r |= 2 << n;
		}
	}

	return r >> 1;
}

// *******************************************************************
//
// generate 7 transcendental values with one function
// p = function selector,  #define the following:
// SIN 1   COS 2   TAN 3   EXP 4   SINH 5	COSH 6	 TANH 7
// n = convergence cutoff, n = 3 is typical,
// higher n gives higher precision, but slower execution
//
float trigfunc(int p, int n, float x)
{
	int 	k;
	float  r, s, t;

	if (p <= 3)
	{
		r = - x * x;	// trig
	}
	else
	{
		r = x * x;		// hyperbolic
	}
	s = 4 * n + 2;
	for (k = n; k > 0; k--)
	{
		s = 4 * k - 2 + r/s;
	}
	switch (p & 3)
	{
	case 0: t =     (s + x)/(s - x);		break;	// exp
	case 1: t =  2 * x * s /(s * s - r);	break;	// sin, sinh
	case 2: t = (s * s + r)/(s * s - r);	break;	// cos, cosh
	case 3: t =  2 * x * s /(s * s + r);	break;	// tan, tanh
	}
	return (t);
}
// ********************************************************

static unsigned Seed1;
static unsigned Seed2;

// *******************************************************************
// These are _really_ expensive.. so not very ideal
// Lookuptables will be a better idea

float fcos(float angle)
{
	return trigfunc(SIN, 3, angle);
}

// *******************************************************************
//
float fsin(float angle)
{
	return trigfunc(COS, 3, angle);
}

//***************************************************************************
//
// Classic fast random method

unsigned sceRand()
{
	Seed1 = (Seed1 + 46906481) ^ Seed2;
	Seed2 = Seed1 ^ ( ((Seed2<<3) | (Seed2 >> 29)) + 103995407);

	return Seed1 - Seed2;
}

// *******************************************************************

void _strcpy(char *d,const char *s)
{
	for (; *s!=0; ++s, ++d) *d=*s;
	*d=0;
}

void _strcat(char *d,const char *s)
{
	for (; *d!=0; ++d) ;
	for (; *s!=0; ++s, ++d) *d=*s;
	*d=0;
}

void _strncpy(char *d,const char *s,int n)
{
	for (; *s!=0 && n>0; ++s, ++d, --n) *d=*s;
	*d=0;
}


char *_strchr(const char *_s, int c)
{
	char *s=(char *)_s;
	for (; *s!=0; s++) {
		if (*s==(char)c) return s;
	}
	return NULL;
}

unsigned int _strlen(const char *s)
{
	unsigned int r=0;
	for (; *s!=0; ++s, ++r) ;
	return r;
}


void _memset(void *d, const long s, unsigned long n)
{
	for (; n>0; n--) *(((char *)d++))=s;
}

int _memcmp(const void *s1, const void *s2, unsigned long n)
{
	for (; n>0; n--) {
		if (*(((unsigned char *)s1))>*(((unsigned char *)s2))) return 1;
		if (*(((unsigned char *)s1++))<*(((unsigned char *)s2++))) return -1;
	}
	return 0;
}

void _memcpy(void *d, const void *s, unsigned long n)
{
	for (; n>0; n--) {
		(*(((unsigned char *)d++))=*(((unsigned char *)s++)));
	}
}


