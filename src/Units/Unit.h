/**
 *	Units
 *	Copyright Pedro Enrique
 */

#ifndef Unit_h
#define Unit_h

#ifdef ARDUINO
#include "Arduino.h"
#else
#include <string>
typedef std::string String;
#endif

template<class Derived>
class Unit
{
public:
	Unit(const String& symbol, double coefficient, double constant) {
		m_symbol = symbol;
		m_coefficient = coefficient;
		m_constant = constant;
	}
	const String symbol() {
		return m_symbol;
	}
	double coefficient() {
		return m_coefficient;
	}
	double constant() {
		return m_constant;
	}
	double baseUnitValue(double from) {
		return m_coefficient * from + m_constant;
	}
	double value(double fromBase) {
		return (fromBase - m_constant) / m_coefficient;
	}
	
	virtual Derived base() = 0;
private:
	String m_symbol;
	double m_coefficient;
	double m_constant;
};

#endif
