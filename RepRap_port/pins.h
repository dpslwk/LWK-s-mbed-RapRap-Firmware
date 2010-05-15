/************************************************************
 *
 * LWK's mbed port of:-
 * Tonokip RepRap firmware rewrite based off of Hydra-mmm firmware.
 * Licence: GPL
 * 
 * Tonokip's firmware can be found @ http://github.com/tonokip/Tonokip-Firmware
 * LWK's firmware can be found @ http://github.com/dpslwk/LWK-s-mbed-RapRap-Firmware
 *
 *
 ************************************************************/
 
 
#ifndef PINS_H
#define PINS_H


#define USE_EXTRUDER_CONTROLLER false

/*
 * The following pins have been reserved for later
 *
 *
 * MOSI    p5
 * MISO    p6
 * SCK     p7
 * BOB3_SD_CS p8        
 *
 * TX-SDA  p9
 * RX-SCL  p10
 */


DigitalOut DEBUG_PIN(LED2);        //on board

DigitalOut X_STEP_PIN(p14);
DigitalOut X_DIR_PIN(p15);
DigitalOut X_ENABLE_PIN(p16);

DigitalOut Y_STEP_PIN(p17);
DigitalOut Y_DIR_PIN(p18);
DigitalOut Y_ENABLE_PIN (p19);

DigitalOut Z_STEP_PIN(p25);
DigitalOut Z_DIR_PIN(p26);
DigitalOut Z_ENABLE_PIN (p27);

DigitalOut E_STEP_PIN(p28);
DigitalOut E_DIR_PIN(p29);
DigitalOut E_ENABLE_PIN (p30);

// Endstops
DigitalOut X_MIN_PIN(p12);
//DigitalOut X_MAX_PIN(-1);
DigitalOut Y_MIN_PIN(p13);
//DigitalOut Y_MAX_PIN(-1);
DigitalOut Z_MIN_PIN(p14);
//DigitalOut Z_MAX_PIN(-1);

DigitalOut LED_PIN(LED1);        //on board
//DigitalOut FAN_PIN(p25);        // can be PwmOut
//DigitalOut PS_ON_PIN(p8);        
//DigitalOut KILL_PIN(p14);


DigitalOut HEATER_0_PIN(p21);        // can be PwmOut
//DigitalOut HEATER_1_PIN(p22);        // can be PwmOut
//DigitalOut HEATER_2_PIN(p23);        // can be PwmOut
//DigitalOut HEATER_BED_PIN(p24);        // can be PwmOut, reverved for heated bed

AnalogIn TEMP_0_PIN(p20);        // Analogin sample
//AnalogiIn TEMP_1_PIN(p19);        // Analogin sample
//AnalogiIn TEMP_1_PIN(p18);        // Analogin sample
//AnalogiIn TEMP_BED_PIN(p17);        // Analogin sample


#endif
