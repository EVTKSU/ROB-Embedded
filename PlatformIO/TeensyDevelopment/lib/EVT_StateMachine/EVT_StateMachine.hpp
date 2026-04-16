#ifndef STATE_MACHINE
#define STATE_MACHINE

/*-----------------------------------------------------------------------------*/
/** 
 * @file   EVT_StateMachine.hpp
 * @brief  Header for StateMachine class
 * 
 * The StateMachine class is used for control of a system with predefined states.
 * Through its use, multiple states can be defined an run easily. 
 * 
 * @author Nyx Turbeville
 * @date   April 13, 2026
*//*---------------------------------------------------------------------------*/

#include <Arduino.h>
#include <functional>

using std::function;

/**
 * @brief Namespace used for communications drivers 
 */
namespace Signals {
  /**
   * @brief Enum used to denote current state of the system 
   */
  enum States : uint8_t {
    NONE = 0,
    IDLE,
    RC,
    AUTO,
    ERROR,
    STOP,
    RESET
  };  


  /**
   * @brief Class used for definition and control of a state machine
   */
  class StateMachine {
    private:
      States currentState; // Current system state 

      static const int numStates = 7; // Number of states 
      const char * stateNames[numStates] = { // State names as a string
        "NONE",
        "IDLE",
        "RC",
        "AUTO",
        "ERROR",
        "STOP",
        "RESET"
      };

      States allStates[numStates] { // Array of all states
        States::NONE,
        States::IDLE,
        States::RC,
        States::AUTO,
        States::ERROR,
        States::STOP,
        States::RESET
      };

      function<void(void)> stateFunctions[numStates]; // Void functions defined for each state 

    public:
      /**
       * @brief Defines a new instance of state machine 
       * 
       * @note While it is possible to have an inital state, 
       *       it is advised for the system to start in the NONE state
       * 
       * @param initState Initial state for the state machine to be defined with (Default NONE)
       */
      StateMachine(States initState = States::NONE);


      /**
       * @brief Sets the current state 
       * 
       * @param newState New state to run
       */
      void setState(States newState);


      /**
       * @brief Runs the current state 
       */
      void runState();


      /**
       * @brief Defines a state behavior given the state and a void function  
       * 
       * @param state State to set the behavior of 
       * @param func Function to run for the given state
       */
      void defineState(States state, function<void(void)> func);


      /**
       * @brief Prints the current state to the Serial monitor 
       */
      void printState();


      /**
       * @brief Sets all states to do nothing
       * 
       * @note If the teensy has an undefined state, it will throw an error 
       *       and exit loop. Because of this, defining all states to at least
       *       have something to run is absolutely necessary
       */
      void setStateDefaults();


      /**
       * @brief Gets the current state as an enum 
       * 
       * @return The current state as a States enum 
       */
      States getState();


      /**
       * @brief Checks if the state machine is in a given state 
       * 
       * @param checkState State to check for 
       * @return Whether or not the state machine is currently running the checked state 
       */
      bool isInState(States checkState);


      /**
       * @brief Gets the given state as a string 
       * 
       * @param state State to get the value of as a string
       * @return The name of the given state as a string
       */
      const char * toString(States state);


      /**
       * @brief Checks for an error condition 
       * 
       * @return Condition for whether or not to throw the system into error state
       */
      bool checkError();
  };
}

#endif // STATE_MACHINE