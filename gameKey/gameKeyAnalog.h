#ifndef __GAMEKEYANALOG_H__
#define __GAMEKEYANALOG_H__

#include <Arduino.h>

#define READ_AVERAGE_COUNT 4
#define BIT16_HALF_UP 32767
#define BIT16_HALF_DOWN -32768

typedef enum {	// Neutral == 0 so true/false will tell if pressed
	DIGITAL_NEUTRAL,
	DIGITAL_UP,
	DIGITAL_DOWN
} digitalDirection;

class gk_analog {
	private:
		// Configuration
		int8_t analogPin;
		uint16_t thresholdLow;
		uint16_t center;
		uint16_t thresholdHigh;
		uint16_t deadzone;
		uint8_t keyDigitalUp;
		uint8_t keyDigitalDown;
		bool analogMode;
		bool invert;
		
		// Internal working numbers
		uint16_t readAverage[READ_AVERAGE_COUNT];
		uint8_t readIndex;
		digitalDirection digiCur;
		digitalDirection digiPrev;

		// Internal Methods

	public:
		// constructors
		gk_analog();
		gk_analog(int8_t analogPin_in);
		// sets
		// void setAnalogPin(int8_t analogPin_in);
		void setThresholdLow(uint16_t thresholdLow_in);
		void setCenter(uint16_t center_in);
		void setThresholdHigh(uint16_t thresholdHigh_in);
		void setDeadzone(uint16_t deadzone_in);
		void setKeyUp(uint8_t keyDigitalUp_in);
		void setKeyDown(uint8_t keyDigitalDOwn_in);
		void setAnalogMode(bool analogMode_in);
		void setInvert(bool invert_in);
		// gets
		int8_t getAnalogPin();
		uint16_t getThresholdLow();
		uint16_t getCenter();
		uint16_t getThresholdHigh();
		uint16_t getDeadzone();
		bool getAnalogMode();
		bool getInvert();
		uint16_t getRawValue();
		uint8_t getKeyUp();
		uint8_t getKeyDown();
		// methods
		void update();
		int16_t reportAnalog();	// return the signed 16 bit integer representing the normalized axis value
		bool reportDigitalUp();	// Return the keyDigitalUp if activated
		bool reportDigitalDown();	// Return the keyDigitalDown if activated
		// digitalDirection reportCurrentDigital();	// Return the digitalDirection indicating current digital state
		// digitalDirection reportPreviousDigital();	// Return the digitalDirection for the previous digital state
		void syncState();	// Sync the previous digital state to current
};

#endif