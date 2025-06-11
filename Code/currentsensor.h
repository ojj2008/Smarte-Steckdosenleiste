#include <SparkFun_ACS37800_Arduino_Library.h>
#include <Wire.h>
#include "Errorausgabe.h"
#include <math.h>



struct CurrentsensorData {
    float spannung = 0;
    float ampere = 0;
    float active_power = 0;
    float reactive_power = 0;
    float apparent_power = 0;
    float power_factor = 0;
    float phasenwinkel = 0;
    bool position_phasenwinkel;
    bool einspeisung;
};

struct CurrentsensorData_Verlauf {
    float apparent_power_verlauf[150] = {};
    float active_power_verlauf[150] = {};
    float powerfactor_verlauf[150] = {};

    int apparent_power_verlauf_length = sizeof(apparent_power_verlauf) / sizeof(apparent_power_verlauf[0]);
    int active_power_verlauf_length = sizeof(active_power_verlauf) / sizeof(active_power_verlauf[0]);
    int powerfactor_verlauf_length = sizeof(powerfactor_verlauf) / sizeof(powerfactor_verlauf[0]);

};


struct CurrentsensorData_Kurve {

    float voltage_kurve[100] = {};
    float current_kurve[100] = {};

    int voltage_kurve_length = sizeof(voltage_kurve) / sizeof(voltage_kurve[0]);
    int current_kurve_length = sizeof(current_kurve) / sizeof(current_kurve[0]);

};






#define currentsensor_I2C_ADDR 0x60
ACS37800 currentsensor;


int Error_currentsensor = 10;


void Currentsensor_initialisieren() {

    if (!currentsensor.begin(currentsensor_I2C_ADDR))
    {
      errorausgabe(Error_currentsensor);
    }

    currentsensor.setBypassNenable(false, true);
    currentsensor.setDividerRes(3640000);
    currentsensor.setSenseRes(3900);
    currentsensor.setCurrentRange(30);
  


}


CurrentsensorData& get_Currentsensor() {
    static CurrentsensorData data;

    currentsensor.readRMS(&data.spannung, &data.ampere);
    currentsensor.readPowerActiveReactive(&data.active_power, &data.reactive_power);
    currentsensor.readPowerFactor(&data.apparent_power, &data.power_factor, &data.position_phasenwinkel, &data.einspeisung);

    data.active_power = data.active_power * -1;

    if(data.power_factor >= -1 && data.power_factor <= 1){
        data.phasenwinkel = acos(data.power_factor) * 57.3; //   (180 / Pi)
    } else {
        data.phasenwinkel = 0;
    }


    return data;
}

CurrentsensorData_Verlauf& update_Currentsensorverlauf() {
    static CurrentsensorData_Verlauf data_verlauf;  // Statisch, damit die Werte erhalten bleiben
    CurrentsensorData data = get_Currentsensor();

    // Array nach rechts verschieben (neueste Werte an Index 0)
    for (int i = 149; i > 0; i--) {
        data_verlauf.active_power_verlauf[i] = data_verlauf.active_power_verlauf[i - 1];
        data_verlauf.apparent_power_verlauf[i] = data_verlauf.apparent_power_verlauf[i - 1];
        data_verlauf.powerfactor_verlauf[i] = data_verlauf.powerfactor_verlauf[i - 1];
    }

    // Neue Messwerte einfügen
    data_verlauf.active_power_verlauf[0] = data.active_power;
    data_verlauf.apparent_power_verlauf[0] = data.apparent_power;
    data_verlauf.powerfactor_verlauf[0] = data.power_factor;

    return data_verlauf;
}



/*
CurrentsensorData_Kurve update_Kurve() {
    CurrentsensorData_Kurve data_kurve;

    float voltage;
    float current;

    


    for(int i = 0; i < data_kurve.voltage_kurve_length; i++){

        currentsensor.readInstantaneous(&voltage, &current);


        data_kurve.voltage_kurve[i] = voltage;
        data_kurve.current_kurve[i] = current;

        delayMicroseconds(1);

    }


    
    return data_kurve;

}

*/


CurrentsensorData_Kurve& update_Kurve() {

    static CurrentsensorData_Kurve data_kurve;


        float voltage, current;
        float previousVoltage = 0;
        unsigned long startTime = millis();
        unsigned long timeout = 100;
    

        while (millis() - startTime < timeout) {
            currentsensor.readInstantaneous(&voltage, &current);
            

            if (previousVoltage < -100 && voltage >= -100) {



                for(int i = 0; i < data_kurve.voltage_kurve_length; i++){

                    currentsensor.readInstantaneous(&voltage, &current);
            
            
                    data_kurve.voltage_kurve[i] = voltage;
                    data_kurve.current_kurve[i] = -current;
            
            
                }
            

                
            }
            
            previousVoltage = voltage;
        }
    

    return data_kurve;

}