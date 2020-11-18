#ifndef BUTTON_H
#define BUTTON_H

#include "Arduino.h"

class Button {
public:
	Button() {}
	~Button() {}
	bool isPressing() 
	{
		if (m_pin == -1) {
			return false;
		}
		return digitalRead(m_pin) == LOW; 
	}
	bool pressed() 
	{
		if (!isPressing()) return false;
		while (isPressing());
		return true;
	}
	void setPin(int pin) 
	{
		pinMode(pin, INPUT);
		digitalWrite(pin, HIGH);
		m_pin = pin;
	}
private:
	int m_pin = -1;
};

#endif