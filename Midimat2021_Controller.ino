
// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(13, 12, 8, 9, 10, 11);

// Poti
int controlChange = 176; // MIDI Kanal 1
int controllerNummer = 21;
int controllerWert = 0;
int controllerWertAlt = 0;
int potiWert = 0;

const int buttonNr = 16; // Anzahl der Buttons insgesamt
const int readPinForButtons1 = 2;
const int readPinForButtons2 = 3;
const int icSelectPinA = 5;
const int icSelectPinB = 6;
const int icSelectPinC = 7;
const int writePinForLEDs = 4; // Enable IC LOW=On
const int ICGreen1 = 16;
const int ICGreen2 = 17;
const int ICRed1 = 18;
const int ICRed2 = 19;
const int shiftButtonNr = 15; // Nummer des Shift-Buttons
const int allButtonNr = 16;  // Alle Parts an
const int buttonNoteDown[8] = {25, 26, 27, 28, 29, 30, 31, 32} ; // 8 Midinoten für die unteren 8 Buttons (jeder Button hat zwei Noten bis auf den Shift-Button}
const int buttonNoteDownShift[8] = { 43, 45, 47, 48, 50, 52, 53, 55};
const int buttonNoteUp[8] = {33, 34, 35, 36, 38, 40, 41, 59};
const int buttonNoteUpShift[8] =  {57, 42, 44, 46, 54, 56, 58, 59};
const int shiftPartButtonNr = 9;
const int partButtonNr = 15 ;
const int partButtons[partButtonNr] = {0, 1, 2, 3, 4, 5, 6 , 7, 8 , 9 , 10 , 11, 12, 13, 14};
const int shiftPartButtons[shiftPartButtonNr] = {0, 1, 2, 3, 4, 5, 6 , 7, 8 };

int button[buttonNr]; // Signal
int buttonOld[buttonNr];
int buttonNote[buttonNr];
int buttonShiftNote[buttonNr];
boolean buttonOn[buttonNr];
boolean buttonOnShift[buttonNr]; // ShiftSignal
boolean shiftButtonWait1;
boolean shiftButtonWait2;
boolean midithru = true;
void setup() {

  //Serial.begin(9600);
  Serial.begin(31250);
  //Serial.begin(38400);

  pinMode(readPinForButtons1, INPUT);  // Buttons 0-7 über IC4051 1
  pinMode(readPinForButtons2, INPUT);  // Buttons 8-15 über IC4051 2
  pinMode(writePinForLEDs, OUTPUT); // Green LEDs über IC4051 3
  pinMode(ICGreen1, OUTPUT); // Red LEDs über IC4051 4
  pinMode(ICGreen2, OUTPUT); // Green LEDs über IC4051 3
  pinMode(ICRed1, OUTPUT); // Red LEDs über IC4051 4
  pinMode(ICRed2, OUTPUT); // Red LEDs über IC4051 4
  pinMode(icSelectPinA, OUTPUT);
  pinMode(icSelectPinB, OUTPUT);
  pinMode(icSelectPinC, OUTPUT);
  logo();
  lcd.clear();
  // Initialisierung der Buttons
  for (int i = 0; i < (buttonNr); i = i + 1)
  {
    button[i] = LOW;
    buttonOld[i] = LOW;
    if ( i < 8) // untere Buttons
    {
      buttonNote[i] = buttonNoteDown[i]; // Vergeben der Midinoten
      buttonShiftNote[i] = buttonNoteDownShift[i];
    }
    if ( i >= 8) // obere Buttons
    {
      buttonNote[i] = buttonNoteUp[i - 8]; // Vergeben der Midinoten
      buttonShiftNote[i] = buttonNoteUpShift[i - 8];
    }



    if ( partButtons[0] == i )
    {
      buttonOn[i] = true;
      lcd.setCursor(i, 1);
      lcd.print(i + 1);
    }
    else
    {
      buttonOn[i] = false;
    }
    buttonOnShift[i] = false;

  }
  setIC(0);
  button[0] = digitalRead(readPinForButtons1);
  button[8] = digitalRead(readPinForButtons2);
 
  
  if ( button[0] == HIGH && button[8] == HIGH)
  {
    lcd.clear();
    // Print a message to the LCD.
    lcd.setCursor(0, 0);
    lcd.print("ZUFFI LIGHT MODE");
    lcd.setCursor(0, 1);
    lcd.print("                ");

    zuffiMode();
  }
   if ( button[0] == HIGH )
   {
    lcd.setCursor(0, 0);
    lcd.print("MIDI THRU OFF");
    midithru = false;
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("                ");
   } 
   else
   {
   //  lcd.clear();
    // Print a message to the LCD.
    lcd.setCursor(0, 0);
    lcd.print("MIDI THRU ON");
    midithru = true;
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("                ");
   }
   

}

void loop() {
  CheckSerial();
  for (int i = 0; i < buttonNr; i = i + 1) {
    checkButton(i);
  }
  readPoti();

  ledLight();
}

void logo()
{
  lcd.begin(16, 2);
  for (int i = 0; i < 16; i++)
  {
    lcd.clear();
    lcd.setCursor(20 - i, 0);
    lcd.print("Thjims");
    lcd.setCursor(i - 16, 1);
    lcd.print("MIDIBASSCONTROL");
    delay(50);
  }
  ledTest();
  //delay(2000);

}


void stopAllShift()
{
  for (int i = 0; i < buttonNr; i = i + 1)
  {
    buttonOnShift[i] = false;
  }
  shiftButtonWait1 = false;
  shiftButtonWait2 = false;
}

void readPoti()
{

  potiWert = analogRead(A0);
  controllerWert = map((1023 - potiWert), 0, 1023, 0, 127);
  if (controllerWert != (controllerWertAlt) && controllerWert != (controllerWertAlt + 1) && controllerWert != (controllerWertAlt - 1)) {
    Serial.write(controlChange);
    Serial.write(controllerNummer);
    Serial.write(controllerWert);
    controllerWertAlt = controllerWert;
    unsigned long startedAt = millis();
    digitalWrite(writePinForLEDs, LOW);
    while (millis() - startedAt < (250 - map((1023 - potiWert), 0, 1023, 0, 16) * 1))

      for (int i = 0; i <= map((1023 - potiWert), 0, 1023, 0, 16) ; i++)
      {
        setIC(i);
        if (i < 8)
        {
          digitalWrite(ICRed1, HIGH);
          delay(1);
          digitalWrite(ICRed1, LOW);
        }
        else
        {
          digitalWrite(ICRed2, HIGH);
          delay(1);
          digitalWrite(ICRed2, LOW);
        }
      }


  }



}

void checkButton(int i) // button i auf Druck ueberpruefen
{

  if (i < (buttonNr / 2) ) { // ersten 8 Buttons (8 Spurbuttons) über IC 1 an DIG 2 registrieren
    // IC Einstellungen setzen
    setIC(i);
    button[i] = digitalRead(readPinForButtons1);
  }

  if (i >= (buttonNr / 2) && i < buttonNr) { // weiteren 8 Buttons (6 Partbuttons + PLAY und Shiftord ) über IC 2 an DIG 4 registrieren
    // IC Einstellungen setzen
    setIC(i);
    button[i] = digitalRead(readPinForButtons2);
  }

  if (button[i] == HIGH && buttonOld[i] == LOW ) {
    if (shiftButtonWait1 && shiftButtonWait2) // Shifttaste ist gedrückt
    {
      Serial.write(144); // 1001 0000 = Note On Kanal 1
      Serial.write(buttonShiftNote[i]); //Note
      Serial.write(127); // VollePulle

      lcd.setCursor(7, 0);  // ShiftTaste deaktivieren
      lcd.print(" ");
      shiftButtonWait1 = false;
      buttonOn[shiftButtonNr] = false; // Shift-LED OFF
      if (!buttonOnShift[i]) // Setze Button Display
      {
        buttonOnShift[i] = true;  // Merke den Buttonaktivität inkl. Shift
        if (i < 8)
        {
          lcd.setCursor(8 + i, 1);
          lcd.print(i + 1);
        }
        else
        {
          lcd.setCursor(i, 0);
          lcd.print(i + 1 - (buttonNr / 2));
        }
      }
      if (isPartButton(i, true)) // switche den Part
      {
        setPart(i, true);
      }
      else
      {
        ledLight();
        delay(1000);
        buttonOnShift[i] = false;
        if (i < 8)
        {
          lcd.setCursor(8 + i, 1);
          lcd.print(" ");
        }
        else
        {
          lcd.setCursor(i, 0);
          lcd.print(" ");
        }
      }
    }     // Shifttaste Ende
    else
    {
      if (!buttonOn[i]) {
        if (shiftButtonNr == i )   // shifttaste wurde gedrückt
        {
          lcd.setCursor(7, 0);
          lcd.print(">");
          shiftButtonWait1 = true;
          shiftButtonWait2 = true;
          buttonOn[shiftButtonNr] = true;
        }
        else     // keine Shifttaste
        {
          Serial.write(144); // 1001 0000 = Note On Kanal 1
          Serial.write(buttonNote[i]); //Note
          Serial.write(127); // VollePulle
          buttonOn[i] = true;
          if (i < (buttonNr / 2))
          {
            lcd.setCursor(i, 1);
            lcd.print(i + 1);
          }
          if (i >= (buttonNr / 2))
          {
            lcd.setCursor(i - (buttonNr / 2), 0);
            lcd.print(i + 1 - (buttonNr / 2));
          }
          if (isPartButton(i, false)) // switche Part
          {
            setPart(i, false);
          }
          else
          {
            ledLight();
            delay(1000);
            buttonOn[i] = false;
            if (i < (buttonNr / 2))
            {
              lcd.setCursor(i, 1);
              lcd.print(" ");
            }
            if (i >= (buttonNr / 2))
            {
              lcd.setCursor(i - (buttonNr / 2), 0);
              lcd.print(" ");
            }

          }
        }
      }
    }
  }
  if (button[i] == LOW && buttonOld[i] == HIGH) {  // Button wurde losgelassen
    if (shiftButtonWait2 && !shiftButtonWait1)
    {
      Serial.write(144); // 1001 0000 = Note On Kanal 1
      Serial.write(buttonShiftNote[i]); //Note
      Serial.write(0); // Weg
      shiftButtonWait2 = false;
    }
    else
    {
      if (!shiftButtonWait2 && !shiftButtonWait1)
      {
        Serial.write(144); // 1001 0000 = Note On Kanal 1
        Serial.write(buttonNote[i]); //Note
        Serial.write(0); // Weg
      }
    }
  }
  buttonOld[i] = button[i];

} // Ende check


void setIC(int ch)
{
  digitalWrite(icSelectPinA, bitRead(ch, 0));
  digitalWrite(icSelectPinB, bitRead(ch, 1));
  digitalWrite(icSelectPinC, bitRead(ch, 2));


}

void writeStatus() {
  Serial.println("");

  for (int i = 0; i < 16; i++)
  {
    Serial.print(i);
    if (buttonOn[i])
    {
      Serial.print("ON");
    }
    else
    {
      Serial.print("OFF");
    }
    if (buttonOnShift[i])
    {
      Serial.print("-REC");
    }
    Serial.print("|");

  }
  Serial.println("");

}
boolean isPartButton(int i, boolean shift)
{
  boolean exists = false;
  if (shift) {
    for (int j = 0; j < shiftPartButtonNr; j++)
    {
      {
        if (shiftPartButtons[j] == i) exists = true;
      }
    }
  }
  else
  {
    for (int j = 0; j < partButtonNr; j++)
    {
      {
        if (partButtons[j] == i) exists = true;
      }
    }
  }
  return exists;
}

void setPart(int i, boolean shift)
{
  for (int j = 0; j < 8; j++)
  {
    if ( partButtons[j] != i )      // lösche alle nicht gedrückten unteren Buttons
    {
      lcd.setCursor(partButtons[j], 1);
      lcd.print(" ");
      lcd.setCursor(8 + partButtons[j], 1);
      lcd.print(" ");
      buttonOn[partButtons[j]] = false;
      buttonOnShift[partButtons[j]] = false;
    }
    if ( partButtons[j] == i && !shift)
    {
      lcd.setCursor(8 + partButtons[j], 1);
      lcd.print(" ");
      buttonOnShift[partButtons[j]] = false;
    }
    else
    {
      lcd.setCursor(partButtons[j], 1);
      lcd.print(" ");
      buttonOn[partButtons[j]] = false;
    }
  }
  for (int j = 8; j < 15; j++)
  {
    if ( partButtons[j] != i )      // lösche alle nicht gedrückten oberen Buttons
    {
      lcd.setCursor(partButtons[j] - 8, 0);
      lcd.print(" ");
      lcd.setCursor(8 + partButtons[j] - 8, 0);
      lcd.print(" ");
      buttonOn[partButtons[j]] = false;
      buttonOnShift[partButtons[j]] = false;
    }
    if ( partButtons[j] == i && !shift)
    {
      lcd.setCursor(8 + partButtons[j] - 8, 0);
      lcd.print(" ");
      buttonOnShift[partButtons[j]] = false;
    }
    else
    {
      lcd.setCursor(partButtons[j] - 8, 0);
      lcd.print(" ");
      buttonOn[partButtons[j]] = false;
    }
  }


}

void ledLight() {
  unsigned long startedAt = millis();
  float intervall = 1;
  float light = 1;
  digitalWrite(writePinForLEDs, LOW);

  while (millis() - startedAt < intervall)
  {
    for (int i = 0; i < (buttonNr / 2); i = i + 1) {
      if (buttonOn[i]) {
        setIC(i);
        digitalWrite(ICGreen1, HIGH);
        delay(light);
        digitalWrite(ICGreen1, LOW);

      }
    }
  }
  digitalWrite(ICGreen1, LOW);

  startedAt = millis();
  while (millis() - startedAt < intervall) {
    for (int i = (buttonNr / 2); i < buttonNr; i = i + 1) {
      if (buttonOn[i]) {
        setIC(i);
        digitalWrite(ICGreen2, HIGH);
        delay(light);
      }
    }
  }

  digitalWrite(ICGreen2, LOW);
  startedAt = millis();
  while (millis() - startedAt < intervall)
  {
    for (int i = 0; i < (buttonNr / 2); i = i + 1) {
      if (buttonOnShift[i]) {
        setIC(i);
        digitalWrite(ICRed1, HIGH);
        delay(light);
      }
    }
  }
  digitalWrite(ICRed1, LOW);

  startedAt = millis();
  while (millis() - startedAt < intervall) {
    for (int i = (buttonNr / 2); i < buttonNr; i = i + 1) {
      if (buttonOnShift[i]) {
        setIC(i);
        digitalWrite(ICRed2, HIGH);
        delay(light);
      }
    }
  }
  digitalWrite(ICRed2, LOW);

  digitalWrite(writePinForLEDs, HIGH);
}

void ledTest()
{
  // 1-8 gruen
  digitalWrite(writePinForLEDs, LOW);
  //digitalWrite(writePinForLED2,LOW);
  //digitalWrite(writePinForLED3,LOW);
  //digitalWrite(writePinForLED4,LOW);
  //digitalWrite(ICGreen1, LOW);
  //digitalWrite(ICGreen2, LOW);
  //digitalWrite(ICRed1, LOW);
  //digitalWrite(ICRed2, LOW);
  //  digitalWrite(writePinForLED1,HIGH);
  for (int i = 0; i < 8; i = i + 1) {
    setIC(i);
    digitalWrite(ICGreen1, HIGH);
    delay(75);
    digitalWrite(ICGreen1, LOW);
  }
  //  digitalWrite(writePinForLEDs,HIGH);
  for (int i = 0; i < 8; i = i + 1) {
    setIC(i);
    digitalWrite(ICGreen2, HIGH);
    delay(75);
    digitalWrite(ICGreen2, LOW);
  }
  //  digitalWrite(writePinForLEDs,HIGH);
  for (int i = 0; i < 8; i = i + 1) {
    setIC(i);
    digitalWrite(ICRed1, HIGH);
    delay(75);
    digitalWrite(ICRed1, LOW);
  }
  //  digitalWrite(writePinForLEDs,HIGH);
  for (int i = 0; i < 8; i = i + 1) {
    setIC(i);
    digitalWrite(ICRed2, HIGH);
    delay(75);
    digitalWrite(ICRed2, LOW);
  }
}

void zuffiMode()
{
  unsigned long startedAt = millis();
  float intervall = 1;
  float light = 1;

  while (true)
  {
    for (int i = 0; i < buttonNr; i = i + 1)
    {
      if (random(1, 4) == 1)
      {
        buttonOn[i] = true;
      }
      else
      {
        buttonOn[i] = false;
      }
      if (random(1, 4) == 1)
      {
        buttonOnShift[i] = true;
      }
      else
      {
        buttonOnShift[i] = false;
      }
    }
    startedAt = millis();
    digitalWrite(writePinForLEDs, LOW);
    potiWert = analogRead(A0);
    controllerWert = map(potiWert, 0, 1023, 0, 127);

    for (int i = 0; i < buttonNr; i = i + 1)
    {
      if (buttonOn[i] && i < 8)
      {
        setIC(i);
        digitalWrite(ICGreen1, HIGH);
        delay(controllerWert * 2);
        digitalWrite(ICGreen1, LOW);
      }
      if (buttonOn[i] && i > 8)
      {
        setIC(i);
        digitalWrite(ICGreen2, HIGH);
        delay(controllerWert * 2);
        digitalWrite(ICGreen2, LOW);
      }
      if (buttonOnShift[i] && i < 8)
      {
        setIC(i);
        digitalWrite(ICRed1, HIGH);
        delay(controllerWert * 2);
        digitalWrite(ICRed1, LOW);
      }
      if (buttonOnShift[i] && i > 8)
      {
        setIC(i);
        digitalWrite(ICRed2, HIGH);
        delay(controllerWert * 2);
        digitalWrite(ICRed2, LOW);
      }
    }



  }
}
void CheckSerial() {
  if (Serial.available() > 0) { // midi thru
    byte incoming_data = Serial.read();
    if (midithru)
    {
    Serial.write(incoming_data);  // pass all Midi Commands to Midioutput
    }
  }
}
