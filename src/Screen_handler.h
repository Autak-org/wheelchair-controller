#ifndef SCREEN_HANDLER_H
#define SCREEN_HANDLER_H

#include "config.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include "image_data.h"
#include "autaklogo.h"
#include "selector_stairs.h"
#include "selector_drive.h"


void drawImage(const uint16_t *image_data, int width, int height, TFT_eSPI *tft);
void createScreen(uint16_t speed, bool mode, TFT_eSPI *tft, TFT_eSprite *img);
void displayBatteries(float v1, float v2, TFT_eSPI *tft, TFT_eSprite *img);
void configureMode(TFT_eSPI *tft, TFT_eSprite *img);

#endif