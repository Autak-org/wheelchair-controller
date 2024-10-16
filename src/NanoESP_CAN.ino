/*  This is the main software of the Autak Wheelchair. It controlls the joystick system, sends commands to the motors and communicates with the actuators controller.
    The following code was developed by Gkeralnto Balla, with contributions from Niklas Degener and Khanh Tran. The last update was made on Friday, 27/09/2024. All files and information related
    to the project can be found on the Github repository "https://github.com/Gkeralnto/autak".
    
    The ESP32 is connected with a TFT SPI screen, four buttons and a joystick. It is also connected with a CAN transceiver for TWAI bus communication.
    
    The pinout can be found in "config.h" file and "User_Setup.h". This is an overview:
    - Button 1 -> GPIO32
    - Button 2 -> GPIO27
    - Button 3 -> GPIO26
    - Power Button -> GPIO33 (Not being used, PWR can be deleted from "config.h")
    
    - Joystick Horizontal Axis -> GPIO34
    - Joystick Vertical Axis -> GPIO35

    - CAN Transciever Tx -> GPIO4
    - CAN Transciever Rx -> GPIO 16

    - TFT Screen MISO -> Not used
    - TFT Screen MOSI -> GPIO23
    - TFT Screen SCK -> GPIO18
    - TFT Screen CS -> GPIO17
    - TFT Screen DC -> GPIO19
    - TFT Screen RST -> GPIO21
    - TFT Screen LED -> Connected to 3.3V supply. 
*/

#include <stdint.h>
#include "driver/twai.h"
#include <WiFi.h> 
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <cmath>
#include <EEPROM.h>
#include "config.h"
#include "TWAI_handler.h"
#include "Screen_handler.h"
#include "PID_Controller.h"

#define LED 22

IPAddress apIP(192, 168, 1, 1); // IP address of the access point

// Web Server
WebServer server(80);

//Initialize pid controllers
PID pid_left(0, 0, 0), pid_right(0, 0, 0);

// TFT screen settings
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);


RTC_DATA_ATTR int bootCount = 0;
twai_message_t transmittedVESCMessage[6];

void print_wakeup_reason(){

  /* This function print the wakeup reason from deepsleep()/
      Arguments:
        -void
      Returns:
        -void
  */
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}


void get_joystick_position(int &xval, int &yval){
  /* This joystick maps the position of the joystick to RPM values for the motors, to be read by the arcade_drive() function.
      Arguments:
        - int &xval: Reference to an int variable which will store the RPM value for x_axis movement
        - int &yval: Reference to an int variable which will store the RPM value for y_axis movement
      Returns:
        - void
  */
  int x = analogRead(JOYSTICKX);
  int y = analogRead(JOYSTICKY);

  //Correct the values according to thresholds and maximum/ minimum values
  if(x >= xMidLevel && x < xUpperThresh) x = xUpperThresh;
  if(x < xMidLevel && x > xLowerThresh) x = xLowerThresh;
  if(y >= yMidLevel && y < yUpperThresh) y = yUpperThresh;
  if(y < yMidLevel && y > yLowerThresh) y = yLowerThresh;

  if(x >= xMax) x = xMax;
  if(x <= xMin) x = xMin;
  if(y>= yMax) y = yMax;
  if(y<= yMin) y = yMin;

  //Set value of x and y axi within the range (-1000, 1000)
  if(x <= xLowerThresh) xval = map(x, (long)xMin, (long)xLowerThresh, -3000, 0);
  if(x >= xUpperThresh) xval = map(x, (long)xUpperThresh, (long)xMax, 0, 3000);
  if(y <= yLowerThresh) yval = map(y, (long)yMin, (long)yLowerThresh, -3000, 0);
  if(y >= yUpperThresh) yval = map(y, (long)yUpperThresh, (long)yMax, 0, 3000);
};

void arcade_drive(int x_axis, int y_axis, int& left_motor, int& right_motor){
  /* Function that calculates the motor inputs according to arcade drive mode. The algorithm and more information on arcade driving
  can be found at: "https://xiaoxiae.github.io/Robotics-Simplified-Website/drivetrain-control/arcade-drive/".
    Arguments:
      - int x_axis: The joystick's x axis value
      - int y_axis: The joystick's y axis value
      - int &left_motor: Reference to integer variable that stores the left motor input value
      - int &right_motor: Reference to integer variable that stores the right motor input value 
    Returns:
      - void    
  */
  int maximum = max(abs(x_axis), abs(y_axis));
  int sum = y_axis + x_axis, difference = y_axis - x_axis;

  if(y_axis >= 0){
    if(x_axis >= 0){
      left_motor = maximum;
      right_motor = difference;
    }
    else{
      left_motor = sum;
      right_motor = maximum;
    }
  }
  else{
    if(x_axis >= 0){
      left_motor = sum;
      right_motor = -maximum;
    }
    else{
      left_motor = -maximum;
      right_motor = difference;
    }
  }
}

void stair_climbing_mode(int& left_assembly, int& right_assembly){
  /* This function implements the stair climbing mote. The current algorithm controls each of the three assemblies' rotations based on certain input.
    The joystick's y axis controls the left assembly, x axis controlls the right assembly and the left and right buttons control the rear assembly.
      Arguments:
        - int& left_assembly: Reference to the integer variable which stores the left assembly's motor speed
        - int& right_assembly: Reference to the integer variable which stores the right assembly's motor speed
      Returns:
        - void
  */
  int y_val = analogRead(JOYSTICKY);
  int x_val = analogRead(JOYSTICKX);
  if(y_val > yMax-200){
    left_assembly = 1500;
    right_assembly = 1500;
  }
  else if(y_val < yMin + 200){
    left_assembly = -1500;
    right_assembly = - 1500;
  }
  else {left_assembly = 0; right_assembly = 0;}

  if(x_val > xMax - 200){
    rear_assembly = 1500;
  }
  else if(x_val < xMin + 200){
    rear_assembly = -1500;
  }
  else rear_assembly = 0;

  if(digitalRead(BTN2)){
    left_motor = 2000;
    right_motor = 2000;
  }
  else if(digitalRead(BTN3)){
    left_motor = -2000;
    right_motor = -2000;
  }
  else {left_motor = 0; right_motor = 0;};
}

void handleRoot(){
  /* Root handler for server communication*/
  String data = print_vesc_message(&receivedMessage);
  server.send(200, "text/html", data);
}

void shutdown(){
  /* This function is called before the ESP enters deepsleep. It makes sure that all systems are properly shut down.
    Arguments: 
      - void
    Returns:
      - void
  */

  // Set the motor RPM to 0 (safety precaution)
  transmittedVESCMessage[0] = createVESCMessage(7, CAN_PACKET_SET_RPM, 0);
  transmittedVESCMessage[1] = createVESCMessage(8, CAN_PACKET_SET_RPM, 0);
  transmittedVESCMessage[2] = createVESCMessage(9, CAN_PACKET_SET_RPM, 0);
  transmittedVESCMessage[3] = createVESCMessage(10, CAN_PACKET_SET_RPM, 0);
  transmittedVESCMessage[4] = createVESCMessage(11, CAN_PACKET_SET_RPM, 0);

  //Shut down TWAI communication
  if(twai_stop() == ESP_OK) Serial.println("TWAI driver stopped succesfully");
  else Serial.println("Failed to stop TWAI driver.");

  //Uninstall TWAI driver
  if(twai_driver_uninstall() == ESP_OK) Serial.println("TWAI driver uninstalled succesfully");
  else Serial.println("Failed to uninstall TWAI driver");

  //Stop the HTTP server
  Serial.println("Stopping HTTP server");
  server.stop();
  Serial.println("HTTP Server stopped");

  //Stop the ESP's access point
  Serial.println("Shutting down AP");
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("AP is disconnected");

  //Clear the TFT screen
  tft.fillScreen(TFT_BLACK);
  Serial.println("TFT screen is cleared");

  //Turn of the power
  esp_deep_sleep_start();
  Serial.println("PWR pin set to LOW");

  Serial.println("Shutdown Completed");
}

void printBin32(uint32_t aByte) {
  for (int8_t aBit = 31; aBit >= 0; aBit--)
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
}

void printBin8(uint32_t aByte) {
  for (int8_t aBit = 7; aBit >= 0; aBit--)
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
}

bool actuatorControllerReady = true;
int lastMsg = 0;
int maxMsg = 5;

void main_loop() {
  /* This is the main loop of the program. As deepsleep is used, this function is called within an infinite loop in the setup phase of the ESP.
    Arguments:
      - void
    Returns:
      - void
  */


  // Set the TWAI communication's error flag to true on each iteration
  flag = true;

  //Read the button states
  btn1 = digitalRead(BTN1);
  btn2 = digitalRead(BTN2);
  btn3 = digitalRead(BTN3);
  btn4 = digitalRead(BTN4);

  //Receive the TWAI data from the actuators controller. The data includes the two battery voltage levels, the electronics compartment's current temperature 
  // and the potentiometers' position.
  for(int i = 0; i < 4; i++){
    if(twai_receive(&receivedMessage, pdMS_TO_TICKS(20)) == ESP_OK){
      if(receivedMessage.identifier == 100){
        voltage1 = receivedMessage.data[0]<<24 | receivedMessage.data[1] << 16 | receivedMessage.data[2] << 8 | receivedMessage.data[3];
        Serial.println("Voltage 1 message received");
      }
      else if(receivedMessage.identifier == 101){
        voltage2 = receivedMessage.data[0]<<24 | receivedMessage.data[1] << 16 | receivedMessage.data[2] << 8 | receivedMessage.data[3];
        Serial.println("Voltage 2 message received");
      }
      else if(receivedMessage.identifier == 102){
        temperature = receivedMessage.data[0]<<24 | receivedMessage.data[1] << 16 | receivedMessage.data[2] << 8 | receivedMessage.data[3];
        Serial.println("Temperature message received");
      }
      else if(receivedMessage.identifier == 42){
        memcpy(&left_assembly_angle, &receivedMessage.data[0], sizeof(float));
        memcpy(&right_assembly_angle, &receivedMessage.data[4], sizeof(float));
        Serial.print("Left Assembly Angle: ");
        Serial.print(left_assembly_angle);
        Serial.print(" | Right Assembly Angle: ");
        Serial.println(right_assembly_angle);
      }else if(receivedMessage.identifier == 103){
        int value = receivedMessage.data[0]<<24 | receivedMessage.data[1] << 16 | receivedMessage.data[2] << 8 | receivedMessage.data[3];
        if(value == 4){
          actuatorControllerReady = true;
          Serial.println("Ready");
        }
      }else{
        Serial.print("Received something from: ");
        printBin32(receivedMessage.identifier);
        /*Serial.print(" ");
        printBin8(receivedMessage.data[0]);
        Serial.print(" ");
        printBin8(receivedMessage.data[1]);
        Serial.print(" ");
        printBin8(receivedMessage.data[2]);
        Serial.print(" ");
        printBin8(receivedMessage.data[3]);
        Serial.print("\n");*/
      }
    }
  }

  //Get the joysticks position
  get_joystick_position(x_value, y_value);

  // Implement functionality for configureation mode, drive mode and stair climbing mode.
  if(!configMode){
    if(driveMode){
      /*This is the drive mode. It reads the joysticks position, calculates the motor speeds and constructs the TWAI messages to control the motors. The assemblies are only 
        controlled by the PID control loops and not from user input*/

      /*The following code is commented out for troubleshooting purposes. *_assembly_target sets the target angle for the two front assemblies, to be used 
        in the PID control loop. The algorithm has a margin of 10 degrees of error, which can be adjusted in the following lines.*/
      // float left_assembly_target = 90.0;
      // float right_assembly_target = 90.0;

      // if(abs(left_assembly_target - left_assembly_angle) < 10) left_assembly = 0;
      // else left_assembly = pid_left.PID_Control(left_assembly_angle, left_assembly_target);

      // if(abs(right_assembly_target - right_assembly_target) < 10) right_assembly = 0;
      // else right_assembly = pid_right.PID_Control(right_assembly_angle, right_assembly_target);
      arcade_drive(x_value, y_value, left_motor, right_motor);
      transmittedVESCMessage[0] = createVESCMessage(7, CAN_PACKET_SET_RPM, 0);
      transmittedVESCMessage[1] = createVESCMessage(8, CAN_PACKET_SET_RPM, 0);
      transmittedVESCMessage[2] = createVESCMessage(9, CAN_PACKET_SET_RPM, -right_motor);
      transmittedVESCMessage[3] = createVESCMessage(10, CAN_PACKET_SET_RPM, 0);
      transmittedVESCMessage[4] = createVESCMessage(11, CAN_PACKET_SET_RPM, -left_motor);
    }
    else{
      /*This is the stair climbing mode. It calculates the assemblies motors' speeds according to the joystick's position and constructs 
      the TWAI controls to be transmitted. The wheelchair can not be driven or steered in this mode*/
      stair_climbing_mode(left_assembly, right_assembly);
      //transmittedVESCMessage[0] = createVESCMessage(7, CAN_PACKET_SET_RPM, rear_assembly);
      //transmittedVESCMessage[1] = createVESCMessage(8, CAN_PACKET_SET_RPM, left_assembly);
      transmittedVESCMessage[2] = createVESCMessage(9, CAN_PACKET_SET_RPM, -right_motor);
      transmittedVESCMessage[3] = createVESCMessage(10, CAN_PACKET_SET_RPM, right_assembly);
      transmittedVESCMessage[4] = createVESCMessage(11, CAN_PACKET_SET_RPM, -left_motor);
    }
    maxMsg = 5;
  }
  else{
    /*This is the configure mode. If the user enters configure mode, the motors' speed is set to 0 for safety reasons*/
    transmittedVESCMessage[0] = createVESCMessage(7, CAN_PACKET_SET_RPM, 0);
    transmittedVESCMessage[1] = createVESCMessage(8, CAN_PACKET_SET_RPM, 0);
    transmittedVESCMessage[2] = createVESCMessage(9, CAN_PACKET_SET_RPM, 0);
    transmittedVESCMessage[3] = createVESCMessage(10, CAN_PACKET_SET_RPM, 0);
    transmittedVESCMessage[4] = createVESCMessage(11, CAN_PACKET_SET_RPM, 0);
    if(y_value > 0){
      transmittedVESCMessage[5] = createActuatorsMessage(99, false, ACTUATOR_EXTEND);
    }else if(y_value < 0){
      transmittedVESCMessage[5] = createActuatorsMessage(99, false, ACTUATOR_RETRACT);
    }else{
      transmittedVESCMessage[5] = createActuatorsMessage(99, false, ACTUATOR_STOP);
    }
    maxMsg = 6;
  }

    // Get the status information of the node
  twai_get_status_info(&status_info);
  if (status_info.state != TWAI_STATE_RUNNING) {
    // If the node is in a non-running state, initiate recovery and start the node again

    //Avoiding recovery as a hotfix for the ERRATA error
    //twai_initiate_recovery();
    twai_driver_uninstall();
    twai_driver_install(&g_config, &t_config, &f_config);
    Serial.println("Initiating recovery");
    esp_err_t startResult = twai_start();
    if (startResult == ESP_OK)
      Serial.println("Device started successfully");
    else if (startResult == ESP_ERR_INVALID_STATE) {
      // If the restart is unsuccessful, display an error and set the error flag to false
      Serial.println("Device failed to start: ESP_ERR_INVALID_STATE");
      flag = false;
    };
  }

  if (flag) {
    // Execute this block only if the TWAI error flag is true

    //Transmit the TWAI messages for the motors
    if(!configMode){
      for(int i=lastMsg; i<maxMsg; i++){
        //esp_err_t transmit_result = twai_transmit(&(transmittedVESCMessage[i]), pdMS_TO_TICKS(20));
        if(actuatorControllerReady){
          if(i==maxMsg-1) lastMsg = 0;

          esp_err_t transmit_result = twai_transmit(&(transmittedVESCMessage[i]), pdMS_TO_TICKS(20));
          if(transmit_result == ESP_OK){
            Serial.print("Message No: ");
            Serial.println(i);
            /*Serial.print(transmittedVESCMessage[i].identifier, HEX);
            Serial.print(transmittedVESCMessage[i].data[0], HEX);
            Serial.print(transmittedVESCMessage[i].data[1], HEX);
            Serial.print(transmittedVESCMessage[i].data[2], HEX);
            Serial.println(transmittedVESCMessage[i].data[3], HEX);*/
            Serial.print(transmittedVESCMessage[i].identifier);
            Serial.print(transmittedVESCMessage[i].data[0]);
            Serial.print(transmittedVESCMessage[i].data[1]);
            Serial.print(transmittedVESCMessage[i].data[2]);
            Serial.println(transmittedVESCMessage[i].data[3]);
          }
          else{ 
            Serial.print(F("Could not transmit VESC message No: "));
            Serial.println(i);
          }
          actuatorControllerReady = false; 
        }else{
          lastMsg = i;
          break;
        }
      }
    }
    //for actuator
    if(configMode){
      //esp_err_t transmit_result = twai_transmit(&(transmittedActuatorsMessage), pdMS_TO_TICKS(20));
      if(actuatorControllerReady){
        esp_err_t transmit_result = twai_transmit(&(transmittedVESCMessage[5]),pdMS_TO_TICKS(20));
        if(transmit_result == ESP_OK){
          Serial.print("Actuator message: ");
          Serial.print(transmittedVESCMessage[5].identifier);
          Serial.print(transmittedVESCMessage[5].data[0]);
          Serial.println(transmittedVESCMessage[5].data[1]);
          //Serial.print(transmittedVESCMessage[5].data[2]);
          //Serial.println(transmittedVESCMessage[5].data[3]);
          /*Serial.print(transmittedActuatorsMessage.identifier);
          Serial.print(" ");
          Serial.print(transmittedActuatorsMessage.data[0]);
          Serial.print(transmittedActuatorsMessage.data[1]);
          Serial.print(transmittedActuatorsMessage.data[2]);
          Serial.println(transmittedActuatorsMessage.data[3]);*/
          actuatorControllerReady = false;
        }
        else{ 
          Serial.print(F("Could not transmit actuator message: "));
        }
      }
    }

    //Show alerts
    uint32_t alerts;
    if (twai_read_alerts(&alerts, pdMS_TO_TICKS(10)) == ESP_OK) {
      // Handle specific alerts
      if (alerts & TWAI_ALERT_ARB_LOST) {
          printf("Arbitration lost! Retransmission might occur.\n");
      }
      if (alerts & TWAI_ALERT_TX_FAILED) {
          printf("Transmission failed! Message might be retransmitted.\n");
      }
      if (alerts & TWAI_ALERT_ERR_PASS) {
          printf("Entered error-passive state! Error count increasing.\n");
      }
      if (alerts & TWAI_ALERT_BUS_OFF) {
          printf("Bus-off condition detected! Communication stopped.\n");
      }
      if (alerts & TWAI_ALERT_RECOVERY_IN_PROGRESS) {
          printf("Recovery from bus-off state in progress.\n");
      }
      /*if (alerts & TWAI_ALERT_RECOVERY_SUCCESS) {
          printf("Successfully recovered from bus-off state.\n");
      }*/
    }

    /*The lines below are commented out. They transmit the actuators' TWAI message to the actuators controller. Uncomment when the actuators controller's behavior is as desired*/
    // if(twai_transmit(&transmittedActuatorsMessage, pdMS_TO_TICKS(50)) == ESP_OK) Serial.println(F("Actuators message transmitted")); // Reduced timeout for quicker response
    // else Serial.println("Could not transmit actuators message");
    twai_get_status_info(&status_info); // Update the TWAI bus status info after message transmission
  }

  //Toggle drive mode and configure mode depending on short or long button press detection
  if(!configMode && shortPress1) driveMode = !driveMode;
  if(lastMode==configMode && longPress1) configMode = !configMode;

  //Enter configuration mode if configMode becomes true, otherwise display the main screen
  if(configMode){
    //Serial.println("config");
    configureMode(&tft, &img);
  }
  else{
    //Serial.println("screen");
    createScreen(abs((int)20), driveMode, &tft, &img);
  }

  //Always have the battery gauges on display 
  displayBatteries(voltage1, voltage2, &tft, &img);

  //Reset the detected states to false
  longPress1 = false;
  longPress2 = false;
  longPress3 = false;
  longPress4 = false;
  shortPress1 = false;
  shortPress2 = false;
  shortPress3 = false;
  shortPress4 = false;

  /*In the following lines the algorithm for short and long button presses is implemented. Each button press and type of press triggers some functionality*/
  if(millis()-releaseTime1 > 300){
    if(!prevBtn1 && btn1){
      pressedTime1 = millis();
    }
    else if(prevBtn1 && btn1){
      elapsedTime1 = millis() - pressedTime1;
      if(elapsedTime1 > 1200){
        Serial.println("Btn1 long press detected.");
        tft.fillScreen(TFT_BLACK);
        while(btn1){
          //Wait for button to be released after long press detection
          btn1 = digitalRead(BTN1);
        }
        tft.fillScreen(0xf80c);
        releaseTime1 = millis();
        longPress1 = true;
      }
    }
    else if(prevBtn1 && !btn1){
      releaseTime1 = millis();
      if(releaseTime1 - pressedTime1 <500){
        Serial.println("Btn1 short press detected.");
        shortPress1 = true;
      }
    }
  }

  if(millis()-releaseTime2 > 300){
    if(!prevBtn2 && btn2){
      pressedTime2 = millis();
    }
    else if(prevBtn2 && btn2){
      elapsedTime2 = millis() - pressedTime2;
      if(elapsedTime2 > 1200){
        releaseTime2 = millis();
        longPress2 = true;
      }
    }
    else if(prevBtn2 && !btn2){
      releaseTime2 = millis();
      if(releaseTime2 - pressedTime2 <500){
        Serial.println("Btn 2 short press detected.");
        shortPress2 = true;
      }
    }
  }

  if(millis()-releaseTime3 > 300){
    if(!prevBtn3 && btn3){
      pressedTime3 = millis();
    }
    else if(prevBtn3 && btn3){
      elapsedTime3 = millis() - pressedTime3;
      if(elapsedTime3 > 1200){
        releaseTime3 = millis();
        longPress3 = true;
      }
    }
    else if(prevBtn3 && !btn3){
      releaseTime3 = millis();
      if(releaseTime3 - pressedTime3 <500){
        Serial.println("Btn 3 short press detected.");
        shortPress3 = true;
      }
    }
  }

  if(millis()-releaseTime4 > 300){
    if(!prevBtn4 && btn4){
      pressedTime4 = millis();
    }
    else if(prevBtn4 && btn4){
      elapsedTime4 = millis() - pressedTime4;
      if(elapsedTime4>2000){
        longPress4 = true;
        Serial.println("Btn4 long press detected");
        tft.fillScreen(TFT_BLACK);
        while(btn4){
          btn4 = digitalRead(BTN4);
        }
        releaseTime4 = millis();
        longPress4 = true;
      }
    }
    else if(prevBtn4 && !btn4){
      releaseTime4 = millis();
      if(releaseTime4 - pressedTime4  < 500){
        Serial.println("Btn 4 short press detected.");
        shortPress4 = true;
      }
    }
  }
  
  //Save the current states to variables
  prevBtn2 = btn2;
  prevBtn1 = btn1;
  prevBtn3 = btn3;
  prevBtn4 = btn4;
  lastMode = configMode;

  /* Display the battery compartment's temperature. The temperature is received via TWAI communication from the actuators controller. 
  The logic for overheat protection is pending.*/
  Serial.print("Battery Compartment Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C.");
}


void setup() {
  //Contains the setup code

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 1);

  //Increment boot number and print it every reboot
  ++bootCount;
  if(bootCount == 1) shutdown();
  Serial.println("Boot number: " + String(bootCount));

  //Set the pinModes for the joystick and the buttons
  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, LOW);
  pinMode(JOYSTICKX, INPUT);
  pinMode(JOYSTICKY, INPUT);
  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(BTN3, INPUT);
  pinMode(BTN4, INPUT);

  // Initialize TFT display
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  tft.init();
  tft.setRotation(3);
  //drawImage(image_data, 320, 240, &tft);
  //Replace image by smaller Autak logo
  tft.fillScreen(0xf80c);
  img.createSprite(160,60);
  for(int y=0; y<60; y++){
    for(int x=0; x<160; x++){
      uint16_t color = pgm_read_word(&autaklogo[y*160+x]);
      img.drawPixel(x, y, color);
    }
  }
  img.pushSprite((240-160)/2, (240-60)/2, 0xf8aa);
  img.deleteSprite();

  // Start serial communication for debugging
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("Serial Started");


  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    Serial.println("Driver Installed");
  else
    Serial.println("Failed to install driver");

  // Start TWAI driver
  if (twai_start() == ESP_OK)
    Serial.println("Driver Started");
  else
    Serial.println("Driver Failed to start");

  // Set the motor RPM at 0 on setup as a safety precaution
  transmittedVESCMessage[0] = createVESCMessage(7, CAN_PACKET_SET_RPM, 0);
  transmittedVESCMessage[1] = createVESCMessage(8, CAN_PACKET_SET_RPM, 0);
  transmittedVESCMessage[2] = createVESCMessage(9, CAN_PACKET_SET_RPM, 0);
  transmittedVESCMessage[3] = createVESCMessage(10, CAN_PACKET_SET_RPM, 0);
  transmittedVESCMessage[4] = createVESCMessage(11, CAN_PACKET_SET_RPM, 0);
  transmittedActuatorsMessage = createActuatorsMessage(99, true, ACTUATOR_STOP);

  // Configure the Access Point
  Serial.println("Configuring Access Point...");
  status = WiFi.mode(WIFI_AP);

  // Set the SSID and the password of the AP
  WiFi.softAP(ssid, password);

  // Configure the IP address of the AP
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP Address: ");
  Serial.println(myIP);

  // Start the server
  server.on("/", handleRoot);   // Define handler for root URL
  server.begin();               // Start the server
  Serial.println("HTTP server started");
  delay(4000);
  system_begin_time = millis();
  driveMode = true;
  tft.fillScreen(0xf80c);
  while(1){
    main_loop();
    if(shortPress4){
      shortPress4 = false;
      printf("Shutting down...");
      shutdown();
    }
  }
}

void loop(){
  //This never runs due to deepsleep
}