#ifndef MAX6675_h
#define MAX6675_h

#include "mbed.h"

class max6675
{
    SPI& spi;
    DigitalOut ncs;
  public:
  
    max6675(SPI& _spi, PinName _ncs);
    void select();
    void deselect();
    
    float read_temp();
  private:
    PinName _CS_pin;
    PinName _SO_pin;
    PinName _SCK_pin;
    int _units;
    float _error;
};

#endif
