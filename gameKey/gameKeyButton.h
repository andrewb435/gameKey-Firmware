#ifndef __GAMEKEYBUTTON_H__
#define __GAMEKEYBUTTON_H__

#include <Arduino.h>
#include "gameKeyConfig.h"

#define DEBOUNCE_INTERVAL 5	//ms

typedef enum {
	KEYB,
	GPAD,
	BOTH
} keyType;	// KEYB for keyboard, GPAD for gamepad button

class gk_button {
	private:
		// configuration
		int8_t locColumn;
		int8_t locRow;
		uint8_t keymap;
		keyType type;

		// internal working variables
		long timeFlux;
		bool stateFlux;
		bool stateCurrent;
		bool statePrev;
	public:
		// Attributes
		// Constructors
		gk_button();
		gk_button(int8_t locColumn_in, int8_t locRow_in);
		// Methods
		void update(matrixData data_in);
		bool getCurrentState();
		bool getPreviousState();
		void stateSync();
		uint8_t getKeymap();
		void putKeymap(uint8_t keymap_in);
		keyType getControlType();
		void putControlType(keyType controlType_in);
		int8_t getCol();
		int8_t getRow();
};

#endif