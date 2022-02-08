/*
    gameKey-firmware runs the atmega32u4 control board in a gameKey device
    Copyright (C) 2022 Andrew Baum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Matthew Heironimus's Arduino joystick library for 32u4 based Arduino boards
// https://github.com/MHeironimus/ArduinoJoystickLibrary
#include <Joystick.h>

// Arduino Keyboard Emulation Library
#include <Keyboard.h>

// Arduino EEPROM Library
#include <EEPROM.h>

// gameKey interfaces
#include "gameKeyUnit.h"

// Version tracking
#define VERSION 1

// Comment to disable serial debug
bool flagSerialDebug = false;
// Verbose debug spams serial, breaking the companion app
bool flagSerialVerbose = false;

// Config flag to disable keyboard presses
bool flagConfigMode = false;

// Debug message interval and timestamp
const long debugSerialInterval = 1000;
long timeLastDebug = millis();


int8_t configColumms[HW_COLS] = {2,3,4,5,6,7};	// Pins in order for columns on the matrix
int8_t configRows[HW_ROWS] = {A1,A0,15,14,16};	// Pins in order for rows

gk_analog configThumbstick[HW_AXES] = {	// Assign pins for analog axis
	gk_analog(A2),
	gk_analog(A3)
};

gk_button configButtons[HW_COLS * HW_ROWS] = {	// Assign matrix positions for buttons
	gk_button(0,0),	// Pinky Base
	gk_button(0,1),
	gk_button(0,2),
	gk_button(0,3),
	gk_button(0,4),

	gk_button(1,0),	// Ring base
	gk_button(1,1),
	gk_button(1,2),
	gk_button(1,3),
	gk_button(1,4),

	gk_button(2,0),	// Middle Base
	gk_button(2,1),
	gk_button(2,2),
	gk_button(2,3),
	gk_button(2,4),

	gk_button(3,0),	// Index Base
	gk_button(3,1),
	gk_button(3,2),
	gk_button(3,3),
	gk_button(3,4),

	gk_button(4,0),	// Index Side
	gk_button(4,1),	// Thumb Side
	gk_button(4,2),	// ThumbJoy Press
	gk_button(4,3),
	gk_button(4,4),

	gk_button(5,0),	// Thumb D-Pad N
	gk_button(5,1),	// E
	gk_button(5,2),	// S
	gk_button(5,3),	// W
	gk_button(5,4)	// Center Press
};

gk_unit controller(	// Controllers, assemble!
	configColumms,
	configRows,
	configThumbstick,
	configButtons
);

// TODO: Actually implement analog thumbstick mode
Joystick_ gamepad(
	JOYSTICK_DEFAULT_REPORT_ID,
	JOYSTICK_TYPE_GAMEPAD,
	(HW_COLS * HW_ROWS), 0, // button, hat count
	true, true, false,		// X, Y, Z
	false, false, false,	// rX, rY, rZ
	false, false,			// Rudder, Throttle
	false, false, false		// Accelerator, Brake, Steering
);

uint8_t timerUSB = 5;	// 2ms per usb frame, 500 updates/second
long timestampUSB = millis();

void setup() {
	pinMode(CONTROL_ENABLE_PIN, INPUT_PULLUP);	// Hardware kill switch, shorted = enabled
	getEEPROM();
	gamepad.begin(false);	// begin joystick without autosendstate
	Serial.begin(115200);
}

void loop() {
	// Debug reporting
	if (flagSerialDebug && flagSerialVerbose) {
		if (millis() - debugSerialInterval >= timeLastDebug) {	// Check report interval and if its passed, reportSerial and reset timestamp
			debugReport();
			timeLastDebug = millis();
		}
	}
	// Check for serial commands in buffer
	if (Serial.available() > 0) {
		cliCheckContents();
	}
	// Run the main loop
	if (!digitalRead(CONTROL_ENABLE_PIN)) {	// Hardware kill switch is shorted
		controller.update();
		//Button Processing
		for (int i = 0; i < (HW_COLS * HW_ROWS); i++) {
			if (controller.getGamepadMode()) {	// true = gamepad mode
				// TODO: Future gamepad button code
				//this is not how it should be done
				//gamepad.setButton(i, controller.buttons[i].getCurrentState());
			} else {	// false = keyboard mode
				if (controller.buttons[i].getCurrentState()) {	// If the button IS PRESSED
					if (!controller.buttons[i].getPreviousState()) {	// and it WAS NOT PRESSED
						if (flagSerialDebug) {
							Serial.print(F("Button state changed to pressed for button"));
							Serial.print(i);
							Serial.print(F(", pressing kb key \""));
							Serial.print(controller.buttons[i].getKeymap());
							Serial.println("\"");
						}
						if (!flagConfigMode) {
							// Only actually press when not in config mode
							Keyboard.press(controller.buttons[i].getKeymap());	// send the keymap char
						}
						controller.buttons[i].stateSync();	// sync the previous state to current
					}
				} else if (!controller.buttons[i].getCurrentState()){	// If the button is NOT PRESSED
					if (controller.buttons[i].getPreviousState()) {	// and it WAS PRESSED
						if (flagSerialDebug) {
							Serial.print(F("Button state changed to unpressed for button"));
							Serial.println(i);
						}
						Keyboard.release(controller.buttons[i].getKeymap());	// release the keymap char
						controller.buttons[i].stateSync();	// sync the previous state to current
					}
				}
			}			
		}
		//Analog Processing
		for (int8_t analogIndex = 0; analogIndex < HW_AXES; analogIndex++) {
			if (controller.axes[analogIndex].getAnalogMode()) {	// Analog mode true, map to joystick
				mapAnalogToJoystick(analogIndex);
			} else {	// Analog mode false, map to keyboard
				mapAnalogToKeyboard(analogIndex);
			}
		}
		if ((millis() - timerUSB) >= timestampUSB) {
			gamepad.sendState();
			timestampUSB = millis();
		}
	} else {	// Hardware kill switch is open
		Keyboard.releaseAll();	// Release all keyboard keys to prevent lockouts
	}
}

void mapAnalogToJoystick(int8_t index) {
	if (index == 0) {
		gamepad.setXAxis(controller.axes[index].reportAnalog());
	} else if (index == 0) {
		gamepad.setYAxis(controller.axes[index].reportAnalog());
	}
}

void mapAnalogToKeyboard(int8_t index) {
	if (controller.axes[index].reportDigitalUp()) {
		// Axis is ++, press KeyUp and release KeyDown
		if (controller.axes[index].getKeyUp() != 0) {
			if (!flagConfigMode) {
				// Only actually press when not in config mode
				Keyboard.press(controller.axes[index].getKeyUp());
			}
		}
		if (controller.axes[index].getKeyDown() != 0) {
			Keyboard.release(controller.axes[index].getKeyDown());
		}
	} else if (controller.axes[index].reportDigitalDown()) {
		// Axis is --, release KeyUp and press KeyDown
		if (controller.axes[index].getKeyUp() != 0) {
			Keyboard.release(controller.axes[index].getKeyUp());
		}
		if (controller.axes[index].getKeyDown() != 0) {
			if (!flagConfigMode) {
				// Only actually press when not in config mode
				Keyboard.press(controller.axes[index].getKeyDown());
			}
		}
	} else {
		// Axis is neither, release both
		if (controller.axes[index].getKeyUp() != 0) {
			Keyboard.release(controller.axes[index].getKeyUp());
		}
		if (controller.axes[index].getKeyDown() != 0) {
			Keyboard.release(controller.axes[index].getKeyDown());
		}
	}
}

void debugReport() {
	Serial.print(F("en: "));
	Serial.println(!digitalRead(CONTROL_ENABLE_PIN));
	Serial.print(F("RawX: "));
	Serial.print(controller.axes[0].getRawValue());
	Serial.print(F(" || "));
	Serial.print(F("RawY: "));
	Serial.println(controller.axes[1].getRawValue());
	Serial.print(F("FilterX: "));
	Serial.print(controller.axes[0].reportAnalog());
	Serial.print(F(" || "));
	Serial.print(F("FilterY: "));
	Serial.println(controller.axes[1].reportAnalog());
	// controller.reportSerial();
}

void cliCheckContents() {
	char buffer[MAX_SERIAL_BYTES] = {'\0'};
	uint8_t length = 0;
	for (uint8_t i = 0; i < MAX_SERIAL_BYTES; i++) {
		char in_char = Serial.read();			
		if (in_char == '\n' || in_char == '\r'){
			cliParse(buffer, length);
			break;
		} else {
			buffer[length] = in_char;
		}
		length++;
	}
}

void cliParse(char* buffer_in, uint8_t length_in) {
	char* args[MAX_ARGUMENT_COUNT];
	if (length_in > 0) {
		for (uint8_t i = 0; i < MAX_ARGUMENT_COUNT; i++) {
			if (i == 0) {
				// First args gets the current buffer then delim by ascii space
				args[i] = strtok(buffer_in, " ");
			} else {
				// Second+ args get filled by anything else in the buffer, delim by ascii space
				args[i] = strtok(NULL, " ");
			}
		}
	}
	// Check command list
	if (!strcmp(args[0], "devinfo")) {			// Received deviceinfo enquiry, respond with device type
		Serial.println(F("gameKey"));
	} else if (!strcmp(args[0], "config")) {	// switch config mode
		cmdConfigMode(args[1]);
	} else if (!strcmp(args[0], "debug")) {		// switch debug mode
		cmdDebugMode(args[1]);
	} else if (!strcmp(args[0], "superdebug")) {// switch debug mode
		cmdSuperDebugMode(args[1]);
	} else if (!strcmp(args[0], "vers")) {		// Report FW Ver
		Serial.println(VERSION);
	} else if (!strcmp(args[0], "savenv")) {	// Save config to eeprom
		cmdSaveEEPROM();
	} else if (!strcmp(args[0], "feat")) {		// Return feature flags
		cmdReportFeatures();
	} else if (!strcmp(args[0], "getname")) {	// Receive and parse devname
		cmdReportName();
	} else if (!strcmp(args[0], "getbuttons")) {	// Report Button configuration
		cmdReportButtons();
	} else if (!strcmp(args[0], "getaxes")) {	// Report Axes configuration
		cmdReportAxes();
	} else if (!strcmp(args[0], "setname")) {	// Receive and parse devname
		cmdRename(args[1]);
	} else if (!strcmp(args[0], "bind")) {		// Receive and parse bind commands
		cmdBind(args[1]);
	} else if (!strcmp(args[0], "unbind")) {	// Receive and parse unbind commands
		cmdUnbind(args[1]);
	} else if (!strcmp(args[0], "reporta")) {	// report current analog values
		cmdReportAnalog();
	} else if (!strcmp(args[0], "setaxis")) {	// report current analog values
		cmdSetAxis(args[1]);
	}
}

void cmdConfigMode(char* arg) {
	if (arg != "\0") {
		int8_t iarg = atoi(arg);
		if (iarg) {
			flagConfigMode = true;
			Serial.println("Enabling config mode, keypresses disabled");
		} else if (!iarg) {
			flagConfigMode = false;
			Serial.println("DISABLING CONFIG MODE, KEYPRESSES ENABLED");
		}
	}
}

void cmdDebugMode(char* arg) {
	if (arg != "\0") {
		int8_t iarg = atoi(arg);
		if (iarg) {
			flagSerialDebug = true;
			Serial.println("Enabling debug mode, many more serial responses");
		} else if (!iarg) {
			flagSerialDebug = false;
			Serial.println("Disabling debug mode, serial feedback minimized");
		}
	}
}

void cmdSuperDebugMode(char* arg) {
	if (arg != "\0") {
		int8_t iarg = atoi(arg);
		if (iarg) {
			flagSerialVerbose = true;
			Serial.println("Enabling SUPER debug mode, COMPANION WILL BREAK");
		} else if (!iarg) {
			flagSerialVerbose = false;
			Serial.println("Disabling SUPER debug mode, companion can be used");
		}
	}
}

void cmdSaveEEPROM() {
	Serial.println("Saving config to EEPROM...");
	putEEPROM();
	Serial.println("Saved!");
}

void cmdReportFeatures() {
	// TODO: get actual featureflags
	Serial.println(0b00000101);
}

void cmdReportName() {
	// TODO: Name report
	Serial.println("gkLeft");
}

void cmdReportButtons() {
	for (uint8_t i = 0; i < HW_COLS*HW_ROWS; i++) {
		Serial.print(i);
		Serial.print("=");
		Serial.print(int(controller.buttons[i].getKeymap()));
		if (i < ((HW_COLS*HW_ROWS)-1)) {
			Serial.print("&");
		}
	}
	Serial.println();
}

void cmdReportAxes() {
	// axisid=threshlow&center&threshigh&dz&up&down&mode&invert|
	for (uint8_t i = 0; i < HW_AXES; i++) {
		Serial.print(i);
		Serial.print("=");
		Serial.print(controller.axes[i].getThresholdLow());
		Serial.print("&");
		Serial.print(controller.axes[i].getCenter());
		Serial.print("&");
		Serial.print(controller.axes[i].getThresholdHigh());
		Serial.print("&");
		Serial.print(controller.axes[i].getDeadzone());
		Serial.print("&");
		Serial.print(int(controller.axes[i].getKeyUp()));
		Serial.print("&");
		Serial.print(int(controller.axes[i].getKeyDown()));
		Serial.print("&");
		Serial.print(controller.axes[i].getAnalogMode());
		Serial.print("&");
		Serial.print(controller.axes[i].getInvert());
		if (i < (HW_AXES-1)) {
			Serial.print("|");
		}
	}
	Serial.println();
}

void cmdRename(char* arg) {
	// TODO: Name parsing/assignment
}

void cmdBind(char* arg) {
	uint8_t bindCmdArgs[MAX_BIND_COMMANDS][BIND_CMD_SEGMENTS] = {NULL, NULL};
	if (arg != "\0") {
		if (flagSerialDebug) {
			Serial.print(F("Binding buttons"));
		}
		for (uint8_t bindIndex = 0; bindIndex < MAX_BIND_COMMANDS; bindIndex++) {
			// Split off each individual passed bind argument, delim by & for blocks and = for cmd/arg
			if (bindIndex == 0) {
				bindCmdArgs[bindIndex][BIND_BUTTON] = atoi(strtok(arg, "="));
				bindCmdArgs[bindIndex][BIND_ARG] = atoi(strtok(NULL, "&"));
			} else {
				bindCmdArgs[bindIndex][BIND_BUTTON] = atoi(strtok(NULL, "="));
				bindCmdArgs[bindIndex][BIND_ARG] = atoi(strtok(NULL, "&"));
			}
			if (bindCmdArgs[bindIndex][BIND_ARG] != NULL) {
				controller.buttons[bindCmdArgs[bindIndex][BIND_BUTTON]].putKeymap(bindCmdArgs[bindIndex][BIND_ARG]);
				if (flagSerialDebug) {
					Serial.print(F(" b"));
					Serial.print(int(bindCmdArgs[bindIndex][BIND_BUTTON]));
					Serial.print(F("="));
					Serial.print(int(bindCmdArgs[bindIndex][BIND_ARG]));
				}
			}
		}
		if (flagSerialDebug) {
			Serial.println();
		}
	}
}

void cmdUnbind(char* arg) {
	int unbindCmdArgs[MAX_BIND_COMMANDS][BIND_CMD_SEGMENTS] = {NULL, NULL};
	if (arg != "\0") {
		if (flagSerialDebug) {
				Serial.print(F("Unbinding buttons "));
		}
		for (uint8_t unbindIndex = 0; unbindIndex < MAX_BIND_COMMANDS; unbindIndex++) {
			// Split off each individual passed bind argument, delim by & for blocks and % for cmd/arg
			if (unbindIndex == 0) {
				unbindCmdArgs[unbindIndex][BIND_BUTTON] = atoi(strtok(arg, "&"));
			} else {
				unbindCmdArgs[unbindIndex][BIND_BUTTON] = atoi(strtok(NULL, "&"));
			}
			if ( unbindCmdArgs[unbindIndex][BIND_BUTTON] != NULL ) {
				controller.buttons[unbindCmdArgs[unbindIndex][BIND_BUTTON]].putKeymap(0x00);
				if (flagSerialDebug) {
					Serial.print(F(" b"));
					Serial.print(unbindCmdArgs[unbindIndex][BIND_BUTTON]);
				}
			}
		}
		if (flagSerialDebug) {
			Serial.println();
		}
	}
}

void cmdReportAnalog() {
	for (uint8_t i = 0; i < HW_AXES; i++) {
		Serial.print(controller.axes[i].getRawValue());
		if (i < (HW_AXES - 1)) {
			Serial.print("&");
		} else {
			Serial.println();
		}
	}
}

void cmdSetAxis(char* arg) {
	uint16_t axisArgs[AXIS_CMD_SEGMENTS] = {NULL};
	if (arg != "\0") {
		if (flagSerialDebug) {
			Serial.print(F("Incoming args: "));
			Serial.println(arg);
		}
		// Split off axis index from configuration block 
		uint8_t cur_axis = atoi(strtok(arg, "="));
		if (flagSerialDebug) {
			Serial.print(F("Configuring axis"));
			Serial.print(cur_axis);
			Serial.print(F(" args:"));
		}
		// Split configuration block into args
		for (uint8_t axisArgIndex = 0; axisArgIndex < AXIS_CMD_SEGMENTS; axisArgIndex++) {
			axisArgs[axisArgIndex] = atoi(strtok(NULL, "&"));
			if (flagSerialDebug) {
				Serial.print(axisArgs[axisArgIndex]);
				Serial.print(F(" "));
			}
		}
		if (flagSerialDebug) {
			Serial.println();
		}
		// Set all the axis parameters from axisArgs[]
		controller.axes[cur_axis].setThresholdLow(axisArgs[0]);
		controller.axes[cur_axis].setCenter(axisArgs[1]);
		controller.axes[cur_axis].setThresholdHigh(axisArgs[2]);
		controller.axes[cur_axis].setDeadzone(axisArgs[3]);
		controller.axes[cur_axis].setKeyUp(axisArgs[4]);
		controller.axes[cur_axis].setKeyDown(axisArgs[5]);
		controller.axes[cur_axis].setAnalogMode(axisArgs[6]);
		controller.axes[cur_axis].setInvert(axisArgs[7]);
	}
}

void putEEPROM() {
	eeprom_settings myeeprom;
	myeeprom.featureFlags = 0; // TODO: Implement feature flags on gameKey class
	// for (uint8_t i = 0; i < 8; i++) {
		myeeprom.devName[0] = "g";	// TODO: Implement device name on gameKey class
		myeeprom.devName[1] = "k";
		myeeprom.devName[2] = "L";
		myeeprom.devName[3] = "e";
		myeeprom.devName[4] = "f";
		myeeprom.devName[5] = "t";
		myeeprom.devName[6] = 0x00;
		myeeprom.devName[7] = 0x00;
	// }
	for (uint8_t i = 0; i < HW_COLS*HW_ROWS; i++) {
		myeeprom.buttons[i].binding = controller.buttons[i].getKeymap();
		myeeprom.buttons[i].controlType = controller.buttons[i].getControlType();
	}
	for (uint8_t i = 0; i < HW_AXES; i++) {
		myeeprom.analog[i].thresholdLow = controller.axes[i].getThresholdLow();
		myeeprom.analog[i].center = controller.axes[i].getCenter();
		myeeprom.analog[i].thresholdHigh = controller.axes[i].getThresholdHigh();
		myeeprom.analog[i].deadzone = controller.axes[i].getDeadzone();
		myeeprom.analog[i].keyDigitalUp = controller.axes[i].getKeyUp();
		myeeprom.analog[i].keyDigitalDown = controller.axes[i].getKeyDown();
		myeeprom.analog[i].analogMode = controller.axes[i].getAnalogMode();
		myeeprom.analog[i].invert = controller.axes[i].getInvert();
	}
	EEPROM.put(EEPROM_MEMORY_START,myeeprom);
}

void getEEPROM() {
	eeprom_settings myeeprom;
	EEPROM.get(EEPROM_MEMORY_START,myeeprom);
	//myeeprom.featureFlags = 0; // TODO: Implement feature flags on gameKey class
	// for (uint8_t i = 0; i < 8; i++) {
		// myeeprom.devName[0] = "g";	// TODO: Implement device name on gameKey class
		// myeeprom.devName[1] = "k";
		// myeeprom.devName[2] = "L";
		// myeeprom.devName[3] = "e";
		// myeeprom.devName[4] = "f";
		// myeeprom.devName[5] = "t";
		// myeeprom.devName[6] = 0x00;
		// myeeprom.devName[7] = 0x00;
	// }
	for (uint8_t i = 0; i < HW_COLS*HW_ROWS; i++) {
		controller.buttons[i].putKeymap(myeeprom.buttons[i].binding);
		controller.buttons[i].putControlType(myeeprom.buttons[i].controlType);
	}
	for (uint8_t i = 0; i < HW_AXES; i++) {
		controller.axes[i].setThresholdLow(myeeprom.analog[i].thresholdLow);
		controller.axes[i].setCenter(myeeprom.analog[i].center);
		controller.axes[i].setThresholdHigh(myeeprom.analog[i].thresholdHigh);
		controller.axes[i].setDeadzone(myeeprom.analog[i].deadzone);
		controller.axes[i].setKeyUp(myeeprom.analog[i].keyDigitalUp);
		controller.axes[i].setKeyDown(myeeprom.analog[i].keyDigitalDown);
		controller.axes[i].setAnalogMode(myeeprom.analog[i].analogMode);
		controller.axes[i].setInvert(myeeprom.analog[i].invert);
	}
}
