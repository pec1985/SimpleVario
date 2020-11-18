/**
 *	Units
 *	Copyright Pedro Enrique
 */

#ifndef UnitLength_h
#define UnitLength_h

#include "Unit.h"

class UnitLength : public Unit<UnitLength>
{
public:
	UnitLength(const char* symbol, double coefficient, double constant = 0.0) :
	Unit(symbol, coefficient, constant) { }

	UnitLength base() {
		return UnitLength::meters();
	}

	static UnitLength megameters() {
		return UnitLength("Mm", 1000000.0);
	}
	static UnitLength kilometers() {
		return UnitLength("km", 1000.0);
	}
	static UnitLength hectometers() {
		return UnitLength("hm", 100.0);
	}
	static UnitLength decameters() {
		return UnitLength("dam", 10.0);
	}
	static UnitLength meters() {
		return UnitLength("m", 1.0);
	}
	static UnitLength decimeters() {
		return UnitLength("dm", 0.1);
	}
	static UnitLength centimeters() {
		return UnitLength("cm", 0.01);
	}
	static UnitLength millimeters() {
		return UnitLength("mm", 0.001);
	}
	static UnitLength micrometers() {
		return UnitLength("µm", 0.000001);
	}
	static UnitLength nanometers() {
		return UnitLength("nm", 0.000000001);
	}
	static UnitLength picometers() {
		return UnitLength("pm", 0.000000000001);
	}
	static UnitLength inches() {
		return UnitLength("in", 0.025400);
	}
	static UnitLength feet() {
		return UnitLength("ft", 0.304800);
	}
	static UnitLength yards() {
		return UnitLength("yd", 0.914400);
	}
	static UnitLength miles() {
		return UnitLength("mi", 1609.34);
	}
	static UnitLength scandinavianMiles() {
		return UnitLength("smi", 10000.0);
	}
	static UnitLength lightyears() {
		return UnitLength("ly", 9461000000000000.0);
	}
	static UnitLength nauticalMiles() {
		return UnitLength("NM", 1852.0);
	}
	static UnitLength fathoms() {
		return UnitLength("ftm", 1.828800);
	}
	static UnitLength furlongs() {
		return UnitLength("fur", 201.168);
	}
	static UnitLength astronomicalUnits() {
		return UnitLength("ua", 149600000000.0);
	}
	static UnitLength parsecs() {
		return UnitLength("pc", 30860000000000000.0);
	}
};

#endif
