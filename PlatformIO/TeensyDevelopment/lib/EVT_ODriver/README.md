# ODriver Class

## Overview 

The `ODriver` module is used for control over and setup of an ODrive motor controller

---

## Important Methods 

There are five main important methods which are widely used outside of the ODriver class itself 
  1. [`update()`](#update)
  2. [`initCalibration()`](#initial-calibration)
  3. [`getActiveErrors()`](#get-active-errors)
  4. [`sendCommand(String cmd)`](#send-command)
  5. [`setup()`](#setup)

---

## Update 

There are two update methods, `updateRC()` and `updateAuto(float steering)`. The `updateRC()` method blinks the builtin LED until the system has been calibrated. After calibration has occurred, if the reset switch is pulled, the system will have to recalibrate. Once any extra calibration has finished, the rest of the method will the map current position target, set a 5% deadband, and send position commands with velocity limit. The `updateAuto(float steering)` method takes in a float as a parameter called `steering`, which is received from the Latte Panda, that will send the target steering position as a command over UART to the ODrive.

---

## Initial Calibration 

The `initCalibration()` method performs the inital calibration of the ODrive motor controller. The `configureTrapTrajLimits()` method is then called to configure the trapezoidal velocity, accecleration, and deceleration limits. Velocity limit and gain values are then sent and configured in the ODrive.

---

## Get Active Errors

The `getActiveErrors()` method recieves an error in the form of an enum from the ODrive and then returns said error.

---

## Send Command

The `sendCommand(String cmd)` method takes in an ASCII Protocal command as a string parameter called `cmd`, which will send said command over UART to the ODrive.

---

## Setup

The `setup()` method checks if the ODrive serial communication is initialized. This method returns a boolean with respect to whether the ODrive serial communication has been defined or not.