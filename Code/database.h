#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>


#define DEVICE "ESP32"

int Error_database = 30;
int Error_database_initialisierung = 31;

InfluxDBClient client_upload(INFLUXDB_URL_SECRET, INFLUXDB_ORG_SECRET, INFLUXDB_BUCKET_SECRET, INFLUXDB_TOKEN_SECRET, InfluxDbCloud2CACert);
Point sensor("Steckdosenleiste_V2.0");




void Database_initialisieren() {

sensor.addTag("esp32", DEVICE);
timeSync(TZ_INFO_SECRET, "pool.ntp.org", "time.nis.gov");

if (client_upload.validateConnection()) {
} else {
  errorausgabe(Error_database); // getLastErrorMessage() if Error = 30
}

}



void upload_to_database(float apparent_Power, float real_power, float reactive_power, float power_factor, int status, int rssi, float CPU_Temp, float PCB_temp) {

timeSync(TZ_INFO_SECRET, "pool.ntp.org", "time.nis.gov");

sensor.addField("Scheinleistung", apparent_Power);
sensor.addField("Wirkleistung", real_power);
sensor.addField("Blindleistung", reactive_power);
sensor.addField("Power_Faktor", power_factor);
sensor.addField("Status", status);
sensor.addField("RSSI", rssi);
sensor.addField("CPU_Temp", CPU_Temp);
sensor.addField("PCB_temp", PCB_temp);


if (!client_upload.writePoint(sensor)) {
    errorausgabe(Error_database_initialisierung); // getLastErrorMessage() if Error = 31
}


}