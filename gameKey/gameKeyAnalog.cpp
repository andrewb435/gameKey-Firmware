#include "gameKeyAnalog.h"

gk_analog::gk_analog() {
	analogPin = -1;
	thresholdLow = 0;
	center = 0;
	thresholdHigh  = 0;
	analogMode = false;
	deadzone = 0;
	invert = false;
	keyDigitalUp = 0x00;
	keyDigitalDown = 0x00;
}

gk_analog::gk_analog(int8_t analogPin_in) {
	analogPin = analogPin_in;
	thresholdLow = 0;
	center = 0;
	thresholdHigh  = 0;
	analogMode = false;
	deadzone = 0;
	invert = false;
	keyDigitalUp = 0x00;
	keyDigitalDown = 0x00;

	pinMode(this->analogPin, INPUT);
}

// Sets
void gk_analog::setThresholdLow(uint16_t thresholdLow_in) {
	this->thresholdLow = thresholdLow_in;
}

void gk_analog::setCenter(uint16_t center_in) {
	this->center = center_in;
}

void gk_analog::setThresholdHigh(uint16_t thresholdHigh_in) {
	this->thresholdHigh = thresholdHigh_in;
}

void gk_analog::setDeadzone(uint16_t deadzone_in) {
	this->deadzone = deadzone_in;
}

void gk_analog::setKeyUp(uint8_t keyDigitalUp_in) {
	this->keyDigitalUp = keyDigitalUp_in;
}

void gk_analog::setKeyDown(uint8_t keyDigitalDown_in) {
	this->keyDigitalDown = keyDigitalDown_in;
}

void gk_analog::setAnalogMode(bool analogMode_in) {
	this->analogMode = analogMode_in;
}

void gk_analog::setInvert(bool invert_in) {
	this->invert = invert_in;
}

// Gets

int8_t gk_analog::getAnalogPin() {
	return(this->analogPin);
}

uint16_t gk_analog::getThresholdLow() {
	return(this->thresholdLow);
}

uint16_t gk_analog::getCenter() {
	return(this->center);
}

uint16_t gk_analog::getThresholdHigh() {
	return(this->thresholdHigh);
}

uint16_t gk_analog::getDeadzone() {
	return(this->deadzone);
}

bool gk_analog::getAnalogMode() {
	return(this->analogMode);
}

bool gk_analog::getInvert() {
	return(this->invert);
}

uint16_t gk_analog::getRawValue() {
	return(analogRead(this->analogPin));
}

uint8_t gk_analog::getKeyUp() {
	return(this->keyDigitalUp);
}

uint8_t gk_analog::getKeyDown() {
	return(this->keyDigitalDown);
}

// Methods

void gk_analog::update() {
	// Always read the pin every update
	readAverage[readIndex] = analogRead(this->analogPin);
	if (readIndex >= READ_AVERAGE_COUNT) {
		readIndex = 0;
	} else {
		readIndex++;
	}
}

int16_t gk_analog::reportAnalog() {
	int16_t value = 0;
	for (uint8_t i = 0; i < READ_AVERAGE_COUNT; i++) {
		value += this->readAverage[i];
	}
	value /= READ_AVERAGE_COUNT;	// Calculate the rolling average

	// Clamp the values to calibration min/max
	if (value > this->thresholdHigh) {
		value = this->thresholdHigh;
	} else if (value < this->thresholdLow) {
		value = this->thresholdLow;
	}

	// Make sure deadzone isn't 0
	if (this->deadzone < 2) {
		this->deadzone = 200;
	}

	// Map to invert/deadzone
	int16_t center_low = this->center - (this->deadzone/2);
	int16_t center_high = this->center + (this->deadzone/2);

	if (value > center_high) {	// Axis reads high
		if (this->invert) {
			value = map(value, center_high, this->thresholdHigh, -1, BIT16_HALF_DOWN);
		} else {
			value = map(value, center_high, this->thresholdHigh, 1, BIT16_HALF_UP);
		}
	} else if (value < center_low) {	// Axis reads low
		if (this->invert) {
			value = map(value, center_low, this->thresholdLow, 1, BIT16_HALF_UP);
		} else {
			value = map(value, center_low, this->thresholdLow, -1, BIT16_HALF_DOWN);
		}
	} else {	// Axis reads inside the deadzone
		value = 0;
	}
	
	// Return the clamped, inverted, normalized value
	return(value);
}

bool gk_analog::reportDigitalUp() {
	if (this->reportAnalog() > 0) {
		return true;
	} else {
		return false;
	}
}

bool gk_analog::reportDigitalDown() {
	if (this->reportAnalog() < 0) {
		return true;
	} else {
		return false;
	}
}

// digitalDirection gk_analog::reportCurrentDigital() {
// 	int16_t analogResult = reportAnalog();
// 	if (analogResult > 0) {
// 		digiCur = DIGITAL_UP;
// 	} else if (analogResult < 0) {
// 		digiCur = DIGITAL_DOWN;
// 	} else {
// 		digiCur = DIGITAL_NEUTRAL;
// 	}
// 	return(digiCur);
// }

// digitalDirection gk_analog::reportPreviousDigital() {
// 	return(this->digiPrev);
// }

void gk_analog::syncState() {
	this->digiPrev = this->digiCur;
}
