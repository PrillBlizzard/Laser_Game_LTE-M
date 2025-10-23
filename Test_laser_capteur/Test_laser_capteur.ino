
const int laserPin = 2;
const int capteurPin = 7;
int val_capteur;


void setup() {
  // put your setup code here, to run once:

  pinMode(laserPin, OUTPUT);
  pinMode(capteurPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  switchLaser();
  delay(250);
  
  val_capteur = digitalRead(capteurPin);
  Serial.println(val_capteur);

  delay(250);

}

void switchLaser(){
  
  digitalWrite(laserPin, !digitalRead(laserPin));
  
}
