# StateMachine Class

## Overview

The `StateMachine` module is used to control the states of the car, and behavior within those states. There are seven states the car can be in: `None`, `IDLE`, `RC`, `AUTO`, `ERROR`, `STOP`, `RESET`.

---

## Important Methods

There are four important methods outside of the constructor:
  1. [`setState(States newState)`](#set-state)
  2. [`defineState(States state, function<void(void)> func)`](#define-state)
  3. [`runState()`](#run-state)
  4. [`checkError()`](#check-error)

---

## Set State

The `setState()` method is used to set the current state of the system. It takes in a value of the `States` enum to set the system state. See [additions](#important-additions) to see the `States` enum values.

---

## Define State

The `defineState()` method is used to define the behavior of the given state. It takes in two parameters: a `States` enum and a lambda expression. The lamdba expression then becomes the behavior that the state runs. Every state must be defined with a lambda expression, so another method, `setStateDefaults()`, is used to define every state in the constructor with an empty lambda expression.

---

## Run State 

The `runState()` method is used to run the current state of the StateMachine instance. It's a really simple method, and is simply called once every iteration in loop.

---

## Check Error

The `checkError()` method is used as an error handler within the state machine. It returns true if the state machine encounters an error or false if it doesn't.

---

## Important Additions

In addition to the methods used to control the state machine, there is also an important enum used throughout the code. This enum allows for the setting and definition of states, and is called `States`.

The enum values and their corresponding states are as follows: 

| Values          | Uses                                               |
| :-------------: | :------------------------------------------------: |
| `States::NONE`  | State to run before any defined setting            |
| `States::IDLE`  | State to run when the car isn't running RC or Auto | 
| `States::RC`    | State to run for RC controls                       |
| `States::AUTO`  | State to run for auto packets and controls         |
| `States::ERROR` | State to run when an error occurs                  |
| `States::STOP`  | State to run as a safe stop when an error occurs   |
| `States::RESET` | State to run to reset various errors               |