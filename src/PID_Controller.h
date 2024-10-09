#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

class PID{
  private:
  float kp;
  float kd;
  float ki;
  float integral;
  float input;
  int previous_time;
  float dt;
  float error;

  public:
  PID();
  PID(float kp, float kd, float ki);
  float get_dt();
  float get_integral();
  void calculate_integral(float input);
  float calculate_derivative(float input);
  float get_error(float input, float target);
  float PID_Control(float input, float target);
};

#endif