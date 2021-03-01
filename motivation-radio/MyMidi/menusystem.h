
// Copyright 2019 Rich Heslip
//
// Author: Rich Heslip
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// definitions for Arduino Menu System
// https://github.com/neu-rah/ArduinoMenu/tree/master/src/menuIO

//using namespace Menu;

#include <EEPROM.h>

#define gfxWidth 128
#define gfxHeight 32
#define fontX 6    // font width in pixels
#define fontY 10  // vertical text spacing
#define MAX_DEPTH 3 // max menu depth?
#define textScale 1


result enterMonitorMode(void) {
  if (monitorMode == 0) {
    monitorMode = 1;
  } else {
    monitorMode = 0;
  }
  return quit;
}

result enterMidiMonitorMode(void) {
  if (monitorMode == 0) {
    monitorMode = 2;
  } else {
    monitorMode = 0;
  }
  updateNeeded = 1;
  result quit;
}

// save settings to eeprom - not yet implemented
result savesetup(void) {
  Serial.println("SAVE This Dude ");
  timerEnd(timer0);
  timerDetachInterrupt(timer0);
  delay(1);
  EEPROM.write(0, gateout[0].MIDIchannel);
  EEPROM.write(1, gateout[1].MIDIchannel);
  EEPROM.write(2, gateout[2].MIDIchannel);
  EEPROM.write(3, gateout[3].MIDIchannel);
  EEPROM.write(4, cvout[0].MIDIchannel);
  EEPROM.write(5, cvout[1].MIDIchannel);
  EEPROM.write(6, cvout[2].MIDIchannel);
  EEPROM.write(7, cvout[3].MIDIchannel);
  EEPROM.write(8, cvout[0].CC_num);
  EEPROM.write(9, cvout[1].CC_num);
  EEPROM.write(10, cvout[2].CC_num);
  EEPROM.write(11, cvout[3].CC_num);
  EEPROM.write(8, cvout[0].CC_num);
  EEPROM.write(9, cvout[1].CC_num);
  EEPROM.write(10, cvout[2].CC_num);
  EEPROM.write(11, cvout[3].CC_num);
  EEPROM.write(12, (int)cvout[0].type);
  EEPROM.write(13, (int)cvout[1].type);
  EEPROM.write(14, (int)cvout[2].type);
  EEPROM.write(15, (int)cvout[3].type);

  EEPROM.write(16, (int)gateout[0].type);
  EEPROM.write(17, (int)gateout[1].type);
  EEPROM.write(18, (int)gateout[2].type);
  EEPROM.write(19, (int)gateout[3].type);
  EEPROM.write(20, gateout[0].CC_NOTE_num);
  EEPROM.write(21, gateout[1].CC_NOTE_num);
  EEPROM.write(22, gateout[2].CC_NOTE_num);
  EEPROM.write(23, gateout[3].CC_NOTE_num);
  EEPROM.write(24, clockDivision);

  EEPROM.commit();
  delay(1);
  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &encTimer, true);
  timerAlarmWrite(timer0, ENC_TIMER_MICROS, true);
  timerAlarmEnable(timer0);

  return quit;
}


// Gate A select mode
TOGGLE(gateout[0].type, subMenu_GATE_A_TYPE, "GATE_A: ", doNothing, noEvent, wrapStyle,
       VALUE("GATE", NOTES_GATE, doNothing, noEvent),
       VALUE("TRIG on NOTE", NOTE_TRIGGER, doNothing, noEvent),
       VALUE("CC to GATE", CC_GATE, doNothing, noEvent)
      );

// Gate B select mode
TOGGLE(gateout[1].type, subMenu_GATE_B_TYPE, "GATE_B: ", doNothing, noEvent, wrapStyle,
       VALUE("GATE", NOTES_GATE, doNothing, noEvent),
       VALUE("TRIG on NOTE", NOTE_TRIGGER, doNothing, noEvent),
       VALUE("CC to GATE", CC_GATE, doNothing, noEvent)
      );

// Gate C select mode
TOGGLE(gateout[2].type, subMenu_GATE_C_TYPE, "GATE_C: ", doNothing, noEvent, wrapStyle,
       VALUE("GATE", NOTES_GATE, doNothing, noEvent),
       VALUE("TRIG on NOTE", NOTE_TRIGGER, doNothing, noEvent),
       VALUE("CC to GATE", CC_GATE, doNothing, noEvent)
      );

// Gate D select mode
TOGGLE(gateout[3].type, subMenu_GATE_D_TYPE, "GATE_D: ", doNothing, noEvent, wrapStyle,
       VALUE("GATE", NOTES_GATE, doNothing, noEvent),
       VALUE("TRIG on NOTE", NOTE_TRIGGER, doNothing, noEvent),
       VALUE("CC to GATE", CC_GATE, doNothing, noEvent),
       VALUE("Clock", MIDI_CLOCK, doNothing, noEvent)
      );

// CV A select mode
TOGGLE(cvout[0].type, subMenu_CV_A_TYPE, "CV_A: ", doNothing, noEvent, wrapStyle,
       VALUE("NOTE to CV", NOTE_CV, doNothing, noEvent),
       VALUE("CC to CV", CC_CV, doNothing, noEvent)
      );

// CV B select mode
TOGGLE(cvout[1].type, subMenu_CV_B_TYPE, "CV_B: ", doNothing, noEvent, wrapStyle,
       VALUE("NOTE to CV", NOTE_CV, doNothing, noEvent),
       VALUE("CC to CV", CC_CV, doNothing, noEvent)
      );

// CV C select mode
TOGGLE(cvout[2].type, subMenu_CV_C_TYPE, "CV_C: ", doNothing, noEvent, wrapStyle,
       VALUE("NOTE to CV", NOTE_CV, doNothing, noEvent),
       VALUE("CC to CV", CC_CV, doNothing, noEvent)
      );

// CV D select mode
TOGGLE(cvout[3].type, subMenu_CV_D_TYPE, "CV_D: ", doNothing, noEvent, wrapStyle,
       VALUE("NOTE to CV", NOTE_CV, doNothing, noEvent),
       VALUE("CC to CV", CC_CV, doNothing, noEvent)
      );

// CV A IN Configuration
TOGGLE(cvins[0].active, subMenu_IN_A_ACTIVE, "CV IN A: ", doNothing, noEvent, wrapStyle,
       VALUE("deactivated", false, doNothing, noEvent),
       VALUE("activated", true, doNothing, noEvent)
      );

TOGGLE(cvins[0].type, subMenu_IN_A_TYPE, "A Type: ", doNothing, noEvent, wrapStyle,
       VALUE("CV TO NOTE", NOTE_CV, doNothing, noEvent),
       VALUE("CV TO CC", CC_CV, doNothing, noEvent)
      );

TOGGLE(cvins[0].polarity, subMenu_IN_A_POLARITY, "POLARITY: ", doNothing, noEvent, wrapStyle,
       VALUE("UNIPOLAR", UNIPOLAR, doNothing, noEvent),
       VALUE("BIPOLAR", BIPOLAR, doNothing, noEvent)
      );

// CV B IN Configuration
TOGGLE(cvins[1].active, subMenu_IN_B_ACTIVE, "CV IN B: ", doNothing, noEvent, wrapStyle,
       VALUE("deactivated", false, doNothing, noEvent),
       VALUE("activated", true, doNothing, noEvent)
      );

TOGGLE(cvins[1].type, subMenu_IN_B_TYPE, "B IN Type: ", doNothing, noEvent, wrapStyle,
       VALUE("CV TO NOTE", NOTE_CV, doNothing, noEvent),
       VALUE("CV TO CC", CC_CV, doNothing, noEvent)
      );

TOGGLE(cvins[1].polarity, subMenu_IN_B_POLARITY, "POLARITY: ", doNothing, noEvent, wrapStyle,
       VALUE("UNIPOLAR", UNIPOLAR, doNothing, noEvent),
       VALUE("BIPOLAR", BIPOLAR, doNothing, noEvent)
      );

// CV C IN Configuration
TOGGLE(cvins[2].active, subMenu_IN_C_ACTIVE, "CV IN C: ", doNothing, noEvent, wrapStyle,
       VALUE("deactivated", false, doNothing, noEvent),
       VALUE("activated", true, doNothing, noEvent)
      );

TOGGLE(cvins[2].type, subMenu_IN_C_TYPE, "C IN Type: ", doNothing, noEvent, wrapStyle,
       VALUE("CV TO NOTE", NOTE_CV, doNothing, noEvent),
       VALUE("CV TO CC", CC_CV, doNothing, noEvent)
      );

TOGGLE(cvins[2].polarity, subMenu_IN_C_POLARITY, "POLARITY: ", doNothing, noEvent, wrapStyle,
       VALUE("UNIPOLAR", UNIPOLAR, doNothing, noEvent),
       VALUE("BIPOLAR", BIPOLAR, doNothing, noEvent)
      );

// CV D IN Configuration
TOGGLE(cvins[3].active, subMenu_IN_D_ACTIVE, "CV IN D: ", doNothing, noEvent, wrapStyle,
       VALUE("deactivated", false, doNothing, noEvent),
       VALUE("activated", true, doNothing, noEvent)
      );

TOGGLE(cvins[3].type, subMenu_IN_D_TYPE, "D IN Type: ", doNothing, noEvent, wrapStyle,
       VALUE("CV TO NOTE", NOTE_CV, doNothing, noEvent),
       VALUE("CV TO CC", CC_CV, doNothing, noEvent)
      );

TOGGLE(cvins[3].polarity, subMenu_IN_D_POLARITY, "POLARITY: ", doNothing, noEvent, wrapStyle,
       VALUE("UNIPOLAR", UNIPOLAR, doNothing, noEvent),
       VALUE("BIPOLAR", BIPOLAR, doNothing, noEvent)
      );

SELECT(clockDivision, subMenu_Clock_div, "Clock Cfg", doNothing, noEvent, wrapStyle,
       VALUE("1/4", 24, doNothing, noEvent),
       VALUE("1/4T", 16, doNothing, noEvent),
       VALUE("1/8D", 18, doNothing, noEvent),
       VALUE("1/8", 12, doNothing, noEvent),
       VALUE("1/8T", 8, doNothing, noEvent),
       VALUE("1/16D", 9, doNothing, noEvent),
       VALUE("1/16", 6, doNothing, noEvent),
       VALUE("1/32T", 4, doNothing, noEvent),
       VALUE("1/32", 3, doNothing, noEvent));

// RH I had to add submenus for each unit setup
// the menu macros can't handle really large menu definitions

MENU(subMenu_UNIT_A_SETUP, "UNIT A", doNothing, noEvent, wrapStyle,
     SUBMENU(subMenu_GATE_A_TYPE),
     FIELD(gateout[0].MIDIchannel, "GATE_A MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(gateout[0].CC_NOTE_num, "GATE_A NOTE/CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),

     SUBMENU(subMenu_CV_A_TYPE),
     FIELD(cvout[0].MIDIchannel, "CV_A MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(cvout[0].CC_num, "CV_A CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),
     EXIT("<Back")
    );

MENU(subMenu_UNIT_B_SETUP, "UNIT B", doNothing, noEvent, wrapStyle,
     SUBMENU(subMenu_GATE_B_TYPE),
     FIELD(gateout[1].MIDIchannel, "GATE_B MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(gateout[1].CC_NOTE_num, "GATE_B NOTE/CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),

     SUBMENU(subMenu_CV_B_TYPE),
     FIELD(cvout[1].MIDIchannel, "CV_B MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(cvout[1].CC_num, "CV_B CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),
     EXIT("<Back")
    );

MENU(subMenu_UNIT_C_SETUP, "UNIT C", doNothing, noEvent, wrapStyle,
     SUBMENU(subMenu_GATE_C_TYPE),
     FIELD(gateout[2].MIDIchannel, "GATE_C MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(gateout[2].CC_NOTE_num, "GATE_C NOTE/CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),

     SUBMENU(subMenu_CV_C_TYPE),
     FIELD(cvout[2].MIDIchannel, "CV_C MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(cvout[2].CC_num, "CV_C CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),
     EXIT("<Back")
    );

MENU(subMenu_UNIT_D_SETUP, "UNIT D", doNothing, noEvent, wrapStyle,
     SUBMENU(subMenu_GATE_D_TYPE),
     FIELD(gateout[3].MIDIchannel, "GATE_D MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(gateout[3].CC_NOTE_num, "GATE_D NOTE/CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),

     SUBMENU(subMenu_CV_D_TYPE),
     FIELD(cvout[3].MIDIchannel, "CV_D MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(cvout[3].CC_num, "CV_D CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),
     EXIT("<Back")
    );

MENU(subMenu_CLOCK_SETUP, "Clock CFG", doNothing, noEvent, wrapStyle,
     SUBMENU(subMenu_Clock_div),
     EXIT("<Back")
    );


MENU(subMenu_IN_A_SETUP, "INPUTS A", doNothing, noEvent, wrapStyle,
     SUBMENU(subMenu_IN_A_ACTIVE),
     FIELD(cvins[0].MIDIchannel, "A MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(cvins[0].ccnum, "CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),
     SUBMENU(subMenu_IN_A_TYPE),
     SUBMENU(subMenu_IN_A_POLARITY),
     EXIT("<Back")
    );

MENU(subMenu_IN_B_SETUP, "INPUTS B", doNothing, noEvent, wrapStyle,
     SUBMENU(subMenu_IN_B_ACTIVE),
     FIELD(cvins[1].MIDIchannel, "B MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(cvins[1].ccnum, "CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),
     SUBMENU(subMenu_IN_B_TYPE),
     SUBMENU(subMenu_IN_B_POLARITY),
     EXIT("<Back")
    );

MENU(subMenu_IN_C_SETUP, "INPUTS C", doNothing, noEvent, wrapStyle,
     SUBMENU(subMenu_IN_C_ACTIVE),
     FIELD(cvins[2].MIDIchannel, "C MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(cvins[2].ccnum, "CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),
     SUBMENU(subMenu_IN_C_TYPE),
     SUBMENU(subMenu_IN_C_POLARITY),
     EXIT("<Back")
    );

MENU(subMenu_IN_D_SETUP, "INPUTS D", doNothing, noEvent, wrapStyle,
     SUBMENU(subMenu_IN_D_ACTIVE),
     FIELD(cvins[3].MIDIchannel, "D MIDI CH ", "", 1, 16, 1, 0, doNothing, noEvent, wrapStyle),
     FIELD(cvins[3].ccnum, "CC# ", "", 0, 127, 1, 0, doNothing, noEvent, wrapStyle),
     SUBMENU(subMenu_IN_D_TYPE),
     SUBMENU(subMenu_IN_D_POLARITY),
     EXIT("<Back")
    );

// MENU(notemaschine_1_SETUP, "NOTE MASCHINE1 ", doNothing, noEvent, wrapeStyle,
//     EXIT("<Back")
//);

// save setup not implemented yet
//   OP("Save Setup",savesetup,enterEvent),

MENU(mainMenu, "        SETUP", doNothing, noEvent, wrapStyle,
     SUBMENU(subMenu_UNIT_A_SETUP),
     SUBMENU(subMenu_UNIT_B_SETUP),
     SUBMENU(subMenu_UNIT_C_SETUP),
     SUBMENU(subMenu_UNIT_D_SETUP),
     SUBMENU(subMenu_IN_A_SETUP),
     SUBMENU(subMenu_IN_B_SETUP),
     SUBMENU(subMenu_IN_C_SETUP),
     SUBMENU(subMenu_IN_D_SETUP),
     SUBMENU(subMenu_CLOCK_SETUP),
     OP("Monitor In", enterMonitorMode, enterEvent),
     OP("MIDI Monitor", enterMidiMonitorMode, enterEvent),
     OP("Save Setup", savesetup, enterEvent),
     EXIT("<Exit Setup Menu")
    );

// define menu colors --------------------------------------------------------
//  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
//monochromatic color table
const colorDef<uint16_t> colors[] MEMMODE = {
  {{WHITE, BLACK}, {WHITE, BLACK, BLACK}}, //bgColor
  {{BLACK, WHITE}, {BLACK, WHITE, WHITE}}, //fgColor
  {{BLACK, WHITE}, {BLACK, WHITE, WHITE}}, //valColor
  {{BLACK, WHITE}, {BLACK, WHITE, WHITE}}, //unitColor
  {{BLACK, WHITE}, {WHITE, WHITE, WHITE}}, //cursorColor
  {{BLACK, WHITE}, {WHITE, BLACK, BLACK}}, //titleColor
};

//the encoder button is a keyboard with only one key
keyMap encBtn_map[] = {{ -ENC_SW, defaultNavCodes[enterCmd].ch}}; //negative pin numbers use internal pull-up, switch is on when low
keyIn<1> encButton(encBtn_map);   //1 is the number of keys

// menu system is driven by the encoder stream
MENU_INPUTS(in, &encStream);

MENU_OUTPUTS(out, MAX_DEPTH
             , ADAGFX_OUT(display, colors, fontX, fontY, {0, 0, gfxWidth / fontX, gfxHeight / fontY})
             , NONE //must have 2 items at least if using this macro
            );

NAVROOT(nav, mainMenu, MAX_DEPTH, in, out);
