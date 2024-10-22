#include "config.h"

//System characteristics
float temperature = 0;
float left_assembly_angle = 0, right_assembly_angle = 0;
int x_value = 0;
int y_value = 0;
int yMax = 3500, yMin = 180, xMax = 3510, xMin = 190;
int yUpperThresh = 1860, yLowerThresh = 1780, xUpperThresh = 1840, xLowerThresh = 1760;
int yMidLevel = 1820, xMidLevel = 1800;
int left_motor = 0, right_motor = 0, left_assembly = 0, right_assembly = 0, rear_assembly = 0;
uint8_t maximumVoltage = 25;
uint16_t system_begin_time;

// VESC communication settings
bool flag;  // Flag to see whether a message should be queued for transmission

//Variables for detecting button presses
uint16_t pressedTime1 = 0, releaseTime1 = 0, elapsedTime1 = 0;
uint16_t pressedTime2 = 0, releaseTime2 = 0, elapsedTime2 = 0;
uint16_t pressedTime3 = 0, releaseTime3 = 0, elapsedTime3 = 0;
uint16_t pressedTime4 = 0, releaseTime4 = 0, elapsedTime4 = 0;
bool shortPress1 = false, shortPress2 = false, longPress1 = false, longPress2 = false;
bool shortPress3 = false, shortPress4 = false, longPress3 = false, longPress4 = false;
bool btn1;
bool btn2;
bool btn3;
bool btn4;
bool prevBtn1;
bool prevBtn2;
bool prevBtn3;
bool prevBtn4;

//Variables to toggle driving/ climbing mode
bool driveMode;
bool lastMode;

//Variables to control backrest and footrest angles
uint8_t minBackAngle = 0;
uint8_t maxBackAngle = 90;
uint8_t minFootAngle = 0;
uint8_t maxFootAngle = 90;
float backAngle = 0;
float footAngle = 0;

//Variables to toggle configuration mode
bool configMode = false;
uint8_t selection = 0;
bool calibrating = false;

//Variables to measure timing
uint16_t startingTime, currentTime;
bool calibrationBegin = false;

// Hotspot settings
const char ssid[] = "Nano ESP32";
const char password[] = "NORAW106";

// Declare transmitted and received message
twai_message_t receivedMessage;
twai_message_t transmittedActuatorsMessage;

// Declare the status information struct
twai_status_info_t status_info;

int status = WL_IDLE_STATUS;