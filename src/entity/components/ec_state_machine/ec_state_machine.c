#include "entity/components/ec_state_machine/ec_state_machine.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static LogConfig _logConfig = {"EC_StateMachine", LOG_LEVEL_INFO, LOG_COLOR_ORANGE};

// -------------------------
// Creation & Destruction
// -------------------------

EC_StateMachine *EC_StateMachine_Create(Entity *entity, char *stateMachineName)
{
    EC_StateMachine *ec_sm = malloc(sizeof(EC_StateMachine));
    // Name
    ec_sm->name = malloc(strlen(stateMachineName) + 1);
    strcpy(ec_sm->name, stateMachineName);
    ec_sm->currentState = NULL;
    // States
    ec_sm->states_size = 0;
    ec_sm->states = NULL;
    // Component
    ec_sm->component = Component_Create(
        ec_sm,
        entity,
        EC_T_STATE_MACHINE,
        EC_StateMachine_Free,
        NULL,
        NULL,
        EC_StateMachine_Update,
        NULL,
        NULL);

    LogCreate(&_logConfig, stateMachineName);
    return ec_sm;
}

void EC_StateMachine_Free(Component *component)
{
    EC_StateMachine *ec_sm = (EC_StateMachine *)component->self;
    if (ec_sm != NULL)
    {
        free(ec_sm->name);
        for (size_t i = 0; i < ec_sm->states_size; i++)
        {
            EC_StateMachine_State_Free(ec_sm->states[i]);
        }
        free(ec_sm->states);
        free(ec_sm);
    }
}

// -------------------------
// State Creation & Management
// -------------------------

EC_StateMachine_State *EC_StateMachine_State_Create(
    EC_StateMachine *stateMachine,
    char *name,
    void (*Tick)(EC_StateMachine_State *state, EC_StateMachine *ec_sm),
    bool (*CanEnter)(EC_StateMachine_State *previousState, EC_StateMachine *ec_sm, int argCount, void **args),
    void (*OnEnter)(EC_StateMachine_State *previousState, EC_StateMachine *ec_sm, int argCount, void **args),
    bool (*CanExit)(EC_StateMachine_State *nextState, EC_StateMachine *ec_sm, int argCount, void **args),
    void (*OnExit)(EC_StateMachine_State *nextState, EC_StateMachine *ec_sm, int argCount, void **args))
{
    EC_StateMachine_State *state = malloc(sizeof(EC_StateMachine_State));
    // ============ State ============ //
    state->name = malloc(strlen(name) + 1);
    strcpy(state->name, name);
    state->Tick = Tick;
    state->CanEnter = CanEnter;
    state->OnEnter = OnEnter;
    state->CanExit = CanExit;
    state->OnExit = OnExit;
    // ============ State Machine ============ //
    stateMachine->states_size++;
    stateMachine->states = realloc(stateMachine->states, sizeof(EC_StateMachine_State *) * stateMachine->states_size);
    stateMachine->states[stateMachine->states_size - 1] = state;
    return state;
}

void EC_StateMachine_State_Free(EC_StateMachine_State *state)
{
    if (state != NULL)
    {
        free(state->name);
        free(state);
    }
}

// -------------------------
// State Management
// -------------------------

bool EC_StateMachine_TrySetState(EC_StateMachine *ec_sm, EC_StateMachine_State *state, int argCount, void **args)
{
    if (ec_sm == NULL || state == NULL)
    {
        return false;
    }

    // Check if current state can exit
    if (ec_sm->currentState != NULL)
    {
        if (ec_sm->currentState->CanExit != NULL && !ec_sm->currentState->CanExit(state, ec_sm, argCount, args))
        {
            Log(&_logConfig, "COULDN'T TRANSITION FROM STATE<%s> TO STATE<%s> GIVEN %d args. (CanExit -> False)",
                ec_sm->currentState->name, state->name, argCount);
            return false;
        }
    }

    // Check if new state can enter
    if (state->CanEnter != NULL && !state->CanEnter(ec_sm->currentState, ec_sm, argCount, args))
    {
        Log(&_logConfig, "[%s] COULDN'T TRANSITION FROM STATE<%s> TO STATE<%s> GIVEN %d args. (CanEnter -> False)",
            ec_sm->name, ec_sm->currentState == NULL ? "NULL" : ec_sm->currentState->name, state->name, argCount);
        return false;
    }

    char *previousName = ec_sm->currentState == NULL ? "<NULL>" : ec_sm->currentState->name;
    Log(&_logConfig, "[%s] Transition from State<%s> to State<%s>", ec_sm->name, previousName, state->name);

    // Exit current state
    if (ec_sm->currentState != NULL && ec_sm->currentState->OnExit != NULL)
    {
        ec_sm->currentState->OnExit(state, ec_sm, argCount, args);
    }

    // Set new state
    EC_StateMachine_State *previousState = ec_sm->currentState;
    ec_sm->currentState = state;

    // Enter new state
    if (ec_sm->currentState->OnEnter != NULL)
    {
        ec_sm->currentState->OnEnter(previousState, ec_sm, argCount, args);
    }

    return true;
}

EC_StateMachine_State *EC_StateMachine_GetCurrentState(EC_StateMachine *ec_sm)
{
    if (ec_sm == NULL)
    {
        return NULL;
    }

    return ec_sm->currentState;
}

// -------------------------
// Lifecycle
// -------------------------

void EC_StateMachine_Update(Component *component)
{
    EC_StateMachine *ec_sm = (EC_StateMachine *)component->self;

    if (ec_sm != NULL && ec_sm->currentState != NULL && ec_sm->currentState->Tick != NULL)
    {
        ec_sm->currentState->Tick(ec_sm->currentState, ec_sm);
    }
}