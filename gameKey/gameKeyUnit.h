#ifndef __GAMEKEYUNIT_H__
#define __GAMEKEYUNIT_H__

#include <Arduino.h>
#include "gameKeyConfig.h"
#include "gameKeyEEPROM.h"
#include "gameKeyAnalog.h"
#include "gameKeyButton.h"
#include "gameKeySerial.h"

class gk_unit {
	private:
		int8_t pinColumns[HW_COLS];
		int8_t pinRows[HW_ROWS];
		matrixData buttonData;
		keyLayer activeLayer;
		bool controllerMode;
		void readMatrix();
	public:
		gk_button buttons[HW_COLS * HW_ROWS];
		gk_analog axes[HW_AXES];
		gk_unit(int8_t pinColumns_in[HW_COLS], int8_t pinRows_in[HW_ROWS], gk_analog axes_in[HW_AXES], gk_button buttons_in[HW_COLS * HW_ROWS]);
		void update();
		void setLayerShift(uint8_t layer_in);
		keyLayer getLayerShift();
		bool setGamepadMode(bool mode_in);
		bool getGamepadMode();
		void reportSerial();
};

#endif