/**
 *	SimpleVario!!
 *	Copyright Pedro Enrique
 */

#ifndef SimpleVario_H
#define SimpleVario_H

class BeeperController;
class Altimeter;
class SimpleVario
{
public:
	SimpleVario();
	~SimpleVario();
	void begin(int pinBuzzer);
	void update(double pressure);
	void initialBeep();
	void setBeepsOnStart(bool beeps) {
		m_beepsOnStart = beeps;
	}
	void setSilent(bool silent) {
		m_silent = silent;
	}
	double climbRate() const {
		return m_climbRate;
	}
	double altitude() const {
		return m_altitude - m_altDiff;
	}
	double altitudeMsl() const {
		return m_altitude;
	}
	void setBeepsOnSink(bool val) {
		m_beepsOnSink = val;
	}
	void setAltitude(double alt);

	void setClimbThreshold(double value);
	void setSinkThreshold(double value);
	double climbThreshold();
	double sinkThreshold();
	const bool silent() {
		return m_silent;
	}
	const bool beepsOnSink() {
		return m_beepsOnSink;
	}
	void forceStopBeep();
private:

	void updateBeep();
	void beep(bool climbing, double freq, double duration);
	double m_climbRate {0};
	double m_altitude {0};

	double m_time {-1};
	double m_altDiff {0};
	int m_pinBuzzer {-1};

	bool m_beepsOnLift {true};
	bool m_beepsOnSink {true};
	bool m_startedBeeping {false};
	bool m_shouldStartBeeping {true};
	bool m_shouldStopBeeping {false};
	double m_canStartBeepingAt {0};
	double m_stopBeepingAt {0};
	bool m_beepsOnStart {true};
	bool m_silent {false};

	BeeperController* m_beeperController {NULL};
	Altimeter* m_altimeter {NULL};
};

#endif
