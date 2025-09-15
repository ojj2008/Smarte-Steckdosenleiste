#include <Arduino.h>

#include "relaistreiber.h"
#include "currentsensor.h"
#include "wlan.h"
//#include "database.h" beresits in Errorausgabe.h
#include "Errorausgabe.h"
//#include "display.h" bereits in relaytreiber.h
#include "temperature.h"
#include "taster.h"
// #include "eeprom_own.h" bereits in wifi.h




unsigned long last_database_upload = 0;
unsigned int interval_database_upload = 30000;

unsigned long last_wifi_check = 0;
unsigned int interval_wifi_check = 5000;

unsigned long last_array_check = 0;
unsigned int interval_array_check = 1000;

unsigned long last_display_check = 0;
unsigned int interval_display_check = 1000;

unsigned long last_taster_check = 0;
unsigned int interval_taster_check = 100;

unsigned long last_ausschalt_check = 60000;
unsigned int interval_ausschalt_check = 2000;



unsigned long last_gesamtverbrauch_update = 0;
unsigned int interval_gesamtverbrauch_update = 500;

double Gesamtverbrauch_startwert = 0;
double Gesamtverbrauch = 0;



int status = 1;
/*
1 = aktiv
2 = kurz vorm ausschalten
*/


int display_status = 1;
/*
1 = W
2 = Diagramm
3 = easter Egg
*/

//int einschaltverzoegerung = 60000; breist in relaistreiber.h
int ausschaltverzoegerung = 5;
int ausschalt_watt_threshold = 25;
double power_factor_threshold = 0.5;





void setup() {

  pinMode(15, INPUT);



  if(debug){
    Serial.begin(115200);
    Serial.println("Test Serialle Verbindung");
  }

  Wire.begin();

  Taster_initialisieren();
  Relaistreiber_initialisieren();
  Einschalten();

  display_initialisieren();
  display_start_screen();
  

  Currentsensor_initialisieren();

  Wifi_initialisieren();
  Wifi_connect();

  OTA_initialisieren();
  Mdns_initialisieren();
  Database_initialisieren();
  Webserver_initialisieren();

  Temperatursensor_initialisieren();

  eeprom_initialisieren();
  eeprom_read_double(addr_gesamtverbrauch, Gesamtverbrauch_startwert);

  eeprom_read_int(addr_verz, ausschaltverzoegerung);
  eeprom_read_int(addr_thres, ausschalt_watt_threshold);
  eeprom_read_int(addr_einschalt, einschaltverzoegerung);
  eeprom_read_double(addr_power_factor, power_factor_threshold);


}


void loop() {





  if(millis() - last_wifi_check >= interval_wifi_check){

  if(WiFi.status() != WL_CONNECTED){
    errorausgabe(Error_Wifi_Verbindungsabbruch);
    Wifi_connect();
  }
  last_wifi_check = millis();

  }

  if(millis() - last_database_upload >= interval_database_upload){

    CurrentsensorData &data = get_Currentsensor();


    upload_to_database(data.apparent_power, data.active_power, data.reactive_power, data.power_factor, status, Wifi_RSSI(), get_CPU_Temp(), get_PCB_Temp(), Gesamtverbrauch_startwert + Gesamtverbrauch);

    last_database_upload = millis();

  }

  /*
  
  Jezt bei Display integriert

  if(millis() - last_array_check >= interval_array_check){

    update_Currentsensorverlauf();

    last_array_check = millis();

  }

  */

  if(millis() - last_taster_check >= interval_taster_check){

       unsigned long current_time = millis();

       if (checkTimeout && (current_time - firstPressTime > multiPressTimeout)) {
           if (pressCount == 1) {
               if(display_status == 1){
                   CurrentsensorData_Verlauf &data_verlauf = update_Currentsensorverlauf();
                   display_power_history(data_verlauf.active_power_verlauf);
                   display_status = 2;
               } else if(display_status == 2){
                   CurrentsensorData &data = get_Currentsensor();
                   display_current_power(data.active_power);
                   display_status = 1;
               }
              
           } else if (pressCount == 3) {
               display_easter_egg();
               delay(3000);
             if(display_status == 1){
                 CurrentsensorData_Verlauf &data_verlauf = update_Currentsensorverlauf();
                 display_power_history(data_verlauf.active_power_verlauf);
                 display_status = 2;
             }else if(display_status == 2){
                 CurrentsensorData &data = get_Currentsensor();
                 display_current_power(data.active_power);
                 display_status = 1;
             }
           }
          
           // Reset für das nächste Drücken
           pressCount = 0;
           checkTimeout = false;
        }

    last_taster_check = millis();

  }


  if(millis() - last_display_check >= interval_display_check){


    if(display_status == 1){
      CurrentsensorData_Verlauf &data_verlauf = update_Currentsensorverlauf();
      display_power_history(data_verlauf.active_power_verlauf);
  }
  if(display_status == 2){
      CurrentsensorData &data = get_Currentsensor();
      display_current_power(data.active_power);
  }

    last_display_check = millis();

  }



  if(millis() - last_ausschalt_check >= interval_ausschalt_check && millis() > einschaltverzoegerung){

    CurrentsensorData &data = get_Currentsensor();

      if(data.power_factor < power_factor_threshold){

        CurrentsensorData_Verlauf &data_verlauf = update_Currentsensorverlauf();

        int max_wert = 0;


        for(int i = 0; i < ausschaltverzoegerung; i++){
          if(data_verlauf.active_power_verlauf[i] > max_wert)
          {max_wert = data_verlauf.active_power_verlauf[i];}
          }
        
          if(max_wert < ausschalt_watt_threshold){

            status = 2;

            eeprom_write_double(addr_gesamtverbrauch, Gesamtverbrauch_startwert + Gesamtverbrauch);
            upload_to_database(data.apparent_power, data.active_power, data.reactive_power, data.power_factor, status, Wifi_RSSI(), get_CPU_Temp(), get_PCB_Temp(), Gesamtverbrauch_startwert + Gesamtverbrauch);
            Ausschalten();
          }
      }

      
  last_ausschalt_check = millis();

  }

  if(millis() - last_gesamtverbrauch_update >= interval_gesamtverbrauch_update){

    CurrentsensorData &data = get_Currentsensor();

    if(data.active_power > 0){
      Gesamtverbrauch += (data.active_power * (interval_gesamtverbrauch_update / 1000.0 / 3600.0)); // in Wh
    }

    last_gesamtverbrauch_update = millis();

  }
  


  ArduinoOTA.handle();
  server.handleClient();

  delay(5);
}

