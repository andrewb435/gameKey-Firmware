
#include <HardwareSerial.h>
#include "gameKeyUnit.h"

gk_unit::gk_unit(int8_t pinColumns_in[HW_COLS], int8_t pinRows_in[HW_ROWS], gk_analog axes_in[HW_AXES], gk_button buttons_in[HW_COLS * HW_ROWS]) {
	for (int8_t i = 0; i < HW_COLS; i++) {
		this->pinColumns[i] = pinColumns_in[i];
		pinMode(this->pinColumns[i], INPUT);
	}
	for (int8_t i = 0; i < HW_ROWS; i++) {
		this->pinRows[i] = pinRows_in[i];
		pinMode(this->pinRows[i], INPUT);
	}
	for (int8_t i = 0; i < (HW_AXES); i++) {
		this->axes[i] = axes_in[i];
		pinMode(this->axes[i].getAnalogPin(), INPUT);
	}
	for (int8_t i = 0; i < (HW_COLS * HW_ROWS); i++) {
		this->buttons[i] = buttons_in[i];
	}
	this->controllerMode = false;
}

void gk_unit::readMatrix() {
	// iterate columns
	for (int8_t mtxCol = 0; mtxCol < HW_COLS; mtxCol++) {
		// change column to output low
		pinMode(pinColumns[mtxCol], OUTPUT);
		digitalWrite(pinColumns[mtxCol], LOW);
		// iterate rows per column
		for (int8_t mtxRow = 0; mtxRow < HW_ROWS; mtxRow++) {
			// bring the row to INPUT_PULLUP
			pinMode(pinRows[mtxRow], INPUT_PULLUP);
			this->buttonData.data[mtxCol][mtxRow] = digitalRead(pinRows[mtxRow]);
			// Set the row back to INPUT
			pinMode(pinRows[mtxRow], INPUT);
		}
		// done with this column, return to high impedence INPUT state
		pinMode(pinColumns[mtxCol], INPUT);
	}
}

void gk_unit::update() {
	readMatrix();
	for (int8_t i = 0; i < (HW_COLS*HW_ROWS); i++) {
		this->buttons[i].update(this->buttonData);

	}
	for (int8_t i = 0; i < HW_AXES; i++) {
		this->axes[i].update();
	}
}

<<<<<<< HEAD
=======
void gk_unit::setLayerShift(uint8_t layer_in){
	this->activeLayer = (keyLayer)layer_in;	// cast the decimal keybind into 
}

keyLayer gk_unit::getLayerShift(){
	return(this->activeLayer);	// cast the decimal keybind into 
}

>>>>>>> layersupport
bool gk_unit::setGamepadMode(bool mode_in) {
	this->controllerMode = mode_in;
}

bool gk_unit::getGamepadMode() {
	return(this->controllerMode);
}

void gk_unit::reportSerial() {
	Serial.print(F("Column pins: "));
	for (int8_t i = 0; i < HW_COLS; i++) {
		Serial.print(" ");
		Serial.print(this->pinColumns[i]);
	}
	Serial.println();

	Serial.print(F("Row pins: "));
	for (int8_t i = 0; i < HW_ROWS; i++) {
		Serial.print(" ");
		Serial.print(this->pinRows[i]);
	}
	Serial.println();

	Serial.print(F("Analog pins: "));
	for (int8_t i = 0; i < (HW_AXES); i++) {
		Serial.print(" ");
		Serial.print(this->axes[i].getAnalogPin());
	}
	Serial.println();

	Serial.print(F("Operating in gamepad mode: "));
	Serial.println(this->controllerMode);
}