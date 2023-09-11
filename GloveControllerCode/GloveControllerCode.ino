#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined(ARDUINO_ARCH_SAMD)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
//#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"
#include "keycode.h"
#define FACTORYRESET_ENABLE         0
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

typedef struct
{
  uint8_t modifier;   /**< Keyboard modifier keys  */
  uint8_t reserved;   /**< Reserved for OEM use, always set to 0. */
  uint8_t keycode[6]; /**< Key codes of the currently pressed keys. */
} hid_keyboard_report_t;


// Report that send to Central every scanning period
hid_keyboard_report_t keyReport1 = { 0, 0, { 0 } }, keyReport2 = { 0, 0, { 0 } }, keyReport3 = { 0, 0, { 0 } } ;
hid_keyboard_report_t previousReport1 = { 0, 0, { 1 } }, previousReport2 = { 0, 0, { 1 } }, previousReport3 = { 0, 0, { 1 } };

// GPIO corresponding to HID keycode (not Ancii Character)
const int FlexInputPins[5]     = {A5, 4, 3, 2, 1}; 
const int CalButton            = {10};
const int ForceInputPins[2]    = {A7, 0};
int inputKeycodes[15] = {HID_KEY_Z, HID_KEY_Q, HID_KEY_W, 
                         HID_KEY_E, HID_KEY_R, HID_KEY_T, 
                         HID_KEY_A, HID_KEY_S, HID_KEY_D, 
                         HID_KEY_F, HID_KEY_G, HID_KEY_H,
                         HID_KEY_J, HID_KEY_K, HID_KEY_X};
int Flex_baseline[5] = {1023, 1023, 1023, 1023, 1023}, Force_baseline[2] = {1023, 1023};
int FlexMax[5] = {0, 0, 0, 0, 0}, ForceMax[2] = {0, 0}; 
int flexState[5] = {0, 0, 0, 0, 0};
int prevflexState[5] = {0, 0, 0, 0, 0};
int forceState[2] = {0, 0};
int prevforceState[2] = {0, 0};


/**************************************************
 **************** Autosent Keycode*****************
 **************************************************/
void sendKeycode(int num){
if (ble.isConnected()){
      
       switch (num)
        {
        case 0:
          keyReport1.keycode[0] = inputKeycodes[num];
          ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport1, 8);     
          Serial.print(F("Letter: "));
          Serial.println(inputKeycodes[num]);
          keyReport1.keycode[0] = 0;
          ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport1, 8); 
        case 14:
          keyReport2.keycode[4] = inputKeycodes[num];
          ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport2, 8);     
          Serial.print(F("Letter: "));
          Serial.println(inputKeycodes[num]);
          keyReport2.keycode[4] = 0;
          ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport2, 8);    
        } 
        
}
}
/**************************************************
 **************** Flex Sensor Keycode**************
 **************************************************/
void sendKeycodeFlex(int num, int state[5], int prevstate[5]){
if(ble.isConnected()){
int FlexReading = analogRead(FlexInputPins[num]);
if (num == 3) /* index finger */ {
  if (FlexReading <= Flex_baseline[num] +  0.75 *(FlexMax[num] - Flex_baseline[num])){
    state[num] = 1;
  } else {state[num] = 0;}
}
else{ /* other fingers */
  if (FlexReading <= Flex_baseline[num] +  0.7*(FlexMax[num] - Flex_baseline[num])){
    state[num] = 1;
  } else {state[num] = 0;}
} 
Serial.print(F("Flex sensor ")); Serial.print(num); Serial.print(F(" reading: ")); Serial.println(FlexReading);
if ((state[num] && !prevstate[num]) || (!state[num] && prevstate[num])){
       keyReport1.keycode[num+1] = inputKeycodes[num+1];     
} 
prevstate[num] = state[num];   
// Only send if it is not the same as previous report
// Send keyboard report
if ( memcmp(&previousReport1, &keyReport1, 8) ){
   Serial.println(F("Entering ble sending"));
   ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport1, 8);
   // copy to previousReport
   memcpy(&previousReport1, &keyReport1, 8);
   Serial.print(F("Letter: "));
   Serial.println(inputKeycodes[num+1]);
}
// reset
keyReport1.keycode[num+1] = 0;
ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport1, 8);
}
}
/**************************************************
 **************** Force Sensor Keycode*************
 **************************************************/
void sendKeycodeForce(int num, int state[2], int prevstate[2]){
if(ble.isConnected() ){
int ForceReading = 1023-analogRead(ForceInputPins[num]);
if (ForceReading <= Force_baseline[num] +  0.9*(ForceMax[num] - Force_baseline[num])){
    state[num] = 3; 
}
else if (ForceReading <= Force_baseline[num] +  0.6*(ForceMax[num] - Force_baseline[num])){
    state[num] = 2;
}
else if (ForceReading <= Force_baseline[num] +  0.3*(ForceMax[num] - Force_baseline[num])){
    state[num] = 1;
} 
else {state[num] = 0;}
Serial.print(F("Force sensor ")); Serial.print(num); Serial.print(F(" reading: ")); Serial.println(ForceReading);

switch (num)
    {
    case 0: 
            Serial.print(F("Prev State and State: "));
            Serial.println(prevstate[num]);
            Serial.println(state[num]);
            if (prevstate[num] != state[num]) {
              keyReport2.keycode[state[num]] = inputKeycodes[6+state[num]];
              Serial.print(F("Letter: "));
              Serial.println(inputKeycodes[6+state[num]]);
              ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport2, 8);
            } 
            
            prevstate[num] = state[num];  
            keyReport2.keycode[state[num]] = 0;
            ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport2, 8);       
    case 1:
            if (prevstate[num] != state[num]) {
               keyReport3.keycode[state[num]] = inputKeycodes[10+state[num]];
               Serial.print(F("Letter: "));
               Serial.println(inputKeycodes[10+state[num]]);
               ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport3, 8);
            }
            prevstate[num] = state[num];  
            keyReport3.keycode[state[num]] = 0;
            ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport3, 8);
     }
}
}
/**************************************************
 **************** CALIBRATION ROUTINE**************
 **************************************************/
void Calibration(){
// reset states
for (int i=0; i<5; i++){
  flexState[i] = 0;
  prevflexState[i] = 0;
}
for (int i=0; i<2; i++){
  forceState[i] = 0;
  prevforceState[i] = 0;
}  
if(ble.isConnected()){
  Serial.println(F("Calibration begin")); 
  /* Flex Sensor Calibration */
  sendKeycode(0); /* Signaling glove is powered on/calbutton is pressed by sending Z. UnityScene: Request User to relax hand */

  delay(10000); 
  
  /* Logging in the baseline while the hand is relaxed */
  for(int i=0; i<5; i++){
    Flex_baseline[i] = analogRead(FlexInputPins[i]);
    Serial.print(F("Flex sensor ")); Serial.print(i); Serial.print(F(" baseline: ")); Serial.println(Flex_baseline[i]);
  };
  Force_baseline[0] = 1023-analogRead(ForceInputPins[0]);
  Force_baseline[1] = 1023-analogRead(ForceInputPins[1]);
  Serial.print(F("Thumb Force sensor ")); Serial.print(F(" baseline: ")); Serial.println(Force_baseline[0]);
  Serial.print(F("Palm Force sensor ")); Serial.print(F(" baseline: ")); Serial.println(Force_baseline[1]);
  sendKeycode(14);
        /* Start timer on glove and Unity if sensing change */
        bool changed1 = false; 
        while(!changed1){
          int count = 0;
          for (int i=0; i<5; i++) {
              if (analogRead(FlexInputPins[i]) < 0.95*Flex_baseline[i]){
                count = count + 1;
              }
          }
          if (count == 1) {
            changed1 = true;
            break;
          }
        };
        if(changed1){
          sendKeycode(14); /* If change is sensed, start timer. UnityScene: Countdown 10s */
          
          delay(10000);
          for(int i=0; i<5; i++){
            FlexMax[i] = analogRead(FlexInputPins[i]); 
            Serial.print(F("Flex sensor ")); Serial.print(i); Serial.print(F(" max: ")); Serial.println(FlexMax[i]);
          }
          sendKeycode(14); /* signaling FlexMax is logged. UnityScene: Times up */ 
         
        };


  /* Force Calibration Routine*/
  /* UnityScene: Thumb Force Sensor */
      /* Start timer on glove and Unity if sensing change */
      bool changed2 = false; 
      while(!changed2){
                  if (1023-analogRead(ForceInputPins[0]) < 0.98*Force_baseline[0]){
                    changed2 = true;
                    break;
                  }
      };
      if (changed2) {
          sendKeycode(14); /* If change is sensed, start timer. UnityScene: Countdown 10s */
          delay(10000);
          ForceMax[0] = 1023-analogRead(ForceInputPins[0]);
          Serial.print(F("Thumb Force sensor ")); Serial.print(F(" max: ")); Serial.println(ForceMax[0]);
          sendKeycode(14); /* signaling ForceMax is logged. UnityScene: Times up */
      };
      
  /* UnityScene: Palm Force Sensor */
      /* Start timer on glove and Unity if sensing change */
      changed2 = false; 
      while(!changed2){
                  if (1023-analogRead(ForceInputPins[1]) < 0.98*Force_baseline[1]){
                    changed2 = true;
                    break;
                  }
      };
      if (changed2) {
          sendKeycode(14); /* If change is sensed, start timer. UnityScene: Countdown 10s */
          delay(10000);
          ForceMax[1] = 1023-analogRead(ForceInputPins[1]);
          Serial.print(F("Thumb Force sensor ")); Serial.print(F(" max: ")); Serial.println(ForceMax[1]);
          sendKeycode(14); /* signaling ForceMax is logged. UnityScene: Times up */
      };

}
}


void setup(void)
{
  //while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit HID Keyboard Example"));
  Serial.println(F("---------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    ble.factoryReset();
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Enable HID Service if not enabled */
  int32_t hid_en = 0;
  
  ble.sendCommandWithIntReply( F("AT+BleHIDEn"), &hid_en);

  if ( !hid_en )
  {
    Serial.println(F("Enable HID Service (including Keyboard): "));
    ble.sendCommandCheckOK(F( "AT+BleHIDEn=On" ));

    /* Add or remove service requires a reset */
    Serial.println(F("Performing a SW reset (service changes require a reset): "));
    !ble.reset();
  }
  
  Serial.println();
  Serial.println(F("Go to your phone's Bluetooth settings to pair your device"));
  Serial.println(F("then open an application that accepts keyboard input"));
  Serial.println();

  pinMode(CalButton, INPUT_PULLUP);
}





void loop(){
   if (digitalRead(CalButton) == LOW) {
    Calibration();
   }
   for (int i = 0; i < 5; i++){
    sendKeycodeFlex(i, flexState, prevflexState);
   }
   for (int i = 0; i < 2; i++){
    sendKeycodeForce(i, forceState, prevforceState);
   }
   // scaning period is 10 ms
   delay(10);
}
   
      
        
           
         
          
         
  
  

  
