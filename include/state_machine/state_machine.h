#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "logging/logger.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct StateMachine_State StateMachine_State;

struct StateMachine_State
{
    char *name;
    void (*Tick)();
    bool (*CanEnter)(StateMachine_State *previousState, int argCount, void **args);
    void (*OnEnter)(StateMachine_State *previousState, int argCount, void **args);
    bool (*CanExit)(StateMachine_State *nextState, int argCount, void **args);
    void (*OnExit)(StateMachine_State *nextState, int argCount, void **args);
};

typedef struct StateMachine
{
    char* name;
    // State
    StateMachine_State *state;
    // Logging
    LogConfig _logConfig;
} StateMachine;

StateMachine *StateMachine_Create(char *name);

void StateMachine_Free(StateMachine *stateMachine);

StateMachine_State *StateMachine_State_Create(
    char *name,
    void (*Tick)(),
    bool (*CanEnter)(StateMachine_State *previousState, int argCount, void **args),
    void (*OnEnter)(StateMachine_State *previousState, int argCount, void **args),
    bool (*CanExit)(StateMachine_State *nextState, int argCount, void **args),
    void (*OnExit)(StateMachine_State *nextState, int argCount, void **args));

void StateMachine_State_Free(StateMachine_State *state);
void StateMachine_Tick(StateMachine* stateMachine);
bool StateMachine_TrySetState(StateMachine* stateMachine, StateMachine_State *state, int argCount, void **args);

#endif