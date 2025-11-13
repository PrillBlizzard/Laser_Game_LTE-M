

#include <IRremote.h>


  IRsend irmitter;
  unsigned int frame = 0xABCD;

void setup() {
  Serial.begin(2000000);
  Serial.println("Starting Emitting");
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
}

void loop() {

  irmitter.sendNEC(frame,32);  
  delay(500);
    
  Serial.println(frame, HEX);
  Serial.println(digitalRead(3));
}
