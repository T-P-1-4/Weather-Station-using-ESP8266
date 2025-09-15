#include <Arduino.h>
#include <AALeC-V3.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>


// put function declarations here:
void setLEDsOff(int);
void setDisplayOff();
bool waitForButtonPress(uint16_t);
void loadEnvVars();
void connectWifi();
void disconnectWifi();

// global vars
unsigned long messureDistanceInMillis = 60000; //15 min aka 900 s 
bool buttonPressed = false;
bool displayON = true;
float currentTemp = 0;
float currentPressure = 0;
String WIFI_SSID, WIFI_PASS, WEATHER_API_TOKEN, WEATHER_API_URL, CLOUD_TOKEN, CLOUD_URL; // Vars from .env file

void setup() {
  aalec.init(5);
  setLEDsOff(5);
  loadEnvVars();
}

void loop() {
  // put your main code here, to run repeatedly:
  connectWifi();
  aalec.print_line(4,"wifi connected");
  delay(10000);
  //do smth
  disconnectWifi();
  aalec.print_line(4,"wifi disconnected");
  
  // update values of p and t
  currentPressure = aalec.get_pressure();
  currentTemp = aalec.get_temp();


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
  if (!LittleFS.begin()) {
    Serial.println("LittleFS konnte nicht gemountet werden");
    return;
  }
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
  Serial.println(CLOUD_TOKEN);
  Serial.println(CLOUD_URL);
  file.close();
}

void connectWifi(){
  Serial.println("\nConnecting to WiFi...");
  WiFi.mode(WIFI_STA);
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
