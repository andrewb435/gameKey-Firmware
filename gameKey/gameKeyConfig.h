#ifndef __GAMEKEYCONFIG_H__
#define __GAMEKEYCONFIG_H__

#define HW_COLS 6
#define HW_ROWS 5
#define HW_AXES 2

#define CONTROL_ENABLE_PIN 10

typedef struct matrixData
{
	bool data[HW_COLS][HW_ROWS];
};

#endif