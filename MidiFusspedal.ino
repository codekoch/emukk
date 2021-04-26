// KEYS
#define KEYNR 13 // Anzahl der Fusstasten
const int KEY[13] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, A0}; // Pinout am Arduino für die Fustasten
const double KEYPITCH [13] = {63.5, 82, 87, 91, 95, 99, 103, 107, 111, 115, 119, 123, 127}; // normale Pitchwerte der Fusstasten im nubass Pitchmodus
const double KEYPITCHLOW [13] = {0, 5, 9.5 , 13 , 17, 21, 25, 29, 33, 37, 41 , 123, 63.5}; // tiefe Pitchwerte der Fusstasten im nubass Pitchmodus
const int KEYPHRASE [13] = {25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 38 };
const int instrumentButton = A1; // wechselt zwischen basscontrol/nubassPitch und Ujam instrument (siehe boolean instrument)
const int basscontrolButton = A5; // wechselt zwischen basscontrol und nubass Pitchmodus (siehe boolean basscontrol)
const int MIDI_SWITCH = 176;
byte instrumentButtonFired = 0;
byte basscontrolButtonFired = 0;
boolean instrument = 1; // 0=basscontrol/nubassPitch, 1=Ujam instrument
boolean basscontrol = 1; // 0=nubassPitch, 1=Basscontrol
boolean waitForPhraseSelection = false;//
byte keyFired[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int startNote = 36;  // C

// MIDI STATUS BYTES
const uint8_t NOTE_OFF = 0x80;
const uint8_t NOTE_ON = 0x90;
const uint8_t NO_VELOCITY = 0x00;
const uint8_t FULL_VELOCITY = 0x45;
const uint8_t KEY_PRESSURE = 0xA0;
const uint8_t CC = 0xB0;
const uint8_t PROGRAM_CHANGE = 0xC0;
const uint8_t CHANNEL_PRESSURE = 0xD0;
const uint8_t PITCH_BEND = 0xE0;
byte midi_start    = 0xfa;
byte midi_stop     = 0xfc;
byte midi_clock    = 0xf8;
byte midi_continue = 0xfb;
byte midi_cc = 0xB0;
boolean midithru = true;
boolean b1 = false;
boolean b2 = false;

int pitchbend = 224;//224 = 11100000 in binary, pitchbend command


void setup() {

  for (int i = 0; i < KEYNR; i++)
  {
    pinMode(KEY[i], INPUT);
  }
  pinMode(A5, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  // Serial.begin(9600);
  Serial1.begin(31250); // RX mididata for recording // TX Mididata for looping / playing
  digitalWrite(A4, HIGH);
}

void loop() {
  checkMidi();
  checkButtons();
  sendNotes();
  setLEDS();
}

void sendNotes()
{
  for (int i = 0; i < KEYNR; i++)
  {
    byte key = digitalRead(KEY[i]);

    if (key == 1 and keyFired[i] == 0)
    {
      if (instrument == 0)
      {

        if (basscontrol)
        {
          if (waitForPhraseSelection)
          {
            MIDImessage(NOTE_ON, KEYPHRASE[i], FULL_VELOCITY);

          }
          else
          {
            MIDImessage(NOTE_ON, (startNote + i + 24), FULL_VELOCITY);
          }
        }
        else
        {

          sendCC(0, 40, KEYPITCH[i]);

        }
      }
      else
      {
        if (!basscontrol)
        {
          MIDImessage(NOTE_ON, (startNote + i ), FULL_VELOCITY);
        }
        else
        {
          if (i < 4)
          {
            MIDImessage(NOTE_ON, (startNote + i + 12 ), FULL_VELOCITY);
          }
          else
          {
            MIDImessage(NOTE_ON, (startNote + i ), FULL_VELOCITY);
          }
        }
      }
      keyFired[i] = 1;
    }
    if (key == 0 and keyFired[i] )
    {
      if (instrument == 0)
      {
        if (basscontrol)
        {
          if (waitForPhraseSelection)
          {

            MIDImessage(NOTE_OFF, KEYPHRASE[i], NO_VELOCITY);
            waitForPhraseSelection = false;
          }
          else
          {
            MIDImessage(NOTE_OFF, (startNote + i + 24), NO_VELOCITY);
          }


        }
      }
      else
      {
        if (!basscontrol)
        {
          MIDImessage(NOTE_OFF, (startNote + i ), NO_VELOCITY);
        }
        else
        {
          if (i < 4)
          {
            MIDImessage(NOTE_OFF, (startNote + i + 12 ), NO_VELOCITY);
          }

          else
          {
            MIDImessage(NOTE_OFF, (startNote + i ), NO_VELOCITY);
          }
        }
      }
      keyFired[i] = 0;

    }
  }
}

void MIDImessage(int cmd, int note, int vel) {
  Serial1.write(cmd);
  Serial1.write(note);
  Serial1.write(vel);
}

void sendCC(byte channel, byte number, byte value) {
  Serial1.write( 0xB0 | channel);
  Serial1.write(number);  // controller number to use
  Serial1.write(value);
}

void checkButtons()
{
  byte value = digitalRead(instrumentButton);
  if ( value == 1 && instrumentButtonFired == 0)
  {
    instrument = !instrument;
    instrumentButtonFired = 1;
    b1 = true;
  }
  if ( value == 0 && instrumentButtonFired == 1)
  {
    instrumentButtonFired = 0;
    b1 = false;
  }
  value = digitalRead(basscontrolButton);
  if ( value == 1 && basscontrolButtonFired == 0)
  {
    basscontrol = !basscontrol;
    basscontrolButtonFired = 1;
    b2 = true;
  }
  if ( value == 0 && basscontrolButtonFired == 1)
  {
    basscontrolButtonFired = 0;
    b2 = false;
  }
  if ( b1 && b2 )
  {
    midithru = !midithru;
  }
}
void setLEDS()
{
  if (basscontrol && !instrument)
  {
    green2();
  }
  if (!basscontrol && !instrument)
  {
    orange2();
  }
  if (basscontrol && instrument )
  {
    red2();
  }
  if (!basscontrol && instrument )
  {
    off2();
  }

}
void red2 ()
{
  digitalWrite(A3, HIGH);
  digitalWrite(A2, LOW);
}

void orange2 ()
{
  digitalWrite(A3, HIGH);
  digitalWrite(A2, HIGH);
}
void off2 ()
{
  digitalWrite(A3, LOW);
  digitalWrite(A2, LOW);
}
void green2()
{
  digitalWrite(A3, LOW);
  digitalWrite(A2, HIGH);
}
void checkMidi()
{
  if (Serial1.available() > 0) {
    byte incoming_data = Serial1.read();
    if ( ((int) incoming_data) == MIDI_SWITCH )
    {
      waitForPhraseSelection = true;
    }
    if ( midithru )
    {
      Serial1.write(incoming_data);
    }
  }
}
