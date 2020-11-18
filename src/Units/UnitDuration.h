/**
 *	Units
 *	Copyright Pedro Enrique
 */

#ifndef UnitDuration_h
#define UnitDuration_h

#include "Unit.h"

class UnitDuration : public Unit<UnitDuration>
{
public:
	UnitDuration(const char* symbol, double coefficient, double constant = 0) :
	Unit(symbol, coefficient, constant) { }
	
	UnitDuration base() {
		return UnitDuration::seconds();
	}
	static UnitDuration microseconds() {
		return UnitDuration("μs", 0.0001);
	}
	static UnitDuration milliseconds() {
		return UnitDuration("ms", 0.001);
	}
	static UnitDuration centiseconds() {
		return UnitDuration("cs", 0.01);
	}
	static UnitDuration deciseconds() {
		return UnitDuration("ds", 0.1);
	}
	static UnitDuration seconds() {
		return UnitDuration("sec", 1.0);
	}
	static UnitDuration minutes() {
		return UnitDuration("min", 60.0);
	}
	static UnitDuration hours() {
		return UnitDuration("hr", 3600.0);
	}
	static UnitDuration day() {
		return UnitDuration("day", 86400.0);
	}
	
};

#endif
