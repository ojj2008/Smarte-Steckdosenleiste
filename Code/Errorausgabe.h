#pragma once

#include <secrets.h>


void errorausgabe (int error) {

    if(debug){
    Serial.println("Error: " + String(error));
    }

}