
typedef unsigned int boolean;

#define false 0
#define true (!false)


void setup();
void loop();
inline void get_command();
inline float code_value();
inline long code_value_long();
inline bool code_seen(char code_string[]);
inline bool code_seen(char code);
inline void process_commands();
inline void FlushSerialRequestResend();
inline void ClearToSend();
inline void get_coordinates();
void linear_move(unsigned long x_steps_remaining, unsigned long y_steps_remaining, unsigned long z_steps_remaining, unsigned long e_steps_remaining);
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
void loop();
inline void get_command();
inline float code_value();
inline long code_value_long();
inline bool code_seen(char code_string[]);
inline bool code_seen(char code);
inline void process_commands();
inline void FlushSerialRequestResend();
inline void ClearToSend();
inline void get_coordinates();
void linear_move(unsigned long x_steps_remaining, unsigned long y_steps_remaining, unsigned long z_steps_remaining, unsigned long e_steps_remaining);
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
inline void manage_inactivity(char debug);