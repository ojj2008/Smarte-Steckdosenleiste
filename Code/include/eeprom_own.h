#include <EEPROM.h>
#include "Errorausgabe.h"



#define EEPROM_SIZE 68

/*
#define EEPROM_ADDR_DOUBLE          0  // 1x double (8 Bytes)
#define EEPROM_ADDR_CHAR_ARRAY_1    8  // 1x char[20] (20 Bytes)
#define EEPROM_ADDR_CHAR_ARRAY_2    28 // 1x char[20] (20 Bytes)
#define EEPROM_ADDR_INT_1           48 // 1x int (4 Bytes)
#define EEPROM_ADDR_INT_2           52 // 1x int (4 Bytes)
#define EEPROM_ADDR_INT_3           56 // 1x int (4 Bytes)
#define EEPROM_ADDR_INT_4           60 // 1x int (4 Bytes)
*/


#define addr_gesamtverbrauch         0  // 1x double (8 Bytes)
#define addr_ssid    8  // 1x char[20] (20 Bytes)
#define addr_pass    28 // 1x char[20] (20 Bytes)
#define addr_verz           48 // 1x int (4 Bytes)
#define addr_thres           52 // 1x int (4 Bytes)
#define addr_einschalt           56 // 1x int (4 Bytes)
#define addr_power_factor         60 // 1x double (8 Bytes)



int Error_eeprom = 50;
int Error_eeprom_initialisierung = 51;



void eeprom_initialisieren() {

  if (!EEPROM.begin(EEPROM_SIZE)) {
    errorausgabe(Error_eeprom_initialisierung);
  }


}

void eeprom_write_double(int address, double value) {

  if (address < 0 || address + sizeof(double) > EEPROM_SIZE) {
    errorausgabe(Error_eeprom);
    return;
  }

  EEPROM.put(address, value);
  if (!EEPROM.commit()) {
    errorausgabe(Error_eeprom);
  }

}

void eeprom_read_double(int address, double &value) {

  if (address < 0 || address + sizeof(double) > EEPROM_SIZE) {
    errorausgabe(Error_eeprom);
    return;
  }

  EEPROM.get(address, value);

}

double eeprom_return_double(int address) {

  if (address < 0 || address + sizeof(double) > EEPROM_SIZE) {
    errorausgabe(Error_eeprom);
    return 0.0; // oder ein anderer Fehlerwert
  }

  double value;
  EEPROM.get(address, value);
  return value;
}

void eeprom_write_int(int address, int value) {

  if (address < 0 || address + sizeof(int) > EEPROM_SIZE) {
    errorausgabe(Error_eeprom);
    return;
  }

  EEPROM.put(address, value);
  if (!EEPROM.commit()) {
    errorausgabe(Error_eeprom);
  }

}

void eeprom_read_int(int address, int &value) {

  if (address < 0 || address + sizeof(int) > EEPROM_SIZE) {
    errorausgabe(Error_eeprom);
    return;
  }

  EEPROM.get(address, value);

}

int eeprom_return_int(int address) {

  if (address < 0 || address + sizeof(int) > EEPROM_SIZE) {
    errorausgabe(Error_eeprom);
    return 0; // oder einen anderen Fehlerwert
  }

  int value;
  EEPROM.get(address, value);
  return value;
}


void eeprom_write_char(int address, char value) {

  if (address < 0 || address + sizeof(char) > EEPROM_SIZE) {
    errorausgabe(Error_eeprom);
    return;
  }

  EEPROM.put(address, value);
  if (!EEPROM.commit()) {
    errorausgabe(Error_eeprom);
  }

}

void eeprom_read_char(int address, char &value) {

  if (address < 0 || address + sizeof(char) > EEPROM_SIZE) {
    errorausgabe(Error_eeprom);
    return;
  }

  EEPROM.get(address, value);

}


void eeprom_write_string(int address, const String &value, int maxLength) {
  if (address < 0 || address + maxLength > EEPROM_SIZE) {
    errorausgabe(Error_eeprom);
    return;
  }

  // Zeichenweise schreiben, maxLength beachten
  for (int i = 0; i < maxLength; i++) {
    char c = (i < value.length()) ? value[i] : '\0'; // Rest mit '\0' auffüllen
    EEPROM.put(address + i, c);
  }

  if (!EEPROM.commit()) {
    errorausgabe(Error_eeprom);
  }
}

void eeprom_read_string(int address, String &value, int maxLength) {
  if (address < 0 || address + maxLength > EEPROM_SIZE) {
    errorausgabe(Error_eeprom);
    return;
  }

  value = "";
  for (int i = 0; i < maxLength; i++) {
    char c;
    EEPROM.get(address + i, c);
    if (c == '\0') break;  // String-Ende
    value += c;
  }
}
