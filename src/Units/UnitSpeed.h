/**
 *	Units
 *	Copyright Pedro Enrique
 */

#ifndef UnitSpeed_h
#define UnitSpeed_h

#include "Unit.h"

class UnitSpeed : public Unit<UnitSpeed>
{
public:
	UnitSpeed(const char* symbol, double coefficient, double constant = 0.0) :
	Unit(symbol, coefficient, constant) { }
	
	UnitSpeed base() {
		return UnitSpeed::metersPerSecond();
	}
	static UnitSpeed metersPerSecond() {
		return UnitSpeed("mps", 1.0);
	}
	static UnitSpeed kilometersPerHour() {
		return UnitSpeed("kmh", 0.277778);
	}
	static UnitSpeed milesPerHour() {
		return UnitSpeed("mph", 0.447040);
	}
	static UnitSpeed knots() {
		return UnitSpeed("kn", 0.514444);
	}
	static UnitSpeed feetPerMinute() {
		return UnitSpeed("fpm", 0.00508);
	}
};

#endif
