/*
  Bluetooth Low Energy MIDI interface for Motivation Radio Eurorack ESP32 module
  R Heslip Nov 2018

  This code implements MIDI over Bluetooth Low Energy

  The code creates a BLE MIDI service and characteristic, advertises the service and waits for a client connection
  it converts note on/off messages to gate outs and control voltages
  gate outs and CV outs are configurable in the menu. default is MIDI note to gate and CV:
  MIDI channel 1 = GATE0 +CV0
  MIDI channel 2 = GATE1 +CV1
  MIDI channel 3 = GATE2 +CV2
  MIDI channel 4 = GATE3 +CV3

  gate outs and CV outs will also respond to MIDI CC (continuous controller messages) if configured for that

  Motivation Radio with the BLEMIDI sketch is intended for use with iPhone and iPad apps - sequencers, controllers etc
  at this point the sketch is MIDI receive only since it will generally be used as a MIDI to CV and GATE converter

  TBD:
  implement more MIDI commands e.g. pitch bend
  add a menu system for configuration

  Dec 1/18 - incorporated BLEmidi decoder from the Pedalino project which resolved problem of notes getting dropped
  Dec 29/18 - code ported from prototype HW to Motivation Radio
  dec 30/18 - added serial midi handlers. units can be controlled by either BTMIDI or serial MIDI (both is not a good idea!)
  BTMIDI in and serial MIDI in are both forwarded to serial MIDI out. this allows units to be cascaded for more CV and GATE outputs
  added CC message handler and the beginnings of a tweekable configuration
  Feb 16/19 - code restructure, added configuration menus
  March 19/19 - corrected BLEMIDI channel to serial MIDI mapping - add 1
  March 24/19 - force gates to be low for at least MIN_GATE_OFF_TIME. fix for apps that send rapid note off-note on sequences which result in gate times too short for modules to recognize
*/

#include <Wire.h>
#include "SPI.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MIDI.h"
#include <menu.h>
#include <menuIO/clickEncoderIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/adafruitGfxOut.h>
#include <menuIO/serialIn.h>
#include "MORAD_IO.h"
#include "DAC.h"
#include "HW_units.h"
#include "MORAD_utility.h"
#include "Smoother.h"
#include "NoteMachine.h"

//#include "menusystem.h"

void maindisplay(void); // function prototype needed for BT connect and disconnect

// create display device
#define OLED_RESET -1  // unused port - to keep the compiler happy
Adafruit_SSD1306 display(128,32,&Wire,OLED_RESET); 

struct SerialMIDISettings : public midi::DefaultSettings
{
  static const long BaudRate = 31250;
};
// must use HardwareSerial for extra UARTs
HardwareSerial MIDISerial(2);

// instantiate the serial MIDI library
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, MIDISerial, MIDI, SerialMIDISettings);

#include "MIDI_handlers.h"

// encoder
ClickEncoder clickEncoder(ENC_B, ENC_A, ENC_SW, 4); // divide by 4 works best with my encoder
ClickEncoderStream encStream(clickEncoder, 1);

// interrupt timer defs
#define ENC_TIMER_MICROS 1000 // 1khz for encoder
hw_timer_t * timer0 = NULL;
int monitorMode = 0;
int testcount = 0;
int updateNeeded = 0;
int clk_counter = 0;
unsigned long triggTime = 0;
int running = 0;
Smoother cvinvalue[4];

// encoder timer interrupt handler at 1khz

void ICACHE_RAM_ATTR encTimer() {
  clickEncoder.service();    // check the encoder inputs
}

#include "menusystem.h"  // has to follow encoder instance and HW_units.h

/*struct cvtomidi {
  bool active;
  int MIDIchannel;
  enum cvtype type;
  int ccnum;
  enum cvinpolarity polarity;
  } cvins[NUM_CV_OUTS] = {
*/

void handleCvIn() {
  for (int i = 0; i < NUM_CV_OUTS; i++) {
    if (cvins[i].active) {
       unsigned cvin = CVin(i);
       int ccvalue = 0;
       if(cvins[i].polarity == BIPOLAR) {
          ccvalue = cvin / 32;
       } else {
          ccvalue = cvin / 16 - 128;
          if(ccvalue < 0) {
            ccvalue = 0;
          }
       }
       int value = cvinvalue[i].read(ccvalue);
       if(lastccvval[i]!=value) {
          MIDI.sendControlChange(cvins[i].ccnum, value, cvins[i].MIDIchannel);
          lastccvval[i]=ccvalue;
       }
    }
  }
}

void handleGateIn() {
    for (int i = 0; i < NUM_CV_OUTS; i++) {
      unsigned gateValue = GateIn(i);
    }
}

void maindisplay(void) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  switch (monitorMode) {
    case 1: {
        display.print("CV1:");
        unsigned cv1 = CVin(0);
        unsigned cv2 = CVin(1);
        display.print(cv1);
        display.print(" CV2:");
        display.print(cv2);
        display.println();
        display.print("CC V= ");
        int x1 = cv1 / 32;
        int x2 = cv1 / 16 - 128;
        display.print(x1);
        display.print(" bp= ");
        display.print(x2);
        display.println();
        display.display();
        break;
      }
    case 0: {
        display.println("    MIDI <-> CV");
        display.println();
        display.println("     ");
        display.println("Press for Setup Menu");
        display.display();
        break;
      }
    case 2: {
        showCCValue(0, 0, 0);
        break;
      }
  }
}

void showCCValue(byte channel, byte number, byte value) {
  if (monitorMode == 2) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    display.print("MIDI CC #");
    display.print(number);
    display.print(" ");
    display.print(value);
    display.print(" ");
    display.println(channel);
    display.print("Clock: ");
    display.print(clk_counter);
    display.display();
  }
  updateNeeded = 0;
}

// 24 = 1/4
// 12 = 1/8
//  6 = 1/16
//  3 = 1/32

void triggerClock() {
  if (gateout[3].type == MIDI_CLOCK && running) {
    if (clk_counter % clockDivision == 0) {
      // Trigger
      GATEout(3, 1);
      triggTime = millis();
    }
  }
  clk_counter++;
}

void doStart() {
  running = 1;
  clk_counter = 0;
}

void doStop() {
  running = 0;
}

void readValue(int* param, int addr, int min, int max) {
  int v = EEPROM.read(addr);
  if (v >= min && v <= max) {
    *param = v;
  }
}

void readBool(bool* param, int addr) {
  int v = EEPROM.read(addr);
  if (v == 0) {
    *param = false;
  } else {
    *param = true;
  }
}

void readValueEnum(cvtype* param, int addr) {
  int v = EEPROM.read(addr);
  if (v == 0) {
    *param = NOTE_CV;
  } else if (v == 1) {
    *param = CC_CV;
  }
}

void readGateEnum(gatetype* param, int addr) {
  int v = EEPROM.read(addr);
  switch (v) {
    case 0:
      *param = NOTES_GATE;
      break;
    case 1:
      *param = NOTE_TRIGGER;
      break;
    case 2:
      *param = CC_GATE;
      break;
    case 3:
      *param = MIDI_CLOCK;
      break;
  }
}

//when menu is suspended
result idle(menuOut& o, idleEvent e) {
  maindisplay();
  return proceed;
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(100);

  pinMode(DAC0_CS, OUTPUT);
  pinMode(DAC1_CS, OUTPUT);
  pinMode(ADC_CS, OUTPUT);
  digitalWrite(DAC0_CS, HIGH);
  digitalWrite(DAC1_CS, HIGH);
  digitalWrite(ADC_CS, HIGH);
  pinMode(GATEout_0, OUTPUT);
  pinMode(GATEout_1, OUTPUT);
  pinMode(GATEout_2, OUTPUT);
  pinMode(GATEout_3, OUTPUT);

  pinMode(GATEin_0, INPUT);
  pinMode(GATEin_0, INPUT);
  pinMode(GATEin_0, INPUT);
  pinMode(GATEin_0, INPUT);

  SPI.begin(SCLK, MISO, MOSI, DAC0_CS); // we actually use a CS pin for each DAC
  SPI.setBitOrder(MSBFIRST);
  SPI.setFrequency(2000000); // ADC max clock 2 Mhz

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.clearDisplay();
  display.println("   MIDI Eric");
  display.println();
  display.println("    Mar 24/2019");
  display.display();

  //  Set up serial MIDI port
  MIDISerial.begin(31250, SERIAL_8N1, MIDIRX, MIDITX ); // midi port

  // set up serial MIDI library callbacks
  MIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandleControlChange(HandleControlChange);
  MIDI.setHandleClock(HandleMidiClock);
  MIDI.setHandleStart(HandleStart);
  MIDI.setHandleStop(HandleStop);
  // Initiate serial MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);
  

  Serial.println("INIT EPROM ");
  readValue(&gateout[0].MIDIchannel, 0, 0, 16);
  readValue(&gateout[1].MIDIchannel, 1, 0, 16);
  readValue(&gateout[2].MIDIchannel, 2, 0, 16);
  readValue(&gateout[3].MIDIchannel, 3, 0, 16);
  readValue(&cvout[0].MIDIchannel, 4, 0, 16);
  readValue(&cvout[1].MIDIchannel, 5, 0, 16);
  readValue(&cvout[2].MIDIchannel, 6, 0, 16);
  readValue(&cvout[3].MIDIchannel, 7, 0, 16);
  readValue(&cvout[0].CC_num, 8, 0, 127);
  readValue(&cvout[1].CC_num, 9, 0, 127);
  readValue(&cvout[2].CC_num, 10, 0, 127);
  readValue(&cvout[3].CC_num, 11, 0, 127);
  readValueEnum(&cvout[0].type, 12);
  readValueEnum(&cvout[1].type, 13);
  readValueEnum(&cvout[2].type, 14);
  readValueEnum(&cvout[3].type, 15);
  readGateEnum(&gateout[0].type, 16);
  readGateEnum(&gateout[1].type, 17);
  readGateEnum(&gateout[2].type, 18);
  readGateEnum(&gateout[3].type, 19);
  readValue(&gateout[0].CC_NOTE_num, 20, 0, 127);
  readValue(&gateout[1].CC_NOTE_num, 21, 0, 127);
  readValue(&gateout[2].CC_NOTE_num, 22, 0, 127);
  readValue(&gateout[3].CC_NOTE_num, 23, 0, 127);
  readValue(&clockDivision, 24, 0, 127);
  readBool(&midiThru, 25);

  // timer for encoder sampling
  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &encTimer, true);
  // Set alarm to trigger ISR (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer0, ENC_TIMER_MICROS, true);
  timerAlarmEnable(timer0);

  // set up the menu system
  nav.idleTask = idle; //point a function to be used when menu is suspended
  nav.idleOn(); // start up in idle state
  nav.navRoot::timeOut = 30; // inactivity timeout

  maindisplay();
}

bool connectedstatus = false; // flag to keep track of status display
unsigned long  measure;
void loop() {

  MIDI.read();  // do serial MIDI
  handleGateIn();
  handleCvIn();
  //do menus on a need to draw basis:
  nav.doInput();
  if (nav.changed(0)) {//only draw if changed
    nav.doOutput();
    display.display();
  }
  measure = millis();
  if (triggTime) {
    if ((measure - triggTime) > 10) {
      GATEout(3, 0);
      triggTime = 0;
    }
  }

  if (monitorMode == 1 || updateNeeded) {
    maindisplay();
  }

}
