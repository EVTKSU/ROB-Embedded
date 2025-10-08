# PID Command Class

## Overview 

The `PidCommand` module is used for simple and automated control of a PID system. This allows for precise control of any value, though it is most usually used for position control due to current lack of a feedforward term.

---

## Important Methods 

There are five important methods, which include the constructor:
  1. [`PidCommand()`](#constructor)
  2. [`calculate()`](#calculate)
  3. [`atSetpoint`](#at-setpoint)
  4. [`setTimingFunction()`](#set-timing-function)
  5. [`sendConsoleOutput()`](#send-console-output)

There are also a few math methods are important. Mainly, a constraint function and an absolute value function. However, those don't need explanation 

---

## Constructor

The main constructor takes in a total of eight parameters, though two of them are optional. Those parameters are as follows: 
  1. `double * in`
      - A pointer to a double which holds the input sensor value
  2. `double * out`
      - A pointer to a double which holds the output of the PID command
  3. `double * set`
      - A pointer to a double which holds the setpoint value 
  4. `double (&outRange)[2]`
      - A reference to an array of the minimum and maximum output value
  5. `double (*func)()`
      - A pointer to a method which returns a double used to get the current time
  6. `double kP`
      - The proportional gain for the PID command 
  7. `double kI`
      - The integral gain for the PID command. One of two optional values which default to zero
  8. `double kD`
      - The derivative gain for the PID command. The other of two optional values which default to zero

Since the constructor uses pointers for all control values, the function to calculate the values can be a void method which only calculates and sets the values. This allows the program to directly access the sensor data without having a complicated call to a calculate method.

The fact mentioned above also allows the setpoint to be changed mid-program, which would allow for the creation of extremely simple path following algorithms.

---

## Calculate

The `calculate()` method runs through and does the math for the current value of the output. It uses a combination of the sensor data, a timer, and the PID gains to find the properly tuned output.

There are three functions which are summed to find the output, which are defined as follows:

$\text{Let } e(t) \text{ be the error at time } t \text{ and } k_p \text{, } k_i \text{, and } k_d \text{ be PID gain constants}$\
$\text{Given the proportional gain, the proportional term } P(t) \text{ is defined as}$

$$
\begin{equation}
  P(t) = k_p e(t)
\end{equation}
$$

$\text{Let } E \text{ be an acceptable value of error where the integral term can be applied}$\
$\text{Additionally, let } s(t) \text{ be the current setpoint value at time t}$\
$\text{Given those values and the integral gain, the integral term } I(t) \text{ can be defined as}$

```math
\begin{equation}
  I(t) =
    \left\{
      \begin{array}{lr}
        k_i \int _0^t e(\tau)d\tau, & |\,e(t)\,| < E \\
        0, & |\,e(t)\,| \ge E
      \end{array}
    \right.
 \end{equation}
```

$\text{Lastly, given the derivative gain, the derivative term } D(t) \text{ can be defined as}$

$$
\begin{equation}
  D(t) = k_d e'(t)
\end{equation}
$$

$\text{Let } u(t) \text{ be the output of the PID system}$\
$\text{The final output is the sum of each term, which can be defined as}$

$$
\begin{equation}
  u(t) = P(t) + I(t) + D(t)
\end{equation} 
$$

With those equations, it can be easily understood how to make a PID controller within code. As shown in the equations, if a PID gain constant is $0$, the term of the output equation is removed. This way, you can use various other control methods like PI, PD, or just proportional control.

---

## At Setpoint

The `atSetpoint()` method returns a boolean value denoting whether or not the error rate of the system is within an acceptable range. 

The error rate is used for this due to the fact that the PID system will have a very low error rate when reaching a steady-state error. So, when the error rate -- given by $\frac{D(t)}{k_d}$ -- reaches a given threshold, the `atSetpoint()` method returns `true`.

---

## Set Timing Function

The `setTimingFunction()` method takes in a pointer to a double method to use as the timing method for the PID system. 

This method should only be called to replace the timing method defined in the constructor. Additionally, for use in Arduino based systems, the timing method will usually be `millis()` unless there is some custom timing method being implemented.

---

## Send Console Output 

The `sendConsoleOutput()` method allows for control of whether or not output is displayed to a console and how it's displayed, in the case that it is. By default, calling the method sets the PID command to send console output using `display()` which makes a graph of key values. However, `sendConsoleOutput(void (*displayFunc)())` can be called to use a custom output method.

Additionally, a boolean can be passed as a parameter to either overload of the method to change the console output state to either `true` or `false`.
