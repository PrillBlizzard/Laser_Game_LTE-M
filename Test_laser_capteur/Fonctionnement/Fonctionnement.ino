
const int laserPin = 2;
const int capteurPin = 7;
int val_capteur;

const int toggle_delay = 100; // en ms
const int sense_delay = 100; // en ms

void setup() {

  pinMode(laserPin, OUTPUT);
  pinMode(capteurPin, INPUT);
  Serial.begin(9600);               // 57600, 115200, 230400, 460800
}

void loop() {
  
  switchLaser();
  get_sensor_value();
  
  //val_capteur = digitalRead(capteurPin);
  //delay(225);
  
  val_capteur = get_sensor_value();
  Serial.println(val_capteur);
}



// fonction pour commuter le laser
void switchLaser(){
  
  digitalWrite(laserPin, !digitalRead(laserPin));
  delay(toggle_delay);
}


//fonction retournant la valeur du capteur
int get_sensor_value(){

  int value = digitalRead(capteurPin);
  delay(sense_delay);
  
  return value; 
}
