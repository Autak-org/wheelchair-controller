#include "TWAI_handler.h"

// Configuration structures
twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_4, GPIO_NUM_16, TWAI_MODE_NORMAL);
twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

uint32_t get_scaling(enum COMMAND_ID id){
  /* This function calculates the scaling of the TWAI command's value for communication with the VESC, in
  compliance with the documentation found at "https://github.com/vedderb/bldc/blob/master/documentation/comm_can.md".
    Arguments:
      - enum COMMAND_ID id: The command's ID
    Returns:
      uint32_t : The scaling factor of the command
  */
  if (id == CAN_PACKET_SET_DUTY || id == CAN_PACKET_SET_CURRENT_REL || id == CAN_PACKET_SET_CURRENT_BRAKE_REL || id == CAN_PACKET_SET_CURRENT_HANDBRAKE_REL) return 100000;
  if (id == CAN_PACKET_SET_CURRENT || id == CAN_PACKET_SET_CURRENT_BRAKE || id == CAN_PACKET_SET_CURRENT_HANDBRAKE) return 1000;
  if (id == CAN_PACKET_SET_POS) return 1000000;
  if (id == CAN_PACKET_SET_RPM) return 1;
  return 0;
};

twai_message_t createVESCMessage(uint8_t vescId, enum COMMAND_ID cmdId, float val){
  /* This function constructs the TWAI message that will be transmitted to the VESCs
    Arguments:
      - uint8_t vescID: The VESC's ID.
      - enum COMMAND_ID cmdId: The commands ID in compliance with the documentation found at "https://github.com/vedderb/bldc/blob/master/documentation/comm_can.md"
      - float val: The desired value
  */
  twai_message_t message;
  uint32_t scale = get_scaling(cmdId);
  int32_t value = val * scale;

  //Set the message's properties
  message.extd = 1;
  message.identifier = (vescId + ((uint8_t)cmdId << 8));
  message.data_length_code = 4;

  message.data[0] = (int8_t)(value >> 24);
  message.data[1] = (int8_t)(value >> 16);
  message.data[2] = (int8_t)(value >> 8);
  message.data[3] = (int8_t)value;

  return message;
};

twai_message_t createActuatorsMessage(uint8_t actId, bool isBackrest, ACTUATOR_ACTION action){
  /* This function constructs the TWAI message that will be processed by the actuators controller to control the actuators
    Arguments:
      - uint8_t actId: The actuator's ID
      - bool isBackrest: Determines wether the backrest or the footrest is controlled
      - enum ACTUATOR_ACTION action: The command that determines the actuator's action.
  */
  twai_message_t message;
  message.extd = 1;
  message.identifier = actId;
  message.data_length_code = 2;
  
  if(isBackrest){
    if(action == ACTUATOR_EXTEND) message.data[0] = 0b1000;
    else if(action == ACTUATOR_RETRACT) message.data[0] = 0b0100;
    else message.data[0] = 0b0000;
  }
  else{
    if(action == ACTUATOR_EXTEND) message.data[0] = 0b0010;
    else if (action == ACTUATOR_RETRACT) message.data[0] = 0b1;
    else message.data[0] = 0b0000;
  }

  return message;
}

void print_twai_status(){ 
  /* This function prints the TWAI bus status in the Serial Monitor
    Arguments:
      - void
    Returns:
      - void
  */
  twai_status_info_t status_info;
  if (twai_get_status_info(&status_info) == ESP_OK) {
    Serial.print("State: ");
    Serial.println(status_info.state);
    Serial.print("Msgs to TX: ");
    Serial.println(status_info.msgs_to_tx);
    Serial.print("Msgs to RX: ");
    Serial.println(status_info.msgs_to_rx);
    Serial.print("TX Errors: ");
    Serial.println(status_info.tx_error_counter);
    Serial.print("RX Errors: ");
    Serial.println(status_info.rx_error_counter);
    Serial.print("Bus Errors: ");
    Serial.println(status_info.bus_error_count);
  } else {
    Serial.println("Failed to get TWAI status");
  }
};

String print_vesc_message(twai_message_t *receivedMessage){
  /* This function is used to construct an html template to be sent on the ESP's server. It may display several data based on the received VESC messages.
    Arguments:
      - twai_message_t *receivedMessage: Pointer to the received message
    Returns:
      - String: The html template to be sent to the server, in string format.
  */
  uint16_t maxCurrentValue = 0xFFFF;

  String returnString = "";
  /*The code below is commented out as it implements different testing functionality. Adjust the function's logic as needed for communication through the server*/
  // int motorID = (receivedMessage->identifier) & 0b1111;
  
  // int dataLength = receivedMessage->data_length_code;

  // int erpm = (receivedMessage->data[0]<<24) | (receivedMessage->data[1]<<16 | (receivedMessage->data[2]) << 8 | (receivedMessage->data[3]));

  // float current = (receivedMessage->data[4]<<8 | (receivedMessage->data[5]));
  // float dutyCycle = (receivedMessage->data[6]<<8 | (receivedMessage->data[7]));

  // erpm = erpm;
  // current /= 10;
  // dutyCycle = dutyCycle / 1000;
  
  // returnString += "<h1>Motor Data</h1> <div> Motor ID: ";
  // returnString += motorID;
  // returnString += "<br>Command Identifier: ";
  // returnString += receivedMessage->identifier >> 8;
  // returnString += "<br>ERPM: ";
  // returnString += erpm;
  // returnString += " RPM<br>Current: ";
  // returnString += current;
  // returnString += " A<br>Duty Cycle: ";
  // returnString += dutyCycle;
  // returnString += "% <br></div>"; 
  // returnString += "<script>window.location.reload();</script>";
  returnString += "<p>Yval</p>";
  returnString += "</p>";
  returnString += analogRead(JOYSTICKY);
  returnString += "</p>";
  returnString += "<script>window.location.reload();</script>";
  return returnString;
};