#ifndef __GAMEKEYEEPROM_H__
#define __GAMEKEYEEPROM_H__

#include "gameKeyConfig.h"
#include "gameKeyButton.h"

#define EEPROM_MEMORY_START 0

typedef struct eeprom_button {
<<<<<<< HEAD
	uint8_t binding;
=======
	uint8_t binding_a;
	uint8_t binding_b;
	uint8_t binding_c;
	uint8_t binding_d;
>>>>>>> layersupport
	keyType controlType;
};

typedef struct eeprom_analog {
	uint16_t thresholdLow;
	uint16_t center;
	uint16_t thresholdHigh;
	uint16_t deadzone;
	char keyDigitalUp;
	char keyDigitalDown;
	bool analogMode;
	bool invert;
};

typedef struct eeprom_settings {	// 752 bytes
	uint8_t featureFlags;
	char* devName[8];
	eeprom_button buttons[HW_ROWS*HW_COLS];
	eeprom_analog analog[HW_AXES];
};

#endif