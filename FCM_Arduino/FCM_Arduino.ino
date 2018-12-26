// External Libaries 
// https://github.com/aharshac/StringSplitter
#include "StringSplitter.h"
#include "ArduinoNTCLibary.h"

// Comunication
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

// Panel
#define panel_power 4
#define panel_led 3
#define panel_button 2


//Periods
unsigned long period_log_temp = millis() + 200;
unsigned long period_heat = millis() + 200;
unsigned long period_wait = millis() + 1000;
bool heater_status = false;
// Heater
#define HEATER 10

float high_temp = 0;
float TARGET_TEMPERATURE = 0;

// Termistors
NTC termistor_1 = NTC();
float termistor_1_temp = 0;

// System
bool SYSTEM = false;

void setup() {
  // initialize serial:
  Serial.begin(9600);
  // reserve 00 bytes for the inputString:
  inputString.reserve(100);  
  
  // Pin modes
  pinMode(HEATER, OUTPUT);
  pinMode(A0,INPUT);
  pinMode(panel_button,OUTPUT);
  pinMode(panel_power,OUTPUT);
  pinMode(panel_led,OUTPUT);
  pinMode(13, OUTPUT);
  
  // Termistor Setup
  termistor_1.setNTCPin(A0);
  termistor_1.setReferanceResistance(9999);
  termistor_1.setReferanceBetaConfidency(4194.0);
  termistor_1.setReferanceResistanceAt25C(99730);
  termistor_1.setReferanceMaxVoltageOfADC(4.60);
  termistor_1.setReferanceMaxADC(1024.00);

}

void loop() {
  digitalWrite(panel_power,HIGH);
  
  // print the string when a newline arrives:
  if (stringComplete) {
    //Serial.print(inputString);
    //parseMessages(inputString);
    parseMessages(inputString);
    //Serial.print("Received:"+inputString+"-");
    inputString = "";
    stringComplete = false;
  }
  
  if(!Serial.available() and millis()>period_log_temp){
      period_log_temp = millis() + 500;
      termistor_1_temp = termistor_1.measureNTCTemperatureCelcius(20);
      Serial.println("SET-CURRENT_TEMPERATURE-" + (String)termistor_1_temp);
  }

  if(SYSTEM){
    heat(termistor_1_temp, TARGET_TEMPERATURE, HEATER);
  }
  else{
    analogWrite(HEATER,0);
    digitalWrite(panel_led,HIGH);
  }

}

void heat(float current_temp ,float target_temp ,int heater){
  int error = 0;
  if(period_heat>millis()){
    digitalWrite(heater,HIGH);
    digitalWrite(panel_led,LOW);
    heater_status = true;
  }
  else if(period_wait>millis()){
    digitalWrite(heater,LOW);
    digitalWrite(panel_led,HIGH);
    heater_status = false;
  }
  else{
    if(current_temp<target_temp){
      if(heater_status)
        period_wait = millis()+1000+(500/(target_temp-current_temp));
      else
        period_heat = millis()+100-(100/(target_temp-current_temp));
    }
    else if (current_temp>target_temp+1){
      digitalWrite(heater,LOW);
      digitalWrite(panel_led,HIGH);
    }
  }
 
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
char inChar = '\0';
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    inChar = (char)Serial.read();
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
    // add it to the inputString
    else{
      inputString += inChar;
    }
    inChar = '\0';
    
    
  }
}
String data = "";

void parseMessages(String inputString){
  
    StringSplitter *splitter = new StringSplitter(inputString, '-', 5);
    data = splitter->getItemAtIndex(0);
    if(data=="GET"){
      data = splitter->getItemAtIndex(1);
      
    }
    else if(data=="SET"){
      data = splitter->getItemAtIndex(1);
      //SET Target Temperature
      if(data=="TARGET_TEMPERATURE"){
        TARGET_TEMPERATURE = splitter->getItemAtIndex(2).toFloat();
        //Serial.println("SET-TOAST-"+splitter->getItemAtIndex(2));
      }
      //Machine On OFF Status
      else if(data=="SYSTEM"){
        data = splitter->getItemAtIndex(2);
        if(data=="ON"){
          SYSTEM = true;
        }
        else{
          SYSTEM = false;
        }
      }  
    
    }
    else{
    }
    data = "";
    delete splitter;
}
