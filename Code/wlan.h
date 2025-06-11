#include "Errorausgabe.h"
#include <secrets.h>


#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>


//!OvertheAir Web Updater




int Error_wifi = 20;
int Error_Wifi_Verbindungsabbruch = 21;
int Error_ota = 22;
int Error_mDns = 23;


int RSSI = 0;


int  Wifi_Timeout = 10000;
unsigned long Wifi_connect_Starttime = 0;
 

WiFiMulti wiFiMulti;




WebServer server(80);



void Wifi_initialisieren() {
    WiFi.mode(WIFI_STA);
    wiFiMulti.addAP(SECRET_SSID_Zuhause, SECRET_PASS_Zuhause);
    wiFiMulti.addAP(SECRET_SSID_Hotspot, SECRET_PASS_Hotpsot);
}



void Wifi_connect() {

    Wifi_connect_Starttime = millis();

    if(debug){
        Serial.println("Verbinden: ");
    }

    while(wiFiMulti.run(Wifi_Timeout) != WL_CONNECTED) {
        delay(500);

        if(debug){
            Serial.println("...");
        }

        if(millis() - Wifi_connect_Starttime >= Wifi_Timeout){
            errorausgabe(Error_wifi);
            return;
        }

    }

    RSSI = WiFi.RSSI();

}

void OTA_initialisieren() {

    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";
    })
    .onEnd([]() {
    })
    .onProgress([](unsigned int progress, unsigned int total) {
    })
    .onError([](ota_error_t error) {
        errorausgabe(Error_ota);
    });



  ArduinoOTA.begin();

}

void Mdns_initialisieren() {

    if (!MDNS.begin("steckdosenleiste02")) {   // Set the hostname to "steckdosenleiste02.local"
        errorausgabe(Error_mDns);
      }

}




int Wifi_RSSI() {
    RSSI = WiFi.RSSI();

    return RSSI;
}


void Webserver_initialisieren() {

  server.on("/", []() {

  CurrentsensorData data = get_Currentsensor();

  server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <title>Steckdosenleiste Webinterface</title>
  <link rel="icon" type="image/png" href="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAKAAAABQCAYAAACeXX40AAAAAXNSR0IArs4c6QAABAlJREFUeF7tndty6yAMRU/+/6PTSU6coa4NWyCMMKtPnVYGsbUshJzL4/l8Pv/xgwKDFHgA4CDlmfatAAACwlAFAHCo/ExeDeDj8fiqRxkJSLUKVAP43r+BsFZ3rvsoAICgMFSBJgBTz7dsyHY8NJ7TTe4G4LZytuXpGBjqMAAOlZ/J3QEkEwKVRYFuAKYgUhdaQrKWbXcAadesBZR1tQBoVQx7VwUuAfCoXfP6G1uzayynHOxyAPcwAuGU3Lg5PRTAtD4ERLeYTjUQAE4Vrvs5OxzAfd+QTHg/yHIrCgMgfcO1wPvGO9pL8l/PksmC68AYLgNuBxMgXAPCkAAC4RrwveMcbQumFlwHPgBcK9YhVxs2A9KkDsmLu1OhAWQ7do93uAEBMFxI1nJoCgA5Fd8XSgC8b2ynWNk0AJIFp+DJ7CQAmiXjAk8FwgCYvp84t0Ae0XmGf/xYQwG0vom9BGlPOPnkhz6wAqCoKwCKQhnNhgBYG8zSS7WOMmRrVtz72mMOY8xuZX45gCWIcurWXFsLe/oU5vV7DmSgrL8nbg/gHqS9VCWwajMoUGpQAmDmSwJqMm4O+FqYtVDOabUMgGfhyZ3EjwC0ntzTec9O8SuDOQTALShW4VsykpIf0nrxbK69jWVLV26CWm2U9UW0uRzAs4xQgrH1MGER/2wu5QbYZ7nSulaHEgAPCABAy+3aZjsUQKU+GrUlndV/aUZTTrqemVuZrw2H668OA+D1Sz+fUYFP9VfZttWxjuxmhxIAT7bgfe1WC1LtdSqUAKgqNZGdmgGV4FsB9DjEKH5FCQcZMIlErl6zgmGt/XIHH486OCqUACgCuM8YpaZyDYBnLRvrWGp2iwAlAALgL15r+5Yq9H9u5KgfzVG7oNrrrLWa2kBWAqrOrdrVavC67uqsSAb8ROuK4Oag3UBNt9u9T1YfrXWr502l3gQACIBFVrxAPuxjsgX/l6WnyKUIq5nNYvea86iX2XqiVn0orXn7PxnwRKnLa6HMJ8NaT8E5SKxjfUH5fDm5UtOq8L1vfDKgLldPKL36gGqGUu0AUOejuyUA+n+7FRmwEVtvKFtr0TSz9ThRswU3AnPF5d5QWnxWt1bVji3Yon4QWwDUA8EWrGvVZHkllN6nYGu2tAgFgBa1nG2PoGzt06U9Ta8+IAA6B36G4TwyZuuBJq3/vA8fNKKDUwiAwQO0onseUFp167n98iTEGo2A9r2hBMCAQY/ukieUABg92gH9A8CAQVndpVooyYCrk9Nx/SUoe8PHIaRjcGcYGgBniNJiPno1r1XZeBSnKoVdFwUAsIusDKoqAICqUth1UQAAu8jKoKoCAKgqhV0XBQCwi6wMqioAgKpS2HVRAAC7yMqgqgI/R+WmPtFyDvwAAAAASUVORK5CYII=" />
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: #f4f4f9;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      flex-direction: column;
    }

    .container {
      background: white;
      border-radius: 15px;
      box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
      padding: 40px;
      width: 80%;
      max-width: 800px;
      text-align: center;
    }

    h1 {
      color: #333;
      font-size: 2.5em;
      margin-bottom: 20px;
    }

    #lineChart {
      width: 100%;
      height: 400px;
      margin-bottom: 20px;
      border-radius: 10px;
      border: 1px solid #ddd;
    }

    button {
      background-color: #e74c3c;
      color: white;
      padding: 15px 25px;
      border: none;
      border-radius: 10px;
      font-size: 1.1em;
      cursor: pointer;
      transition: background-color 0.3s ease;
      margin-top: 30px;
      margin-bottom: 30px;
    }

    .button-container {
      display: flex;
      gap: 10px;
      justify-content: center; /* Buttons horizontal zentrieren */
      align-items: center; /* Buttons vertikal zentrieren */
    }

    button:hover {
      background-color: #c0392b;
    }

    footer {
      margin-top: 20px;
      color: #888;
      font-size: 0.9em;
    }

    .output-container {
      display: flex;
      align-items: center;
      justify-content: center;
      margin-top: 20px;
      gap: 20px;
    }

    .output-field {
      font-size: 1.2em;
      color: #333;
      background: #f4f4f9;
      border: 1px solid #ddd;
      border-radius: 5px;
      padding: 10px 15px;
    }

    .status-image {
      width: 50px;
      height: 50px;
    }

    .Steckdosenleiste-image {
      margin-top: 40px;
      width: 200px;
      height: 100px;
    }


    .slider-container {
    text-align: center;
}

.slider {
    position: relative;
    width: 450px;  /* Breiter */
    height: 60px;  /* Höher */
    background: #ccc;
    border-radius: 30px;
    margin: 10px auto;
    display: flex;
    align-items: center;
    cursor: pointer;
}

.slider-handle {
    position: absolute;
    top: 50%;
    left: 0%;
    width: 100px;  /* Breiter */
    height: 50px; /* Höher */
    background: #007bff;
    border-radius: 50px;
    transform: translateY(-50%);
    transition: left 0.3s ease-in-out;
    cursor: grab;
}

.labels {
    position: absolute;
    width: 100%;
    display: flex;
    justify-content: space-between;
    padding: 0 10px;
    font-size: 18px;
    font-weight: bold;
    color: white;
    pointer-events: none;
}

.labels span:last-child {
    margin-right: 17px;
}


.Verlauf {
    margin-top: 40px;
    margin-bottom: 40px;
    }
    
 .section {
     margin-top: 40px;
     margin-bottom: 40px;
 }

 #powerTriangle {
            max-width: 500px;
            margin: 20px auto;
            background-color: white;
}





</style>
</head>
<body>
  <div class="container">
    <h1>Steckdosenleiste Webinterface</h1>

    <div class="slider-container">
      <div class="slider" id="slider">
          <div class="slider-handle" id="sliderHandle"></div>
          <div class="labels">
              <span>Kurve</span>
              <span>Verbrauch</span>
              <span>Verlauf</span>
              <span>Info</span>
          </div>
      </div>
  </div>



  <canvas id="lineChart01"></canvas>
  <canvas id="lineChart02"></canvas>
  

  <div class="Verlauf">
      <button onclick="verlauf_anzeigen()">Zu externen Darstellung</button>
  </div>
  

  <div class="section" id="options-section">
      <h2>Optionen</h2>

      <div class="output-container">
        <div id="outputFieldRssi" class="output-field">RSSI : </div>
        <img id="RSSIImage" class="status-image" src="" alt="Status">
      </div>
          <canvas id="powerTriangle" width="500" height="400"></canvas>
      <div>
        <img id="status-image" class="Steckdosenleiste-image" src="" alt="Status">
      </div>
      <div class="button-container">
          <button onclick="shutdown()">Ausschalten</button>
          <button onclick="window.location.href='/abschnitt01';">Abschnitt01</button>
          <button onclick="window.location.href='/abschnitt02';">Abschnitt02</button>
      </div>

  </div>


  </div>




  <footer>
    <p>Steckdosenleiste Webinterface &copy; 2024</p>
  </footer>







  <script>


let myChart01;
let myChart02;

async function fetchDataChart01() {
  const response = await fetch('/chart-data02');
  const chartData01 = await response.json();


      function generateXLabels(length) {
      let labels = [];
      let time = 0;
      const interval = 10;  // Alle 15 Messwerte einen neuen Tick setzen

      for (let i = 0; i < length; i++) {
        if (i % interval === 0) {
          labels.push(`${time} sec`);
          time += 10; // Erhöht um 10 ms für den nächsten Tick
        } else {
          labels.push('');
        }
      }

      return labels;
    }


  const data01 = {
    labels: generateXLabels(150),
    datasets: [
      {
        label: 'Apparent Power',
        data: chartData01.apparent_power,
        borderColor: 'rgba(75, 192, 192, 1)',
        backgroundColor: 'rgba(75, 192, 192, 0.2)',
        fill: true,
        tension: 0.4
      },
      {
        label: 'Active Power',
        data: chartData01.active_power,
        borderColor: 'rgba(255, 99, 132, 1)',
        backgroundColor: 'rgba(255, 99, 132, 0.2)',
        fill: true,
        tension: 0.4
      }
    ]
  };


if (!myChart01) {
    const ctx01 = document.getElementById('lineChart01').getContext('2d');
    
    const config01 = {  // Config muss zuerst deklariert werden
        type: 'line',
        data: data01,
        options: {
            responsive: true,
            scales: {
                x: {
                    title: { display: true, text: 'Vergangene Zeit in Sekunden' },
                    beginAtZero: true
                },
                y: {
                    title: { display: true, text: 'Watt' },
                    beginAtZero: true
                }
            }
        }
    };

    myChart01 = new Chart(ctx01, config01); // Jetzt wird es korrekt erstellt

} else {
    myChart01.data = data01;
    myChart01.update();
  }
}

async function fetchDataChart02() {
  const response = await fetch('/chart-data01');
  const chartData02 = await response.json();

    function generateXLabels(length) {
      let labels = [];
      let time = 0;
      const interval = 14;  // Alle 14 Messwerte einen neuen Tick setzen

      for (let i = 0; i < length; i++) {
        if (i % interval === 0) {
          labels.push(`${time} ms`);
          time += 10; // Erhöht um 10 ms für den nächsten Tick
        } else {
          labels.push('');
        }
      }

      return labels;
    }


  // Chart.js Daten und Konfiguration
  const data02 = {
    labels: generateXLabels(100),
    datasets: [
      {
        label: 'Voltage (V)',
        data: chartData02.voltage,
        borderColor: 'rgba(75, 192, 192, 1)',
        backgroundColor: 'rgba(75, 192, 192, 0.2)',
        fill: false,
        tension: 0.4,
        yAxisID: 'y1'  // Voltage uses the first Y-axis
      },
      {
        label: 'Current (A)',
        data: chartData02.current,
        borderColor: 'rgba(255, 99, 132, 1)',
        backgroundColor: 'rgba(255, 99, 132, 0.2)',
        fill: false,
        tension: 0.4,
        yAxisID: 'y2'  // Current uses the second Y-axis
      }
    ]
  };

  const config02 = {
    type: 'line',
    data: data02,
    options: {
      responsive: true,
      scales: {
        x: {
          title: {
            display: true,
            text: 'Vergangene Zeit in Millisekunden'
          },
          ticks: {
            autoSkip: false,
          },
        },
        y1: {
          title: {
            display: true,
            text: 'Voltage (V)'
          },
          min: -300,
          max: 300,
          ticks: {
            stepSize: 100
          }
        },
        y2: {
          title: {
            display: true,
            text: 'Current (A)'
          },
          suggestedMin: -5,
          suggestedMax : 5,
          ticks: {
            stepSize: 5
          }
        }
      },
      plugins: {
        legend: {
          position: 'top',
        }
      }
    }
  };





  if (!myChart02) {
    const ctx02 = document.getElementById('lineChart02').getContext('2d');
    myChart02 = new Chart(ctx02, config02);
  } else {
    myChart02.data = data02;
    myChart02.update();
  }
}


async function fetchData() {
  const response = await fetch('/json');
  const data = await response.json();
  
    const rssiField = document.getElementById('outputFieldRssi');
    rssiField.innerHTML = `RSSI: ${data.rssi}`;


      const image01 = document.getElementById('RSSIImage');

      if (data.rssi < -70) {
        image01.src="data:image/png;base64,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";
      } else {
        image01.src="data:image/png;base64,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";
      }
    
      
      let P = data.active_power;
      let Q = data.reactive_power;
      let S = data.apparent_power;
      
      drawTriangle(P, Q, S);


    
      const image03 = document.getElementById('status-image');

      if (data.master == true && data.Section01 == false && data.Section02 == true){
        image03.src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAHgAAAAyCAYAAACXpx/YAAAAAXNSR0IArs4c6QAAAm9JREFUeF7tm0F2wyAMROsrdN9l73+gLrvvFdzn9NEQGzEDxAlSpsuAZWm+hICky7qu65v+wiqwCHBYtpfABDg2XwEOzleABTi6AsHjUw8W4OAKBA9PFSzAwRUIHh6s4GVZgksQM7x0QUkB1m2mryTYilKAfTFr8laAm+TyN3l6wKW+f4828Sp2pwWcAJRg1sZQjb2a3ekAt8DT3L90Rkk7zSYrzzZUifk4eg6NW+9Cz6HxGewOVfDIudhaenv6KxIajZ8B4ufj82L2/fvrYB75UxtvtTsM+F5AUNCooq3nn2W3BiItqz1J3mqXApxXau5Ur3il53ptJfACfO3HFqPDTRY6SiRR2aX6v9lntyu1bEZVux/fQx5NGit5GLuo0kpxn2H3UMEI1hkVbFX0JkLLMYkBnOLb27U+fxQINtFbE+cG8KanVTWbIIyATNUxdpDgJfgjdtH7WhO7FcRTAaOlFVV9go7sMMlRm8MA7nlHj93pATM7PKZnlATtEYwBI7vHjRa1ix7ZbDCA2eWppXrR7ppJmJHjDLL/qF2/AAMSvSDQEv0ou8OAUabWNm3omMPaRq0CjVvvQc/VxltvnHIf7ml3CDALgJ2HBD0DRM+yP7r8ozjReIsOUwFOPa90BLJ6ueY6+jZpv0xZ8GpnVrRaoPMumzCl9jKjv9NVsCVc/nnPFxyvand6wKgiNV5XQICDZ4gAC/BVAfbOObhm7sKjf5PlLjI5fKMA/NcV6eVbAQH2zQ96L8BQIt8TBNg3P+i9AEOJfE8QYN/8oPcCDCXyPUGAffOD3gswlMj3BAH2zQ96L8BQIt8TBNg3P+j9L9gZo5h/hP05AAAAAElFTkSuQmCC";
      }

      if (data.master == true && data.Section01 == true && data.Section02 == false){
        image03.src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAHgAAAAyCAYAAACXpx/YAAAAAXNSR0IArs4c6QAAAodJREFUeF7tm0FWxCAQRM0V3Lv0/gdy6d4rxJf4cAgC1XSCkJpyOQwdqn8X6ZBxWdd1fdEfbQYWAaZluwsTYG6+AkzOV4AFmD0D5Pp0DxZg8gyQy5ODBZg8A+TyoIOXZSFPAae8cEBpAqzTzHsVwWZKAb4Xs6bVCnBTuu735ekB5+77V9wmesTtEXN/SZDpfaw5mBZwEJUTUhtDHusRt0fMGOyZHEwHuAXe6O+Ovj4qgjA+TZMVVxtyYjyO5qHx0rVq83rETIFckYNTDj7zXFzadkr3lq+3913v6+fHH90o2bVxb1x0TU/RTAnYerNHjkMJq4GoJaZHXBQTOa80v0dck4Njp8ZAvQvKzUOxBBiVzU+3nRquChi152Gydav+vdknC0FwN2kIcM7FPeJaYmIUf2H8R9z9qBLB6uFgi7iZAde66dJYqrnkvq1Ycq7MfY6KfHuTUPzZ7HYRy6J6Va8APzKLCqpkwiJgtLUi14eloTi14pgZsKWo0+9cZZaWuAfAlscYy9aaEz9CnAdCmHNoVDKNjCf2iBw0vy68CvCZ578cBPRI1gIEdaYtsUavdWrAaIv2Plt64noL+5aAPRVs7QDj2N4TJ7Q7eON6IaN5aLyUb0uBNzvYC7d1keg6KClo3LOeHjFRMdbycAvAQWDJ4bmOceR3a48rs6z10CCi/y70Vi1yYKnTrm3lVrAtiW4BljZys651WsBxQ5JC8rzgKIGOPz8bN3cecDZmvKt51jo9YI/7NeeRAQEmrwYBFuCj3cnzQSnP/JssSvVPJAoedDxRLiilCjAl1ugWiw46yPXTy5ODyRELsACTZ4BcnhwswOQZIJcnBwsweQbI5cnBAkyeAXJ53zmQo5gtfB/nAAAAAElFTkSuQmCC";
      }

      if (data.master == true && data.Section01 == true && data.Section02 == true){
        image03.src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAHgAAAAyCAYAAACXpx/YAAAAAXNSR0IArs4c6QAAAnlJREFUeF7tmj1yhTAMhOEK6VPm/gdKmT5XIAMZ5zlge2WBwNJsmhQ2i7Sf5T/evCzLMvEvrAMzAYdluyVGwLH5EnBwvgRMwNEdCJ4f12ACDu5A8PRYwQQc3IHg6cEKnuc5uAUx00sXlCLAvM30NQjWoiRgX8y6oiXgLrv8dR4ecGndv2KZsNC10Nw+EhT2PlIPhgWckiol0mpDNWaha6GZgz3jwXCAe+A93ffp96NBkNqH2WTlow1VYt6OnkPttXe1nrPQ3AO5woNTFXzmXFybdmpry/f7x5bv29fnIW9kdqtdq2uhiQBrYj0NWLrYo4pDkFrJtYyx0LXQRHDXdo0HIsB5peZAUaI9Ux/S0iR3h2mlHEeN9XCThbbnCYp0qv5b7LPbFQkEyegt6aBBo9G10LzLgw0wgmVRwZ5M8xTrYRc9TVP1Z7Mr2H1ykmRLU5hGB017rOBfp1verp+KioDR1IqqPkFGOq1jAQHjTVYXYMkx5s4KlpwJNTNDr6425/177oj1MEWjH75fFVTJpLPG1Z630LXQlG60WgMS+dr9PVibKArE09HDU6wqwJIprtRHsgTkz2lucdLzrYGo1bXQRFWsiVV00SExSgs6f+7KGcFa10usQwFOI3j9L7kCTbv3p/o+/X6JX8MBzmeLGugeY0u716t1W/E8HeuwgPegc1CSikVLRunsflbXQjOvUo0HwwNGoNjedoCAg48QAibglwPSO+fgnrlLT/ybLHeZMeB/DsCbLPrl2wEC9s0PRk/A0CLfHQjYNz8YPQFDi3x3IGDf/GD0BAwt8t2BgH3zg9ETMLTIdwcC9s0PRk/A0CLfHQjYNz8Y/Q/sWcGYqhXlaAAAAABJRU5ErkJggg==";
      }

      if (data.master == true && data.Section01 == false && data.Section02 == false){
        image03.src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAHgAAAAyCAYAAACXpx/YAAAAAXNSR0IArs4c6QAAAihJREFUeF7tm+tWxCAMhO37P3Q91YNLK2EGaHdJGP8S0jBfEi7qtu/7/qWfsApsAhyW7c/CBDg2XwEOzleABTi6AsHXpz1YgIMrEHx5qmABDq5A8OXBCt62LbgEMZeXHigpwHrN9JUER1EKsC9mTdEKcJNc/oynB1za9+/YJlbxOy3gBKAEszaGamw1v9MBboEn2990Rkk7zSErzzZUifk4mofGrW+heWh8Br9DFTxyL7Zab8/+ioRG4zOAaEnYlniHAd8FpBdCWqw1f3W/FOC8UnOgveKV5vX6EuBzPV91rAJGV4k0mW3Vf5t99rqSDgk9neDaqmqLa9nTI/n9BxjBeqKCa+31EPsK3zo1MoBrc0vfKiWg1YFaYn2X3xPgQ0sr0w+RGQGZSmH9tMJg/Lb6fBcIq5ONxksBRq0VVX2CjvwwyVGzYQD3fMOzXxMwc43pPRh5FszbmYE6RVsn1bsA3yFabR8fOcB59yvAoGcvD7hnT2NPq6xv1EnQeMvL0FMvTk/5HapgFgBrtzKIJQCnvdiq8NIDhGwd/TbpmsW1Vs6CbUkK687JdKDa3E/6napFl4Qs3bFHTsX5jeD6vYh+pwfMVI9sbAUEOHh2CLAAvxRg35yDa+ZuefTfZLlbmQI+KQD/dUV6+VZAgH3zg9ELMJTIt4EA++YHoxdgKJFvAwH2zQ9GL8BQIt8GAuybH4xegKFEvg0E2Dc/GL0AQ4l8Gwiwb34w+m+dUIWYQJvOswAAAABJRU5ErkJggg==";
      }


}
          function drawTriangle(P, Q, S) {
    let canvas = document.getElementById('powerTriangle');
    let ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    let scale = 300 / S; // Größere Skalierung für ein größeres Dreieck
    let baseX = 150, baseY = 300;
    let pX = baseX + P * scale, pY = baseY;
    let qX = pX, qY = baseY - Q * scale;

    // Dreieck zeichnen
    ctx.beginPath();
    ctx.moveTo(baseX, baseY);
    ctx.lineTo(pX, pY);
    ctx.lineTo(qX, qY);
    ctx.closePath();
    ctx.strokeStyle = 'black';
    ctx.stroke();

    // P (Wirkleistung) – Blau
    ctx.strokeStyle = 'blue';
    ctx.beginPath();
    ctx.moveTo(baseX, baseY);
    ctx.lineTo(pX, pY);
    ctx.stroke();
    ctx.fillStyle = 'blue';
    ctx.fillText(`P = ${P.toFixed(2)} W`, (baseX + pX) / 2.2, baseY + 20);

    // Q (Blindleistung) – Rot
    ctx.strokeStyle = 'red';
    ctx.beginPath();
    ctx.moveTo(pX, pY);
    ctx.lineTo(qX, qY);
    ctx.stroke();
    ctx.fillStyle = 'red';
    ctx.fillText(`Q = ${Q.toFixed(2)} var`, pX + 15, (pY + qY) / 2);

    // S (Scheinleistung) – Orange
    ctx.strokeStyle = 'orange';
    ctx.beginPath();
    ctx.moveTo(baseX, baseY);
    ctx.lineTo(qX, qY);
    ctx.stroke();
    ctx.fillStyle = 'orange';
    ctx.fillText(`S = ${S.toFixed(2)} VA`, (baseX + qX) / 2.6, (baseY + qY) / 2 - 40);

    // Winkelbogen für φ
    ctx.beginPath();
    ctx.arc(baseX, baseY, 25, 0, -Math.atan(Q / P), true);
    ctx.strokeStyle = 'black';
    ctx.stroke();
    ctx.fillText(`φ`, baseX + 30, baseY - 5);

    // Power-Faktor (cos φ) in der Mitte
    let powerFactor = P / S;
    ctx.fillStyle = 'green';
    ctx.font = "bold 14px Arial";
    ctx.fillText(`Power Factor = ${powerFactor.toFixed(2)}`, (baseX + qX) / 2.5, (baseY + qY) / 2 + 40);
}




setInterval(() => {
    fetchData();
    fetchDataChart01();
    fetchDataChart02();
}, 5000);

fetchData();
fetchDataChart01();
fetchDataChart02();


    function shutdown() {
      alert("Die Steckdosenleiste wird ausgeschaltet!");
      window.location.href='/ausschalten';
    }


    function verlauf_anzeigen() {
      window.location.href = "https://developerojj.grafana.net/public-dashboards/ea2d6ea7f2ce46f4804e80f520c5e819";
    }


const positions = [0, 120, 260, 350];
const sliderHandle = document.getElementById("sliderHandle");
const slider = document.getElementById("slider");

let isDragging = false;

const sections = {
    lineChart01: document.getElementById('lineChart01'),
    lineChart02: document.getElementById('lineChart02'),
    optionsSection: document.getElementById('options-section'),
    Verlauf: document.querySelector('.Verlauf'),
};

// Hide all sections initially
function hideAllSections() {
    Object.values(sections).forEach((section) => {
        section.style.display = 'none';
    });
}

// Show the section based on the selected slider position
function showSection(index) {
    hideAllSections();

    switch (index) {
        case 0:
            sections.lineChart02.style.display = 'block'; // Show the first chart
            break;
        case 1:
            sections.lineChart01.style.display = 'block'; // Show the second chart
            break;
        case 2:
            sections.Verlauf.style.display = 'block'; // Show external representation button
            break;
        case 3:
            sections.optionsSection.style.display = 'block'; // Show the options section
            break;
    }
}

// Update the slider position and visible section
function moveSlider(positionIndex) {
    sliderHandle.style.left = positions[positionIndex] + "px";
    showSection(positionIndex);
    updateURLFragment(positionIndex); // Update the URL fragment
}

// Update the URL fragment
function updateURLFragment(index) {
    switch (index) {
        case 0:
            window.location.hash = 'lineChart01';
            break;
        case 1:
            window.location.hash = 'lineChart02';
            break;
        case 2:
            window.location.hash = 'Verlauf';
            break;
        case 3:
            window.location.hash = 'options-section';
            break;
    }
}


// Extract section from the URL
function getSectionFromURL() {
    const urlFragment = window.location.hash.substring(1); // Removes the '#' symbol
    return urlFragment;
}

// Show the section based on the URL fragment or default to 'lineChart02' if none exists
function showSectionBasedOnURL() {
    const section = getSectionFromURL();
    
    // Überprüfe, ob das Fragment aus der URL ein gültiger Abschnitt ist
    switch (section) {
        case 'lineChart01':
            moveSlider(0); // Move to lineChart01
            break;
        case 'lineChart02':
            moveSlider(1); // Move to lineChart02
            break;
        case 'Verlauf':
            moveSlider(2); // Move to Verlauf
            break;
        case 'options-section':
            moveSlider(3); // Move to options-section
            break;
        default:
            moveSlider(1); // Standardmäßig auf lineChart02 setzen
            break;
    }
}

slider.addEventListener("click", (e) => {
    if (isDragging) return;

    let sliderRect = slider.getBoundingClientRect();
    let clickX = e.clientX - sliderRect.left;

    let closestIndex = positions.reduce((prev, curr, index) =>
        Math.abs(curr - clickX) < Math.abs(positions[prev] - clickX) ? index : prev, 0
    );

    moveSlider(closestIndex);
});

sliderHandle.addEventListener("mousedown", (e) => {
    isDragging = true;
    document.addEventListener("mousemove", onDrag);
    document.addEventListener("mouseup", stopDrag);
});

function onDrag(e) {
    if (!isDragging) return;

    let sliderRect = slider.getBoundingClientRect();
    let newLeft = e.clientX - sliderRect.left - sliderHandle.offsetWidth / 2;

    if (newLeft < positions[0]) newLeft = positions[0];
    if (newLeft > positions[2]) newLeft = positions[2];

    sliderHandle.style.left = newLeft + "px";
}

function stopDrag() {
    isDragging = false;
    document.removeEventListener("mousemove", onDrag);
    document.removeEventListener("mouseup", stopDrag);

    let leftPos = parseInt(sliderHandle.style.left);
    let closestIndex = positions.reduce((prev, curr, index) =>
        Math.abs(curr - leftPos) < Math.abs(positions[prev] - leftPos) ? index : prev, 0
    );

    moveSlider(closestIndex);
}

window.addEventListener('load', () => {
    showSectionBasedOnURL(); // Show section based on URL on page load
});




</script>
</body>
</html>
)rawliteral");
      });


 server.onNotFound([]() {
   server.send(404, "text/html", "<h1>404: Seite nicht gefunden</h1>");
 });


 server.on("/chart-data02", []() {
  CurrentsensorData_Verlauf data_verlauf = update_Currentsensorverlauf();
  
  String json = "{ \"apparent_power\": [";
  for (int i = 0; i < data_verlauf.apparent_power_verlauf_length; i++) {
      json += String(data_verlauf.apparent_power_verlauf[i]);
      if (i < data_verlauf.apparent_power_verlauf_length - 1) json += ",";
  }
  json += "], \"active_power\": [";
  for (int i = 0; i < data_verlauf.active_power_verlauf_length; i++) {
      json += String(data_verlauf.active_power_verlauf[i]);
      if (i < data_verlauf.active_power_verlauf_length - 1) json += ",";
  }
  json += "] }";

  server.send(200, "application/json", json);
});



server.on("/chart-data01", []() {
  CurrentsensorData_Kurve data_kurve = update_Kurve();
  
  String json = "{ \"voltage\": [";
  for (int i = 0; i < data_kurve.voltage_kurve_length; i++) {
      json += String(data_kurve.voltage_kurve[i]);
      if (i < data_kurve.voltage_kurve_length - 1) json += ",";
  }
  json += "], \"current\": [";
  for (int i = 0; i < data_kurve.current_kurve_length; i++) {
      json += String(data_kurve.current_kurve[i]);
      if (i < data_kurve.current_kurve_length - 1) json += ",";
  }
  json += "] }";

  server.send(200, "application/json", json);
});


server.on("/json", []() {
  CurrentsensorData data_currentsensor = get_Currentsensor();
  Relaistreiber data_relay = get_Relaistreiber();

  StaticJsonDocument<300> doc;

  doc["spannung"] = data_currentsensor.spannung;
  doc["ampere"] = data_currentsensor.ampere;
  doc["active_power"] = data_currentsensor.active_power;
  doc["reactive_power"] = data_currentsensor.reactive_power;
  doc["apparent_power"] = data_currentsensor.apparent_power;
  doc["power_factor"] = String(abs(data_currentsensor.power_factor));
  doc["phasenwinkel"] = data_currentsensor.phasenwinkel;
  doc["position_phasenwinkel"] = data_currentsensor.position_phasenwinkel;
  doc["einspeisung"] = data_currentsensor.einspeisung;
  doc["aktiv_seit"] = millis() / 1000;
  doc["rssi"] = Wifi_RSSI();

  doc["Einschaltstrombegrenzer"] = data_relay.einschaltstrombegrenzer;
  doc["master"] = data_relay.master;
  doc["Section01"] = data_relay.section01;
  doc["Section02"] = data_relay.section02;


  String json;
  serializeJson(doc, json);

  server.send(200, "application/json", json);
});


server.on("/ausschalten", []() {

  Ausschalten();

  server.send(200, "text/html", R"rawliteral(
  <!DOCTYPE html>
  <html lang="de">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Automatische Weiterleitung</title>
    <script>
      window.location.href = "http://steckdosenleiste02#options-section";
    </script>
  </head>
  <body>
    <h1>Sie werden weitergeleitet...</h1>
  </body>
  </html>
  )rawliteral");
});

server.on("/abschnitt01", []() {

  Abschnitt01();

  server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html lang="de">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Automatische Weiterleitung</title>
      <script>
        window.location.href = "http://steckdosenleiste02#options-section";
      </script>
    </head>
    <body>
      <h1>Sie werden weitergeleitet...</h1>
    </body>
    </html>
    )rawliteral");
});

server.on("/abschnitt02", []() {

  Abschnitt02();

  server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html lang="de">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Automatische Weiterleitung</title>
      <script>
        window.location.href = "http://steckdosenleiste02#options-section";
      </script>
    </head>
    <body>
      <h1>Sie werden weitergeleitet...</h1>
    </body>
    </html>
    )rawliteral");
});

server.begin();


}




/*
void Webserver_initialisieren() {


server.on("/", []() {

CurrentsensorData data = get_Currentsensor();

server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <title>Steckdosenleiste Webinterface</title>
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: #f4f4f9;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      flex-direction: column;
    }

    .container {
      background: white;
      border-radius: 15px;
      box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
      padding: 40px;
      width: 80%;
      max-width: 800px;
      text-align: center;
    }

    h1 {
      color: #333;
      font-size: 2.5em;
      margin-bottom: 20px;
    }

    #lineChart {
      width: 100%;
      height: 400px;
      margin-bottom: 20px;
      border-radius: 10px;
      border: 1px solid #ddd;
    }

    button {
      background-color: #e74c3c;
      color: white;
      padding: 15px 25px;
      border: none;
      border-radius: 10px;
      font-size: 1.1em;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }

    button:hover {
      background-color: #c0392b;
    }

    footer {
      margin-top: 20px;
      color: #888;
      font-size: 0.9em;
    }

    .output-container {
      display: flex;
      align-items: center;
      justify-content: center;
      margin-top: 20px;
      gap: 20px;
    }

    .output-field {
      font-size: 1.2em;
      color: #333;
      background: #f4f4f9;
      border: 1px solid #ddd;
      border-radius: 5px;
      padding: 10px 15px;
    }

    .status-image {
      width: 50px;
      height: 50px;
    }

  </style>
</head>
<body>
  <div class="container">
    <h1>Steckdosenleiste Webinterface</h1>
    <canvas id="lineChart"></canvas>
    <button onclick="showPasswordPrompt()">Ausschalten</button>

    <div class="output-container">
      <div id="outputField" class="output-field">Power Factor: </div>
      <img id="statusImage" class="status-image" src="" alt="Status">
    </div>
  </div>

  <footer>
    <p>Steckdosenleiste Webinterface &copy; 2024</p>
  </footer>

  <script>
        
let myChart;

async function fetchData() {
  const response = await fetch('/chart-data');
  const chartData = await response.json();

  const data = {
    labels: Array.from({ length: chartData.apparent_power.length }, (_, i) => i + 1),
    datasets: [
      {
        label: 'Apparent Power',
        data: chartData.apparent_power,
        borderColor: 'rgba(75, 192, 192, 1)',
        backgroundColor: 'rgba(75, 192, 192, 0.2)',
        fill: true,
        tension: 0.4
      },
      {
        label: 'Active Power',
        data: chartData.active_power,
        borderColor: 'rgba(255, 99, 132, 1)',
        backgroundColor: 'rgba(255, 99, 132, 0.2)',
        fill: true,
        tension: 0.4
      }
    ]
  };

  if (!myChart) {
    const ctx = document.getElementById('lineChart').getContext('2d');
    myChart = new Chart(ctx, {
      type: 'line',
      data: data,
      options: {
        responsive: true,
        scales: {
          x: { title: { display: true, text: 'Vergangene Zeit in Sekunden' }, beginAtZero: true },
          y: { title: { display: true, text: 'Watt' }, beginAtZero: true }
        },
        plugins: { legend: { position: 'top' } }
      }
    });
  } else {
    myChart.data = data;
    myChart.update();
  }
}



    function shutdown() {
      alert("Die Steckdosenleiste wird ausgeschaltet!");
    }

function updateSensorData() {
    fetch('/debug-sensor')
        .then(response => response.json()) // JSON-Antwort parsen
        .then(data => {
            // Power Factor extrahieren
            let powerFactor = data.power_factor;
            console.log("Power Factor: " + powerFactor);

            // Power Factor anzeigen
            document.getElementById("outputField").innerHTML = "Power Factor: " + powerFactor;

            // Bild basierend auf dem Power Factor ändern
            let img = document.getElementById("statusImage");
            if (powerFactor < 0.85) {
                img.src = "path-to-your-low-power-image.png";  // Hier den Pfad zum Bild für niedrigen Power Factor angeben
            } else {
                img.src = "path-to-your-high-power-image.png";  // Hier den Pfad zum Bild für hohen Power Factor angeben
            }
        })
        .catch(error => console.error("Fehler beim Abrufen der Daten:", error)); // Fehlerbehandlung
}


fetchData();
updateData();
setInterval(fetchData, 5000); // Automatische Aktualisierung
setInterval(updateData, 1000);

  </script>
</body>
</html>
        )rawliteral");
      });



      server.on("/chart-data", []() {

        CurrentsensorData_Verlauf data_verlauf = update_Currentsensorverlauf();

        String json = "{ \"apparent_power\": [";
        for (int i = 0; i < data_verlauf.apparent_power_verlauf_length; i++) {
            json += String(data_verlauf.apparent_power_verlauf[i]);
            if (i < data_verlauf.apparent_power_verlauf_length - 1) json += ",";
        }
        json += "], \"active_power\": [";
        for (int i = 0; i < data_verlauf.active_power_verlauf_length; i++) {
            json += String(data_verlauf.active_power_verlauf[i]);
            if (i < data_verlauf.active_power_verlauf_length - 1) json += ",";
        }
        json += "] }";
        
        server.send(200, "application/json", json);
    });






    server.onNotFound([]() {
        server.send(404, "text/html", "<h1>404: Seite nicht gefunden</h1>");
      });



    server.on("/debug-sensor", []() {
      CurrentsensorData data = get_Currentsensor(); // Aktuelle Sensordaten abrufen
  
      String json = "{";
      json += "\"spannung\": " + String(data.spannung) + ",";
      json += "\"ampere\": " + String(data.ampere) + ",";
      json += "\"active_power\": " + String(data.active_power) + ",";
      json += "\"reactive_power\": " + String(data.reactive_power) + ",";
      json += "\"apparent_power\": " + String(data.apparent_power) + ",";
      json += "\"power_factor\": " + String(data.power_factor) + ",";
      json += "\"phasenwinkel\": " + String(data.phasenwinkel) + ",";
      json += "\"position_phasenwinkel\": " + String(data.position_phasenwinkel) + ",";
      json += "\"einspeisung\": " + String(data.einspeisung);
      json += "}";
  
      server.send(200, "text/html", "<html><head><title>Debug Sensor</title></head><body>"
                                    "<h1>Debug Sensordaten</h1>"
                                    "<pre>" + json + "</pre>"
                                    "</body></html>");
  });
    

    server.begin();

}

*/