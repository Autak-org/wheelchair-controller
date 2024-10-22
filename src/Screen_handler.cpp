#include "Screen_handler.h"
#include "TWAI_handler.h"

void drawImage(const uint16_t *image_data, int width, int height, TFT_eSPI *tft){
  //Function to display images on the TFT Screen
  tft->startWrite();
  for(int y=0; y<height; y++){
    for(int x=0; x<width; x++){
      uint16_t color = pgm_read_word(&image_data[y*width + x]);
      tft->drawPixel(x, y, color);
    }
  }
  tft->endWrite();
};

void createScreen(uint16_t speed, bool mode, TFT_eSPI *tft, TFT_eSprite *img){
  //Function to generate the main UI on the TFT Screen
  int color;

  /*tft->setCursor(30, 30);
  img->createSprite(30, 50);
  img->fillSprite(0xf80c);
  img->pushSprite(10, 105, TFT_BLACK);
  img->deleteSprite();
  img->createSprite(30, 50);
  img->fillSprite(0xf80c);
  img->pushSprite(280, 105, TFT_BLACK);
  img->deleteSprite();*/

  //Tachometer
  int tacho_posX = (240-140)/2-10;
  int tacho_posY = 70;

  img->createSprite(140, 100);
  img->fillSprite(0xf80c);
  img->setTextColor(TFT_WHITE, 0xf80c);
  img->setTextSize(2);
  // img->drawString("Speed", 25, 10, 4);
  img->setTextSize(2);
  img->drawString(String(speed/10), 30, 20, 4);
  img->drawString(String(speed%10), 62, 20, 4);
  img->setTextSize(1);
  img->drawString("kph", 95, 40, 4);
  img->pushSprite(tacho_posX, tacho_posY);
  img->deleteSprite();

  //Mode Selector
  int mode_posX = (240-170)/2;
  int mode_posY = 170;

  img->createSprite(170, 50);
  for(int y=0; y<50; y++){
    for(int x=0; x<170; x++){
      uint16_t color = mode?pgm_read_word(&selector_drive[y*170 + x]):pgm_read_word(&selector_stairs[y*170+x]);
      img->drawPixel(x, y, color);
    }
  }
  img->pushSprite(mode_posX, mode_posY);
  img->deleteSprite();

  int logo_posX = (240-160)/2;
  int logo_posY = 12;

  //Autak Logo
  img->createSprite(160,60);
  for(int y=0; y<60; y++){
    for(int x=0; x<160; x++){
      uint16_t color = pgm_read_word(&autaklogo[y*160+x]);
      img->drawPixel(x, y, color);
    }
  }
  img->pushSprite(logo_posX, logo_posY, 0xf8aa);
  img->deleteSprite();
};

void displayBatteries(uint8_t c1, uint8_t c2, TFT_eSPI *tft, TFT_eSprite *img){
  //Function to display the battery gauges on the TFT Screen
  int color;
  //Battery 1
  int bat1_posX = 5;
  int bat1_posY = (240-100)/2-20;

  img->createSprite(100,100);
  img->fillSprite(TFT_BLUE);
  img->setTextColor(TFT_WHITE, 0xf80c);
  img->drawRect(12, 50, 10, 35, TFT_BLACK);
  img->fillRect(13, 51, 8, 33, 0xf80c);
  int height = (float)c1/100 * 33;
  if (c1>50) color = TFT_GREEN;
  else if(c1 <50 && c1>20) color = TFT_ORANGE;
  else color = TFT_RED;
  img->fillRect(13, 84-height, 8, height, color);
  
  //New, print batteries separately
  img->setCursor(13, 40);
  img->setTextSize(1);
  img->print(F("B1"));
  img->setCursor(10, 87);
  img->printf(("%i%%"), (int)(c1));
  img->pushSprite(bat1_posX, bat1_posY, TFT_BLUE);
  img->deleteSprite();

  //Battery 2
  int bat2_posX = 240-40;
  int bat2_posY = (240-100)/2-20;

  img->createSprite(100,100);
  img->fillSprite(TFT_BLUE);
  img->setTextColor(TFT_WHITE, 0xf80c);
  img->drawRect(12, 50, 10, 35, TFT_BLACK);
  img->fillRect(13, 51, 8, 33, 0xf80c);
  height = (float)c2/100 * 33;
  if (c2>50) color = TFT_GREEN;
  else if(c2 <50 && c2>20) color = TFT_ORANGE;
  else color = TFT_RED;
  img->fillRect(13, 84-height, 8, height, color);
  img->setCursor(13, 40);
  img->setTextSize(1);
  img->print(F("B2"));
  img->setCursor(10, 87);
  img->printf(("%i%%"), (int)(c2));
  img->pushSprite(bat2_posX, bat2_posY, TFT_BLUE);
  img->deleteSprite();
};

void configureMode(TFT_eSPI *tft, TFT_eSprite *img, int config_state){
  //Function that generates the configuration menu on the TFT Screen and also implements the configuration menu functionality
  //Toggle all mode selectors to false
  bool calibrationMode = false;
  bool backrestMode = false;
  bool footrestMode = false;
  String menu[] = {"Calibration", "Footrest", "Backrest", "Seat", "Left ASM", "Right ASM", "Rear ASM"};

  //Navigation Arrows
  tft->setCursor(30, 30);
  img->createSprite(30, 50);
  /*img->drawLine(30, 0, 0, 25, TFT_WHITE);
  img->drawLine(0, 25, 30, 50, TFT_WHITE);
  img->pushSprite(10, 105, TFT_BLACK);*/
  img->drawLine(18, 0, 0, 15, TFT_WHITE);
  img->drawLine(0, 15, 18, 30, TFT_WHITE);
  img->pushSprite(20, 140, TFT_BLACK);
  img->deleteSprite();
  img->createSprite(30, 50);
  /*img->drawLine(0, 0, 30, 25, TFT_WHITE);
  img->drawLine(30, 25, 0, 50, TFT_WHITE);
  img->pushSprite(280, 105, TFT_BLACK);*/
  img->drawLine(0, 0, 18, 15, TFT_WHITE);
  img->drawLine(18, 15, 0, 30, TFT_WHITE);
  img->pushSprite(200, 140, TFT_BLACK);
  img->deleteSprite();
  //Autak Logo
  img->createSprite(160,60);
  for(int y=0; y<60; y++){
    for(int x=0; x<160; x++){
      uint16_t color = pgm_read_word(&autaklogo[y*160+x]);
      img->drawPixel(x, y, color);
    }
  }
  img->pushSprite(40, 10, 0xf8aa);
  img->deleteSprite();


  switch(config_state){
    case CALIBRATION:
      //Calibration menu
      if(!calibrating){
        img->createSprite(170, 50);
        img->fillSprite(0xf80c);
        img->setTextSize(2);
        img->drawString(menu[config_state], 15, 10);
        img->pushSprite(40, 180, TFT_BLACK);
        img->deleteSprite();

        img->createSprite(160, 90);
        img->fillSprite(0xf80c);
        img->print(F("Press Mode to\nbegin\ncalibration"));
        img->pushSprite(40, 90);
        img->deleteSprite();
        if(shortPress1){
          //If a short press is detected, toggle calibration flag on
          calibrating = true;
          Serial.println(F("Starting to calibrate"));
          startingTime = millis(); // Keep track of starting time
          Serial.print(F("Starting time: "));
          Serial.println(startingTime);
        }
      }else{
        //Keep track of current time (only serial, no graphics)
        currentTime = millis();
        Serial.print(F("Current time: "));
        Serial.print(currentTime);
        Serial.print(F(", Starting time: "));
        Serial.print(startingTime);
        Serial.print(F(", Elapsed time: "));
        Serial.println(currentTime - startingTime);
        if((currentTime - startingTime) < 8000){
          if(!calibrationBegin){
            yUpperThresh = INT_MIN;
            yLowerThresh = INT_MAX;
            yMin = INT_MAX;
            yMax = INT_MIN;
            calibrationBegin = true;
          }
      
          //Begin the calibration process
          if(currentTime - startingTime < 4000){
            img->createSprite(160, 90);
            img->fillSprite(0xf80c);
            img->setTextSize(2);
            img->print(F("Let the\njoystick rest\nfor 4 sec"));
            img->pushSprite(40, 90);
            img->deleteSprite();
            x_value = analogRead(JOYSTICKX);
            y_value = analogRead(JOYSTICKY);
            if(x_value>xUpperThresh) xUpperThresh = x_value;
            if(x_value<xLowerThresh) xLowerThresh = x_value;
            if(y_value>yUpperThresh) yUpperThresh = y_value;
            if(y_value<yLowerThresh) yLowerThresh = y_value;
            xMidLevel = (xUpperThresh + xLowerThresh)/2;
            yMidLevel = (yUpperThresh + yLowerThresh)/2;
          }
          else{
            img->createSprite(160, 90);
            img->fillSprite(0xf80c);
            img->setTextSize(2);
            img->print(F("Move the\njoystick in\ncircles for\n4 sec"));
            img->pushSprite(40, 90);
            img->deleteSprite();
            x_value = analogRead(JOYSTICKX);
            y_value = analogRead(JOYSTICKY);
            if(x_value>xMax) xMax = x_value;
            if(x_value<xMin) xMin = x_value;
            if(y_value>yMax) yMax = y_value;
            if(y_value<yMin) yMin = y_value;
          }
        }
        else{
          calibrationBegin = false; 
          calibrating = false; //When the calibration is done, toggle calibration mode off
          yUpperThresh = yUpperThresh + 50;
          yLowerThresh = yLowerThresh - 50;
          yMax = yMax - 75;
          yMin = yMin + 75;
          xUpperThresh = xUpperThresh + 50;
          xLowerThresh = xLowerThresh - 50;
          xMax = xMax - 75;
          xMin = xMin + 75;
        } 
      }
      break;
    case BACKREST:
      //Backrest adjust menu
      img->createSprite(170, 50);
      img->fillSprite(0xf80c);
      img->setTextSize(2);
      img->drawString(menu[config_state], 37, 10);
      img->pushSprite(40, 180, TFT_BLACK);
      img->deleteSprite();
      //Change back angle depending on the joystick input
      if(analogRead(JOYSTICKY)>yMax-400){
        //transmittedActuatorsMessage = createActuatorsMessage(99, true, ACTUATOR_EXTEND);
        img->createSprite(200, 100);
        img->fillSprite(0xf80c);
        img->drawLine(50, 50, 100, 0, TFT_WHITE);
        img->drawLine(100, 0, 150, 50, TFT_WHITE);
        img->drawLine(50, 70, 100, 20, TFT_WHITE);
        img->drawLine(100, 20, 150, 70, TFT_WHITE);
        img->pushSprite(60, 70);
        img->deleteSprite();
      }
      else if(analogRead(JOYSTICKY)<yMin+400){
        //transmittedActuatorsMessage = createActuatorsMessage(99, true, ACTUATOR_RETRACT);
        img->createSprite(200, 100);
        img->fillSprite(0xf80c);
        img->drawLine(50, 30, 100, 80, TFT_WHITE);
        img->drawLine(100, 80, 150, 30, TFT_WHITE);
        img->drawLine(50, 50, 100, 100, TFT_WHITE);
        img->drawLine(100, 100, 150, 50, TFT_WHITE);
        img->pushSprite(60, 70);
        img->deleteSprite();
      }
      else{
        //transmittedActuatorsMessage = createActuatorsMessage(99, true, ACTUATOR_STOP);
        img->createSprite(160, 90);
        img->fillSprite(0xf80c);
        img->setTextColor(TFT_WHITE, 0xf80c);
        //img->print(F("Move the joystick up or down to adjust the backrest."));
        img->print(F("Adjust the\nbackrest."));
        img->pushSprite(40,90);
        img->deleteSprite();
      }
      if(backAngle>=maxBackAngle) backAngle = maxBackAngle;
      if(backAngle<=minBackAngle) backAngle = minBackAngle;  
      break;
    case FOOTREST:
      //Footrest adjust menu
      img->createSprite(170, 50);
      img->fillSprite(0xf80c);
      img->setTextSize(2);
      img->drawString(menu[config_state], 37, 10);
      img->pushSprite(40, 180, TFT_BLACK);
      img->deleteSprite();
      //Change foot rest angle depending on the joystick input
      if(analogRead(JOYSTICKY)>yMax-400){
        //transmittedActuatorsMessage = createActuatorsMessage(99, false, ACTUATOR_EXTEND);
        img->createSprite(200, 100);
        img->fillSprite(0xf80c);
        img->drawLine(50, 50, 100, 0, TFT_WHITE);
        img->drawLine(100, 0, 150, 50, TFT_WHITE);
        img->drawLine(50, 70, 100, 20, TFT_WHITE);
        img->drawLine(100, 20, 150, 70, TFT_WHITE);
        img->pushSprite(60, 70);
        img->deleteSprite();
      }
      else if(analogRead(JOYSTICKY)<yMin+400){
        //transmittedActuatorsMessage = createActuatorsMessage(99, false, ACTUATOR_RETRACT);
        img->createSprite(200, 100);
        img->fillSprite(0xf80c);
        img->drawLine(50, 30, 100, 80, TFT_WHITE);
        img->drawLine(100, 80, 150, 30, TFT_WHITE);
        img->drawLine(50, 50, 100, 100, TFT_WHITE);
        img->drawLine(100, 100, 150, 50, TFT_WHITE);
        img->pushSprite(60, 70);
        img->deleteSprite();
      }
      else{
        //transmittedActuatorsMessage = createActuatorsMessage(99, false, ACTUATOR_STOP);
        img->createSprite(160, 90);
        img->fillSprite(0xf80c);
        img->setTextColor(TFT_WHITE, 0xf80c);
        img->print(F("Adjust the\nfootrest."));
        img->pushSprite(40,90);
        img->deleteSprite();
      }
      if(footAngle>=maxFootAngle) footAngle = maxFootAngle;
      if(footAngle<=minFootAngle) footAngle = minFootAngle;
      break;
    case SEAT:
      //Footrest adjust menu
      img->createSprite(170, 50);
      img->fillSprite(0xf80c);
      img->setTextSize(2);
      img->drawString(menu[config_state], 37, 10);
      img->pushSprite(40, 180, TFT_BLACK);
      img->deleteSprite();
      //Change foot rest angle depending on the joystick input
      if(analogRead(JOYSTICKY)>yMax-400){
        //transmittedActuatorsMessage = createActuatorsMessage(99, false, ACTUATOR_EXTEND);
        img->createSprite(200, 100);
        img->fillSprite(0xf80c);
        img->drawLine(50, 50, 100, 0, TFT_WHITE);
        img->drawLine(100, 0, 150, 50, TFT_WHITE);
        img->drawLine(50, 70, 100, 20, TFT_WHITE);
        img->drawLine(100, 20, 150, 70, TFT_WHITE);
        img->pushSprite(60, 70);
        img->deleteSprite();
      }
      else if(analogRead(JOYSTICKY)<yMin+400){
        //transmittedActuatorsMessage = createActuatorsMessage(99, false, ACTUATOR_RETRACT);
        img->createSprite(200, 100);
        img->fillSprite(0xf80c);
        img->drawLine(50, 30, 100, 80, TFT_WHITE);
        img->drawLine(100, 80, 150, 30, TFT_WHITE);
        img->drawLine(50, 50, 100, 100, TFT_WHITE);
        img->drawLine(100, 100, 150, 50, TFT_WHITE);
        img->pushSprite(60, 70);
        img->deleteSprite();
      }
      else{
        //transmittedActuatorsMessage = createActuatorsMessage(99, false, ACTUATOR_STOP);
        img->createSprite(160, 90);
        img->fillSprite(0xf80c);
        img->setTextColor(TFT_WHITE, 0xf80c);
        img->print(F("Adjust the\nseat."));
        img->pushSprite(40,90);
        img->deleteSprite();
      }
      if(footAngle>=maxFootAngle) footAngle = maxFootAngle;
      if(footAngle<=minFootAngle) footAngle = minFootAngle;
      break;
    case ASSEMBLY_LEFT:
      img->createSprite(170, 50);
      img->fillSprite(0xf80c);
      img->setTextSize(2);
      img->drawString(menu[config_state], 15, 10);
      img->pushSprite(40, 180, TFT_BLACK);
      img->deleteSprite();

      img->createSprite(160, 90);
      img->fillSprite(0xf80c);
      img->setTextSize(2);
      //img->drawString("Adjust the left wheel assembly", 10, 10);
      img->setTextColor(TFT_WHITE, 0xf80c);
      img->print(F("Adjust the\nleft wheel\nassembly."));
      img->pushSprite(40, 90, TFT_BLACK);
      img->deleteSprite();
      break;
    case ASSEMBLY_RIGHT:
      img->createSprite(170, 50);
      img->fillSprite(0xf80c);
      img->setTextSize(2);
      img->drawString(menu[config_state], 15, 10);
      img->pushSprite(40, 180, TFT_BLACK);
      img->deleteSprite();

      img->createSprite(160, 90);
      img->fillSprite(0xf80c);
      img->setTextSize(2);
      //img->setCursor(10, 10);
      //img->drawString("Adjust the right wheel assembly", 10, 10);
      img->setTextColor(TFT_WHITE, 0xf80c);
      img->print(F("Adjust the\nright wheel\nassembly."));
      img->pushSprite(40, 90, TFT_BLACK);
      img->deleteSprite();
      break;
    case ASSEMBLY_REAR:
      img->createSprite(170, 50);
      img->fillSprite(0xf80c);
      img->setTextSize(2);
      img->drawString(menu[config_state], 15, 10);
      img->pushSprite(40, 180, TFT_BLACK);
      img->deleteSprite();

      img->createSprite(160, 90);
      img->fillSprite(0xf80c);
      img->setTextSize(2);
      //img->drawString("Adjust the rear wheel assembly", 10, 10);
      img->setTextColor(TFT_WHITE, 0xf80c);
      img->print(F("Adjust the\nrear wheel\nassembly."));
      img->pushSprite(40, 90, TFT_BLACK);
      img->deleteSprite();
      break;
    default:
      break;
  }
};