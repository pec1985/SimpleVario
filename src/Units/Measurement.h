/**
 *	Units
 *	Copyright Pedro Enrique
 */

#ifndef Meassurement_h
#define Meassurement_h

#include "Unit.h"

template<class T>
class Measurement
{
public:
	Measurement(double value, T unit, double r = -1.0) : m_unit(unit), m_value(value), m_roundTo(r) { }
	Measurement convertedTo(T otherUnit, double roundTo = -1.0) {
		double fromBase = m_unit.baseUnitValue(m_value);
		double toBase = otherUnit.value(fromBase);
		return Measurement(toBase, otherUnit, roundTo);
	}
	double value() {
		return m_value;
	}
	T unit() {
		return m_unit;
	}
	T baseUnit() {
		return m_unit.base();
	}
	void roundTo(double r) 
	{
		m_roundTo = round;
	}

	String description() {
#ifdef ARDUINO
		if (m_roundTo == -1.0) {
			return String(int(m_value)) + m_unit.symbol();
		} else {
			double roundTo = 1.0 / m_roundTo;
			auto result = round(m_value * roundTo) / roundTo;
			if (ceil(m_roundTo) == m_roundTo) {
				return String(int(result)) + m_unit.symbol();
			}
			return String(result) + m_unit.symbol();
		}
#else
		size_t count = std::to_string(m_value).size();
		char buffer[count];
		sprintf(buffer, "%.2f", m_value);
		return String(buffer) + " " + m_unit.symbol();
#endif
	}
private:
	T m_unit;
	double m_value;
	double m_roundTo;
};

template<class T>
Measurement<T> operator+(Measurement<T>& first, Measurement<T>& second)
{
	auto f = first.convertedTo(first.baseUnit()).value();
	auto s = second.convertedTo(second.baseUnit()).value();
	return Measurement<T>(f + s, first.baseUnit());
}
template<class T>
Measurement<T> operator-(Measurement<T>& first, Measurement<T>& second)
{
	auto f = first.convertedTo(first.baseUnit()).value();
	auto s = second.convertedTo(second.baseUnit()).value();
	return Measurement<T>(f - s, first.baseUnit());
}
template<class T>
Measurement<T> operator*(Measurement<T>& first, Measurement<T>& second)
{
	auto f = first.convertedTo(first.baseUnit()).value();
	auto s = second.convertedTo(second.baseUnit()).value();
	return Measurement<T>(f * s, first.baseUnit());
}
template<class T>
Measurement<T> operator/(Measurement<T>& first, Measurement<T>& second)
{
	auto f = first.convertedTo(first.baseUnit()).value();
	auto s = second.convertedTo(second.baseUnit()).value();
	return Measurement<T>(f / s, first.baseUnit());
}
template<class T>
bool operator==(Measurement<T>& first, Measurement<T>& second)
{
	auto f = first.convertedTo(first.baseUnit()).value();
	auto s = second.convertedTo(second.baseUnit()).value();
	return f == s;
}
template<class T>
bool operator!=(Measurement<T>& first, Measurement<T>& second)
{
	auto f = first.convertedTo(first.baseUnit()).value();
	auto s = second.convertedTo(second.baseUnit()).value();
	return f != s;
}

#endif
