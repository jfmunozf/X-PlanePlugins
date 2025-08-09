#define ButtonPin 3

void setup() {
  Serial.begin(115200);
  pinMode(ButtonPin, INPUT);
}
void loop() {
  int ButtonState = digitalRead(ButtonPin);
  if (ButtonState == LOW) {
    Serial.println("0"); }
  else {
    Serial.println("1"); 
  }

  delay(100);
}
