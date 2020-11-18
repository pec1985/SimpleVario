/**
 *	SimpleVario!!
 *	Copyright Pedro Enrique
 */

#ifndef SimpleGPS_h
#define SimpleGPS_h

#include <Arduino.h>

// #define P_TESTING
class SoftwareSerial;
class SimpleGPS {
 public:

	enum SimpleProps {
		kTimestamp = 0,
		kLatitude = 1,
		kLongitude = 2,
		kAltitude = 3,
		kSpeed = 4,
		kDate = 5,
		kMetersAltitude = 6,
		kAltitudeAccuracy = 7,
		kHeading = 8
	};
	SimpleGPS() {};
	SimpleGPS(SoftwareSerial* serial);
	void begin();
	bool update();
	inline bool fixed() const {
		return m_fixed;
	}	
	inline String stringFixed() const {
		return m_fixed ? "1" : "0";
	}
	inline String timestamp() const {
		return m_info[kTimestamp];
	}
	inline double altitude() const {
		return m_info[kMetersAltitude].toFloat();
	}
	inline double altitudeAccuracy() {
		return String(m_info[kAltitudeAccuracy]).toFloat();
	}
	inline double knots() const {
		return m_info[kSpeed].toFloat();
	}
	inline String date() {
		return m_info[kDate];
	}

	inline String stringAltitude() const {
		auto a = m_info[kMetersAltitude];
		return a.length() > 0 ? a : "0";
	}
	inline String stringAltitudeAccuracy() {
		auto a = m_info[kAltitudeAccuracy];
		return a.length() > 0 ? a : "0";
	}
	inline String stringKnots() const {
		auto a = m_info[kSpeed];
		return a.length() > 0 ? a : "0";
	}
	inline String stringLatitude() const {
		auto a = m_info[kLatitude];
		return a.length() > 0 ? a : "0";
	}
	inline String stringLongitude() const {
		auto a = m_info[kLongitude];
		return a.length() > 0 ? a : "0";
	}
	inline String stringHeading() const {
		auto a = m_info[kHeading];
		return a.length() > 0 ? a : "0";
	}
	String toIGC(double baroMeters);
 private:
	String toIGCMeters(String meters);

#ifdef P_TESTING
 	double m_timer {0};
#endif
	 SoftwareSerial* m_serial {NULL};
	 String m_info[11];
	 String m_sentance;
	 bool m_fixed;
	 void parse();
	 void resetValues();
 };
#endif
