# ESP-Wetter-Tracker [Link to english version](#esp-weather-tracker-hier-zur-deutschen-version)

Der Wettertracker sammelt lokale Temperatur und Luftdruckdaten. In einem Intervall von ca. 15 min werden außerdem Anfragen an die WeatherAPI gesendet, welche weitere Standorte berücksichtigt und so Timeseries-Daten anlegt. Zur übertragung der Daten auf eine Clouddistribution wurde ein Python websocket erstellt, mit diesem kann per knopfdruck aus vom ESP verbunden werden. 

## Hard und Software 
Für diese Projekt wurde der ESP8266 D1_mini genutzt uin Kombinatio mit der Hardware des Aalener Lern Computers (AALeC). 

- ESP8266 D1_mini Hardware [Link](https://www.az-delivery.de/products/d1-mini)
- AALeC Hardware [Link](https://github.com/informatik-aalen/AALeC-Hardware)
- AALeC Software [Link](https://github.com/informatik-aalen/AALeC-V3)

## Installation
Um Projekt lokal zu nutzen benötigen Sie neben der Hardware auch die Software. Als entwicklungsumgebung in VS Code die platformIO Extension installieren und den ESP über eine datenkabel mit dem PC verbinden. Im anschluss folgende Dateien im **data** Verzeichnis Anlegen:

- config.env (siehe example_config.env)
- poi.json (siehe example_poi.json)

 Nun können Sie die Software mit einem  **Upload** auf den ESP spielen. Außerdem die zwei von Ihnen angelegten Dateien mit den zwei Befehlen **Build Filesystem Image** und **Upload Filesystem Image** auf den ESP aufspielen.

 Nun am gewünschten Ort platzierern und den ESP per USB-Kabel mit Strom versorgen.



# ESP-Weather-Tracker [Hier zur deutschen Version](#esp-wetter-tracker-link-to-english-version)

The weather tracker collects local temperature and air pressure data. In an interval of about 15 minutes, additional requests are sent to the WeatherAPI, which takes other locations into account and thus creates timeseries data. A Python websocket was created for transferring the data to a cloud distribution, allowing the ESP to connect at the press of a button.  

## Hardware and Software  
For this project, the ESP8266 D1_mini was used in combination with the hardware of the Aalener Learning Computer (AALeC).  

- ESP8266 D1_mini Hardware [Link](https://www.az-delivery.de/products/d1-mini)  
- AALeC Hardware [Link](https://github.com/informatik-aalen/AALeC-Hardware)  
- AALeC Software [Link](https://github.com/informatik-aalen/AALeC-V3)  

## Installation  
To use the project locally, you will need both the hardware and the software.  
As a development environment in VS Code, install the PlatformIO extension and connect the ESP to your PC using a data cable.  
Then create the following files in the **data** directory:  

- config.env (see example_config.env)  
- poi.json (see example_poi.json)  

You can now upload the software to the ESP using **Upload**.  
Additionally, upload the two files you created to the ESP using the commands **Build Filesystem Image** and **Upload Filesystem Image**.  

Finally, place the ESP at the desired location and power it via a USB cable.  
