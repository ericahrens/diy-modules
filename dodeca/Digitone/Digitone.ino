#include <MIDI.h>

#define ON_VALUE 1023
#define CC_MAX 511

const uint8_t CHANA = 1;//set the MIDI channel here!
const uint8_t CHANB = 2;
const uint8_t CHANC = 3;

uint8_t out2pin[] = {23, 0, 22, 25, 20, 6, 21, 5, 9, 4, 10, 3};//output number to actual teensy pin, dont change.
uint8_t whitekeys[] = {4, 0, 6, 0, 8, 0, 0, 0, 0, 0, 0, 0};//non zero keys sent to output number.
uint8_t pulses;
uint8_t sixteenthnotes;
uint8_t quartertoggle;
uint8_t wholetoggle;
bool playing;
int noteTable[] = {0, 98, 195, 293, 390 , 487, 584, 682, 779, 877, 975 };

uint8_t cc2activea[] = {71, 72, 73, 74};
uint8_t cc2outa[] = {2, 3, 4, 5};
uint8_t cc2activeb[] = {71, 72, 73};
uint8_t cc2outb[] = {8, 9, 10};

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void ligthPattern(int i, int value) {
    if (out2pin[i] == 0) analogWrite(A14, value);
    else analogWrite(out2pin[i], value );
}

void initAnimate(int time) {
  for (int i = 0; i < 12; i += 2) {
    ligthPattern(i, ON_VALUE);
    ligthPattern(i+1, ON_VALUE);
    delay(time);
    ligthPattern(i, 0);
    ligthPattern(i+1, 0);
  }
  delay(time);
  for (int i = 10; i >= 0; i -= 2) {
    ligthPattern(i, ON_VALUE);
    ligthPattern(i+1, ON_VALUE);
    delay(time);
    ligthPattern(i, 0);
    ligthPattern(i+1, 0);
  }
}

void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  if (channel == CHANA) {
    if (velocity != 0) {
      analogWrite(out2pin[0], ON_VALUE);
      int xv = 0;
      int stp;
      if (pitch >= 24) {
        int rst = pitch % 12;
        int idx = (pitch - 24) / 12;
        stp = (noteTable[idx + 1] - noteTable[idx]) / 12;
        xv = noteTable[idx] + stp * rst;
        analogWrite(A14, xv);
      }
      Serial.print("Note=");
      Serial.print(pitch);
      Serial.print(" ");
      Serial.print(stp);
      Serial.print(" ");
      Serial.println(xv);
    }
    else {
      analogWrite(out2pin[0], 0);
    }
  } else if (channel == CHANB) {
    if (velocity != 0) {
      analogWrite(out2pin[6], ON_VALUE);
    }
    else {
      analogWrite(out2pin[6], 0);
    }
  } else if (channel == CHANC) {
    if (velocity != 0) {
      analogWrite(out2pin[7], ON_VALUE);
    }
    else {
      analogWrite(out2pin[7], 0);
    }
  }
}

void HandleNoteOff(byte channel, byte pitch, byte velocity) {
  if (channel == CHANA) {
    analogWrite(out2pin[0], 0);
  }
  else if (channel == CHANB) {
    analogWrite(out2pin[6], 0);
  }
  else if (channel == CHANC) {
    analogWrite(out2pin[7], 0);
  }

}


void HandleControlChange (byte channel, byte number, byte value) {
  if (channel == CHANA) {
    for (int i = 0; i < 4; i ++) {
      if ( number  == cc2activea[i]) { //ignore wrong channel or CC numbers
        analogWrite(out2pin[cc2outa[i]], value << 2);
      }
    }
  }
  if (channel == CHANB) {
    for (int i = 0; i < 4; i ++) {
      if ( number  == cc2activeb[i]) { //ignore wrong channel or CC numbers
        analogWrite(out2pin[cc2outb[i]], value << 2);
      }
    }
  }
}

void HandleClock(void) {
  if (playing) {
    pulses ++;
    if (pulses > 11 ) {
      pulses = 0;
      sixteenthnotes ++;
    }
    if (pulses < 1) {
      analogWrite(out2pin[11], ON_VALUE);
    }
    else analogWrite(out2pin[11], 0);
  }
}

void HandleStart() {
  pulses = 0;
  sixteenthnotes = 0;
  playing = 1;
}

void HandleStop() {
  playing = 0; //remove this to have continuous clock pulses on dodeca after sequencer is stopped
  digitalWriteFast(out2pin[11], LOW); //turn pulse off when stopped even if it is in the middle of "on" cycle
}


void HandleContinue() {
  //maybe only needed for song position pointer
}

void setup() {
  for (int i = 0; i < 12; i ++) {
    if (out2pin[i]) {
      pinMode(out2pin[i], OUTPUT);
      analogWriteFrequency(out2pin[i], 200000);
    }
  }

  analogWriteResolution(10);

  initAnimate(100);
  initAnimate(100);
  // Connect the Handlers to the library, so it is called upon reception.
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
  MIDI.setHandleControlChange(HandleControlChange);
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandleClock(HandleClock);
  MIDI.setHandleStart(HandleStart);
  MIDI.setHandleStop(HandleStop);
  MIDI.setHandleContinue(HandleContinue);

  Serial.begin(9600);
  Serial.println("Start");
}

int ct = 0;

void loop() {
  MIDI.read();
}
