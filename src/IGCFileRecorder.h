/**
 *	SimpleVario!!
 *	Copyright Pedro Enrique
 */

#ifndef IGCFileRecorder_hpp
#define IGCFileRecorder_hpp

#include <Arduino.h>
#include "SimpleArray.h"

class SimpleGPS;
class LiquidCrystal_I2C;
class SimpleVario;
class IGCFileRecorder
{
public:
	IGCFileRecorder();
	void update(SimpleGPS& gpsInfo, SimpleVario& vario);
	void setPilotName(String name) {
		m_pilotName = name;
	}
	void setGliderType(String glider) {
		m_gliderType = glider;
	}
	void setDate(String date) {
		m_date = date;
	}
	void setUtc(int utc) {
		m_utc = utc;
	}
	bool recording() {
		return m_recording;
	}
	bool showResults() {
		return m_showResults;
	}
	double travelledDistance();
	int lineCount() {
		return m_lineCount;
	}
	double highestAltitude() {
		return m_highestAltitude;
	}
	String totalTime();
	void reset();

private:
	String createHeader();
	String createFileName();
	void startRecording();
	void stopRecording();
	double convertToDecimal(const String& point);
	double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d);
	struct queue_item {
		String sentance;
		int speed;
	};

	String m_firstSentance {""};

	SimpleArray<queue_item> m_queue;
	double m_write_timer { 0 };
	double m_bufferTime { 10 };
	double m_minSpeed { 5 };
	double m_highestAltitude {0.0};

	bool m_recording { false };
	bool m_showResults { false };
	int m_lineCount { 0 };
	int m_utc { 0 };
	String m_date { "000000" };
	String m_pilotName { "Pedro Enrique" };
	String m_gliderType { "Wills Wing Sport 2" };
	String m_currentFile;

};

#endif /* IGCFileRecorder_hpp */
