#ifndef SETTINGS_H
#define SETTINGS_H

#include "Arduino.h"
#include "Units/UnitSpeed.h"
#include "Units/UnitLength.h"
#include "Units/Measurement.h"
#include "LiquidCrystal_I2C.h"
#include "Button.h"

class Settings
{
public:
	Settings();
	~Settings();
	void begin(int menuButtonPin, int upButtonPin, int downButtonPin);
	void readSettings();
	void promtMainMenu();
	void promtSecondaryMenu();
	bool menuButtonPressed() {
		return m_buttonMenu.pressed();
	}
	bool upButtonPressed() {
		return m_buttonUp.pressed();
	}
	bool downButtonPressed() {
		return m_buttonDown.pressed();
	}
	void hasSdCard(bool _hasSdCard) {
		m_hasSdCard = _hasSdCard;
	}
	const bool isMetricSystem() { return m_isMetricSystem; }
	const bool beepsOnStart() { return m_beepsOnStart; }
	const bool sinkAlarmOn() { return m_sinkAlarmOn; }
	const int timeZone() { return  m_timeZone; }
	const bool soundOff() { return m_soundOff; }
	String pilotName() const { return m_pilotName; }
	const String gliderModel() { return m_gliderModel; }
	const Measurement<UnitSpeed> climbThreshold() { return m_climbThreshold; }
	const Measurement<UnitSpeed> sinkThreshold() { return m_sinkThreshold; }
	const Measurement<UnitLength> altitude() { return m_altitude; }
	void setAltitude(Measurement<UnitLength>& altitude) { m_altitude = altitude; }
	void setLCD(LiquidCrystal_I2C& lcd) { m_lcd = lcd; }
	void setGPSAlt(double meters) { m_gpsAlt = meters; }
	void saveSettings();
private:
	bool countTo(int count, int num);
	double startCounter(double startAt, bool decrement, bool isMetersPerSecond, String& symbol);
	void promptThresholdMenu(bool climbThreshold);
	void promptGPSAltitudeMenu();
	void promptAltitudeMenu();
	bool promtConfirmation(const String& title, bool def);
	void printYesNo(bool yes);
	LiquidCrystal_I2C m_lcd { LiquidCrystal_I2C(0,0,0) };
	Button m_buttonMenu;
	Button m_buttonUp;
	Button m_buttonDown;

	bool m_isMetricSystem { false };
	Measurement<UnitSpeed> m_climbThreshold { 
		Measurement<UnitSpeed>(20, UnitSpeed::feetPerMinute())
	};
	Measurement<UnitSpeed> m_sinkThreshold { 
		Measurement<UnitSpeed>(-250, UnitSpeed::feetPerMinute()) 
	};
	Measurement<UnitLength> m_altitude {
		Measurement<UnitLength>(0, UnitLength::feet())
	};
	bool m_useGPSAlt { 
		false 
	};
	bool m_beepsOnStart { 
		true 
	};
	bool m_sinkAlarmOn { 
		true
	};
	bool m_soundOff { 
		false
	};
	int m_timeZone {
		-7
	};
	double m_gpsAlt {
		0.0
	};
	bool m_hasSdCard {
		false
	};

	String m_pilotName {
		"Pedro Enrique"
	};
	String m_gliderModel {
		"Wills Wing - Sport 2 136"
	};
};

#endif



