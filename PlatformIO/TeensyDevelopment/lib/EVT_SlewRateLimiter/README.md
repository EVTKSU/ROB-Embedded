# SlewRateLimiter Class 

## Overview

The `SlewRateLimiter` module is used to create a code-side limiter for the amount a variable is allowed to change per second. This allows, in practice, for control over things like acceleration or velocity.

---

## Important Methods

There are two important methods outside of the constructor:
  1. [`calculate(double targetValue)`](#calculate)
  2. [`setRate(double rate)`](#set-rate)

In the most basic sense, they calculate the new value and set the maximum change rates, respectively.

---

## Calculate

The calculate method works by finding the maximum allowed values in either the positive or negative direction and constraining the change in the value between them.

Effectively, this is defined by the following:

$\text{Let f(t) be a function of time giving the current value}$
$\text{The change in value } \frac{df}{dt} \text{ can be defined as a piecewise funciton}$
$\text{With a constant for time } t_0 \text{ we can define a function of time change}$

$\text{Using two constants -- } \Delta f_{neg} \text{ and } \Delta f_{pos} \text{ -- we can define } \frac{df}{dt} \text{ as}$

```math
\begin{equation}
  \frac{df}{dt} = 
    \left\{
      \begin{array}{lr}
        \Delta f_{neg}, & f(t + t_0) < \Delta f_{neg} \\
        f(t + t_0), & \Delta f_{neg} < f(t + t_0) < \Delta f_{pos} \\
        \Delta f_{pos}, & f(t + t_0) > \Delta f_{pos}
      \end{array}
    \right.
 \end{equation}
```

Using $\text{Equation 1}$, we can calculate the allowed change, $\frac{df}{dt}$, since the previous iteration 

---

## Set Rate

The set rate method is much more simple than `calculate()`. It allows you to set the allowed amount of value change for the `SlewRateLimiter` as a whole. Within the methods, there are two ways to do this. Using the same notation as [Equation 1](#calculate), those two ways are as follows:
  1. `setRate(double rate)`
      - $\Delta f_{pos} = \Delta f_{neg} = rate$
  2. `setRate(double pos, double neg)`: 
      - $\Delta f_{pos} = pos$
      - $\Delta f_{neg} = neg$

With this, you can control either an absolute rate of change, or a different rate of change in either direction 

---
