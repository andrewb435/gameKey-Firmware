#include <Keyboard.h>
#include "gameKeyButton.h"

gk_button::gk_button() {
	this->locColumn = -1;
	this->locRow = -1;
	this->type = KEYBOARD_BUTTON;
	this->keyA = 0x00;
	this->keyB = 0x00;
	this->keyC = 0x00;
	this->keyD = 0x00;
	this->stateCurrent = 0;
	this->timeFlux = millis();
}

gk_button::gk_button(int8_t locColumn_in, int8_t locRow_in) {
	this->locColumn = locColumn_in;
	this->locRow = locRow_in;
	this->type = KEYBOARD_BUTTON;
	this->keyA = 0x00;
	this->keyB = 0x00;
	this->keyC = 0x00;
	this->keyD = 0x00;
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

uint8_t gk_button::getKeymap(keyLayer layer_in) {
	switch (layer_in)
	{
	case LAYER_A:
		return(this->keyA);
		break;
	case LAYER_B:
		return(this->keyB);
		break;
	case LAYER_C:
		return(this->keyC);
		break;
	case LAYER_D:
		return(this->keyD);
		break;
	default:	// Default return is LAYER_A
		return(this->keyA);
		break;
	}
}

void gk_button::putKeymap(keyLayer layer_in, uint8_t keymap_in) {
	switch (layer_in)
	{
	case LAYER_A:
		this->keyA = keymap_in;
		break;
	case LAYER_B:
		this->keyB = keymap_in;
		break;
	case LAYER_C:
		this->keyC = keymap_in;
		break;
	case LAYER_D:
		this->keyD = keymap_in;
		break;
	default:	// Default set is LAYER_A
		this->keyA = keymap_in;
		break;
	}
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
