#include "PidCommand.hpp"

/* -------------------- PidCommand Constructors -------------------- */

int PidCommand::commands = 1;


PidCommand::PidCommand(float *in, float *out, float *set, float (&outRange)[2], float (*func)(), float kP, float kI, float kD) {
  // Control variable pointers
  _input = in;
  _output = out;
  _setpoint = set;

  // Output range values 
  minOutput = outputRange[0] = outRange[0];
  maxOutput = outputRange[1] = outRange[1];

  // Timing function 
  setTimingFunction(func);

  // PID constants 
  _kP = kP;
  _kI = kI;
  _kD = kD;

  // PID Command ID
  commandID = commands;
  commands++;
}


PidCommand::PidCommand(float *in, float *out, float *set, float (*func)(), float kP, float kI, float kD) {
  // Control variable pointers
  _input = in;
  _output = out;
  _setpoint = set;

  // Output range values 
  minOutput = outputRange[0] = -100;
  maxOutput = outputRange[1] = 100;

  // Timing function 
  setTimingFunction(func);

  // PID constants 
  _kP = kP;
  _kI = kI;
  _kD = kD;

  // PID Command ID
  commandID = commands;
  commands++;
}

/* ----------------------------------------------------------------- */



/* ----------------------- PidCommand Methods ---------------------- */

void PidCommand::calculate() {
  if (!isStopped) {
    // Proportional term 
    error = *_setpoint - *_input;

    // Integral term 
    if (absVal(error) < kIntegrationLimit) {
      errorSum += error * deltaT;
    } else {
      errorSum = 0;
    }

    // Derivative term 
    errorRate = (error - lastError) / deltaT;

    // Sets values for feedback loop
    lastError = error;
    deltaT = timeFunc() - lastTimestamp;
    lastTimestamp = timeFunc();

    // Output
    *_output = constrainOutput(
      (_kP * error) + (_kI * errorSum) + (_kD * errorRate),
      outputRange
    );
  } else {
    *_output = 0;
  }

  
  if (consoleOutput && hasOutputMethod) {
    displayMethod();
  } else if (consoleOutput) {
    display();
  }
}


void PidCommand::setIntegrationLimit(float limit) {
  kIntegrationLimit = limit;
}


void PidCommand::eStop() {
  isStopped = true;
  *_output = 0;
}


float PidCommand::getError() {
  return error;
}


float PidCommand::getErrorSum() {
  return errorSum;
}


float PidCommand::getErrorRate() {
  return errorRate;
}


int PidCommand::getCommandID() {
  return commandID;
}


bool PidCommand::atSetpoint() {
  return errorRate <= finishedValue;
}


bool PidCommand::atSetpoint(float threshold) {
  return errorRate <= threshold;
}


void PidCommand::setFinishedValue(float value) {
  finishedValue = value;
}


void PidCommand::setTimingFunction(float (*func)()) {
  timeFunc = func;
} 


void PidCommand::sendConsoleOutput(bool sendOutput) {
  consoleOutput = sendOutput;
  hasOutputMethod = false;
}


void PidCommand::sendConsoleOutput(void (*displayFunc)(), bool sendOutput) {
  displayMethod = displayFunc;
  consoleOutput = sendOutput;

  hasOutputMethod = true;
}


void PidCommand::display() {
  // Prints the setpoint for comparison
  Serial.print(">Setpoint:");
  Serial.println(*_setpoint);

  // Prints the error
  Serial.print(">Error:");
  Serial.println(error * _kP);

  // Prints the error sum
  Serial.print(">Error Sum:");
  Serial.println(errorSum * _kI);

  // Prints the error rate
  Serial.print(">Error Rate:");
  Serial.println(errorRate * _kD);

  // Prints the current position
  Serial.print(">Current Position:");
  Serial.println(*_input);
}


template <class T>
inline T PidCommand::constrainOutput(T x, T min, T max) {
  return x >= min ? x <= max ? x : max : min;
}


template<class T>
inline T PidCommand::constrainOutput(T x, T (&range)[2]) {
  return constrainOutput(x, range[0], range[1]);
}


template<class T>
inline T PidCommand::absVal(T value) {
  return value < 0 ? (value * -1) : value;
}

/* ----------------------------------------------------------------- */