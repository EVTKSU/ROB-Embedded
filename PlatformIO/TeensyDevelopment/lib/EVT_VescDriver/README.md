# VESC Driver Class

## Overview
The VescDriver module is used to handle the UART communication and control logic between the teensy and the VESC motor controller. The intention is to use it to be able to control the motors on a kart with RC and Automatic controls recieved by the Teens. It is to be used as a safe  interface between the RC transmitter and autonumous inputs and the VESC power outputs: the ERPM and Brakeing currents.
it also sets up for error handling.

---

## Important Methods

There are three methods included in the `VescDriver` driver that ennables users to control the VESC FlipSky 75100 motor controller: 
  1. [`updateRC(uint16_t throttleChannel, uint16_t brakeChannel)`](#update)
  2. [`updateAuto(float erpm, float brake)`](#update)
  3. [`errorToString(mc_fault_code VESC_ERR)`](#error-handling)

--- 

## Update

There are two update methods: `updateRC(uint16_t throttleChannel, uint16_t brakeChannel)` and `updateAuto(float erpm, float brake)`. 

The `updateRC(uint16_t throttleChannel, uint16_t brakeChannel)` method takes input from an RC reciever, maps the values, and changes the values sent to the VESC. Forward throttle commands positive ERPM. Reverse throttle commands negative motor current so it can drive backward from rest. The separate brake channel commands VESC braking current and overrides throttle while active.

The `updateAuto(float erpm, float brake)` is meant to be used when the teensy is recieving commands from Automode (or self driving). This method recieves brake current and ERPM values directly from the Auto controls and pushes them to the VESC motor controller. This method also limits the values to where brake current and ERPM cannot be set at the same time.

## Error-handling

There is one error handling method: `errorToString(mc_fault_code VESC_ERR)`

The `errorToString(mc_fault_code VESC_ERR)` method reads VESC fault codes and returns a string indicating which fault, if any, is currently occuring in the VESC motor controller.
