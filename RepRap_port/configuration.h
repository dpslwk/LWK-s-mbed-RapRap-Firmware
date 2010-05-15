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

#ifndef PARAMETERS_H
#define PARAMETERS_H

// NO RS485/EXTRUDER CONTROLLER SUPPORT
// PLEASE VERIFY PIN ASSIGNMENTS FOR YOUR CONFIGURATION!!!!!!!
#define MOTHERBOARD 5 // ATMEGA168 0, SANGUINO 1, MOTHERBOARD = 2, MEGA 3, ATMEGA328 4, LWK's Mega 5

// THERMOCOUPLE SUPPORT UNTESTED... USE WITH CAUTION!!!!
const bool USE_THERMISTOR = true; //Set to false if using thermocouple

// Calibration formulas
// e_extruded_steps_per_mm = e_feedstock_steps_per_mm * (desired_extrusion_diameter^2 / feedstock_diameter^2)
// new_axis_steps_per_mm = previous_axis_steps_per_mm * (test_distance_instructed/test_distance_traveled)
// units are in millimeters or whatever length unit you prefer: inches,football-fields,parsecs etc

//Calibration variables
float x_steps_per_unit = 55.3633218;
float y_steps_per_unit = 53.87328733;
float z_steps_per_unit = 4473.9;
float e_steps_per_unit = 16.73050624;
float max_feedrate = 3000;

//For Inverting Stepper Enable Pins (Active Low) use 0, Non Inverting (Active High) use 1
const bool X_ENABLE_ON = 0;
const bool Y_ENABLE_ON = 0;
const bool Z_ENABLE_ON = 0;
const bool E_ENABLE_ON = 0;

//Disables axis when it's not being used.
const bool DISABLE_X = false;
const bool DISABLE_Y = false;
const bool DISABLE_Z = true;
const bool DISABLE_E = false;

const bool INVERT_X_DIR = true;
const bool INVERT_Y_DIR = true;
const bool INVERT_Z_DIR = false;
const bool INVERT_E_DIR = false;

//Endstop Settings
const bool ENDSTOPS_INVERTING = true;
const bool min_software_endstops = false; //If true, axis won't move to coordinates less than zero.
const bool max_software_endstops = true;  //If true, axis won't move to coordinates greater than the defined lengths below.
const int X_MAX_LENGTH = 200;
const int Y_MAX_LENGTH = 200;
const int Z_MAX_LENGTH = 120;

#define BAUDRATE 19200

#endif
