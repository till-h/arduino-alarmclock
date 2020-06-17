#include "Events.h"
#include "ClockConfig.h"


// Event sources

// Define static class variables of ButtonSource (i.e. allocate some memory)
uint8_t ButtonSource::pin;
uint32_t ButtonSource::lastButtonChange;
_buttonChange ButtonSource::buttonChange;

ButtonSource::ButtonSource(uint8_t pin, uint32_t longPressThreshold)
{
    ButtonSource::pin = pin; // Use namespace to distinguish static member from argument
    longPressThreshold = longPressThreshold;
    buttonChange = none;
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pin), buttonCallback, CHANGE); // when button is released. TODO allow CHANGE
}

void ButtonSource::buttonCallback()
{
    lastButtonChange = micros();
    if (digitalRead(pin) == HIGH)
    {
        buttonChange = press;
    }
    else
    {
        buttonChange = release;
    }
}

// TODO use internal FSM to distinguish long / short button presses.
anEvent ButtonSource::poll()
{
    uint32_t now = micros();
    if (buttonChange == release)
    {
        // if ((now - lastButtonChange < longPressThreshold) && (buttonChange == pressed))
        // {
        //     unservedButtonChange = false;
        //     return (anEvent){buttonPress, lastButtonChange, 0};
        // }
        // else if (unservedButtonChange)
        // {
        //     unservedButtonChange = false;
        //     return (anEvent){longButtonPress, lastButtonChange, 0};
        // }
        // else
        // {
        //     return (anEvent){none, micros(), 0};
        // }
        buttonChange = none;
        return (anEvent){buttonPress, now, 0};
    }
}


RotationSource::RotationSource(uint8_t pin1, uint8_t pin2):
    dial(RotaryDial(pin1, pin2))
{}

anEvent RotationSource::poll()
{
    int32_t rot = dial.getRotation();
    uint32_t now = micros();
    if (rotation != 0)
    {
        return (anEvent){eventType::rotation, now, rot};
    }
    else
    {
        return (anEvent){noEvent, now, 0};
    }
}


TimerSource::TimerSource():
    delay(0),
    isActive(false),
    startTime(0)
{}

void TimerSource::start(uint32_t delay)
{
    isActive = true;
    startTime = micros();
    this->delay = delay;
}

void TimerSource::start()
{
    isActive = true;
    startTime = micros();
}

void TimerSource::cancel()
{
    isActive = false;
}

anEvent TimerSource::poll()
{
    uint32_t now = micros();
    if (isActive)
    {
        if (now - startTime > delay)
        {
            isActive = false;
            return (anEvent){timeout, now, 0};
        }
    }
    return (anEvent){noEvent, now, 0};
}


// Scheduler

Scheduler::Scheduler():

    butSrc(ButtonSource(PUSH, 3000000)), // TODO use 3E6 possible?
    rotSrc(RotationSource(ENC1, ENC2)),
    timSrc(TimerSource())
{}

void Scheduler::setup(FSM fsm, softwareState initialState)
{
    state = initialState;
    this->fsm = fsm;
}

void Scheduler::run()
{
    while (true)
    {
        // run churn function of current state
        runChurnFunc(state);

        // poll events
        anEvent event;
        event = butSrc.poll();
        if (event.type == noEvent)
        {
            event = rotSrc.poll();
        }
        if (event.type == noEvent)
        {
            event = timSrc.poll();
        }
        if (event.type != noEvent)
        {
            runTranFunc(state, event);
            state = getNewState(state, event);
            break; // break out of polling sources for the old state
        }
        
    }
}

softwareState Scheduler::getNewState(softwareState state, anEvent event)
{
    uint8_t stateIndex = getIndex(state);
    uint8_t tranIndex = getTranIndex(stateIndex, event.type);
    return fsm.state[stateIndex].tran[tranIndex].newState;
}

uint8_t Scheduler::getIndex(softwareState state)
{
    for (uint8_t i = 0; i < fsm.numStates; i++)
    {
        if (fsm.state[i].state == state)
        {
            return i;
        }   
    }
    return 0;
}

void Scheduler::runChurnFunc(softwareState state)
{
    fsm.state[getIndex(state)].churnFunc();
}

uint8_t Scheduler::getTranIndex(uint8_t stateIndex, eventType event)
{
    for (uint8_t i = 0; i < fsm.state[stateIndex].numTrans; i++)
    {
        if (fsm.state[stateIndex].tran[i].event == event)
        {
            return i;
        }
    }
    return 0;
}

void Scheduler::runTranFunc(softwareState state, anEvent event)
{
    uint8_t stateIndex = getIndex(state);
    uint8_t tranIndex = getTranIndex(stateIndex, event.type);
    fsm.state[stateIndex].tran[tranIndex].transitionFunc(event);
}