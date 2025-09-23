#include <Arduino.h>
#include <AALeC-V3.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define MAX_POI 20


// put function declarations here:
void setLEDsOff(int);
void setDisplayOff();
bool waitForButtonPress(uint16_t);
void loadEnvVars();
void connectWifi();
void disconnectWifi();
void loadPoi();
void printSingleData(String s[], size_t len);
void writeDataToCSV(String filename);
void apiRequests();
bool cloudUpload(String filename);
void deleteFile(String filename);

// global vars
unsigned long messureDistanceInMillis = 900000; //15 min aka 900 s 
unsigned long MAX_FILE_SIZE = 1024*50; //50kB RAM Limit
unsigned long DATA_ROW_SIZE = 250; // max 250B per Data row
bool displayON = true;
float currentTemp = 0;
float currentPressure = 0;
String WIFI_SSID, WIFI_PASS, WEATHER_API_TOKEN, WEATHER_API_URL,WEATHER_API_FINGERPRINT, CLOUD_TOKEN, CLOUD_URL; // Vars from .env file
String PointsOfInterest [MAX_POI]; // placeholder array for points of interest
String crutialColumns [] = {"name", "lat", "lon", "localtime", "last_updated", "temp_c", "is_day",
                           "condition_text", "condition_id", "wind_kph", "wind_degree", "wind_dir",
                           "pressure_mb", "pressure_in", "humidity", "cloud", "uv"};
String crutialValues [sizeof(crutialColumns)/sizeof(crutialColumns[0])]; // placeholder for values, length of column array
String currentFilename = "data";
int currentFileCounter = 1;


void setup() {
  // start LittleFS to use file management
  if (!LittleFS.begin()) {
    Serial.println("LittleFS konnte nicht gemountet werden");
    return;
  }
  aalec.init(5);
  setLEDsOff(5);
  loadEnvVars();
  loadPoi();
}

void loop() {
  // put your main code here, to run repeatedly:
  connectWifi();
  aalec.print_line(4,"wifi connected");
  apiRequests();
  disconnectWifi();
  aalec.print_line(4,"wifi disconnected");
  
  // update values of p and t
  currentPressure = aalec.get_pressure();
  currentTemp = aalec.get_temp();

  //write local data to file
  crutialValues[0] = "ESP";
  crutialValues[5] = String (currentTemp);
  crutialValues[12] = String (currentPressure);
  writeDataToCSV(currentFilename + String(currentFileCounter));
  std::fill(std::begin(crutialValues), std::end(crutialValues), "");

  //test read file:
  File f = LittleFS.open("/"+currentFilename+ String(currentFileCounter) +".csv", "r");
  Serial.println(f.readString());


  unsigned long startTime = millis();
  // wait till difference between start and current time is less than delayTime
  while ((millis() - startTime < messureDistanceInMillis)){

    if (waitForButtonPress(2000))  displayON = !displayON;

    if (displayON){
    aalec.print_line(2, "Druck: "+ String(currentPressure));
    aalec.print_line(3,"Temperatur: "+String(currentTemp));
    }
  }
}

void setLEDsOff(int n){
  // turn all LEDs off
  RgbColor colors[n];
  for (int i = 0; i < n; i++) {
      colors[i] = c_off;
  }
  aalec.set_rgb_strip(colors);
}

void setDisplayOff(){
  // turn all Display lines off 
  for (int i=0;i<5;i++){
    aalec.print_line(i,"");
  }
}

bool waitForButtonPress(uint16_t delayTime){
  unsigned long startTime = millis();
  // wait till difference between start and current time is less than delayTime
  while ((millis() - startTime < delayTime)){
    
    if (aalec.get_button())
    {
      delay(750);   // delay so that button is not pressed several times 
      if (displayON) setDisplayOff(); // if display was on, clear display
      return true; // if button is pressed return true 
    }
    delay(10); // Small delay to avoid busy-waiting
  }
  return false; // Return false if the button was not pressed 
}

void loadEnvVars(){
  File file = LittleFS.open("/config.env", "r");
  if (!file) {
    aalec.print_line(5,"Fehler: config.env nicht gefunden!");
    return;
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');

    int separator = line.indexOf('=');
    if (separator > 0) {
      String key = line.substring(0, separator);
      String value = line.substring(separator + 1);

      key.trim();
      value.trim();

      if (key == "WIFI_SSID") {
        WIFI_SSID = value;
      } else if (key == "WIFI_PASS") {
        WIFI_PASS = value;
      }else if (key == "WEATHER_API_TOKEN") {
        WEATHER_API_TOKEN = value;
      }else if (key == "WEATHER_API_URL") {
        WEATHER_API_URL = value;
      }else if (key=="WEATHER_API_FINGERPRINT"){
        WEATHER_API_FINGERPRINT = value;
      }else if (key == "CLOUD_TOKEN") {
        CLOUD_TOKEN = value;
      }else if (key == "CLOUD_URL") {
        CLOUD_URL = value;
      }
    }
  }
  // Seriell debug output
  Serial.println(WIFI_SSID);
  Serial.println(WIFI_PASS);
  Serial.println(WEATHER_API_TOKEN);
  Serial.println(WEATHER_API_URL);
  Serial.println(WEATHER_API_FINGERPRINT);
  Serial.println(CLOUD_TOKEN);
  Serial.println(CLOUD_URL);
  file.close();
}

void connectWifi(){
  Serial.println("\nConnecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  //Due to problems with dns server I had to change to google dns
  while (WiFi.status() != WL_CONNECTED) {
    waitForButtonPress(1000);
    Serial.print(".");
  }
  IPAddress local = WiFi.localIP();
  IPAddress gateway = WiFi.gatewayIP();
  IPAddress subnet = WiFi.subnetMask();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.config(local, gateway, subnet, IPAddress(8,8,8,8), IPAddress(8,8,4,4));
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    waitForButtonPress(1000);
    Serial.print(".");
  }
  Serial.println("\r\nWiFi connected!");
}

void disconnectWifi(){
  WiFi.disconnect();
}

void loadPoi(){
  File file = LittleFS.open("/poi.json", "r");
  if (!file) {
   Serial.println("Fehler: poi.json nicht gefunden!");
    return;
  }

  DynamicJsonDocument doc(1024); // dynamic json size bigger then file size of poi.json
  DeserializationError error = deserializeJson(doc, file); //parse
  file.close();

  if (error) {
    Serial.println( String("Fehler beim Parsen: ") + error.c_str());
    return;
  }

  // read
  JsonArray poiArray = doc["POI"];
  int i = 0;
  for (const char* poi : poiArray) {
    if (i < MAX_POI) {
      PointsOfInterest[i] = poi; 
      i++;
    }
  }

  // debug
  for (int j = 0; j < i; j++) {
    Serial.println(String("POI[") + j + "]: " + PointsOfInterest[j]);
  }
}

void printSingleData(String s[], size_t len){
  for (int i=0; i <len;i++){
    Serial.println(crutialColumns[i]+": "+ s[i]);
  }
}

void writeDataToCSV(String filename){
  File csv = LittleFS.open("/"+filename+".csv", "a"); 
  if(!csv){
    Serial.println("Fehler im LittleFS, beim öffnen oder anlegen der Datei");
    return;
  }
  size_t size = csv.size();
  Serial.println(size);
  if (size == 0){ //create header if file is empty
    Serial.println("Datei ist leer");
    String header = "";
    for(String column : crutialColumns){
      header = header + column + ";";
    }
    header.remove(header.length()-1); //remove last ;
    csv.print(header+"\n");
  }
  else if ((size + DATA_ROW_SIZE) > 1024){  // MAX_FILE_SIZE
    Serial.println("Datei ist voll");

    currentFileCounter++; //increase counter
    
    if (cloudUpload(currentFilename + String(currentFileCounter-1))){//upload old file
      deleteFile(currentFilename + String(currentFileCounter-1)+".csv");
    } 
    writeDataToCSV(currentFilename + String(currentFileCounter)); //safe data to new file
    
  }
  else{ // add data
    Serial.println("Daten werden geschrieben");
    String row = "";
    for (String value: crutialValues){
      row = row + value +";";
    }
    row.remove(row.length()-1); //remove last ;
    csv.print(row+"\n");
  }

}

void apiRequests(){
  WiFiClientSecure client;
  client.setFingerprint(WEATHER_API_FINGERPRINT.c_str());
  HTTPClient https;

  for (String point : PointsOfInterest){
    if (point == "") continue;
    // replace empty space and umlaute
    point.replace(" ","%20");
    point.replace("ä", "%C3%A4");
    point.replace("ö", "%C3%B6");
    point.replace("ü", "%C3%BC");
    point.replace("ß", "%C3%9F");

    String url = WEATHER_API_URL + "key=" + WEATHER_API_TOKEN + "&q=" + point + "&aqi=no";
    Serial.println("Request URL: " + url);
    waitForButtonPress(50);

    if (https.begin(client, url)) {
      int httpCode = https.GET();

      if (httpCode == HTTP_CODE_OK) {
        String res = https.getString();
        Serial.println("New API data:");
        
        // now deserialize response
        DynamicJsonDocument doc(4096); 
        DeserializationError error = deserializeJson(doc, res);

        if (error) {
          Serial.println("JSON parse failed: " + String(error.c_str()));
          return;
        }
        for (int i = 0; i<int (sizeof(crutialColumns)/sizeof(crutialColumns[0])); i++){
          if (i<4){
            crutialValues[i] = String(doc["location"][crutialColumns[i]]);
          } else if (i < 7){
            crutialValues[i] = String(doc["current"][crutialColumns[i]]);
          } else if (i == 7) crutialValues[i] = String(doc["current"]["condition"]["text"]);
            else if (i == 8) crutialValues[i] = String(doc["current"]["condition"]["code"]);
            else{
              crutialValues[i] = String(doc["current"][crutialColumns[i]]);
            }
        }
        //printSingleData(crutialValues, int(sizeof(crutialValues)/sizeof(crutialValues[0])));
        writeDataToCSV(currentFilename + String(currentFileCounter));
        std::fill(std::begin(crutialValues), std::end(crutialValues), "");

      } else {
        Serial.println("API call error: " + String(httpCode));
        Serial.println( https.errorToString(httpCode));
      }
      https.end();
    }
  }
}

bool cloudUpload(String filename){
  //upload old file.
  return true;
}

void listFiles() {
    Serial.println("Dateien im LittleFS:");
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    Serial.printf("  %s\t (%d Bytes)\n", dir.fileName().c_str(), dir.fileSize());
  }
}

void deleteFile(String filename){
   if (LittleFS.exists(filename)) {
    Serial.println(filename +" existiert");
    if (LittleFS.remove(filename)) {
      Serial.println("Datei erfolgreich gelöscht");
    } else {
      Serial.println("Fehler beim Löschen");
    }
   }
   if (LittleFS.exists(filename)) Serial.println(filename +" existiert immernoch");
   listFiles();
}

