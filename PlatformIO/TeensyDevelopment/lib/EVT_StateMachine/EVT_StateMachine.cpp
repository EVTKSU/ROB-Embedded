#include <EVT_StateMachine.hpp>

namespace Signals {
  StateMachine::StateMachine(States initstate) {
    currentState = initstate;
    setStateDefaults();
  }


  void StateMachine::setState(States newState) {
    if (newState < numStates ) {
      currentState = newState;
    } else {
      Serial.printf("Invalid State: %d\n", newState);
      currentState = States::IDLE;
    }

    printState();
  }


  void StateMachine::runState() {
    stateFunctions[currentState]();
  }


  void StateMachine::defineState(States state, function<void(void)> func) {
    stateFunctions[state] = func;
  }


  void StateMachine::printState() {
    Serial.printf(
      "Current State: %s [%u]\n",
      toString(currentState),
      currentState
    );
  }


  void StateMachine::setStateDefaults() {
    for (States state : allStates) {
      defineState(state, [&] () {});
    }
  }


  States StateMachine::getState() {
    return currentState;
  }


  bool StateMachine::isInState(States checkState) {
    return (currentState == checkState);
  }


  const char * StateMachine::toString(States state) {
    return stateNames[state];
  }


  bool StateMachine::checkError() {
    // Add implementation later
    return false;
  }
}