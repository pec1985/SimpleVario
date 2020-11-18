/**
 *	SimpleVario!!
 *	Copyright Pedro Enrique
 */

#include <Arduino.h>
#include "SimpleVario.h"

#include <stdlib.h>
#include <math.h>

#ifndef DBL_MIN
#define DBL_MIN 2.2250738585072014e-308
#endif

#ifndef DBL_MAX
#define DBL_MAX 1.7976931348623157e+308
#endif

#define PE_POS_INFINITY 0x7ff0000000000000L
#define PE_NEG_INFINITY 0xfff0000000000000L
#define STANDARD_SEA_LEVEL_PRESSURE 101325.0


// Utility methods for "tone" and noTone" to avoid chirping
// ========================================================
static bool quiet = false;
static void NoTone(uint8_t pin) {
	if (quiet) return;
	quiet = true;
	noTone(pin);
	pinMode(pin, LOW);
}
 
static void Tone(uint8_t pin, unsigned int frequency, unsigned long duration = 0) {
	if (duration == 0) {
		tone(pin, frequency, duration);
	} else {
		tone(pin, frequency);
	}
	quiet = false;
}
// ========================================================

class KalmanFilter
{
public:
	KalmanFilter() {
		x_abs_ = 0.0;
		x_vel_ = 0.0;
		p_abs_abs_ = 0.0;
		p_abs_vel_ = 0.0;
		p_vel_vel_ = 0.0;
		var_accel_ = 1.0;
		reset(0.0, 0.0);
	}
	KalmanFilter(double var_accel) {
		x_abs_ = 0.0;
		x_vel_ = 0.0;
		p_abs_abs_ = 0.0;
		p_abs_vel_ = 0.0;
		p_vel_vel_ = 0.0;
		var_accel_ = var_accel;
		reset(0.0, 0.0);
	}
	void reset(double abs_value, double vel_value) {
		x_abs_ = abs_value;
		x_vel_ = vel_value;
		p_abs_abs_ = 1.e10;
		p_abs_vel_ = 0.0;
		p_vel_vel_ = var_accel_;
	}

	// Updates state given a sensor measurement of the absolute value of x,
	// the variance of that measurement, and the interval since the last
	// measurement in seconds. This interval must be greater than 0; for the
	// first measurement after a reset(), it's safe to use 1.0.
	void update(double z_abs, double var_z_abs, double dt) {
		// Note: math is not optimized by hand. Let the compiler sort it out.
		// Predict step.
		// Update state estimate.
		x_abs_ += x_vel_ * dt;
		// Update state covariance. The last term mixes in acceleration noise.
		p_abs_abs_ += 2.*dt*p_abs_vel_ + dt*dt*p_vel_vel_ + var_accel_*dt*dt*dt*dt/4.;
		p_abs_vel_ +=					   dt*p_vel_vel_ + var_accel_*dt*dt*dt/2.;
		p_vel_vel_ +=									 + var_accel_*dt*dt;

		// Update step.
		double y = z_abs - x_abs_;  // Innovation.
		double s_inv = 1. / (p_abs_abs_ + var_z_abs);  // Innovation precision.
		double k_abs = p_abs_abs_*s_inv;  // Kalman gain
		double k_vel = p_abs_vel_*s_inv;
		// Update state estimate.
		x_abs_ += k_abs * y;
		x_vel_ += k_vel * y;
		// Update state covariance.
		p_vel_vel_ -= p_abs_vel_*k_vel;
		p_abs_vel_ -= p_abs_vel_*k_abs;
		p_abs_abs_ -= p_abs_abs_*k_abs;
	}
	// The absolute value of x.
	inline double getXAbs() const {
		return x_abs_;
	}
	// The rate of change of x.
	inline double getXVel() const {
		return x_vel_;
	}

private:
	double x_abs_;  // The absolute value of x.
	double x_vel_;  // The rate of change of x.

	// Covariance matrix for the state.
	double p_abs_abs_;
	double p_abs_vel_;
	double p_vel_vel_;

	// The variance of the acceleration noise input in the system model.
	double var_accel_;

};

class Altimeter
{
public:
	Altimeter() {
		setSealevelPressure(STANDARD_SEA_LEVEL_PRESSURE);
		m_kalmanFilter = new KalmanFilter(1.0);
		m_positionNoise = 0.2;
		m_altDamp = 0.05;
	}
	~Altimeter() {
		delete m_kalmanFilter;
	}
	void setSealevelPressure(double pressure) {
		m_rawPressure = pressure;
		m_seaLevelPressure = pressure;
		m_dampedAltStarted = false;
	}
	void addPressure(double pressure, double time) {
		m_rawPressure = pressure;
		m_rawAltitude = 44330.0 * (1.0 - pow((m_rawPressure / m_seaLevelPressure), 0.190295));
		if (m_dampedAltStarted) {
			m_dampedAltitude = m_dampedAltitude + m_altDamp * (m_rawAltitude - m_dampedAltitude);
		} else {
			m_dampedAltitude = m_rawAltitude;
			m_dampedAltStarted = true;
		}
		m_kalmanFilter->update(m_dampedAltitude, m_positionNoise, time);
	}
	void setAltitude(double alt) {
		m_dampedAltitude = alt;
		m_dampedAltStarted = true;
		m_seaLevelPressure = m_rawPressure / pow(1.0 - (m_dampedAltitude / 44330.0), 5.255);;
	}
	double altitude() {
		return m_kalmanFilter->getXAbs();
	}
	double varioValue() {
		return m_kalmanFilter->getXVel();
	}
private:
	double m_seaLevelPressure;
	double m_rawPressure;
	double m_rawAltitude;
	double m_positionNoise;
	double m_altDamp;
	double m_dampedAltitude;
	KalmanFilter* m_kalmanFilter;
	bool m_dampedAltStarted;
};

struct Point {double x; double y;};
class PiecewiseLinearFunction
{
public:
	PiecewiseLinearFunction(int capacity) {
		m_points = (Point*)malloc(sizeof(Point)*capacity);
		m_posInfValue = PE_POS_INFINITY;
		m_negInfValue = PE_NEG_INFINITY;
		m_pointsCount = 0;
	}
	~PiecewiseLinearFunction() {
		free(m_points);
	}
	void addPoint(const Point &point) {
		if (m_pointsCount == 0) {
			m_points[m_pointsCount] = point;
		}
		else if (point.x > (m_points[m_pointsCount - 1].x)) {
			m_points[m_pointsCount] = point;
		} else {
			for (int i = 0; i < m_pointsCount; i++) {
				if (m_points[i].x > point.x) {
					m_points[i] =  point;
					return;
				}
			}
		}
		m_pointsCount++;
	}
	double getValue(double x) {
		if (x == PE_POS_INFINITY) {
			return m_posInfValue;
		} else if (x == PE_NEG_INFINITY) {
			return m_negInfValue;
		} else if (m_pointsCount == 1) {
			return m_points[0].y;
		} else {
			Point point;
			Point lastPoint = m_points[0];
			if (x <= lastPoint.x) {
				return lastPoint.y;
			}
			for (int i = 1; i < m_pointsCount; i++) {
				point = m_points[i];
				if (x <= point.x) {
					double ratio = (x - lastPoint.x) / (point.x - lastPoint.x);
					return lastPoint.y + ratio * (point.y - lastPoint.y);
				}
				lastPoint = point;
			}
			return lastPoint.y;
		}

	}
private:
	int m_pointsCount;
	Point* m_points;
	double m_posInfValue;
	double m_negInfValue;

};

class BeeperController
{
public:
	BeeperController() {
		m_sinking = false;
		m_beeping = false;
		m_climbing = false;
		m_var = 0.0;
		m_base = 1000.0;
		m_increment = 100.0;
		m_sinkBase = 500.0;
		m_sinkIncrement = 100.0;
		m_climbThreshold = 0.1; // 20 FPM
		m_sinkThreshold = -2.0; // -400 FPM
		m_piecewise = new PiecewiseLinearFunction(6);
		m_piecewise->addPoint({0.135, 0.4755});
		m_piecewise->addPoint({0.441, 0.3619});
		m_piecewise->addPoint({1.029, 0.2238});
		m_piecewise->addPoint({1.559, 0.1565});
		m_piecewise->addPoint({2.471, 0.0985});
		m_piecewise->addPoint({3.571, 0.0741});
	}
	~BeeperController() {
		if (m_piecewise != NULL) delete m_piecewise;
	}
	double rateFromLiftTone() {
		double hZ = m_base + m_increment * m_var;
		double rate = hZ / 1000.0;
		if (rate < 0.5) {
			rate = 0.5;
		} else if (rate > 2.0) {
			rate = 2.0;
		} else if (rate == 1.0) {
			rate = 1.0 + DBL_MIN;
		}
		return rate * 1000.0;
	}
	double rateFromSinkTone() {
		double hZ = m_sinkBase + m_sinkIncrement * m_var;
		double rate = hZ / 500.0;
		if (rate < 0.5) {
			rate = 0.5;
		} else if (rate > 2.0) {
			rate = 2.0;
		} else if (rate == 1.0f) {
			rate = 1.0 + DBL_MAX;
		}
		return rate * 500.0;
	}
	void addValue(double newValue) {
		m_var = newValue;
		if (newValue > m_climbThreshold) {
			m_climbing = true;
			m_sinking = false;
			m_beeping = true;
			return;
		}
		if (newValue < m_climbThreshold && newValue > m_sinkThreshold) {
			m_beeping = false;
			m_climbing = false;
			m_sinking = false;
			return;
		}
		if (newValue < m_sinkThreshold) {
			m_climbing = false;
			m_sinking = true;
			m_beeping = true;
			return;
		}
	}
	double duration() {
		return m_piecewise->getValue(m_var) * 1200.0;
	}
	inline bool sinking() {
		return m_sinking;
	}
	inline bool climbing() {
		return m_climbing;
	}
	inline bool beeping() {
		return m_beeping;
	}
	inline void setClimbThreshold(double value) {
		m_climbThreshold = value;
	}
	inline void setSinkThreshold(double value) {
		m_sinkThreshold = value;
	}
	inline double climbThreshold() {
		return m_climbThreshold;
	}
	inline double sinkThreshold() {
		return m_sinkThreshold;
	}
private:
	PiecewiseLinearFunction* m_piecewise;
	double m_climbThreshold;
	double m_sinkThreshold;
	double m_var;
	bool m_climbing;
	bool m_sinking;
	bool m_beeping;
	double m_increment;
	double m_base;
	double m_sinkBase;
	double m_sinkIncrement;
};

SimpleVario::SimpleVario() {

}

SimpleVario::~SimpleVario() {
	if (m_altimeter != NULL) delete m_altimeter;
	if (m_beeperController != NULL) delete m_beeperController;
}
void SimpleVario::begin(int pinBuzzer)
{
	m_pinBuzzer = pinBuzzer;
	m_time = millis();
	m_altimeter = new Altimeter();
	m_beeperController = new BeeperController();
}

void SimpleVario::initialBeep()
{
	if (m_silent) return;
	if (!m_beepsOnStart) return;
	for (int i = 0; i < 3; i++) {
		delay(100);
		Tone(m_pinBuzzer, 1600);
		delay(100);
	   	NoTone(m_pinBuzzer);
	}
}

void SimpleVario::update(double pressure)
{
	auto time = millis() - m_time;
	m_altimeter->addPressure(pressure, (time)/1000);
	m_climbRate = m_altimeter->varioValue();
	m_altitude = m_altimeter->altitude();
	m_beeperController->addValue(m_climbRate);
	updateBeep();
	m_time = millis();
}

void SimpleVario::updateBeep()
{
	if (m_beeperController->beeping()) {
		bool climbing = m_beeperController->climbing();
		bool sinking = m_beeperController->sinking();
		if (climbing || sinking) {
			double duration = m_beeperController->duration();
			double now = millis();
			if (now >= m_stopBeepingAt && m_shouldStopBeeping) {
				if (climbing) {
					beep(false, 0, 0);
                }
				m_shouldStopBeeping = false;
				m_shouldStartBeeping = true;
				m_startedBeeping = false;
				m_canStartBeepingAt = now + duration;
			}
			else if (now >= m_canStartBeepingAt && m_shouldStartBeeping) {
				m_shouldStartBeeping = false;
				m_startedBeeping = true;
				m_stopBeepingAt = now + duration;
				m_shouldStopBeeping = true;
			}

			if (m_startedBeeping) {
				auto freq = climbing ? m_beeperController->rateFromLiftTone() : m_beeperController->rateFromSinkTone();
				beep(climbing, freq, duration);
			}
		} else {
			beep(false, 0, 0);
		}
	} else {
  		beep(false, 0, 0);
	}
}

void SimpleVario::setAltitude(double alt) {
	m_altDiff = m_altitude - alt;
	if (m_silent || !m_beepsOnStart) return;
	for (int i = 0; i < 3; i++) {
		delay(100);
		Tone(m_pinBuzzer, 800);
		delay(100);
		NoTone(m_pinBuzzer);
	}
}

void SimpleVario::setClimbThreshold(double value) {
	m_beeperController->setClimbThreshold(value);
}
void SimpleVario::setSinkThreshold(double value) {
	m_beeperController->setSinkThreshold(value);
}
double SimpleVario::climbThreshold() {
	return m_beeperController->climbThreshold();
}
double SimpleVario::sinkThreshold() {
	return m_beeperController->sinkThreshold();
}

void SimpleVario::beep(bool climbing, double freq, double duration)
{
	if (m_silent) return;
	if (freq == 0) {
		NoTone(m_pinBuzzer);
	} else {
		auto hasSound = climbing ? m_beepsOnLift : m_beepsOnSink;
		if (hasSound) {
			Tone(m_pinBuzzer, freq, duration);
		}
	}
}

void SimpleVario::forceStopBeep() 
{
	NoTone(m_pinBuzzer);
}