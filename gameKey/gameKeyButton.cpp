#include <Keyboard.h>
#include "gameKeyButton.h"

gk_button::gk_button() {
	this->locColumn = -1;
	this->locRow = -1;
	this->type = KEYB;
	this->keymap = 0x00;
	this->stateCurrent = 0;
	this->timeFlux = millis();
}

gk_button::gk_button(int8_t locColumn_in, int8_t locRow_in) {
	this->locColumn = locColumn_in;
	this->locRow = locRow_in;
	this->type = KEYB;
	this->keymap = 0x00;
	this->stateCurrent = 0;
	this->timeFlux = millis();
}

void gk_button::update(matrixData data_in) {
	if (data_in.data[this->locColumn][this->locRow] != this->stateCurrent) {
		timeFlux = millis();
		this->stateFlux = data_in.data[this->locColumn][this->locRow];
	}
	if ((millis() - timeFlux) >= DEBOUNCE_INTERVAL) {
		stateCurrent = stateFlux;
	}
}

bool gk_button::getCurrentState() {
	return(this->stateCurrent);
}

bool gk_button::getPreviousState() {
	return(this->statePrev);
}

void gk_button::stateSync() {
	this->statePrev = this->stateCurrent;
}

uint8_t gk_button::getKeymap() {
	return(this->keymap);
}

void gk_button::putKeymap(uint8_t keymap_in) {
	this->keymap = keymap_in;
}

keyType gk_button::getControlType() {
	return(this->type);
}

void gk_button::putControlType(keyType controlType_in) {
	this->type = controlType_in;
}

int8_t gk_button::getCol() {
	return(this->locColumn);
}

int8_t gk_button::getRow() {
	return(this->locRow);
}
