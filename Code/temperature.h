#include <math.h>
#include <Arduino.h>
#include "Errorausgabe.h"


int Temp_PCB_Pin = 33;

float temp_CPU, temp_PCB = -1;


int Error_internal_temp_sensor = 40;
int Error_external_temp_sensor = 41;


float calculate_temperature (int raw) {

    float A = 1.11492089e-3;
    float B = 2.372075385e-4;
    float C = 6.954079529e-8;
  
  
    float V =  raw / 4096 * 3.3;
  
    float R = (10000 * V ) / (3.3 - V);
    
  
    float logR  = log(R);
    float R_th = 1.0 / (A + B * logR + C * logR * logR * logR);
  
    float kelvin = R_th - V*V/(2.5 * R)*1000;
    float celsius = kelvin - 273.15;
  
    return celsius;
  
}


void Temperatursensor_initialisieren() {

    temp_CPU = temperatureRead();

    temp_PCB = calculate_temperature(analogRead(Temp_PCB_Pin));

    if(temp_CPU == -1){
        errorausgabe(Error_internal_temp_sensor);
    }

    if(temp_PCB == -1){
        errorausgabe(Error_external_temp_sensor);
    }
}



float get_CPU_Temp () {

    temp_CPU = temperatureRead();
    
    return temp_CPU;

}


float get_PCB_Temp () {
//!Überarbeiten
    temp_PCB = 0;
    
    return temp_PCB;

}



