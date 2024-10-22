#ifndef CONFIG_H
#define CONFIG_H

#include "driver/twai.h"
#include <WiFi.h> 

//Define button pins
#define BTN4 14
#define BTN3 26
#define BTN2 27
#define BTN1 32
#define PWR 33

//Define joystick pins
#define JOYSTICKX 34
#define JOYSTICKY 35

//System characteristics
extern float temperature;
extern float left_assembly_angle, right_assembly_angle;
extern int x_value;
extern int y_value;
extern int yMax, yMin, xMax, xMin;
extern int yUpperThresh, yLowerThresh, xUpperThresh, xLowerThresh;
extern int yMidLevel, xMidLevel;
extern int left_motor, right_motor, left_assembly, right_assembly, rear_assembly;
extern float voltage1, voltage2;
//extern int speed;
extern uint8_t maximumVoltage;
extern uint16_t system_begin_time;

// VESC communication settings
extern bool flag;  // Flag to see whether a message should be queued for transmission

//Variables for detecting button presses
extern uint16_t pressedTime1, releaseTime1, elapsedTime1;
extern uint16_t pressedTime2, releaseTime2, elapsedTime2;
extern uint16_t pressedTime3, releaseTime3, elapsedTime3;
extern uint16_t pressedTime4, releaseTime4, elapsedTime4;
extern bool shortPress1 , shortPress2, longPress1, longPress2;
extern bool shortPress3, shortPress4, longPress3, longPress4;
extern bool btn1;
extern bool btn2;
extern bool btn3;
extern bool btn4;
extern bool prevBtn1;
extern bool prevBtn2;
extern bool prevBtn3;
extern bool prevBtn4;

//Variables to toggle driving/ climbing mode
extern bool driveMode;
extern bool lastMode;

//Variables to control backrest and footrest angles
extern uint8_t minBackAngle;
extern uint8_t maxBackAngle;
extern uint8_t minFootAngle;
extern uint8_t maxFootAngle;
extern float backAngle;
extern float footAngle;

//Variables to toggle configuration mode
extern bool configMode;
extern uint8_t selection;
extern bool calibrating;

//Variables to measure timing
extern uint16_t startingTime, currentTime;
extern bool calibrationBegin;

// Hotspot settings
extern const char ssid[];
extern const char password[];

// Declare transmitted and received message
extern twai_message_t receivedMessage;
extern twai_message_t transmittedActuatorsMessage;

// Declare the status information struct
extern twai_status_info_t status_info;

extern int status;

#endif