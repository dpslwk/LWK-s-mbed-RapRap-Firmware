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
 
#include "mbed.h"
#include "main.h"
#include "configuration.h"
#include "pins.h"
#include "ThermistorTable.h"

DigitalOut myled(LED1);
Timer t;
Serial pc(USBTX, USBRX);

#define max(a,b) ((a)>(b)?(a):(b))
#define HIGH 1
#define LOW 0


int main() {
    setup();
    while(1) {
        loop();
    }
}


// look here for descriptions of gcodes: http://linuxcnc.org/handbook/gcode/g-code.html
// http://objects.reprap.org/wiki/Mendel_User_Manual:_RepRapGCodes

//Implemented Codes
//-------------------
// G0 -> G1
// G1  - Coordinated Movement X Y Z E
// G4  - Dwell S<seconds> or P<milliseconds>
// G90 - Use Absolute Coordinates
// G91 - Use Relative Coordinates
// G92 - Set current position to cordinates given

//RepRap M Codes
// M104 - Set target temp
// M105 - Read current temp
// M109 - Wait for current temp to reach target temp.

//Custom M Codes

// **M80  - Turn on Power Supply
// **M81  - Turn off Power Supply
// ** disabled by LWK
// M82  - Set E codes absolute (default)
// M83  - Set E codes relative while in Absolute Coordinates (G90) mode
// M84  - Disable steppers until next move
// M85  - Set inactivity shutdown timer with parameter S<seconds>. To disable set zero (default)
// M92  - Set axis_steps_per_unit - same syntax as G92

//Stepper Movement Variables
bool direction_x, direction_y, direction_z, direction_e;
unsigned long previous_micros=0, previous_micros_x=0, previous_micros_y=0, previous_micros_z=0, previous_micros_e=0, previous_millis_heater;
unsigned long x_steps_to_take, y_steps_to_take, z_steps_to_take, e_steps_to_take;
float destination_x =0.0, destination_y = 0.0, destination_z = 0.0, destination_e = 0.0;
float current_x = 0.0, current_y = 0.0, current_z = 0.0, current_e = 0.0;
float x_interval, y_interval, z_interval, e_interval; // for speed delay
float feedrate = 1500, next_feedrate;
float time_for_move;
long gcode_N, gcode_LastN;
bool relative_mode = false;  //Determines Absolute or Relative Coordinates
bool relative_mode_e = false;  //Determines Absolute or Relative E Codes while in Absolute Coordinates mode. E is always relative in Relative Coordinates mode.

// comm variables
#define MAX_CMD_SIZE 256
char cmdbuffer[MAX_CMD_SIZE];
char serial_char;
int serial_count = 0;
boolean comment_mode = false;
char *strchr_pointer; // just a pointer to find chars in the cmd string like X, Y, Z, E, etc

//manage heater variables
int target_raw = 0;
int current_raw;

//Inactivity shutdown variables
unsigned long previous_millis_cmd=0;
unsigned long max_inactive_time = 0;

void setup()
{ 
/*
  //Initialize Step Pins
  if(X_STEP_PIN > -1) pinMode(X_STEP_PIN,OUTPUT);
  if(Y_STEP_PIN > -1) pinMode(Y_STEP_PIN,OUTPUT);
  if(Z_STEP_PIN > -1) pinMode(Z_STEP_PIN,OUTPUT);
  if(E_STEP_PIN > -1) pinMode(E_STEP_PIN,OUTPUT);
  
  //Initialize Dir Pins
  if(X_DIR_PIN > -1) pinMode(X_DIR_PIN,OUTPUT);
  if(Y_DIR_PIN > -1) pinMode(Y_DIR_PIN,OUTPUT);
  if(Z_DIR_PIN > -1) pinMode(Z_DIR_PIN,OUTPUT);
  if(E_DIR_PIN > -1) pinMode(E_DIR_PIN,OUTPUT);
*/
  //Steppers default to disabled.
  if(X_ENABLE_PIN > -1) if(!X_ENABLE_ON) X_ENABLE_PIN.write(HIGH);
  if(Y_ENABLE_PIN > -1) if(!Y_ENABLE_ON) Y_ENABLE_PIN.write(HIGH);
  if(Z_ENABLE_PIN > -1) if(!Z_ENABLE_ON) Z_ENABLE_PIN.write(HIGH);
  if(E_ENABLE_PIN > -1) if(!E_ENABLE_ON) E_ENABLE_PIN.write(HIGH);
/*  
  //Initialize Enable Pins
  if(X_ENABLE_PIN > -1) pinMode(X_ENABLE_PIN,OUTPUT);
  if(Y_ENABLE_PIN > -1) pinMode(Y_ENABLE_PIN,OUTPUT);
  if(Z_ENABLE_PIN > -1) pinMode(Z_ENABLE_PIN,OUTPUT);
  if(E_ENABLE_PIN > -1) pinMode(E_ENABLE_PIN,OUTPUT);

  if(HEATER_0_PIN > -1) pinMode(HEATER_0_PIN,OUTPUT);
*/  
  pc.baud(BAUDRATE);
  pc.printf("start\n\r");
  t.start();
}


void loop()
{
  get_command();
  manage_heater();
  
  manage_inactivity(1); //shutdown if not receiving any new commands
}

inline void get_command() 
{ 

  if( pc.readable() > 0 ) {
    serial_char = pc.getc();
    if(serial_char == '\n' || serial_char == '\r' || serial_char == ':' || serial_count >= (MAX_CMD_SIZE - 1) ) 
    {
      if(!serial_count) return; //if empty line
      cmdbuffer[serial_count] = 0; //terminate string
      pc.printf("Echo:");
      pc.printf("%s\n\r", &cmdbuffer[0]);
      
      process_commands();
      
      comment_mode = false; //for new command
      serial_count = 0; //clear buffer
      //pc.printf("ok\n\r"); 
    }
    else
    {
      if(serial_char == ';') comment_mode = true;
      if(!comment_mode) cmdbuffer[serial_count++] = serial_char; 
    }
  }  
}


//#define code_num (strtod(&cmdbuffer[strchr_pointer - cmdbuffer + 1], NULL))
//inline void code_search(char code) { strchr_pointer = strchr(cmdbuffer, code); }
inline float code_value() { return (strtod(&cmdbuffer[strchr_pointer - cmdbuffer + 1], NULL)); }
inline long code_value_long() { return (strtol(&cmdbuffer[strchr_pointer - cmdbuffer + 1], NULL, 10)); }
inline bool code_seen(char code_string[]) { return (strstr(cmdbuffer, code_string) != NULL); }  //Return True if the string was found

inline bool code_seen(char code)
{
  strchr_pointer = strchr(cmdbuffer, code);
  return (strchr_pointer != NULL);  //Return True if a character was found
}



inline void process_commands()
{
  unsigned long codenum; //throw away variable
  
  if(code_seen('N'))
  {
    gcode_N = code_value_long();
    if(gcode_N != gcode_LastN+1 && (strstr(cmdbuffer, "M110") == NULL) ) {
    //if(gcode_N != gcode_LastN+1 && !code_seen("M110") ) {   //Hmm, compile size is different between using this vs the line above even though it should be the same thing. Keeping old method.
      pc.printf("Serial Error: Line Number is not Last Line Number+1, Last Line:");
      pc.printf("%ld\n\r", &gcode_LastN);
      FlushSerialRequestResend();
      return;
    }
    
    if(code_seen('*'))
    {
      char checksum = 0;
      char count=0;
      while(cmdbuffer[count] != '*') checksum = checksum^cmdbuffer[count++];
     
      if( (int)code_value() != checksum) {
        pc.printf("Error: checksum mismatch, Last Line:");
        pc.printf("%ld\n\r", &gcode_LastN);
        FlushSerialRequestResend();
        return;
      }
      //if no errors, continue parsing
    }
    else 
    {
      pc.printf("Error: No Checksum with line number, Last Line:");
      pc.printf("%ld\n\r", &gcode_LastN);
      FlushSerialRequestResend();
      return;
    }
    
    gcode_LastN = gcode_N;
    //if no errors, continue parsing
  }
  else  // if we don't receive 'N' but still see '*'
  {
    if(code_seen('*'))
    {
      pc.printf("Error: No Line Number with checksum, Last Line:");
      pc.printf("%ld\n\r", &gcode_LastN);
      return;
    }
  }

  //continues parsing only if we don't receive any 'N' or '*' or no errors if we do. :)
  
  if(code_seen('G'))
  {
    switch((int)code_value())
    {
      case 0: // G0 -> G1
      case 1: // G1
        get_coordinates(); // For X Y Z E F
        x_steps_to_take = abs(destination_x - current_x)*x_steps_per_unit;
        y_steps_to_take = abs(destination_y - current_y)*y_steps_per_unit;
        z_steps_to_take = abs(destination_z - current_z)*z_steps_per_unit;
        e_steps_to_take = abs(destination_e - current_e)*e_steps_per_unit;

        #define X_TIME_FOR_MOVE ((float)x_steps_to_take / (x_steps_per_unit*feedrate/60000000))
        #define Y_TIME_FOR_MOVE ((float)y_steps_to_take / (y_steps_per_unit*feedrate/60000000))
        #define Z_TIME_FOR_MOVE ((float)z_steps_to_take / (z_steps_per_unit*feedrate/60000000))
        #define E_TIME_FOR_MOVE ((float)e_steps_to_take / (e_steps_per_unit*feedrate/60000000))
        
        time_for_move = max(X_TIME_FOR_MOVE,Y_TIME_FOR_MOVE);
        time_for_move = max(time_for_move,Z_TIME_FOR_MOVE);
        time_for_move = max(time_for_move,E_TIME_FOR_MOVE);

        if(x_steps_to_take) x_interval = time_for_move/x_steps_to_take;
        if(y_steps_to_take) y_interval = time_for_move/y_steps_to_take;
        if(z_steps_to_take) z_interval = time_for_move/z_steps_to_take;
        if(e_steps_to_take) e_interval = time_for_move/e_steps_to_take;
        
        #define DEBUGGING false
        if(DEBUGGING) {
          pc.printf("destination_x: "); pc.printf("%f\n\r", &destination_x); 
          pc.printf("current_x: "); pc.printf("%f\n\r", &current_x); 
          pc.printf("x_steps_to_take: "); pc.printf("%lu\n\r", &x_steps_to_take); 
          pc.printf("X_TIME_FOR_MVE: "); pc.printf("%f\n\r", X_TIME_FOR_MOVE); 
          pc.printf("x_interval: "); pc.printf("%f\n\r", &x_interval); 
          pc.printf("\n\r");
          pc.printf("destination_y: "); pc.printf("%f\n\r", &destination_y); 
          pc.printf("current_y: "); pc.printf("%f\n\r", &current_y); 
          pc.printf("y_steps_to_take: "); pc.printf("%lu\n\r", &y_steps_to_take); 
          pc.printf("Y_TIME_FOR_MVE: "); pc.printf("%f\n\r", Y_TIME_FOR_MOVE); 
          pc.printf("y_interval: "); pc.printf("%f\n\r", &y_interval); 
          pc.printf("\n\r");
          pc.printf("destination_z: "); pc.printf("%f\n\r", &destination_z); 
          pc.printf("current_z: "); pc.printf("%f\n\r", &current_z); 
          pc.printf("z_steps_to_take: "); pc.printf("%lu\n\r", &z_steps_to_take); 
          pc.printf("Z_TIME_FOR_MVE: "); pc.printf("%f\n\r", Z_TIME_FOR_MOVE); 
          pc.printf("z_interval: "); pc.printf("%f\n\r", &z_interval); 
          pc.printf("\n\r");
          pc.printf("destination_e: "); pc.printf("%f\n\r", &destination_e); 
          pc.printf("current_e: "); pc.printf("%f\n\r", &current_e); 
          pc.printf("e_steps_to_take: "); pc.printf("%lu\n\r", &e_steps_to_take); 
          pc.printf("E_TIME_FOR_MVE: "); pc.printf("%f\n\r", E_TIME_FOR_MOVE); 
          pc.printf("e_interval: "); pc.printf("%f\n\r", &e_interval); 
          pc.printf("\n\r");
        }
        
        linear_move(x_steps_to_take, y_steps_to_take, z_steps_to_take, e_steps_to_take); // make the move
        ClearToSend();
        return;
      case 4: // G4 dwell
        codenum = 0;
        if(code_seen('P')) codenum = code_value(); // milliseconds to wait
        if(code_seen('S')) codenum = code_value()*1000; // seconds to wait
        previous_millis_heater = t.read_ms();  // keep track of when we started waiting
        while((t.read_ms() - previous_millis_heater) < codenum ) manage_heater(); //manage heater until time is up
        break;
      case 90: // G90
        relative_mode = false;
        break;
      case 91: // G91
        relative_mode = true;
        break;
      case 92: // G92
        if(code_seen('X')) current_x = code_value();
        if(code_seen('Y')) current_y = code_value();
        if(code_seen('Z')) current_z = code_value();
        if(code_seen('E')) current_e = code_value();
        break;
        
    }
  }

  if(code_seen('M'))
  {
    
    switch( (int)code_value() ) 
    {
      case 104: // M104
        if (code_seen('S')) target_raw = temp2analog(code_value());
        break;
      case 105: // M105
        pc.printf("T:");
        pc.printf("%f\n\r", analog2temp(TEMP_0_PIN.read()) ); 
        if(!code_seen('N')) return;  // If M105 is sent from generated gcode, then it needs a response.
        break;
      case 109: // M109 - Wait for heater to reach target.
        if (code_seen('S')) target_raw = temp2analog(code_value());
        previous_millis_heater = t.read_ms(); 
        while(current_raw < target_raw) {
          if( (t.read_ms()-previous_millis_heater) > 1000 ) //Print Temp Reading every 1 second while heating up.
          {
            pc.printf("T:");
            pc.printf("%f\n\r", analog2temp(TEMP_0_PIN.read()) ); 
            previous_millis_heater = t.read_ms(); 
          }
          manage_heater();
        }
        break;
 /* ***LWK*** disabled for now
      case 80: // M81 - ATX Power On
        if(PS_ON_PIN > -1) pinMode(PS_ON_PIN,OUTPUT); //GND
        break;
      case 81: // M81 - ATX Power Off
        if(PS_ON_PIN > -1) pinMode(PS_ON_PIN,INPUT); //Floating
        break;
 */
      case 82:
        relative_mode_e = false;
        break;
      case 83:
        relative_mode_e = true;
        break;
      case 84:
        disable_x();
        disable_y();
        disable_z();
        disable_e();
        break;
      case 85: // M85
        code_seen('S');
        max_inactive_time = code_value()*1000; 
        break;
      case 92: // M92
        if(code_seen('X')) x_steps_per_unit = code_value();
        if(code_seen('Y')) y_steps_per_unit = code_value();
        if(code_seen('Z')) z_steps_per_unit = code_value();
        if(code_seen('E')) e_steps_per_unit = code_value();
        break;
    }
    
  }
  
  ClearToSend();
}

inline void FlushSerialRequestResend()
{
  char cmdbuffer[100]="Resend:";
//  ltoa(gcode_LastN+1, cmdbuffer+7, 10);
// pc.flush();                              // ***LWK*** mbed has no .flush
  pc.printf("%s\n\r", &cmdbuffer);
  ClearToSend();
}

inline void ClearToSend()
{
  previous_millis_cmd = t.read_ms();
  pc.printf("ok\n\r"); 
}

inline void get_coordinates()
{
  if(code_seen('X')) destination_x = (float)code_value() + relative_mode*current_x;
  else destination_x = current_x;                                                       //Are these else lines really needed?
  if(code_seen('Y')) destination_y = (float)code_value() + relative_mode*current_y;
  else destination_y = current_y;
  if(code_seen('Z')) destination_z = (float)code_value() + relative_mode*current_z;
  else destination_z = current_z;
  if(code_seen('E')) destination_e = (float)code_value() + (relative_mode_e || relative_mode)*current_e;
  else destination_e = current_e;
  if(code_seen('F')) {
    next_feedrate = code_value();
    if(next_feedrate > 0.0) feedrate = next_feedrate;
  }
  
  //Find direction
  if(destination_x >= current_x) direction_x=1;
  else direction_x=0;
  if(destination_y >= current_y) direction_y=1;
  else direction_y=0;
  if(destination_z >= current_z) direction_z=1;
  else direction_z=0;
  if(destination_e >= current_e) direction_e=1;
  else direction_e=0;
  
  
  if (min_software_endstops) {
    if (destination_x < 0) destination_x = 0.0;
    if (destination_y < 0) destination_y = 0.0;
    if (destination_z < 0) destination_z = 0.0;
  }

  if (max_software_endstops) {
    if (destination_x > X_MAX_LENGTH) destination_x = X_MAX_LENGTH;
    if (destination_y > Y_MAX_LENGTH) destination_y = Y_MAX_LENGTH;
    if (destination_z > Z_MAX_LENGTH) destination_z = Z_MAX_LENGTH;
  }
  
  if(feedrate > max_feedrate) feedrate = max_feedrate;
}

void linear_move(unsigned long x_steps_remaining, unsigned long y_steps_remaining, unsigned long z_steps_remaining, unsigned long e_steps_remaining) // make linear move with preset speeds and destinations, see G0 and G1
{
  //Determine direction of movement
  if (destination_x > current_x) X_DIR_PIN.write(!INVERT_X_DIR);
  else X_DIR_PIN.write(INVERT_X_DIR);
  if (destination_y > current_y) Y_DIR_PIN.write(!INVERT_Y_DIR);
  else Y_DIR_PIN.write(INVERT_Y_DIR);
  if (destination_z > current_z) Z_DIR_PIN.write(!INVERT_Z_DIR);
  else Z_DIR_PIN.write(INVERT_Z_DIR);
  if (destination_e > current_e) E_DIR_PIN.write(!INVERT_E_DIR);
  else E_DIR_PIN.write(INVERT_E_DIR);
  
  //Only enable axis that are moving. If the axis doesn't need to move then it can stay disabled depending on configuration.
  if(x_steps_remaining) enable_x();
  if(y_steps_remaining) enable_y();
  if(z_steps_remaining) enable_z();
  if(e_steps_remaining) enable_e();

  if(X_MIN_PIN > -1) if(!direction_x) if(X_MIN_PIN != ENDSTOPS_INVERTING) x_steps_remaining=0;
  if(Y_MIN_PIN > -1) if(!direction_y) if(Y_MIN_PIN != ENDSTOPS_INVERTING) y_steps_remaining=0;
  if(Z_MIN_PIN > -1) if(!direction_z) if(Z_MIN_PIN != ENDSTOPS_INVERTING) z_steps_remaining=0;
  
  previous_millis_heater = t.read_ms();

  //while(x_steps_remaining > 0 || y_steps_remaining > 0 || z_steps_remaining > 0 || e_steps_remaining > 0) // move until no more steps remain
  while(x_steps_remaining + y_steps_remaining + z_steps_remaining + e_steps_remaining > 0) // move until no more steps remain
  { 
    if(x_steps_remaining) {
      if ((t.read_us()-previous_micros_x) >= x_interval) { do_x_step(); x_steps_remaining--; }
      if(X_MIN_PIN > -1) if(!direction_x) if(X_MIN_PIN != ENDSTOPS_INVERTING) x_steps_remaining=0;
    }
    
    if(y_steps_remaining) {
      if ((t.read_us()-previous_micros_y) >= y_interval) { do_y_step(); y_steps_remaining--; }
      if(Y_MIN_PIN > -1) if(!direction_y) if(Y_MIN_PIN != ENDSTOPS_INVERTING) y_steps_remaining=0;
    }
    
    if(z_steps_remaining) {
      if ((t.read_us()-previous_micros_z) >= z_interval) { do_z_step(); z_steps_remaining--; }
      if(Z_MIN_PIN > -1) if(!direction_z) if(Z_MIN_PIN != ENDSTOPS_INVERTING) z_steps_remaining=0;
    }    
    
    if(e_steps_remaining) if ((t.read_us()-previous_micros_e) >= e_interval) { do_e_step(); e_steps_remaining--; }
    
    if( (t.read_ms() - previous_millis_heater) >= 500 ) {
      manage_heater();
      previous_millis_heater = t.read_ms();
      
      manage_inactivity(2);
    }
  }
  
  if(DISABLE_X) disable_x();
  if(DISABLE_Y) disable_y();
  if(DISABLE_Z) disable_z();
  if(DISABLE_E) disable_e();
  
  // Update current position partly based on direction, we probably can combine this with the direction code above...
  if (destination_x > current_x) current_x = current_x + x_steps_to_take/x_steps_per_unit;
  else current_x = current_x - x_steps_to_take/x_steps_per_unit;
  if (destination_y > current_y) current_y = current_y + y_steps_to_take/y_steps_per_unit;
  else current_y = current_y - y_steps_to_take/y_steps_per_unit;
  if (destination_z > current_z) current_z = current_z + z_steps_to_take/z_steps_per_unit;
  else current_z = current_z - z_steps_to_take/z_steps_per_unit;
  if (destination_e > current_e) current_e = current_e + e_steps_to_take/e_steps_per_unit;
  else current_e = current_e - e_steps_to_take/e_steps_per_unit;
}


inline void do_x_step()
{
  X_STEP_PIN.write( HIGH);
  previous_micros_x = t.read_us();
  //delayMicroseconds(3);
  X_STEP_PIN.write( LOW);
}

inline void do_y_step()
{
  Y_STEP_PIN.write( HIGH);
  previous_micros_y = t.read_us();
  //delayMicroseconds(3);
  Y_STEP_PIN.write( LOW);
}

inline void do_z_step()
{
  Z_STEP_PIN.write( HIGH);
  previous_micros_z = t.read_us();
  //delayMicroseconds(3);
  Z_STEP_PIN.write( LOW);
}

inline void do_e_step()
{
  E_STEP_PIN.write( HIGH);
  previous_micros_e = t.read_us();
  //delayMicroseconds(3);
  E_STEP_PIN.write( LOW);
}

inline void disable_x() { if(X_ENABLE_PIN > -1) X_ENABLE_PIN.write(!X_ENABLE_ON); }
inline void disable_y() { if(Y_ENABLE_PIN > -1) Y_ENABLE_PIN.write(!Y_ENABLE_ON); }
inline void disable_z() { if(Z_ENABLE_PIN > -1) Z_ENABLE_PIN.write(!Z_ENABLE_ON); }
inline void disable_e() { if(E_ENABLE_PIN > -1) E_ENABLE_PIN.write(!E_ENABLE_ON); }
inline void  enable_x() { if(X_ENABLE_PIN > -1) X_ENABLE_PIN.write( X_ENABLE_ON); }
inline void  enable_y() { if(Y_ENABLE_PIN > -1) Y_ENABLE_PIN.write( Y_ENABLE_ON); }
inline void  enable_z() { if(Z_ENABLE_PIN > -1) Z_ENABLE_PIN.write( Z_ENABLE_ON); }
inline void  enable_e() { if(E_ENABLE_PIN > -1) E_ENABLE_PIN.write( E_ENABLE_ON); }

inline void manage_heater()
{
  current_raw = TEMP_0_PIN.read();                  // If using thermistor, when the heater is colder than targer temp, we get a higher analog reading than target, 
  if(USE_THERMISTOR) current_raw = 1023 - current_raw;   // this switches it up so that the reading appears lower than target for the control logic.
  
  if(current_raw >= target_raw) HEATER_0_PIN.write(LOW);
  else HEATER_0_PIN.write(HIGH);
}

// Takes temperature value as input and returns corresponding analog value from RepRap thermistor temp table.
// This is needed because PID in hydra firmware hovers around a given analog value, not a temp value.
// This function is derived from inversing the logic from a portion of getTemperature() in FiveD RepRap firmware.
float temp2analog(int celsius) {
  if(USE_THERMISTOR) {
    int raw = 0;
    char i;
    
    for (i=1; i<NUMTEMPS; i++)
    {
      if (temptable[i][1] < celsius)
      {
        raw = temptable[i-1][0] + 
          (celsius - temptable[i-1][1]) * 
          (temptable[i][0] - temptable[i-1][0]) /
          (temptable[i][1] - temptable[i-1][1]);
      
        break;
      }
    }

    // Overflow: Set to last value in the table
    if (i == NUMTEMPS) raw = temptable[i-1][0];

    return 1023 - raw;
  } else {
    return celsius * (1024.0/(5.0*100.0));
  }
}

// Derived from RepRap FiveD extruder::getTemperature()
float analog2temp(int raw) {
  if(USE_THERMISTOR) {
    int celsius = 0;
    char i;

    for (i=1; i<NUMTEMPS; i++)
    {
      if (temptable[i][0] > raw)
      {
        celsius  = temptable[i-1][1] + 
          (raw - temptable[i-1][0]) * 
          (temptable[i][1] - temptable[i-1][1]) /
          (temptable[i][0] - temptable[i-1][0]);

        break;
      }
    }

    // Overflow: Set to last value in the table
    if (i == NUMTEMPS) celsius = temptable[i-1][1];

    return celsius;
    
  } else {
    return raw * ((5.0*100.0)/1024.0);
  }
}

inline void kill(char debug)
{
  HEATER_0_PIN.write(LOW);
  
  disable_x();
  disable_y();
  disable_z();
  disable_e();

// ***LWK*** disabled  
//  if(PS_ON_PIN > -1) pinMode(PS_ON_PIN,INPUT);
  
  while(1)
  {
    if(debug == 1) pc.printf("Inactivity Shutdown, Last Line: ");
    if(debug == 2) pc.printf("Linear Move Abort, Last Line: ");
    pc.printf("%ld\n\r", &gcode_LastN);
    wait(5000); // 5 Second delay
  }
}

inline void manage_inactivity(char debug) { if( (t.read_ms()-previous_millis_cmd) >  max_inactive_time ) if(max_inactive_time) kill(debug); }
