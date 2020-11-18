/**
 *	SimpleVario!!
 *	Copyright Pedro Enrique
 */

#include "SimpleGPS.h"
#include "SimpleArray.h"
#include "SoftwareSerial.h"
#include "Utils.h"

static String GPGGA = "$GPGGA";
static String GPGSA = "$GPGSA";
static String GPRMC = "$GPRMC";

#define P_GPS_TIME 1

#define P_GPS_LAT_GGA_NUM 2
#define P_GPS_LAT_GGA_CHR 3
#define P_GPS_LON_GGA_NUM 4
#define P_GPS_LON_GGA_CHR 5

#define P_GPS_LAT_RMC_NUM 3
#define P_GPS_LAT_RMC_CHR 4
#define P_GPS_LON_RMC_NUM 5
#define P_GPS_LON_RMC_CHR 6

#define P_GPS_FIX 6
#define P_GPS_SATS 7
#define P_GPS_ALT 9
#define P_GPS_DATE 9
#define P_GPS_SPEED 7
#define P_GPS_HEADING 8
#define P_GPS_ALT_ACC 17

SimpleGPS::SimpleGPS(SoftwareSerial* serial) {
	m_serial = serial;
	m_fixed = "0";
}

void SimpleGPS::begin()
{
	// 0 NMEA_SEN_GLL, // GPGLL interval - Geographic Position - Latitude longitude
	// 1 NMEA_SEN_RMC, // GPRMC interval - Recommended Minimum Specific GNSS Sentence
	// 2 NMEA_SEN_VTG, // GPVTG interval - Course over Ground and Ground Speed
	// 3 NMEA_SEN_GGA, // GPGGA interval - GPS Fix Data
	// 4 NMEA_SEN_GSA, // GPGSA interval - GNSS DOPS and Active Satellites
	// 5 NMEA_SEN_GSV, // GPGSV interval - GNSS Satellites in View
	// 18 NMEA_SEN_MCHN, // PMTKCHN interval – GPS channel status 
	delay(100);
	m_serial->println("$PMTK314,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29");
	// $PMTK220,100*2F //Will set the GPS to 10hz (or updates every 100 milliseconds)
	// $PMTK220,250*29 //Will set the GPS to 4hz (or updates every 250 milliseconds)
	// $PMTK220,1000*1F //Will set the GPS to 1hz (updates every 1000 milliseconds)
	delay(100);
	m_serial->println("$PMTK220,250*29");
	m_info[kTimestamp] = "000000";
	m_info[kDate] = "0";
	resetValues();
}

void SimpleGPS::resetValues()
{
	m_info[kAltitude] = "00000";
	m_info[kMetersAltitude] = "0.0";
	m_info[kAltitudeAccuracy] = "1000";
	m_info[kLatitude] = "0000000N";
	m_info[kLongitude] = "00000000W";
	m_info[kHeading] = "0.0";
	m_info[kSpeed] = "0.0";
}

bool SimpleGPS::update()
{
	auto available = false;
	while (m_serial->available()) 
	{
		available = true;
		char c = m_serial->read();
		if (c == '$') {
			parse();
			m_sentance = "$";
		} else {
			m_sentance += c;
		}
#ifdef P_TESTING
		if ((millis() - m_timer) >= 1000) {
			m_timer = millis();
			Serial.println("--------------------------");
			Serial.println("Fixed				:" + String(m_fixed ? "true" : "false"));
			Serial.println("kTimestamp			:" + m_info[kTimestamp]);
			Serial.println("kLatitude			:" + m_info[kLatitude]);
			Serial.println("kLongitude			:" + m_info[kLongitude]);
			Serial.println("kAltitude			:" + m_info[kAltitude]);
			Serial.println("kSpeed				:" + m_info[kSpeed]);
			Serial.println("kDate				:" + m_info[kDate]);
			Serial.println("kMetersAltitude			:" + m_info[kMetersAltitude]);
			Serial.println("kAltitudeAccuracy		:" + m_info[kAltitudeAccuracy]);
			Serial.println("==========================");
		}
#endif
	}
	return available;
}

void SimpleGPS::parse() {
	if (m_sentance.length() < 7) {
		return;
	}

	/**
	 *  $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
	 * 
	 * Where:
	 * [1]     123519       Fix taken at 12:35:19 UTC
	 * [2,3]   4807.038,N   Latitude 48 deg 07.038' N
	 * [4,5]   01131.000,E  Longitude 11 deg 31.000' E
	 * [6]     1            Fix quality: 0 = invalid
	 *                                1 = GPS fix (SPS)
	 *                                2 = DGPS fix
	 *                                3 = PPS fix
	 * [7]     08           Number of satellites being tracked
	 * [8]     0.9          Horizontal dilution of position
	 * [9,10]  545.4,M      Altitude, Meters, above mean sea level
	 * [11,12] 46.9,M       Height of geoid (mean sea level) above WGS84 ellipsoid
	 * [12]    (empty field) time in seconds since last DGPS update
	 * [13]    (empty field) DGPS station ID number
	 * [14]    *47          the checksum data, always begins with *
	 **/

	auto first = m_sentance.substring(0, 6);
	if (first == GPGGA) {
		auto parts = StringSplit(m_sentance, ',');
		{
#ifdef P_TESTING
			{
				Serial.println("Number of satelites: " + parts[7]);
			}
#endif
			auto tmp = parts[P_GPS_TIME];
			if (tmp.length() > 5) {
				m_info[kTimestamp] = tmp.substring(0, 6);
			}
		}
		{
			auto tmp = parts[P_GPS_LAT_GGA_NUM];
			if (tmp.length() > 7) {
				int i = tmp.indexOf('.');
				m_info[kLatitude] = tmp.substring(0, 8);
				m_info[kLatitude].remove(i, 1);
				m_info[kLatitude] += parts[P_GPS_LAT_GGA_CHR];

			}
		}
		{
			auto tmp = parts[P_GPS_LON_GGA_NUM];
			if (tmp.length() > 8) {
				int i = tmp.indexOf('.');
				m_info[kLongitude] = tmp.substring(0, 9);
				m_info[kLongitude].remove(i, 1);
				m_info[kLongitude] += parts[P_GPS_LON_GGA_CHR];
			}
		}
		{
			m_fixed = parts[P_GPS_FIX] != "0";
			if (!m_fixed) {
				resetValues();
			}
		}
		{
			auto m = parts[P_GPS_ALT];
			if (m.length() == 0) {
				m = "0.0";
			} 
			m_info[kMetersAltitude] = m;
			m_info[kAltitude] = toIGCMeters(m_info[kMetersAltitude]);
		}
		return;
	}
	/**
	 * $GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39
	 * 
	 * Where:
	 * [1]     A        Auto selection of 2D or 3D fix (M = manual) 
	 * [2]     3        3D fix - values include: 1 = no fix
	 *                                          2 = 2D fix
	 *                                          3 = 3D fix
	 * [3-14]  04,05... PRNs of satellites used for fix (space for 12) 
	 * [15]    2.5      PDOP (dilution of precision) 
	 * [16]    1.3      Horizontal dilution of precision (HDOP) 
	 * [17]    2.1      Vertical dilution of precision (VDOP)
	 * [18]    *39      the checksum data, always begins with *
	 **/
	if (first == GPGSA) {
		auto parts = StringSplit(m_sentance, ',');
		if (parts.size() >= P_GPS_ALT_ACC) {
			auto acc = StringSplit(parts[P_GPS_ALT_ACC], '*')[0];
			if (acc.length() == 0 ) {
				acc = "999";
			}
			m_info[kAltitudeAccuracy] = acc;

		}
		return;
	} 
	/**
	 * $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
	 * 
	 * Where:
	 * [1]    123519       Fix taken at 12:35:19 UTC
	 * [2]    A            Status A=active or V=Void.
	 * [3,4]  4807.038,N   Latitude 48 deg 07.038' N
	 * [5,6]  01131.000,E  Longitude 11 deg 31.000' E
	 * [7]    022.4        Speed over the ground in knots
	 * [8]    084.4        Track angle in degrees True
	 * [9]    230394       Date - 23rd of March 1994
	 * [10]   003.1,W      Magnetic Variation
	 * [11]   *6A          The checksum data, always begins with *
	 **/
	if (first == GPRMC) {
		auto parts = StringSplit(m_sentance, ',');
		auto size = parts.size();
		// Commenting for now
		// ------------------------------------------
		// if (size >= P_GPS_LAT_RMC_CHR) 
		// {
		// 	auto tmp = parts[P_GPS_LAT_RMC_NUM];
		// 	if (tmp.length() > 7) {
		// 		int i = tmp.indexOf('.');
		// 		m_info[kLatitude] = tmp.substring(0, 8);
		// 		m_info[kLatitude].remove(i, 1);
		// 		m_info[kLatitude] += parts[P_GPS_LAT_RMC_CHR];
		// 	}
		// }
		// if (size >= P_GPS_LON_RMC_CHR)
		// {
		// 	auto tmp = parts[P_GPS_LON_RMC_NUM];
		// 	if (tmp.length() > 8) {
		// 		int i = tmp.indexOf('.');
		// 		m_info[kLongitude] = tmp.substring(0, 9);
		// 		m_info[kLongitude].remove(i, 1);
		// 		m_info[kLongitude] += parts[P_GPS_LON_RMC_CHR];
		// 	}
		// }
		// ------------------------------------------
		if (size >= P_GPS_SPEED) 
		{
			m_info[kSpeed] = parts[P_GPS_SPEED];
		}
		if (size >= P_GPS_DATE) 
		{
			m_info[kDate] = parts[P_GPS_DATE];
		}
		if (size >= P_GPS_HEADING) 
		{
			m_info[kHeading] = parts[P_GPS_HEADING];
		}
		return;
	} 
}

String SimpleGPS::toIGC(double baroMeters) {
	auto alt_baro = toIGCMeters(String(baroMeters));
	auto alt_gps = toIGCMeters(m_info[kMetersAltitude]);
	String r("B" + timestamp() + m_info[kLatitude] + m_info[kLongitude] + "A" + alt_baro + alt_gps);
	if (r.length() != 35) {
		return String("BAD - ") + r;
	}
	return r;
}
String SimpleGPS::toIGCMeters(String meters)
{
	if (meters.indexOf('.') != -1) {
		meters.remove(meters.indexOf('.'));
	}
	auto diff = 5 - meters.length();
	String zeros("");
	for (unsigned int i = 0; i <diff; i++) {
		zeros += "0";
	}
	return zeros + meters;
}
