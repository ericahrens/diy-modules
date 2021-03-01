#ifndef SMOOTHER_H_
#define SMOOTHER_H_

#define BUFFER_SIZE 10

class Smoother {
  public:
    Smoother() {
      lastread = -1;
    }

    int read(int value);

  private:
    int lastValue;
    unsigned long lastread;
};


int Smoother::read(int value) {
  unsigned long time = millis();
  if(lastread == -1 ) {
    lastread = value;
    return value;
  }
  int diff = abs(value-lastValue);
  if(diff > 1 || diff == 0) {
    lastread = time;
    lastValue = value;
    return value;
  }
  if((time-lastread) < 50) {
    int acc = lastValue + value;
    lastValue = acc/2;
    return lastValue;
  }
  lastread = time;
  lastValue = value;
  return value;
}

#endif
