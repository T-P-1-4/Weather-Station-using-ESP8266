#include <Arduino.h>
#include <AALeC-V3.h>


// put function declarations here:
int myFunction(int, int);
void setLEDsOff(int);
void setDisplayOff();
bool wait_for_button_press(uint16_t);

// global vars
unsigned long messureDistanceInMillis = 60000; //15 min aka 900 s 
int counter=0;
bool buttonPressed = false;
bool displayON = true;
float currentTemp = 0;
float currentPressure = 0;

void setup() {
  // put your setup code here, to run once:
  aalec.init(5);
  int result = myFunction(counter, 1);
  aalec.print_line(1,result);
  setLEDsOff(5);
}

void loop() {
  // put your main code here, to run repeatedly:
  // update values of p and t
  currentPressure = aalec.get_pressure();
  currentTemp = aalec.get_temp();

  
  unsigned long startTime = millis();
  // wait till difference between start and current time is less than delayTime
  while ((millis() - startTime < messureDistanceInMillis)){

    if (wait_for_button_press(2000))  displayON = !displayON;

    counter = myFunction(counter,2);
    if (displayON){
    aalec.print_line(1,counter);
    aalec.print_line(2, "Druck: "+ String(currentPressure));
    aalec.print_line(3,"Temperatur: "+String(currentTemp));
    }
  }

}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
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

bool wait_for_button_press(uint16_t delayTime){
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