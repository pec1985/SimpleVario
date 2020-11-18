
#include "Settings.h"
#include "SdFat/SdFat.h"
#include "Utils.h"

#define KEY_UNIT_SYSTEM     "UNIT_SYSTEM"
#define KEY_CLIMB_THRESHOLD "CLIMB_THRESHOLD"
#define KEY_SINK_THRESHOLD  "SINK_THRESHOLD"
#define KEY_BEEP_ON_START   "BEEP_ON_START"
#define KEY_SINK_ALARM_ON   "SINK_ALARM_ON"
#define KEY_TIMEZONE_UTC    "TIMEZONE_UTC"
#define KEY_PILOT_NAME      "PILOT_NAME"
#define KEY_GLIDER_TYPE     "GLIDER_TYPE"
#define KEY_SOUND_OFF       "SOUND_OFF"

Settings::Settings() { }
Settings::~Settings() { }

void Settings::begin(int menuButtonPin, int upButtonPin, int downButtonPin)
{
	m_buttonMenu.setPin(menuButtonPin);
	m_buttonUp.setPin(upButtonPin);
	m_buttonDown.setPin(downButtonPin);
}

void Settings::readSettings()
{
	if (!m_hasSdCard) return;
	SdFile settings;
	settings.open("SETTINGS.TXT", FILE_READ);
	if (!settings.isOpen()) {
		lcdPrint(m_lcd, "SETTINGS.TXT", "", true);
		lcdPrint(m_lcd, "   MISSING!", "", false);
		delay(2000);
		return;
	}

	SimpleArray<String> lines;
	String str;
	while (settings.available()) {
		char c = settings.read();
		if (c == '\n') {
			lines.push(str);
			str = "";
		} else {
			str += c;
		}
	}
	double climbThreshold = 0;
	double sinkThreshold = 0;

	for (int i = 0, len = lines.size(); i < len; i++) 
	{
		auto line = lines[i];
		if (line.startsWith("//")) continue;
		auto parts = StringSplit(line, '=');
		if (parts.size() == 2) {
			auto key = parts[0];
			auto value = parts[1].trim();
			if (key == KEY_UNIT_SYSTEM)
			{
				m_isMetricSystem = (value == "METRIC");
			}
			else if (key == KEY_CLIMB_THRESHOLD)
			{
				climbThreshold = value.toFloat();
			}
			else if (key == KEY_SINK_THRESHOLD)
			{
				sinkThreshold = value.toFloat();
			}
			else if (key == KEY_BEEP_ON_START)
			{
				m_beepsOnStart = (value == String("TRUE"));
			}
			else if (key == KEY_SINK_ALARM_ON)
			{
				m_sinkAlarmOn = (value == "TRUE");
			}
			else if (key == KEY_TIMEZONE_UTC)
			{
				m_timeZone = value.toInt();
		 	}
			else if (key == KEY_PILOT_NAME)
			{
				m_pilotName = value;
			}
			else if (key == KEY_GLIDER_TYPE)
			{
				m_gliderModel = value;
		 	} else if (key == KEY_SOUND_OFF) {
		 		m_soundOff = (value == "TRUE");
		 	}
		}
	}
	settings.close();

	if (climbThreshold > 0) {
		if (m_isMetricSystem) {
			m_climbThreshold = Measurement<UnitSpeed>(climbThreshold, UnitSpeed::metersPerSecond());
		} else {
			m_climbThreshold = Measurement<UnitSpeed>(climbThreshold, UnitSpeed::feetPerMinute());
		}
	}
	if (sinkThreshold < 0) {
		if (m_isMetricSystem) {
			m_sinkThreshold = Measurement<UnitSpeed>(sinkThreshold, UnitSpeed::metersPerSecond());
		} else {
			m_sinkThreshold = Measurement<UnitSpeed>(sinkThreshold, UnitSpeed::feetPerMinute());
		}
	}
}

void Settings::saveSettings()
{
	if (!m_hasSdCard) return;

	SdFile settings;
	settings.open("SETTINGS.TXT", O_WRITE | O_CREAT);
	settings.remove();
	settings.open("SETTINGS.TXT", O_WRITE | O_CREAT);

	if (m_isMetricSystem) {
		m_climbThreshold = m_climbThreshold.convertedTo(UnitSpeed::metersPerSecond());
		m_sinkThreshold = m_sinkThreshold.convertedTo(UnitSpeed::metersPerSecond());
	} else {
		m_climbThreshold = m_climbThreshold.convertedTo(UnitSpeed::feetPerMinute());
		m_sinkThreshold = m_sinkThreshold.convertedTo(UnitSpeed::feetPerMinute());
	}

	settings.println("------------------------------------------");
	settings.println(String(KEY_UNIT_SYSTEM) + String("=") + String(m_isMetricSystem ? "METRIC" : "IMPERIAL"));
	settings.println(String(KEY_CLIMB_THRESHOLD) + String("=") + String(m_climbThreshold.value()));
	settings.println(String(KEY_SINK_THRESHOLD) + String("=") + String(m_sinkThreshold.value()));
	settings.println(String(KEY_BEEP_ON_START) + String("=") + String(m_beepsOnStart ? "TRUE" : "FALSE"));
	settings.println(String(KEY_SINK_ALARM_ON) + String("=") + String(m_sinkAlarmOn ? "TRUE" : "FALSE"));
	settings.println(String(KEY_TIMEZONE_UTC) + String("=") + String(m_timeZone));
	settings.println(String(KEY_SOUND_OFF) + String("=") + String(m_soundOff ? "TRUE" : "FALSE"));
	settings.println("------------------------------------------");
	settings.println(String(KEY_PILOT_NAME) + String("=") + m_pilotName);
	settings.println(String(KEY_GLIDER_TYPE) + String("=") + m_gliderModel);
	settings.println("------------------------------------------");

	settings.close();
}

void Settings::printYesNo(bool yes)
{
	if (yes) {
		lcdPrint(m_lcd, "   NO   >YES<   ", "", false);
	} else {
		lcdPrint(m_lcd, "  >NO<   YES    ", "", false);
	}
}

void Settings::promtMainMenu() 
{
	// m_useGPSAlt = promtConfirmation("GPS altitude?", m_useGPSAlt);
	// if (m_useGPSAlt) {
	// 	m_altitude = Measurement<UnitLength>(m_gpsAlt, UnitLength::meters());
	// }
	promptAltitudeMenu();
	// climb
	promptThresholdMenu(true);
	// sink
	promptThresholdMenu(false);
	m_soundOff = promtConfirmation("Sound Off?", m_soundOff);
	saveSettings();
}

void Settings::promtSecondaryMenu()
{
	m_isMetricSystem = promtConfirmation("Metric system?", m_isMetricSystem);
	m_sinkAlarmOn = promtConfirmation("Sink Alarm?", m_sinkAlarmOn);
	m_beepsOnStart = promtConfirmation("Beep on start?", m_beepsOnStart);	
	saveSettings();
}

bool Settings::promtConfirmation(const String& title, bool def)
{
	bool result = def;
	lcdPrint(m_lcd, title, "", true);
	printYesNo(def);
	while (true) {
		auto upButton = upButtonPressed();
		auto downButton = downButtonPressed();
		if (upButton || downButton) {
			result = upButton;
			printYesNo(result);
			delay(250);
		} else if (menuButtonPressed()) {
			delay(250);
			return result;
		}
	}
	return result;
}


void Settings::promptAltitudeMenu() 
{
	m_altitude = m_isMetricSystem ? 
		m_altitude.convertedTo(UnitLength::meters()) : 
		m_altitude.convertedTo(UnitLength::feet());
	
	lcdPrint(m_lcd, "Current Alt:", "", true);
	lcdPrint(m_lcd, "", m_altitude.description(), false);
	while (true) 
	{
		auto upButton = m_buttonUp.isPressing();
		auto downButton = m_buttonDown.isPressing();
		if (upButton || downButton) 
		{
			m_altitude = m_isMetricSystem ? 
				m_altitude.convertedTo(UnitLength::meters()) : 
				m_altitude.convertedTo(UnitLength::feet());

			auto symbol = m_altitude.unit().symbol();
			auto result = startCounter(m_altitude.value(), downButton, false, symbol);
			
			m_altitude = m_isMetricSystem ?
				Measurement<UnitLength>(result, UnitLength::meters()) :
				Measurement<UnitLength>(result, UnitLength::feet());	

			lcdPrint(m_lcd, "", m_altitude.description(), false);

		} else if (menuButtonPressed()) {
			delay(250);
			return;
		}
	}
}
void Settings::promptThresholdMenu(bool climbThreshold)
{
	Measurement<UnitSpeed>& threshold = climbThreshold ? m_climbThreshold : m_sinkThreshold;
	threshold = m_isMetricSystem ? 
		threshold.convertedTo(UnitSpeed::metersPerSecond(), 0.02) : 
		threshold.convertedTo(UnitSpeed::feetPerMinute());

	lcdPrint(m_lcd, climbThreshold ? "Climb Threshold:" : "Sink Threshold:" , "", true);
	lcdPrint(m_lcd, "", threshold.description(), false);
	while (true) 
	{
		auto upButton = m_buttonUp.isPressing();
		auto downButton = m_buttonDown.isPressing();
		if (upButton || downButton) 
		{
			threshold = m_isMetricSystem ? 
				threshold.convertedTo(UnitSpeed::metersPerSecond(), 0.02) : 
				threshold.convertedTo(UnitSpeed::feetPerMinute());

			auto symbol = threshold.unit().symbol();
			auto result = startCounter(threshold.value(), downButton, m_isMetricSystem, symbol);

			threshold = m_isMetricSystem ?
				Measurement<UnitSpeed>(result, UnitSpeed::metersPerSecond(), 0.02) :
				Measurement<UnitSpeed>(result, UnitSpeed::feetPerMinute());	

		} else if (menuButtonPressed()) {
			delay(250);
			return;
		}
	}
}

bool Settings::countTo(int count, int num) {
	return count >= num && (count % num) == 0;
}

double Settings::startCounter(double startAt, bool decrement, bool isMetersPerSecond, String& symbol) 
{
	if (isMetersPerSecond) startAt /= 0.02;

	bool isPositive = startAt > 0;
	int count = 0;
	double result = 0.0;
	Button& btn = decrement ? m_buttonDown : m_buttonUp;
	while (btn.isPressing()) 
	{
		if (count < 5) {
			count ++;
			delay(250);
		} else 
		if (countTo(abs(count), 100)) {
			count += 100;
			delay(250);
		} else
		if (countTo(abs(count), 10)) {
			count += 10;
			delay(250);
		} else
		if (countTo(abs(count), 5)) {
			count += 5;
			delay(250);
		} else {
			count++;
			delay(250);		
		}
		if (decrement) {
			result = isMetersPerSecond ? startAt - double(count) : (double)(round(startAt) - double(count));
		} else {
			result = isMetersPerSecond ? startAt + double(count) : (double)(round(startAt) + double(count));
		}
		if (isMetersPerSecond) {
			lcdPrint(m_lcd, "", String(result * 0.02, 2) + symbol, false);
		} else {
			lcdPrint(m_lcd, "", String(result, 0) + symbol, false);
		}
	}
	return isMetersPerSecond ? (result * 0.02) : round(result);
}

