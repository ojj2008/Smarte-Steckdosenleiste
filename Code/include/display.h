

#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

#include "bitmap.h"



/*
#define TFT_CS   5
#define TFT_RST  4
#define TFT_DC   2
#define TFT_MOSI 23
#define TFT_SCLK 18

#define TFT_BLACK       0x0000 
#define TFT_NAVY        0x000F 
#define TFT_DARKGREEN   0x03E0 
#define TFT_DARKCYAN    0x03EF 
#define TFT_MAROON      0x7800 
#define TFT_PURPLE      0x780F 
#define TFT_OLIVE       0x7BE0 
#define TFT_LIGHTGREY   0xD69A 
#define TFT_DARKGREY    0x7BEF 
#define TFT_BLUE        0x001F 
#define TFT_GREEN       0x07E0 
#define TFT_CYAN        0x07FF 
#define TFT_RED         0xF800 
#define TFT_MAGENTA     0xF81F 
#define TFT_YELLOW      0xFFE0 
#define TFT_WHITE       0xFFFF 
#define TFT_ORANGE      0xFDA0 
#define TFT_GREENYELLOW 0xB7E0 
#define TFT_PINK        0xFE19
#define TFT_BROWN       0x9A60 
#define TFT_GOLD        0xFEA0 
#define TFT_SILVER      0xC618 
#define TFT_SKYBLUE     0x867D 
#define TFT_VIOLET      0x915C 
*/



	



TFT_eSPI tft = TFT_eSPI();


void display_initialisieren() {

  tft.init();
  tft.setRotation(1);
	tft.setSwapBytes(true);

}


void display_start_screen() {
    
    tft.fillScreen(TFT_BLACK);

    tft.pushImage(0, 0, 160, 80, opening_screen);

}



void display_easter_egg() {
    
    tft.fillScreen(TFT_BLACK);

    tft.pushImage(0, 0, 160, 80, easter_egg);

}


void display_current_power(int power) {

	tft.fillScreen(TFT_BLACK);
	  
	int  hunderter = (power / 100) % 10;
	int  zehner = (power / 10) % 10;
	int  einer = power % 10;

	uint16_t color = TFT_GREEN;

	if(power >= 250){color = TFT_YELLOW;}
	if(power >= 500){color = TFT_RED;}

	  	  
	if(power >= 1000){
		tft.drawBitmap(10, 5, Above, 42, 10, color);
	  
		hunderter = 9;
		zehner = 9;
		einer = 9;
	  
	}else{
		tft.drawBitmap(10, 5, Around, 51, 10, color);
	}
	  
	if(hunderter != 0){
		tft.drawBitmap(10, 25, ZahlenArray[hunderter], 25, 30, color);
	}
	  
	tft.drawBitmap(35, 25, ZahlenArray[zehner], 25, 30, color);
	tft.drawBitmap(60, 25, ZahlenArray[einer], 25, 30, color);

	tft.drawBitmap(90, 25, Watt, 30, 30, color);


}


uint16_t rgbTo565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}




// Funktion zur Berechnung der Farbe basierend auf einem Wert von 0 bis 999
uint16_t getGradientColor(int value) {

    uint8_t r, g, b;
    
    if (value <= 500) {
        // Übergang von Grün (0,255,0) zu Orange (255,165,0)
        float ratio = value / 500.0;
        r = (uint8_t)(ratio * 255);
        g = (uint8_t)(255 - ratio * 90); // von 255 zu 165
        b = 0;
    } else {
        // Übergang von Orange (255,165,0) zu Rot (255,0,0)
        float ratio = (value - 500) / 499.0;
        r = 255;
        g = (uint8_t)(165 - ratio * 165); // von 165 zu 0
        b = 0;
    }
    
    return rgbTo565(r, g, b);
}



void display_power_history(float* power_history) {

  
  #define X_START  35  // X-axis starting position
  #define X_END   150  // X-axis ending position
  #define Y_START  5  // Y-axis starting position
  #define Y_END    55  // Y-axis ending position

  unsigned int numPoints = abs(X_START - X_END);

  int max_wert = 0;
  int max_wert_10 = 0;

  float display_array[numPoints];

  for(int i = 0; i < 10; i++){
	if(power_history[i] > max_wert_10)
	{max_wert_10 = power_history[i];}
  }


if(max_wert_10 >= 1000){

	display_current_power(power_history[0]);

	}else{


  for(int i = 0; i < numPoints - 1; i++){
  	if(power_history[i] > 900){display_array[i] = 0;}else{display_array[i] = power_history[i];}
  	if(display_array[i] > max_wert){max_wert = display_array[i];}
  }
	


  int hunderter = ceil(max_wert / 100.0) * 100;

  tft.fillScreen(TFT_BLACK);

  tft.setTextFont(1); 
  tft.setFreeFont(&FreeSansBold9pt7b); // Apply custom font
  tft.setTextColor(TFT_WHITE);

  // Draw X and Y axes using variables
  tft.drawLine(X_START, Y_END, X_END, Y_END, TFT_WHITE); // X-axis
  tft.drawLine(X_START, Y_START, X_START, Y_END, TFT_WHITE);  // Y-axis

  // Draw arrowheads
  tft.drawLine(X_END, Y_END, X_END - 5, Y_END - 3, TFT_WHITE);
  tft.drawLine(X_END, Y_END, X_END - 5, Y_END + 3, TFT_WHITE);
  tft.drawLine(X_START, Y_START, X_START - 3, Y_START + 5, TFT_WHITE);
  tft.drawLine(X_START, Y_START, X_START + 3, Y_START + 5, TFT_WHITE);

  // Axis Labels
  tft.setCursor(145, 75);
  tft.print("SEC");

  tft.setCursor(1, 12);
  tft.print("W");

  // X-axis Labels (Shifted left for alignment)
  tft.setCursor(55, 75);
  tft.print("60");

  tft.setCursor(105, 75);
  tft.print("120");


  // Y-axis Labels (Shifted left for alignment)
  tft.setCursor(8, 71);
  tft.print("0");
  tft.setCursor(0, 35);
  tft.print(hunderter);

  
  for (int i = 0; i < numPoints - 1; i++) {

  if(power_history[i] != 0){

	int x1 = X_START + i;
	int y1 = map(power_history[i], 0, hunderter, Y_END, Y_START);

	uint16_t color = getGradientColor(power_history[i]);
	
	tft.drawRect(x1, y1, 2, 2, color);
  }
  }

}


}



