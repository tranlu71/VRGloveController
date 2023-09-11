 #include <Arduino.h>
int pin = A0;
void setup() {
  Serial.begin(9600);
  

}

void loop() {
  int forcereading = analogRead(pin);
  Serial.println(forcereading);
  delay(500);

}
