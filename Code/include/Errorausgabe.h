#pragma once

#include <secrets.h>
#include "database.h"


void errorausgabe (int error) {

    if(debug){
    Serial.println("Error: " + String(error));
    }else{
      upload_to_database_error(error);
    }

}

