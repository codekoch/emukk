#include <ButtonDebounce.h>
#define BTN_PIN 10
#define LED_BLUE 3
#define LED_YELLOW 4
#define SWITCH_PIN 11
bool analog;
bool midi;
ButtonDebounce button1(BTN_PIN, 50);

void setup() {
  Serial.begin(31250);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SWITCH_PIN, OUTPUT);
  digitalWrite(SWITCH_PIN, LOW);

  digitalWrite(LED_BLUE, LOW);
  pinMode(LED_YELLOW, OUTPUT);
  digitalWrite(LED_YELLOW, LOW);
  button1.setCallback(button1pressed);
  analog = true;
  midi = false;
}
void loop() {
  if (analog)
  {
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(SWITCH_PIN, LOW);
  }
  else
  {
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(SWITCH_PIN, HIGH);
  }
  if (midi)
  {
    handleMidi();
       digitalWrite(LED_BLUE, HIGH);
  }
  else
  {
    digitalWrite(LED_BLUE, LOW);
  }

  button1.update();
}
void button1pressed(int state)
{
  if (!state)
  {
    return;
  }
  analog=!analog;
  midi=!midi;
}
void handleMidi(){
  if (Serial.available()) {
    uint8_t newByte = Serial.read();
    Serial.write(newByte);
   
  }
 
}
