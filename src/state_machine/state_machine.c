#include "state_machine/state_machine.h"

static LogConfig _logConfig = {"StateMachine", LOG_LEVEL_INFO, LOG_COLOR_ORANGE};

StateMachine *StateMachine_Create(char *name)
{
    LogCreate(&_logConfig, name);
    StateMachine *stateMachine = malloc(sizeof(StateMachine));
    stateMachine->name = name;
    stateMachine->state = NULL;
    stateMachine->_logConfig.name = name;
    stateMachine->_logConfig.logLevel = LOG_LEVEL_INFO;
    stateMachine->_logConfig.color = LOG_COLOR_DEFAULT;
    return stateMachine;
}

void StateMachine_Free(StateMachine *stateMachine)
{
    free(stateMachine);
}

StateMachine_State *StateMachine_State_Create(
    char *name,
    void (*Tick)(),
    bool (*CanEnter)(StateMachine_State *previousState, int argCount, void **args),
    void (*OnEnter)(StateMachine_State *previousState, int argCount, void **args),
    bool (*CanExit)(StateMachine_State *nextState, int argCount, void **args),
    void (*OnExit)(StateMachine_State *nextState, int argCount, void **args))
{
    StateMachine_State *state = malloc(sizeof(StateMachine_State));
    state->name = name;
    state->CanEnter = CanEnter;
    state->OnEnter = OnEnter;
    state->OnExit = OnExit;
    return state;
}

void StateMachine_State_Free(StateMachine_State *state)
{
    free(state);
}

bool StateMachine_TrySetState(StateMachine *stateMachine, StateMachine_State *state, int argCount, void **args)
{
    if (stateMachine->state != NULL)
    {
        if (stateMachine->state->CanExit != NULL && !stateMachine->state->CanExit(state, argCount, args))
        {
            Log(&stateMachine->_logConfig, "COULDN'T TRANSITION FROM STATE<%s> TO STATE<%s> GIVEN %d args. (CanExit -> False)", stateMachine->state->name, state->name, argCount);
            return false;
        }
    }
    if (state->CanEnter != NULL && !state->CanEnter(stateMachine->state, argCount, args))
    {
        Log(&stateMachine->_logConfig, "COULDN'T TRANSITION FROM STATE<%s> TO STATE<%s> GIVEN %d args. (CanEnter -> False)", stateMachine->state == NULL ? "NULL" : stateMachine->state->name, state->name, argCount);
        return false;
    }
    char *previousName = stateMachine->state == NULL ? "<NULL>" : stateMachine->state->name;
    Log(&stateMachine->_logConfig, "Transition from State<%s> to State<%s>", previousName, state->name);
    if (stateMachine->state != NULL)
    {
        stateMachine->state->OnExit(state, argCount, args);
    }
    stateMachine->state = state;
    stateMachine->state->OnEnter(state, argCount, args);
}