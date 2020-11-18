
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "src/LiquidCrystal_I2C.h"
#include "src/SdFat/SdFat.h"
#include "src/SimpleGPS.h"
#include "src/MS5611.h"
#include "src/SimpleVario.h"
#include "src/IGCFileRecorder.h"
#include "src/Utils.h"
#include "src/Units/UnitSpeed.h"
#include "src/Units/UnitLength.h"
#include "src/Units/Measurement.h"
#include "src/Settings.h"

static LiquidCrystal_I2C m_lcd(0x3F, 16, 2);
static SoftwareSerial m_gpsSerial(0, 1);
static SimpleGPS m_gps(&m_gpsSerial);
static MS5611 m_ms5611;
static SimpleVario m_vario;
static IGCFileRecorder m_recorder;
static SdFat m_sd;
static Settings m_settings;

static double m_timeSinceGPS = 0;
static double m_lcd_timer = 0;
static bool m_useMetricSystem = true;
static bool m_hasTime = false;
static bool m_manualAltitude = false;
static bool m_hasSdCard = false;
static bool m_showFlighTime = true;
static bool m_showTotalDistance = false;

static void adjustAltitude();
static void updateLCD();
static void applySettings();
static void checkForSettings();
static void checkInflightOptions();
static void setClock();

void setup()
{
	m_gpsSerial.begin(9600);
	m_gps.begin();
	m_vario.begin(21);
	m_ms5611.begin();
	m_lcd.begin(16, 2);
	m_settings.begin(14, 16, 15);

	m_lcd.backlight();
	m_lcd.clear();
	lcdPrint(m_lcd, "SimpleVario v2.0", "", true); 

	while (m_gps.date() == "0" || m_gps.timestamp() == "000000")
	{
		m_gps.update();
	}

	if (!m_sd.begin(10))
	{
		m_lcd.clear();
		m_lcd.setCursor(0, 0);
		m_lcd.print("NO SD CARD");
		m_hasSdCard = false;
		delay(100);
	}
	else
	{
		m_hasSdCard = true;
	}
	m_settings.setLCD(m_lcd);
	m_settings.hasSdCard(m_hasSdCard);
	m_settings.readSettings();
	m_recorder.setPilotName(m_settings.pilotName());
	m_recorder.setGliderType(m_settings.gliderModel());
	applySettings();
	m_vario.initialBeep();

}

void loop() 
{
	if (m_recorder.showResults())
	{
		showResults();
	}
	// Allow user to change the default settings only when not flying
	if (!m_recorder.recording())
	{
		// check for settings button pressed
		checkForSettings();
		// reset altitude to gps altitude, if needed
		adjustAltitude();
		// set time to gps clock
		setClock();
	}
	else
	{
		// Limited button options while flying
		checkInflightOptions();
	}
	// update GPS
	m_gps.update();
	// update vario
	m_vario.update(m_ms5611.getPressure());
	// update file recorder
	m_recorder.update(m_gps, m_vario);
	// update LCD
	updateLCD();
}

static void showResults()
{
	Measurement<UnitLength> distance(m_recorder.travelledDistance(), UnitLength::kilometers());
	Measurement<UnitLength> altitude(m_recorder.highestAltitude(), UnitLength::meters());
	if (!m_useMetricSystem)
	{
		distance = distance.convertedTo(UnitLength::miles(), 0.25);
		altitude = altitude.convertedTo(UnitLength::feet());
	}
	auto totalTime = m_recorder.totalTime();
	lcdPrint(m_lcd, "Stats:", altitude.description(), true); 
	lcdPrint(m_lcd, totalTime, distance.description(), false);

	m_recorder.reset();
	m_showFlighTime = true;
	m_showTotalDistance = false;

	while (true) {
		if (m_settings.menuButtonPressed()) return;
		if (m_settings.upButtonPressed()) return;
		if (m_settings.downButtonPressed()) return;
		delay(20);
	}
}
static void applySettings()
{
	auto climbThreshold = m_settings.climbThreshold();
	auto sinkThreshold = m_settings.sinkThreshold();
	// Convert to metric system when setting the values to the vario
	m_vario.setClimbThreshold(climbThreshold.convertedTo(UnitSpeed::metersPerSecond()).value());
	m_vario.setSinkThreshold(sinkThreshold.convertedTo(UnitSpeed::metersPerSecond()).value());
	m_vario.setBeepsOnStart(m_settings.beepsOnStart());
	m_vario.setBeepsOnSink(m_settings.sinkAlarmOn());
	m_vario.setSilent(m_settings.soundOff());
	m_useMetricSystem = m_settings.isMetricSystem();
}

static void setClock()
{
	// If gps is not locked, return
	if (!m_gps.fixed()) return;
	// If time has been set already, return
	if (m_hasTime) return;

	auto time = m_gps.timestamp();
	auto date = m_gps.date();
	setTime(
		/* hour   */ time.substring(0, 2).toInt(),
		/* minute */ time.substring(2, 4).toInt(),
		/* second */ time.substring(4, 6).toInt(),
		/* day    */ date.substring(0, 2).toInt(),
		/* month  */ date.substring(2, 4).toInt(),
		/* year   */ date.substring(4, 6).toInt()
	);
	// Set UTC offset
	adjustTime(m_settings.timeZone() * SECS_PER_HOUR);
	m_recorder.setDate(date);
	m_hasTime = true;
	m_timeSinceGPS = millis();
}

static void checkForSettings() 
{
	auto now = millis();
	if (!m_settings.menuButtonPressed()) return;

	m_vario.forceStopBeep();
	if (millis() - now > 2000) 
	{
		// If over 2 seconds, get to the secondary menu
		m_settings.promtSecondaryMenu();
	}
	else 
	{
		// Else, go to the primary menu
		Measurement<UnitLength> currentAltitude(m_vario.altitude(), UnitLength::meters());
		m_settings.setAltitude(currentAltitude);
		m_settings.setGPSAlt(m_gps.altitude());
		m_settings.promtMainMenu();
		auto newAltitude = m_settings.altitude();
		if (newAltitude != currentAltitude)
		{
			// If the altitude has changed, toggle the manualAltitude var
			// and set the vario's new altitude
			auto meters = newAltitude.convertedTo(UnitLength::meters());
			m_vario.setAltitude(meters.value());
			m_manualAltitude = true;
		}
	}
	applySettings();
}

static void checkInflightOptions() 
{
	// Button 1 - Change display time
	if (m_settings.menuButtonPressed())
	{
		m_showFlighTime = !m_showFlighTime;
		return;
	}
	// Button 2 - Toggle sink alarm
	if (m_settings.downButtonPressed())
	{
		m_showTotalDistance = !m_showTotalDistance;
		return;
	}
	// Button 3 - Toggle sound
	if (m_settings.upButtonPressed())
	{
		bool flag = !m_vario.silent();
		m_vario.forceStopBeep();
		m_vario.setSilent(flag);
		lcdPrint(m_lcd, "Vario sound:", flag ? "OFF" : "ON ", true);
		delay(1000);
		return;
	}
}

static void adjustAltitude() 
{
	// If altitude has been set manually, return
	if (m_manualAltitude) return;
	// If altitude has not been set and 5 minutes have ellapsed...
	if (millis() - m_timeSinceGPS < 300000) return;
	m_vario.setAltitude(m_gps.altitude());
	m_timeSinceGPS = millis();
}

static void updateLCD()
{
	// Update the screen once every 1/4 of a second
	if ((millis() - m_lcd_timer) < 250) return;

	String timeString;
	String speedString;
	String climbRateString;
	String altitudeString;

	Measurement<UnitSpeed> speed(m_gps.knots(), UnitSpeed::knots());
	Measurement<UnitSpeed> climbRate(m_vario.climbRate(), UnitSpeed::metersPerSecond());
	Measurement<UnitLength> rawAltitude(m_vario.altitude(), UnitLength::meters());

	if (m_recorder.recording())
	{
		timeString = m_showFlighTime ? m_recorder.totalTime() : readableTime();
	}
	else
	{
		timeString = m_gps.fixed() ? readableTime() : String("NO GPS");	
	}

	if (m_useMetricSystem)
	{
		speedString = speed.convertedTo(UnitSpeed::kilometersPerHour()).description();
		climbRateString = climbRate.convertedTo(UnitSpeed::metersPerSecond(), 0.02).description();
		altitudeString = rawAltitude.convertedTo(UnitLength::meters()).description();
	}
	else
	{
		speedString = speed.convertedTo(UnitSpeed::milesPerHour()).description();
		climbRateString = climbRate.convertedTo(UnitSpeed::feetPerMinute(), 25).description();
		altitudeString = rawAltitude.convertedTo(UnitLength::feet()).description();
	}
	lcdPrint(m_lcd, timeString, altitudeString, true);
	if (m_showTotalDistance) {
		Measurement<UnitLength> distance(m_recorder.travelledDistance(), UnitLength::kilometers());
		if (!m_useMetricSystem) {
			distance = distance.convertedTo(UnitLength::miles(), 0.25);
		}
		lcdPrint(m_lcd, distance.description(), climbRateString, false);
	} else {
		lcdPrint(m_lcd, speedString, climbRateString, false);
	}

	m_lcd_timer = millis();
}
