#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library

#define TFT_CS     7  // define chip select pin
#define TFT_DC     9  // define data/command pin
#define TFT_RST    8  // define reset pin, or set to -1 and connect to Arduino RESET pin

#define BUFF_SIZE 2048

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

float p = 3.1415926;

IntervalTimer myTimer;
IntervalTimer uiTimer;

void setup(void) {
  Serial.begin(9600);
  Serial.print(F("Hello! ST77xx TFT Test"));

  // if the display has CS pin try with SPI_MODE0
  tft.init(240, 240, SPI_MODE2);    // Init ST7789 display 240x240 pixel

  // if the screen is flipped, remove this command
  tft.setRotation(2);

  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  myTimer.begin(handleInput, 25); // 100kHz
  uiTimer.begin(updateUi, 20000);
  myTimer.priority(80);
  uiTimer.priority(120);
}

int encoder = 0;
int lastvalue = 0;
int buffer1[BUFF_SIZE];
int buffer2[BUFF_SIZE];
int buffer3[BUFF_SIZE];
int buffercount = 0;
int frame = 0;
int freqDivide = 1; // = 10kz
long samplecount = 0;

int lastFrame = -1;
int go = 1;

void updateUi() {
  go = 0;
  int offsetx = analyseBuffer();
  //updateEncoder(offsetx);
  //updateFrame(offsetx);
  go = 1;
}

String noteValues[] = { "C ","C ", "C#", "D " , "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };

long time;

void handleInput() {
  if (buffercount == 0) {
    time = micros();
  }
  if (samplecount % freqDivide == 0) {
    buffer1[buffercount] = min(1023 - analogRead(A0), 1024);
    buffer2[buffercount] = min(1023 - analogRead(A1), 1024);
    buffercount++;
  }
  samplecount++;
  if (buffercount > BUFF_SIZE) {
    frame++;
    buffercount = 0;
    int offsetx = analyseBuffer();
    updateFrame(offsetx, micros() - time);
  }
}

float distance = 0;

int analyseBuffer() {
  int lastV = 0;  int riseStart = 0;
  int xoff = -1;
  for (int i = 0; i < BUFF_SIZE; i++) {
    int stepDiff = buffer1[i] - lastV;
    if (i > 0 && lastV < 512 && buffer1[i] >= 512) {
      buffer3[i] = 1023;
      if (xoff < 0) {
        xoff = i;
      }
    } else {
      buffer3[i] = 0;
    }
    lastV = buffer1[i];
  }

  int currentState = 0;
  int firstRiseIndex = -1;

  int overallDistance = 0;
  int distances = 0;
  float distCount = 0.0f;
  for (int i = 0; i < BUFF_SIZE; i++) {
    if (currentState == 0) { // Nothing detected
      if (buffer3[i] > 0) {
        currentState = 1; // Found first Rise;
        firstRiseIndex = i;
      }
    } else if (currentState == 1) { // We are on the rise
      if (buffer3[i] == 0) { // Rise has Ended
        currentState = 2;
      }
    } else if (currentState == 2) {
      if (buffer3[i] > 0) {
        if(distances > 2) {
          overallDistance += (i - firstRiseIndex);
          distCount = distCount + 1.0;
        }
        distances++;
        firstRiseIndex = i;
        currentState = 1;
      }
    }
  }
  if(distances > 0) {
    distance = (float)overallDistance / distCount;
  } else {
    distance = 0;
  }
  return min(max(xoff-4, 0), BUFF_SIZE - 240);
}

void FASTRUN loop() {

  //noInterrupts();
  //fcopy = frame;
  // interrupts();
  //delay(50);
}


void updateFrame(int offsetx, long framelength) {
  if (lastFrame != frame) {

    float steplen = (float)framelength / (float)(BUFF_SIZE+1);
    double freq = distance > 0 ? 1000000.0 / (double)(distance * steplen) : 0;
    double notes = 69 + 12 * log2(freq/440);
    int nv = max(0,round(notes)%12);
    int cents = (float)(notes - round(notes)) * 100 ;

    tft.fillRect(40, 0, 200, 27, ST77XX_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setTextWrap(false);
    tft.print("Fq=");
    tft.setCursor(40, 0);
    tft.setTextColor(ST77XX_BLUE);
    tft.print(String(freq));
    tft.print(" ");
    if(abs(cents) < 4) {
      tft.setTextColor(ST77XX_GREEN);
    } else {
      tft.setTextColor(ST77XX_RED);
    }
    tft.print(noteValues[nv+1]);
    tft.print(cents);

    tft.setCursor(90, 0);

    lastFrame = frame;
    tft.fillRect(0, 60, 240, 180, ST77XX_BLACK);
    tft.drawLine(0, 137, 240, 137, ST77XX_WHITE);

    drawGraph(ST77XX_RED, &buffer1[0], offsetx);
    drawGraph(ST77XX_ORANGE, &buffer2[0], offsetx);
  }
}


void drawGraph(int color, int* buffer, int offset) {
  int x = 0;
  int y = 239 - (buffer[offset] / 5);
  for (int i = 1; i < 240; i++) {
    int p = 239 - (buffer[i + offset] / 5);
    tft.drawLine(x, y, i, p, color);
    delay(0);
    x = i;
    y = p;
  }

}

/*
  void testlines(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6) {
    tft.drawLine(0, 0, x, tft.height() - 1, color);
    delay(0);
  }
  for (int16_t y = 0; y < tft.height(); y += 6) {
    tft.drawLine(0, 0, tft.width() - 1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6) {
    tft.drawLine(tft.width() - 1, 0, x, tft.height() - 1, color);
    delay(0);
  }
  for (int16_t y = 0; y < tft.height(); y += 6) {
    tft.drawLine(tft.width() - 1, 0, 0, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6) {
    tft.drawLine(0, tft.height() - 1, x, 0, color);
    delay(0);
  }
  for (int16_t y = 0; y < tft.height(); y += 6) {
    tft.drawLine(0, tft.height() - 1, tft.width() - 1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6) {
    tft.drawLine(tft.width() - 1, tft.height() - 1, x, 0, color);
    delay(0);
  }
  for (int16_t y = 0; y < tft.height(); y += 6) {
    tft.drawLine(tft.width() - 1, tft.height() - 1, 0, y, color);
    delay(0);
  }
  }

  void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextSize(30);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
  }

  void testfastlines(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t y = 0; y < tft.height(); y += 5) {
    tft.drawFastHLine(0, y, tft.width(), color1);
  }
  for (int16_t x = 0; x < tft.width(); x += 5) {
    tft.drawFastVLine(x, 0, tft.height(), color2);
  }
  }

  void testdrawrects(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6) {
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, color);
  }
  }

  void testfillrects(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x = tft.width() - 1; x > 6; x -= 6) {
    tft.fillRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, color1);
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, color2);
  }
  }

  void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x = radius; x < tft.width(); x += radius * 2) {
    for (int16_t y = radius; y < tft.height(); y += radius * 2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
  }

  void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x = 0; x < tft.width() + radius; x += radius * 2) {
    for (int16_t y = 0; y < tft.height() + radius; y += radius * 2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
  }

  void testtriangles() {
  tft.fillScreen(ST77XX_BLACK);
  int color = 0xF800;
  int t;
  int w = tft.width() / 2;
  int x = tft.height() - 1;
  int y = 0;
  int z = tft.width();
  for (t = 0 ; t <= 15; t++) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x -= 4;
    y += 4;
    z -= 4;
    color += 100;
  }
  }

  void testroundrects() {
  tft.fillScreen(ST77XX_BLACK);
  int color = 100;
  int i;
  int t;
  for (t = 0 ; t <= 4; t += 1) {
    int x = 0;
    int y = 0;
    int w = tft.width() - 2;
    int h = tft.height() - 2;
    for (i = 0 ; i <= 16; i += 1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x += 2;
      y += 3;
      w -= 4;
      h -= 6;
      color += 1100;
    }
    color += 100;
  }
  }

  void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
  }

  void mediabuttons() {
  // play
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_RED);
  delay(500);
  // pause
  tft.fillRoundRect(25, 90, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_GREEN);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_GREEN);
  delay(500);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_BLUE);
  delay(50);
  // pause color
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_RED);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_RED);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_GREEN);
  }*/
