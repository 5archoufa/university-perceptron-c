#ifndef EC_STATE_MACHINE_H
#define EC_STATE_MACHINE_H

#include "entity/entity.h"
#include "logging/logger.h"
#include <stdlib.h>
#include <stdbool.h>

// ----------------------------------------
// Types 
// ----------------------------------------

// Forward declaration
typedef struct EC_StateMachine_State EC_StateMachine_State;

// Component structure that contains the state machine
typedef struct EC_StateMachine
{
    Component *component;
    char *name;
    // States
    size_t states_size;
    EC_StateMachine_State **states;
    EC_StateMachine_State *currentState;
} EC_StateMachine;

// State structure integrated into the entity component system
struct EC_StateMachine_State
{
    char *name;
    void (*Tick)(EC_StateMachine_State *state, struct EC_StateMachine *ec_sm);
    bool (*CanEnter)(EC_StateMachine_State *previousState, struct EC_StateMachine *ec_sm, int argCount, void **args);
    void (*OnEnter)(EC_StateMachine_State *previousState, struct EC_StateMachine *ec_sm, int argCount, void **args);
    bool (*CanExit)(EC_StateMachine_State *nextState, struct EC_StateMachine *ec_sm, int argCount, void **args);
    void (*OnExit)(EC_StateMachine_State *nextState, struct EC_StateMachine *ec_sm, int argCount, void **args);
};

// -------------------------
// Creation & Destruction
// -------------------------

EC_StateMachine *EC_StateMachine_Create(Entity *entity, char *stateMachineName);
void EC_StateMachine_Free(Component *component);

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
    void (*OnExit)(EC_StateMachine_State *nextState, EC_StateMachine *ec_sm, int argCount, void **args));

void EC_StateMachine_State_Free(EC_StateMachine_State *state);

bool EC_StateMachine_TrySetState(EC_StateMachine *ec_sm, EC_StateMachine_State *state, int argCount, void **args);

// -------------------------
// Lifecycle
// -------------------------

void EC_StateMachine_Update(Component *component);

#endif