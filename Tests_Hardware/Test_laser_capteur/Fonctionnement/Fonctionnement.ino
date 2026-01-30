
const int laserPin = 9;
const int capteurPin = A0;
int val_capteur;

const int toggle_delay = 1;// en ms


void setup() {

  pinMode(laserPin, OUTPUT);
  Serial.begin(2000000);               // 57600, 115200, 230400, 460800, 
}

void loop() {
  
  switchLaser();
  
}



// fonction pour commuter le laser
void switchLaser(){
  
  digitalWrite(laserPin, !digitalRead(laserPin));
  delay(toggle_delay);
}


//fonction retournant la valeur du capteur
int get_sensor_value(){

  int value = analogRead(capteurPin); // changer en foction de ce que l'on veut vérifier
  //delay(sense_delay);
  
  return value; 
}
