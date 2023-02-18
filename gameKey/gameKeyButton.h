#ifndef __GAMEKEYBUTTON_H__
#define __GAMEKEYBUTTON_H__

#include <Arduino.h>
#include "gameKeyConfig.h"

#define DEBOUNCE_INTERVAL 5	//ms

typedef enum {
<<<<<<< HEAD
	KEYB,
	GPAD,
	BOTH
} keyType;	// KEYB for keyboard, GPAD for gamepad button

=======
	NONE = 1,
	KEYBOARD_BUTTON,
	GAMEPAD_BUTTON,
	BOTH,
	LAYERSHIFT
} keyType;	// KEYB for keyboard, GPAD for gamepad button

typedef enum {	// ASCII codes correspond to ununsed codes from the Arduino extended ASCII map, and are mapped in the companion app
	LAYER_A = 0xFF,	// 255
	LAYER_B = 0xFE,	// 254
	LAYER_C = 0xFD,	// 253
	LAYER_D = 0xFC	// 252
} keyLayer;

>>>>>>> layersupport
class gk_button {
	private:
		// configuration
		int8_t locColumn;
		int8_t locRow;
<<<<<<< HEAD
		uint8_t keymap;
=======
		uint8_t keyA;
		uint8_t keyB;
		uint8_t keyC;
		uint8_t keyD;
>>>>>>> layersupport
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
<<<<<<< HEAD
		uint8_t getKeymap();
		void putKeymap(uint8_t keymap_in);
=======
		uint8_t getKeymap(keyLayer layer_in);
		void putKeymap(keyLayer layer_in, uint8_t keymap_in);
>>>>>>> layersupport
		keyType getControlType();
		void putControlType(keyType controlType_in);
		int8_t getCol();
		int8_t getRow();
};

#endif