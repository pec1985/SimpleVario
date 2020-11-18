/**
 *	SimpleVario!!
 *	Copyright Pedro Enrique
 */

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "TimeLib.h"
#include "SimpleArray.h"
#include "LiquidCrystal_I2C.h"

static SimpleArray<String> StringSplit(String str, char deli) 
{
	SimpleArray<String> array;
	String tmp;
	for (int i = 0, length = str.length(); i < length; i++) {
		char c = str[i];
		if (c == deli) {
			array.push(tmp);
			tmp = "";
		} else {
			tmp.concat(c);
		}
	}
	array.push(tmp);
	return array;
}
static inline String iString(int n)
{
	return String(n).length() == 1 ? String("0") + String(n) : String(n);
}
static inline String readableTime() 
{
	return iString(hour()) + ":" + iString(minute()) + ":" + iString(second());
}

static void lcdPrint(LiquidCrystal_I2C& lcd, const String& left, const String& right, bool firstLine) 
{
	static String _left1("");
	static String _left2("");
	static String _right1("");
	static String _right2("");

	if (firstLine) {
		if (_left1 == left && _right1 == right) return;
		_left1 = left;
		_right1 = right;
	} else {
		if (_left2 == left && _right2 == right) return;
		_left2 = left;
		_right2 = right;
	}
	auto spaces = 16 - left.length() - right.length();
	if (spaces < 0) return;

	String spacesString = "";
	while (spaces--) {
		spacesString += " ";
	}
	lcd.setCursor(0, firstLine ? 0 : 1);
	lcd.print(left + spacesString + right);
}

#endif
