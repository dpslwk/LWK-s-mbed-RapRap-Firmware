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
 *
 ************************************************************/

#ifndef MAIN_H
#define MAIN_H

typedef unsigned int boolean;

#define max(a,b) ((a)>(b)?(a):(b))
#define HIGH 1
#define LOW 0
#define false 0
#define true 1


void setup();
void loop();
void reverse(char s[]);
void itoa(int n, char s[]);
inline void get_command();
inline float code_value();
inline int code_value_long();
inline bool code_seen(char code_string[]);
inline bool code_seen(char code);
inline void process_commands();
inline void FlushSerialRequestResend();
inline void ClearToSend();
inline void get_coordinates();
void linear_move(unsigned int x_steps_remaining, unsigned int y_steps_remaining, unsigned int z_steps_remaining, unsigned int e_steps_remaining);
inline void do_x_step();
inline void do_y_step();
inline void do_z_step();
inline void do_e_step();
inline void disable_x();
inline void disable_y();
inline void disable_z();
inline void disable_e();
inline void  enable_x();
inline void  enable_y();
inline void  enable_z();
inline void  enable_e();
inline void manage_heater();
float temp2analog(int celsius);
float analog2temp(int raw);
inline void kill(char debug);
inline void manage_inactivity(char debug);void setup();


#endif
