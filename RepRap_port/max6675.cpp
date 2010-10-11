
#include <mbed.h>
#include "max6675.h"

max6675::max6675(SPI& _spi, PinName _ncs) : spi(_spi), ncs(_ncs) {

}

float max6675::read_temp() {
    short value = 0;
    float temp = 0;
    
    uint8_t highByte=0;
    uint8_t lowByte=0;
    
    select();
    wait(.25); //This delay is needed else it does'nt seem to update the temp

    highByte = spi.write(0);
    lowByte = spi.write(0);
    deselect();


    if (lowByte & (1<<2)) {
        error("No Probe");
    } else {
        value = (highByte << 5 | lowByte>>3);
    }

    temp = (value*0.25); // Multiply the value by 0.25 to get temp in ˚C or
                         //  * (9.0/5.0)) + 32.0;   // Convert value to ˚F (ensure proper floats!)

return temp;
}

void max6675::select() {
    ncs = 0;
}

void max6675::deselect() {
    ncs = 1;
}
