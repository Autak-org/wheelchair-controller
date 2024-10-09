#include "PID_Controller.h"
#include "Arduino.h"

PID::PID(){
  this->kp = 0;
  this->kd  = 0;
  this->ki = 0;
  this->integral = 0;
  this->error = 0;
  this->previous_time = millis();
  this->dt = 1.0;
}

PID::PID(float kp, float kd, float ki){
  this->kp = kp;
  this->kd  = kd;
  this->ki = ki;
  this->integral = 0;
  this->error = 0;
  this->previous_time = millis();
  this->dt = 1.0;
};

float PID::get_dt(){
  int current_time = millis();
  float dt = (current_time - this->previous_time) / 1000.0;
  this->previous_time = current_time;
  this->dt = dt;
  return dt;
}

float PID::get_integral(){
  return this->integral;
}

float PID::calculate_derivative(float error){
  float dt = this->dt;
  float delta_error = error - this->error;
  this->error = error;
  return delta_error / dt; 
};

void PID::calculate_integral(float error){
  float dt = this->dt;
  float previous_integral = this->integral;
  this->integral += error * dt;
  if(this->integral > 100) this->integral = previous_integral;
};

float PID::get_error(float input, float target){
  return target - input;
};

float PID::PID_Control(float input, float target){
  float error = get_error(input, target);
  get_dt();
  float derivative = calculate_derivative(error);
  calculate_integral(error);
  float integral = this->integral;

  return this->kp * error + this->kd * derivative + this->ki * integral;
};