#include "jalaali.h"

static inline int mfloor(double d)
{
	return (int)d;
}

static inline int leap_solar(int  year)
{
    return ((year % 4) == 0) && (!(((year % 100) == 0) && ((year % 400) != 0)));
}

static inline double mod(double a,double b)
{
    return a - (b * mfloor(a / b));
}

static inline int mceil(double d)
{
	if(((int)(d * 1000000) % 1000000) > 0)
	{
		return (int)d + 1;
	}
	else
	{
		return (int)d;
	}
}

static double solar_to_julian(int year, int month, int day)
{
	return (SOLAR_EPOCH - 1) + (365 * (year - 1)) + mfloor((year - 1) / 4) + (0 - mfloor((year - 1) / 100)) + mfloor((year - 1) / 400) + mfloor((((367 * month) - 362) / 12) + ((month <= 2) ? 0 : (leap_solar(year) ? -1 : -2)) + day);
}

static float jalaali_to_julian(int year, int month, int day)
{
	double epbase, epyear;
	epbase = year - ((year >= 0) ? 474 : 473);
	epyear = 474 + mod(epbase, 2820);
	return day + ((month <= 7) ? ((month - 1) * 31) : (((month - 1) * 30) + 6)) + mfloor(((epyear * 682) - 110) / 2816) + (epyear - 1) * 365 + mfloor(epbase / 2820) * 1029983 + (JALAALI_EPOCH - 1);
}

static int julian_to_jalaali(int *j_year, int *j_mon, int *j_mday, double jd)
{
	int year, mon;
	double depoch, cycle, cyear, ycycle, aux1, aux2, yday;

	jd = mfloor(jd) + 0.5;
	depoch = jd - jalaali_to_julian(475, 1, 1);
	cycle = mfloor(depoch / 1029983);
	cyear = mod(depoch, 1029983);

	if (cyear == 1029982)
	{
		ycycle = 2820;
	}
	else
	{
		aux1 = mfloor(cyear / 366);
		aux2 = mod(cyear, 366);
		ycycle = mfloor(((2134 * aux1) + (2816 * aux2) + 2815) / 1028522) + aux1 + 1;
	}

	year = ycycle + (2820 * cycle) + 474;
	yday = (jd - jalaali_to_julian(year, 1, 1)) + 1;
	mon = (yday <= 186) ? mceil(yday / 31) : mceil((yday - 6) / 30);

	*j_year = year;
	*j_mon = mon;
	*j_mday = (jd - jalaali_to_julian(year, mon, 1)) + 1;
	return 0;
}

int solar_to_jalaali(int *year, int *mon, int *mday)
{
	return julian_to_jalaali(year, mon, mday, solar_to_julian(*year, *mon, *mday));
}
