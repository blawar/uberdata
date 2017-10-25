#ifndef IDLE_H
#define IDLE_H

#define LAST_DIRECTION_UNDEF 0
#define LAST_DIRECTION_CLOSE 1
#define LAST_DIRECTION_OPEN  2

#define PWMIDLE_RESET_PID         0x1
#define PWMIDLE_RESET_JUSTLIFTED  0x2
#define PWMIDLE_RESET_DONOTHING   0x4
#define PWMIDLE_RESET_JUSTCRANKED 0x8
#define PWMIDLE_RESET_CALCNEWTARG 0x10
#define PWMIDLE_RESET_DPADDED     0x20

class Idle
{
public:
Idle();
void voltage_compensation();
void ac_idleup();
void fan_ctl_idleup();
void ctl_init();
void test_mode();
void on_off();
void iac_warmup();
void pwm_warmup();
void closed_loop_throttlepressed(int rpm_thresh);
void closed_loop_throttlelifted(int rpm_thresh, int rpm_targ);
void closed_loop_pid(int targ_rpm, int rpm_thresh, char savelast);
int closed_loop_newtarg(int targ_rpm);
void closed_loop_calc_dp_decay();
void target_lookup();
void closed_loop();
void ctl();
int move_IACmotor();
private:
};

extern Idle idle;

#endif
