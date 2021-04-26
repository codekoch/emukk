#define RED 12
#define GREEN 13
//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(0,1);
byte midi_start = 0xfa;
byte midi_pointer = 0xF2;
byte midi_stop = 0xfc;
byte midi_clock = 0xf8;
byte midi_continue = 0xfb;
byte midi_noteOn = 0x99;
byte midi_noteOff = 0x89;
byte midi_velMax = 0x7F;
byte midi_velMin = 0x00;
unsigned int QPM = 4;  // QUARTERS PER MEASURE
unsigned int NOM = 1;  // MINIMUM NUMBERS OF MEASURE OF THE LOOP
unsigned int PPQ = 24; // TICK OR PULSES PER QUARTER

unsigned int Tick_Counter = 0;

// INTERNALS
byte play_flag = 0;
byte incoming_data;

void setup() {
  Serial.begin(31250);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
}

void loop() {
  CheckSerial();
  Leds();
  if (Tick_Counter == NOM * QPM * PPQ) {
    Tick_Counter = 0;
  }

}

void CheckSerial() {
  if (Serial.available() > 0) {
    incoming_data = Serial.read();
    Serial.write(incoming_data);  // pass all Midi Commands to Midioutput
    if (incoming_data == midi_start) {
      play_flag = 1;
      Tick_Counter = 0;
    }
    else if (incoming_data == midi_continue) {
      play_flag = 1;
    }
    else if (incoming_data == midi_stop) {
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, LOW);
      play_flag = 0;
    }
    else if ((incoming_data == midi_clock) && (play_flag == 1)) {
      Tick_Counter++;
    }
  }
}
void Leds()
{
  if (Tick_Counter > 0 && (Tick_Counter % 24) < 6 )
  {
    if ( Tick_Counter < 6 )
    {
      digitalWrite(RED, HIGH);
    }
    digitalWrite(GREEN, HIGH);
  }
  else
  {
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, LOW);
  }
}
