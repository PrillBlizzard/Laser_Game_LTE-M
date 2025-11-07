

#include <IRremote.h>


  IRsend irmitter;
  unsigned int frame = 0x00;

void setup() {
  Serial.begin(2000000);
  irmitter.enableIROut(38);
  Serial.println("Starting Emitting");
}

void loop() {

  frame += 1;
  irmitter.sendNEC(frame,8);  
  delay(1000);
}
