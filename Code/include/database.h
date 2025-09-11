#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "Errorausgabe.h"


#define DEVICE "ESP32"

int Error_database = 30;
int Error_database_initialisierung = 31;

InfluxDBClient client_upload(INFLUXDB_URL_SECRET, INFLUXDB_ORG_SECRET, INFLUXDB_BUCKET_SECRET, INFLUXDB_TOKEN_SECRET);
Point sensor("Steckdosenleiste02");




void Database_initialisieren() {

sensor.addTag("esp32", DEVICE);
timeSync(TZ_INFO_SECRET, "pool.ntp.org", "time.nis.gov");

if (client_upload.validateConnection()) {
} else {
  //errorausgabe(Error_database); // getLastErrorMessage() if Error = 30
}

}



void upload_to_database(float apparent_Power, float real_power, float reactive_power, float power_factor, int status, int rssi, float CPU_Temp, float PCB_temp, double verbrauch_gesamt) {

timeSync(TZ_INFO_SECRET, "pool.ntp.org", "time.nis.gov");

sensor.addField("Scheinleistung", apparent_Power);
sensor.addField("Wirkleistung", real_power);
sensor.addField("Blindleistung", reactive_power);
sensor.addField("Power_Faktor", power_factor);
sensor.addField("Status", status);
sensor.addField("RSSI", rssi);
sensor.addField("CPU_Temp", CPU_Temp);
sensor.addField("PCB_temp", PCB_temp);
sensor.addField("Uptime", millis()/1000);
sensor.addField("Heap", ESP.getFreeHeap());
sensor.addField("Gesamtverbrauch", verbrauch_gesamt);


if (!client_upload.writePoint(sensor)) {
    //errorausgabe(Error_database_initialisierung); // getLastErrorMessage() if Error = 31
}

}


void upload_to_database_error(int errorcode) {

timeSync(TZ_INFO_SECRET, "pool.ntp.org", "time.nis.gov");

sensor.addField("Error", errorcode);

}