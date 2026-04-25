#ifndef TWAI_HANDLER_H
#define TWAI_HANDLER_H

#include <Arduino.h>
#include "driver/twai.h"
#include "config.h"

extern twai_general_config_t g_config;
extern twai_timing_config_t t_config;
extern twai_filter_config_t f_config;
//Enumerator for TWAI communication
enum COMMAND_ID{
  CAN_PACKET_SET_DUTY = 0,
  CAN_PACKET_SET_CURRENT = 1,
  CAN_PACKET_SET_CURRENT_BRAKE = 2,
  CAN_PACKET_SET_RPM = 3,
  CAN_PACKET_SET_POS = 4,
  CAN_PACKET_SET_CURRENT_REL = 10,
  CAN_PACKET_SET_CURRENT_BRAKE_REL = 11,
  CAN_PACKET_SET_CURRENT_HANDBRAKE = 12,
  CAN_PACKET_SET_CURRENT_HANDBRAKE_REL = 13
};

//Enumerates the actuators that are used within the code
enum ACTUATOR{
  FOOTREST_ACTUATOR = 0,
  BACKREST_ACTUATOR = 1,
  SEAT_ACTUATOR = 2
};

enum ACTUATOR_ACTION{
  ACTUATOR_EXTEND,
  ACTUATOR_RETRACT,
  ACTUATOR_STOP
};

uint32_t get_scaling(enum COMMAND_ID id);
twai_message_t createVESCMessage(uint8_t vescId, enum COMMAND_ID cmdId, float val);
twai_message_t createActuatorsMessage(uint8_t actId, ACTUATOR actuator, ACTUATOR_ACTION action);
void print_twai_status();
String print_vesc_message(twai_message_t *receivedMessage);

#endif