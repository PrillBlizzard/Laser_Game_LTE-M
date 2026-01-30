#include <VCNL3040.h>

VCNL3040 docky;

uint8_t res;

void setup() {
  Serial.begin(2000000);

  docky.begin();
  docky.startReading();
  
}

void loop() {

  res = docky.readPSData();
  if(res != 0){
    Serial.println(res);
  }
  delayMicroseconds(26);
  
}
