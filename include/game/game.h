#ifndef GAME_H
#define GAME_H

// Physics
#include "physics/physics-manager.h"
// Input
#include "input/input_manager.h"
// State Machine
#include "entity/components/ec_state_machine/ec_state_machine.h"

// Define a callback that triggers when application quits
typedef void (*GameQuitCallback)(void);

// ----------------------------------------
// Inputs
// ----------------------------------------

extern InputContext *INPUT_CONTEXT_GAMEPLAY;
extern InputContext *INPUT_CONTEXT_TRIGLE;
extern EC_StateMachine *SM_GAME;
extern EC_StateMachine_State *SM_STATE_GAMEPLAY;
extern EC_StateMachine_State *SM_STATE_TRIGLE;

// -------------------------
// Entity Types
// -------------------------

void Game_Awake();
void Game_Start();
void Game_Update();
void Game_LateUpdate();
void Game_FixedUpdate();
void Game_Free();
void Game_EndOfFrame();
void Game_SubscribeOnQuit(GameQuitCallback callback);
void Game_UnsubscribeOnQuit(GameQuitCallback callback);

#endif