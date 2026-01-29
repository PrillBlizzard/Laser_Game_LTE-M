

#include <IRremote.h>


  IRsend irmitter;
  unsigned int frame = 0xFF11;

void setup() {
  Serial.begin(2000000);

  pinMode(2,OUTPUT);
  pinMode(13,OUTPUT);
  pinMode(8,OUTPUT);
}

void loop() {

  if(digitalRead(8))
  {

    digitalWrite(13,HIGH);
    irmitter.sendNEC(frame,32);  
     delay(100);
    digitalWrite(13,LOW);
    delay(100);
    
  }

  //Serial.println(frame, HEX);
  //Serial.println(digitalRead(3));
}
