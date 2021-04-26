#define MAX_TRACKS 3 // Track 0-2  
#define MAX_MIDIDATA_RECORDS 5000
#define BLINKTIME 12 // LED blinktime

// LEDS
const int RED[3] = {22, 23, 24};
const int GREEN[3] = {25, 26, 27};
#define YELLOW 28
#define WHITE 29
#define BLUE 30

// BUTTONS
#define BUTTONNR 4
const int BUTTON[4] = {2, 3, 4, 5} ; // 2-4 Trackbutton, 5 Stopbutton
int buttonState[4];             // the current reading from the input pin
int lastButtonState[4] = {LOW, LOW, LOW, LOW}; // the previous reading from the input pin
#define MIDITROUGHBUTTON 6


// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime[4] = {0, 0, 0, 0}; // the last time the output pin was toggled
unsigned long debounceDelay[4] = {50, 50, 50, 50}; // the debounce time; increase if the output flickers

// MIDI STATUS BYTES
byte midi_start    = 0xfa;
byte midi_stop     = 0xfc;
byte midi_clock    = 0xf8;
byte midi_continue = 0xfb;

// VARIABLES
bool recording;
bool playing;
bool insync;
bool waitingForRecording;
bool waitingForPlaying;
bool getNewLoopStartTime;
bool clockSignal;
bool loopLight;
bool stopped;
bool midithrough;
bool allErased;
int counter;
int currentTrack;
int nextTrack;
int currentDataIndex[MAX_TRACKS];
int maxDataIndex[MAX_TRACKS];
int beats[MAX_TRACKS];
int beat;
byte MIDIdata[MAX_TRACKS][MAX_MIDIDATA_RECORDS];
unsigned long MIDIdataTime[MAX_TRACKS][MAX_MIDIDATA_RECORDS];
unsigned long loopStartTime;


void setup() {

  for (int i = 0; i < BUTTONNR; i++)
  {
    pinMode(BUTTON[i], INPUT);
  }
  for (int i ; i < 3; i++)
  {
    pinMode(RED[i], OUTPUT);
    pinMode(GREEN[i], OUTPUT);
  }
  pinMode(BLUE, OUTPUT);
  pinMode(WHITE, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  //Serial.begin(9600);
  Serial1.begin(31250); // RX mididata for recording // TX Mididata for looping / playing
  Serial2.begin(31250); // RX mididata for syncing
  LEDTest();
  reset();

}

void loop() {

  clockSignal = false;
  checkMidiSyncsignal();
  if (recording)
  {
    recordMidi();
  }
  if (playing)
  {
    playMidi();
  }
  if (!playing && !recording && !waitingForRecording && !waitingForPlaying)
  {
    if (midithrough)
    {
      if (Serial1.available() > 0)
      {
        byte incoming_data = Serial1.read();

        Serial1.write(incoming_data);
      }
    }
  }
  reactToButtons();

  if (clockSignal) {
    if ( counter == (24 * 4 - 1) )
    {
      counter = 0;
      if (recording)
      {
        beats[currentTrack]++;
      }
      if (playing)
      {
        if (beat == ( beats[currentTrack] - 1))
        {
          beat = 0;
        }
        else
        {
          beat++;
        }
      }
    }
    else
    {
      counter++;
    }
  }
  if (allErased)
  {
    if ((counter % 24) > BLINKTIME)
    {
      for (int i = 0; i < 3; i++)
      {
        digitalWrite(GREEN[i], LOW);
        digitalWrite(RED[i], LOW);
      }
      allErased = false;
    }
  }
}

void reactToButtons() {
  for (int i = 0; i < (BUTTONNR - 1); i++) // first three buttons are Trackbuttons
  {
    // read the state of the switch into a local variable:
    int reading = digitalRead(BUTTON[i]);

    // check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH), and you've waited long enough
    // since the last press to ignore any noise:

    // If the switch changed, due to noise or pressing:
    if (reading != lastButtonState[i]) {
      // reset the debouncing timer
      lastDebounceTime[i] = millis();
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay[i]) {
      // whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:

      // if the button state has changed:
      if (reading != buttonState[i]) {
        buttonState[i] = reading;

        // only toggle the LED if the new button state is HIGH
        if (buttonState[i] == HIGH) {
          // Serial.println("button");
          // Serial.println(i);

          if ( i  != currentTrack) // coming from antoher Track
          {
            nextTrack = i;
            if (!waitingForPlaying && !waitingForRecording && maxDataIndex[nextTrack] == 0 ) // nothing is activated and nothing was recorded on this track or track is playing -> start new recording
            {
              waitingForRecording = true;
            }
            if (!waitingForPlaying && !waitingForRecording && maxDataIndex[nextTrack] > 0 ) // change from recording to playing
            {
              waitingForPlaying = true;
            }
          }
          else
          {
            if (!waitingForPlaying && !waitingForRecording && !recording && maxDataIndex[currentTrack] == 0 || playing) // nothing is activated and nothing was recorded on this track or track is playing -> start new recording
            {
              waitingForRecording = true;
              nextTrack = currentTrack;
            }
            if (!waitingForPlaying && !waitingForRecording && recording || maxDataIndex[currentTrack] > 0 && !playing) // change from recording to playing
            {
              waitingForPlaying = true;
              nextTrack = currentTrack;
            }
          }
        }
      }
    }

    // save the reading. Next time through the loop, it'll be the lastButtonState:
    lastButtonState[i] = reading;
  }

  // Stop Button
  int reading = digitalRead(BUTTON[3]);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState[3]) {
    // reset the debouncing timer
    lastDebounceTime[3] = millis();
  }

  if ((millis() - lastDebounceTime[3]) > debounceDelay[3]) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState[3]) {
      buttonState[3] = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState[3] == HIGH) {
        if (stopped)
        {
          eraseAllTracks();
        }
        else
        {
          // Stop was pressed
          // Serial.println("Stopped");
          stopAll();
        }

      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState[3] = reading;

  if (digitalRead(MIDITROUGHBUTTON) == HIGH) // check Midithru Button
  {
    midithrough = false;
  }
  else
  {
    midithrough = true;
  }

}


void recordMidi()
{
  if ( maxDataIndex[currentTrack] >= MAX_MIDIDATA_RECORDS)
  {
    displayError();
  }
  else
  {
    if (Serial1.available() > 0) {

      byte incoming_data = Serial1.read();
      if (midithrough)
      {
        Serial1.write(incoming_data);
      }
      MIDIdata[currentTrack][currentDataIndex[currentTrack]] = incoming_data;
      MIDIdataTime[currentTrack][currentDataIndex[currentTrack]] = (micros() - loopStartTime);
      currentDataIndex[currentTrack]++;
      maxDataIndex[currentTrack]++;
    }
  }
}
void playMidi()
{
  unsigned long elapsedTime = (micros() -  loopStartTime);
  if (midithrough && !waitingForRecording && !waitingForPlaying)
  {
    if (Serial1.available() > 0)
    {
      byte incoming_data = Serial1.read();

      Serial1.write(incoming_data);
    }
  }
  if ( elapsedTime < MIDIdataTime[currentTrack][maxDataIndex[currentTrack] - 1] )
  {
    if (elapsedTime >  MIDIdataTime[currentTrack][currentDataIndex[currentTrack]]) // time to fire recorded Midisignal and switch to next recorded signal
    {
      Serial1.write( MIDIdata[currentTrack][currentDataIndex[currentTrack]]);
      currentDataIndex[currentTrack]++;
    }
  }
  else
  {
    getNewLoopStartTime = true;
  }
}

void checkMidiSyncsignal()
{
  if (Serial2.available() > 0) {
    byte incoming_data = Serial2.read();
    if (incoming_data == midi_start) {
      insync = true;
      counter = 0;
    }
    else if (incoming_data == midi_continue) {
      insync = true;
    }
    else if (incoming_data == midi_stop) {
      // digitalWrite(LED_WHITE, LOW);
      stopAll();
      insync = false;
    }
    else if ((incoming_data == midi_clock) && insync) { // clocksignal received

      if ( waitingForRecording && counter == 0 && (beat == 0 || stopped))  // start recording at next beat 1
      {
        trackLEDsOff(currentTrack);
        currentTrack = nextTrack;
        getNewLoopStartTime = true;
        waitingForRecording = false;
        recording = true;
        playing = false;
        maxDataIndex[currentTrack] = 0; // new data -> erase old data
        beats[currentTrack] = 0;
        beat = 0; // bug?
      }
      if ( waitingForPlaying && counter == 0 && (beat == 0 || stopped))  // start playing at next beat 1
      {
        trackLEDsOff(currentTrack);
        currentTrack = nextTrack;
        waitingForPlaying = false;
        playing = true;
        recording = false;
        getNewLoopStartTime = true;
      }
      if (getNewLoopStartTime && counter == 0 && (beat == 0 || stopped)) // set new loopStartTime
      {
        getNewLoopStartTime = false;
        stopped = false;
        loopStartTime = micros();
        currentDataIndex[currentTrack] = 0;  // set loopdataindex to 0 to begin with first record
        loopLight = true;
      }

      // LEDs BEHAVOIR
      if ((counter % 24) < BLINKTIME)
      {
        digitalWrite(YELLOW, HIGH);
      }
      else
      {
        digitalWrite(YELLOW, LOW);
      }
      if (counter < BLINKTIME)
      {
        digitalWrite(WHITE, HIGH);
        if (loopLight)
        {
          digitalWrite(BLUE, HIGH);
        }
      }
      else
      {
        if (loopLight)
        {
          digitalWrite(BLUE, LOW);
          loopLight = false;
        }
        digitalWrite(WHITE, LOW);
      }
      if (waitingForRecording)
      {
        if ((counter % 24) < BLINKTIME)
        {
          digitalWrite(RED[currentTrack], HIGH);
        }
        else
        {
          digitalWrite(RED[currentTrack], LOW);
        }
      }
      if (recording)
      {
        digitalWrite(RED[currentTrack], HIGH);
        digitalWrite(GREEN[currentTrack], LOW);
      }
      if (playing)
      {
        digitalWrite(GREEN[currentTrack], HIGH);
        digitalWrite(RED[currentTrack], LOW);
      }
      if (waitingForPlaying)
      {
        if ((counter % 24) < BLINKTIME)
        {
          digitalWrite(GREEN[nextTrack], HIGH);
        }
        else
        {
          digitalWrite(GREEN[nextTrack], LOW);
        }
      }
      if (waitingForRecording)
      {
        if ((counter % 24) < BLINKTIME)
        {
          digitalWrite(RED[nextTrack], HIGH);
        }
        else
        {
          digitalWrite(RED[nextTrack], LOW);
        }
      }
      // END OF LEDS BEHAVOIR
      clockSignal = true;
    }
  }
}

// LEDs OFF
void LEDsOff()
{
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(GREEN[i], LOW);
    digitalWrite(RED[i], LOW);
  }
  digitalWrite(YELLOW, LOW);
  digitalWrite(WHITE, LOW);
  digitalWrite(BLUE, LOW);
}
void trackLEDsOff(int track)
{
  digitalWrite(GREEN[track], LOW);
  digitalWrite(RED[track], LOW);
}
// LEDs ON
void LEDsOn()
{
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(GREEN[i], HIGH);
    digitalWrite(RED[i], HIGH);
  }
  digitalWrite(YELLOW, HIGH);
  digitalWrite(WHITE, HIGH);
  digitalWrite(BLUE, HIGH);
}

void LEDTest() {
  LEDsOn();
  delay(200);
  LEDsOff();
  delay(200);
  LEDsOn();
  delay(200);
  LEDsOff();
}

// RESET
void reset()
{
  counter = 0;
  playing = false;
  recording = false;
  insync = false;
  allErased = false;
  currentTrack = 0;
  nextTrack = 0;
  loopLight = false;
  beat = 0;
  waitingForRecording = false;
  waitingForPlaying = false;
  loopStartTime = 0.0;
  stopped = false;
  for (int i = 0; i < MAX_TRACKS; i++)
  {
    currentDataIndex[i] = 0;
    maxDataIndex[i] = 0;
    beats[i] = 0;
    for (int j = 0; j < MAX_MIDIDATA_RECORDS; j++)
    {
      MIDIdata[i][j] = 0;
      MIDIdataTime[i][j] = 0;
    }
  }
}

void displayError()
{
  if ((counter % 24) < BLINKTIME)
  {
    LEDsOn();
  }
  else
  {
    LEDsOff();
  }
}

// STOP
void stopAll()
{
  playing = false;
  recording = false;
  waitingForRecording = false;
  waitingForPlaying = false;
  loopStartTime = 0.0;
  stopped = true;
  for (int i = 0; i < MAX_TRACKS; i++)
  {
    currentDataIndex[i] = 0;
  }
  LEDsOff();
}

void  eraseAllTracks()
{
  for (int i = 0; i < MAX_TRACKS; i++)
  {
    currentDataIndex[i] = 0;
    maxDataIndex[i] = 0;
    beats[i] = 0;
    for (int j = 0; j < MAX_MIDIDATA_RECORDS; j++)
    {
      MIDIdata[i][j] = 0;
      MIDIdataTime[i][j] = 0;
    }

  }
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(GREEN[i], HIGH);
    digitalWrite(RED[i], HIGH);
  }
  allErased = true;
}
