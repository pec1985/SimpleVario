/**
 *	SimpleVario!!
 *	Copyright Pedro Enrique
 */

#include <SPI.h>
#include "IGCFileRecorder.h"
#include "SdFat/SdFat.h"
#include "SimpleGPS.h"
#include "TimeLib.h"
#include "LiquidCrystal_I2C.h"
#include "SimpleVario.h"
#include "Utils.h"
#include "Units/Measurement.h"
#include "Units/UnitDuration.h"

#define earthRadiusKm 6371.0
#define deg2rad(DEG) (DEG * M_PI / 180)

static void dateTimeCallback(uint16_t* date, uint16_t* time) 
{
	*date = FAT_DATE(year(), month(), day());
	*time = FAT_TIME(hour(), minute(), second());
}

IGCFileRecorder::IGCFileRecorder() 
{
	SdFile::dateTimeCallback(dateTimeCallback);
}

void IGCFileRecorder::update(SimpleGPS &gpsInfo, SimpleVario& vario)
{
	double now = millis();
	if ((now - m_write_timer) < 1000) return;
	m_write_timer = now;
	if (!gpsInfo.fixed()) return;

	queue_item item {
		/* sentance */ gpsInfo.toIGC(vario.altitude()),
		/* speed    */ (int)round(gpsInfo.knots())
	};
	m_queue.push(item);
	if (m_queue.size() < m_bufferTime) return;
	if (m_queue.size() > m_bufferTime) m_queue.shift();

	auto speedSum = 0;
	for (auto i = 0; i < m_bufferTime; i++)
	{
		speedSum += m_queue[i].speed;
	}
	auto average = speedSum / m_bufferTime;

	// end recording
	if (average < m_minSpeed)
	{
		if (m_recording == true)
		{
			stopRecording();
		}
		return;
	}

	// start recording
	if (m_recording == false)
	{
		startRecording();
		return;
	}

	SdFile igcFile;
	igcFile.open(m_currentFile.c_str(), FILE_WRITE);
	igcFile.println(item.sentance);
	igcFile.close();
	m_lineCount++;
	m_highestAltitude = max(m_highestAltitude, vario.altitude());

}

String IGCFileRecorder::createHeader()
{
	String header;
	header += String("APEC002 SimpleVario\r\n");
	header += String("HFDTE" + m_date + "\r\n");
	header += String("HFPLTPILOT:" + m_pilotName + "\r\n");
	header += String("HFGTYGLIDERTYPE:" + m_gliderType);
	return header;
}

String IGCFileRecorder::createFileName()
{
	return
		 String(year()) + "-" +
		iString(month()) + "-" +
		iString(day()) + "_" + 
		iString(hour()) + "." + 
		iString(minute()) + "." + 
		iString(second()) + ".igc"; 
}

String IGCFileRecorder::totalTime()
{
	Measurement<UnitDuration> totalTime(m_lineCount, UnitDuration::seconds());
	auto hours = (int)totalTime.convertedTo(UnitDuration::hours()).value() % 60;
	auto minutes = (int)totalTime.convertedTo(UnitDuration::minutes()).value() % 60;
	auto seconds = (int)totalTime.value() % 60;
	return iString(hours) + ":" + iString(minutes) + ":" +iString(seconds);
}

void IGCFileRecorder::reset() {
	m_lineCount = 0;
	m_highestAltitude = 0;
	m_currentFile = "";
	m_lineCount = 0;
	m_showResults = false;
}

void IGCFileRecorder::stopRecording()
{
	m_recording = false;
	m_showResults = true;
}

void IGCFileRecorder::startRecording()
{
	m_recording = true;
	m_currentFile = createFileName();
	m_firstSentance = m_queue.first().sentance;

	SdFile igcFile;
	igcFile.open(m_currentFile.c_str(), FILE_WRITE);
	igcFile.println(createHeader());
	auto insert = false;
	for (auto i = 0; i < m_bufferTime; i++)
	{
		auto item = m_queue[i];
		if (insert == false && item.speed > 0)
		{
			insert = true;
			if (i == 0) {
				m_lineCount++;
				igcFile.println(item.sentance);
				continue;
			}
			m_lineCount++;
			igcFile.println(m_queue[i-1].sentance);
		}
		if (insert)
		{
			m_lineCount++;
			igcFile.println(item.sentance);
		}
	}
	igcFile.close();
}

double IGCFileRecorder::travelledDistance() {

	auto lat1 = convertToDecimal(m_firstSentance.substring(7, 7+8));
	auto lon1 = convertToDecimal(m_firstSentance.substring(15, 15+9));

	auto lat2 = convertToDecimal(m_queue.last().sentance.substring(7, 7+8));
	auto lon2 = convertToDecimal(m_queue.last().sentance.substring(15, 15+9));

	return distanceEarth(lat1, lon1, lat2, lon2);
}

double IGCFileRecorder::convertToDecimal(const String& point) {
	if (point == "0") return 0.0;
	double multiplier = ((point.indexOf("S") > -1) || (point.indexOf("W") > 1)) ? -1.0 : 1.0;
	double degrees;
	double minutes;
	// Latitude, 8 chars: 3718157N
    if (point.length() == 8) {
    	// 37
        degrees = point.substring(0, 0+2).toFloat();
        // 18 + (157 / 1000)
        minutes = point.substring(2, 2+2).toFloat() + (point.substring(4, 4+3).toFloat() / 1000);
    } 
    // Longitude: 12153538W
    else {
    	// 121
        degrees = point.substring(0, 0+3).toFloat();
        // 53 + (538 / 1000)
        minutes = point.substring(3, 3+2).toFloat() + (point.substring(5, 5+3).toFloat() / 1000);
    }
    return (degrees + (minutes / 60)) * multiplier;
}

/**
 * Returns the distance between two points on the Earth.
 * Direct translation from http://en.wikipedia.org/wiki/Haversine_formula
 * @param lat1d Latitude of the first point in degrees
 * @param lon1d Longitude of the first point in degrees
 * @param lat2d Latitude of the second point in degrees
 * @param lon2d Longitude of the second point in degrees
 * @return The distance between the two points in kilometers
 */
double IGCFileRecorder::distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d) {
	double lat1r = deg2rad(lat1d);
	double lon1r = deg2rad(lon1d);
	double lat2r = deg2rad(lat2d);
	double lon2r = deg2rad(lon2d);
	double u = sin((lat2r - lat1r)/2);
	double v = sin((lon2r - lon1r)/2);
	return 2.0 * earthRadiusKm * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}

