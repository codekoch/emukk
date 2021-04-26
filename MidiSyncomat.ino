#include <math.h>
// JAMMAN SYNC
#include <SoftwareSerial.h>
SoftwareSerial jmSerial(11,10); // RX, TX

// DATA FOR CALCULATIONS
unsigned int QPM = 4;  // QUARTERS PER MEASURE
unsigned int NOM = 1;  // MINIMUM NUMBERS OF MEASURE OF THE LOOP
unsigned int PPQ = 24; // TICK OR PULSES PER QUARTER
unsigned int BPM = 0;  // BEATS PER MINUTE

// TIMINGS AND COUNTERS
unsigned int Tick_Counter = 0;
unsigned long jamsync_measure_timer;
unsigned long jamsync_loop_timer;
unsigned long jamsync_link_timer ;


// SYSEX ARRAYS
unsigned int jamsync_sync[] {0x00, 0xF0, 0x00, 0x00, 0x10, 0x7F, 0x62, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7};
unsigned int jamsync_link[] {0x00, 0xF0, 0x00, 0x00, 0x10, 0x7F, 0x62, 0x01, 0x00, 0x01, 0x01, 0xF7};
unsigned int jamsync_stop[] {0x00, 0xF0, 0x00, 0x00, 0x10, 0x7F, 0x62, 0x01, 0x4A, 0x01, 0x04, 0x46, 0x30, 0x42, 0x02, 0x04, 0x40, 0x03, 0x4E, 0x46, 0x2E, 0x40, 0x04, 0x5C, 0xF7};


// OUTPUT PINS
#define SYNC_OUTPUT_PIN 6 // Audio Sync Digital Pin

// MIDI STATUS BYTES
byte midi_start    = 0xfa;
byte midi_stop     = 0xfc;
byte midi_clock    = 0xf8;
byte midi_continue = 0xfb;

// INTERNALS
byte play_flag = 0;
byte incoming_data;

// LEDS
#define LED_BLUE 7
#define LED_RED 5
#define LED_GREEN 2

// BUTTONS
#define BTN_BLUE 8
#define BTN_RED 9
#define BTN_GREEN 12

// POTI
#define POT_READ_PIN 0 // read potentiometer pin

// DISPLAY
#define TM1637_DISPLAY
#include <TM1637Display.h>
#define TM1637_CLK_PIN 3
#define TM1637_DIO_PIN 4
#define TM1637_BRIGHTNESS 0x0f
TM1637Display display(TM1637_CLK_PIN, TM1637_DIO_PIN);
uint8_t tm1637_data[4] = {0x00, 0x00, 0x00, 0x00};


// VARIABLES
float korgPeriod = 1.0;


// CONSTANTS
const int LED_BRIGHTNESS[24] = { 255, 255, 255, 255, 255, 246, 236, 225, 213, 200, 184, 165, 142, 113, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // LED flashing animnation for 24 clock impulses


void setup() {
  display.setBrightness(TM1637_BRIGHTNESS);
  pinMode(SYNC_OUTPUT_PIN, OUTPUT);
  digitalWrite(SYNC_OUTPUT_PIN, LOW);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, LOW);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  Serial1.begin(31250);
  jmSerial.begin(31250);
  jamsync_link_timer = micros();

}

void loop() {
  //noInterrupts();
  // READ THE INCOMING MIDI
  CheckSerial();

  // KEEP LINK ACTIVE
  if (micros() - jamsync_link_timer > 400000) {
    sendLink();
  }

  // SEND SYNC COMMAND
  if (Tick_Counter == NOM * QPM * PPQ) {
    GetBPM();
    Sendjamsync_sync();
    Tick_Counter = 0;
  }
  int value = analogRead(POT_READ_PIN);
  if ( value < 341 && Tick_Counter == 0)
  {
    korgPeriod = 0.5;
  }
  if ( value > 683 && Tick_Counter == 0)
  {
    korgPeriod = 2;
  }
  if ( (value > 340) && (value < 684) && Tick_Counter == 0)
  {
    korgPeriod = 1;
  }
  //interrupts();
}

void CheckSerial() {
  if (Serial1.available() > 0) {
    incoming_data = Serial1.read();
    Serial1.write(incoming_data);  // pass all Midi Commands to Midioutput
    if (incoming_data == midi_start) {
      digitalWrite(LED_BLUE, HIGH);
      play_flag = 1;
      if (BPM > 0) {
        Sendjamsync_sync();
      }
      jamsync_measure_timer = micros();
      Tick_Counter = 0;
    }
    else if (incoming_data == midi_continue) {
      play_flag = 1;
    }
    else if (incoming_data == midi_stop) {
      digitalWrite(LED_BLUE, LOW);
      play_flag = 0;
      sendStop();
    }
    else if ((incoming_data == midi_clock) && (play_flag == 1)) {
      if ( Tick_Counter % ((int) (12 * korgPeriod)) == 0 )
      {
        digitalWrite(SYNC_OUTPUT_PIN, HIGH);
        digitalWrite(SYNC_OUTPUT_PIN, LOW);
      }
      analogWrite(LED_RED, LED_BRIGHTNESS[Tick_Counter % 24]);
      Tick_Counter++;
    }
  }
  
}

void sendLink() {
  for (int i = 1; i < 12; i++) {
    jmSerial.write(jamsync_link[i]);
  }
  jamsync_link_timer = micros();
}

void sendStop() {
  for (int i = 1; i < 25; i++) {
    jmSerial.write(jamsync_stop[i]);
  }
}

void GetBPM() {
  jamsync_measure_timer = micros() - jamsync_measure_timer;
  jamsync_loop_timer = jamsync_measure_timer;
  BPM = round(QPM * 60000000.0 / jamsync_measure_timer);
  jamsync_measure_timer = micros();
  setDisplayValue(BPM);
}

void Sendjamsync_sync() {
  unsigned long LoopTime;
  int b163 = 0;
  int w;
  int x;
  int y;
  int z = 0; //xor checksum


  // LoopTime = floor(1000* NOM * QPM * 60.0 / BPM);
  LoopTime = floor(jamsync_loop_timer / 1000.0);
  x = floor(log(LoopTime / 2000.0) / log(4.0));
  b163 = (LoopTime / (2000.0 * pow(4.0, x))) > 2;
  y = 2 * pow(2, b163) * pow(4, x);
  w = floor(LoopTime / y);

  // BPM
  jamsync_sync[8]  = 66 + 8 * ((63 < BPM) && (BPM < 128) || BPM > 191) ;
  jamsync_sync[12] = (4 * BPM > 127 && 4 * BPM < 256) * (4 * BPM - 128) +
                     (2 * BPM > 127 && 2 * BPM < 256) * (2 * BPM - 128) +
                     (BPM > 127 && BPM < 256) * (BPM - 128);
  jamsync_sync[13] = 1 * (BPM > 127) + 66;

  // lenght
  jamsync_sync[16] = 64 + 8 * b163;
  jamsync_sync[21] = 64 + x;
  jamsync_sync[20] = 128 * (0.001 * w - 1);
  jamsync_sync[19] = pow(128.0, 2) * (0.001 * w - 1 - jamsync_sync[20] / 128.0);
  jamsync_sync[18] = pow(128.0, 3) * (0.001 * w - 1 - jamsync_sync[20] / 128.0 - jamsync_sync[19] / pow(128.0, 2));

  // command
  jamsync_sync[22] = 5;

  // checksum
  for (int i = 8; i < 23; i++) {
    z = z ^ int(jamsync_sync[i]); // checksum XOR
  }
  jamsync_sync[23] = z;

  for (int i = 1; i < 25; i++) {
    jmSerial.write(int(jamsync_sync[i]));
  }
}

void setDisplayValue(int value) {
  tm1637_data[0] = value >= 1000 ? display.encodeDigit(value / 1000) : 0x00;
  tm1637_data[1] = value >= 100 ? display.encodeDigit((value / 100) % 10) : 0x00;
  tm1637_data[2] = value >= 10 ? display.encodeDigit((value / 10) % 10) : 0x00;
  tm1637_data[3] = display.encodeDigit(value % 10);
  display.setSegments(tm1637_data);
}
