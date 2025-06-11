
#include <DRV8830.h>
#include "Errorausgabe.h"
#include "display.h"

int einschaltverzoegerung = 60000;

#define einschaltstrombegrenzer_I2C_ADDR 0x64
Adafruit_DRV8830 einschaltstrombegrenzer;


#define master_I2C_ADDR 0x62
Adafruit_DRV8830 master;


#define section01_I2C_ADDR 0x66
Adafruit_DRV8830 section01;


#define section02_I2C_ADDR 0x68
Adafruit_DRV8830 section02;



struct Relaistreiber {
bool einschaltstrombegrenzer;
bool master;
bool section01;
bool section02;
};

Relaistreiber data;


int Error_einschaltstrombegrenzer = 1;
int Error_master = 2;
int Error_section01 = 3;
int Error_section02 = 4;


void Relaistreiber_initialisieren() {

    if (! einschaltstrombegrenzer.begin(einschaltstrombegrenzer_I2C_ADDR)) {
        errorausgabe(Error_einschaltstrombegrenzer);
      }
    
      if (! master.begin(master_I2C_ADDR)) {
        errorausgabe(Error_master);
      }
    
      if (! section01.begin(section01_I2C_ADDR)) {
        errorausgabe(Error_section01);
      }
    
      if (! section02.begin(section02_I2C_ADDR)) {
        errorausgabe(Error_section02);
      }

}


void Einschaltstrombegrenzer(bool status){

    if(!status){
    einschaltstrombegrenzer.run(FORWARD);
    data.einschaltstrombegrenzer = false;
    }else{
    einschaltstrombegrenzer.run(BACKWARD);
    data.einschaltstrombegrenzer = true;
    }

    einschaltstrombegrenzer.setSpeed(255);

    delay(10);

    einschaltstrombegrenzer.run(RELEASE);

}

void Master(bool status){

    if(!status){
        master.run(FORWARD);
        data.master = false;
    }else{
        master.run(BACKWARD);
        data.master = true;
    }

    master.setSpeed(255);

    delay(10);

    master.run(RELEASE);

}


void Section01(bool status){

    if(!status){
        section01.run(FORWARD);
        data.section01 = false;
    }else{
        section01.run(BACKWARD);
        data.section01 = true;
    }

    section01.setSpeed(255);

    delay(10);

    section01.run(RELEASE);

}


void Section02(bool status){

    if(!status){
        section02.run(FORWARD);
        data.section02 = false;
    }else{
        section02.run(BACKWARD);
        data.section02 = true;
    }

    section02.setSpeed(255);

    delay(10);

    section02.run(RELEASE);

}



void Einschalten() {
    Einschaltstrombegrenzer(true);
    delay(50);
    Master(true);
    Section01(true);
    Section02(true);
    delay(200);
    Einschaltstrombegrenzer(false);
}



void Ausschalten() {
    Einschaltstrombegrenzer(true);

    tft.fillScreen(TFT_BLACK);
  
    tft.setTextFont(1); 
    tft.setFreeFont(&FreeSansBold9pt7b);
  
    tft.setCursor(25, 20);
    tft.print("Ausschalten");
    #
    
    tft.drawRect(20, 30, 120, 25, TFT_WHITE);
  
    for (int x = 10; x < 120; x++) {
      tft.drawLine(x, 30, x, 55, TFT_WHITE);
  
      if(digitalRead(16) == LOW){
        Einschaltstrombegrenzer(false);
        einschaltverzoegerung = millis() + 30000;
        return;
      }
  
      delay(20);
    }


    Master(false);
    Section01(false);
    Section02(false);
    delay(2000); //Vielleicht später error
}


void Abschnitt01(){

    if(data.section01){
        Section01(false);
    }else{
        Section01(true);
    }
}

void Abschnitt02(){

    if(data.section02){
        Section02(false);
    }else{
        Section02(true);
    }
}

Relaistreiber get_Relaistreiber() {
return data;
}