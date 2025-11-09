#include "entity/components/ec_player/ec_player.h"
// Perceptron
#include "perceptron.h"
#include "game/game.h"
// Entity
#include "entity/entity.h"
// Transform
#include "entity/transform.h"
// Human
#include "entity/components/ec_human/ec_human.h"
// Input
#include "input/input_manager.h"
// Island
#include "entity/components/island/island.h"
// V3
#include "utilities/math/v2.h"
#include "utilities/math/v3.h"
#include "utilities/math/stupid_math.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// -------------------------
// Constants
// -------------------------
static LogConfig _logConfig = {"EC_Player", LOG_LEVEL_INFO, LOG_COLOR_BLUE};
static KeyState *PLAYER_INPUT_FORWARD = NULL;
static KeyState *PLAYER_INPUT_LEFT = NULL;
static KeyState *PLAYER_INPUT_BACKWARD = NULL;
static KeyState *PLAYER_INPUT_RIGHT = NULL;
static KeyState *PLAYER_INPUT_LOOK_LEFT = NULL;
static KeyState *PLAYER_INPUT_LOOK_RIGHT = NULL;

// -------------------------
// Entity Events
// -------------------------

static void EC_Player_Update(Component *component)
{
    EC_Player *ec_player = component->self;
    // ============ Creature Input ============ // 
    V2 input_move = {
        PLAYER_INPUT_RIGHT->isDown - PLAYER_INPUT_LEFT->isDown,
        PLAYER_INPUT_FORWARD->isDown - PLAYER_INPUT_BACKWARD->isDown,
    };
    V2 input_look = {
        PLAYER_INPUT_LOOK_RIGHT->isDown - PLAYER_INPUT_LOOK_LEFT->isDown,
        0
    };

    ec_player->human->creature->controller.input_move = V2_NORM(input_move);
    ec_player->human->creature->controller.input_look = V2_NORM(input_look);
}

// -------------------------
// Creation & Freeing
// -------------------------

static void EC_Player_Free(Component *component)
{
    EC_Player *ec_player = component->self;
    // Note: InputListener is owned by the InputContext and will be freed when context is freed
    // Do not call InputListener_Free here to avoid double-free
    free(ec_player);
}

static EC_Player *EC_Player_Create(Entity *entity, EC_Human *human, InputListener *inputListener)
{
    EC_Player *ec_player = malloc(sizeof(EC_Player));
    ec_player->human = human;
    ec_player->inputListener = inputListener;
    // Component
    ec_player->component = Component_Create(ec_player, entity, EC_T_PLAYER, EC_Player_Free, NULL, NULL, NULL, EC_Player_Update, NULL);
    return ec_player;
}

EC_Player *EC_Player_Prefab(EC_Island *ec_island, Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale)
{
    Entity *e_player = Entity_Create(parent, false, "Player", TS, position, rotation, scale);
    // ============ Human ============ // 
    EC_Human *ec_human = EC_Human_Create(ec_island, e_player);
    // ============ Input ============ // 
    InputListener *inputListener = InputContext_AddListener(INPUT_CONTEXT_GAMEPLAY, "Player Input");
    InputMapping *mapping = InputListener_AddMapping(inputListener, INPUT_CONTEXT_GAMEPLAY->id);
    PLAYER_INPUT_FORWARD = InputMapping_AddKey(mapping, GLFW_KEY_W);
    PLAYER_INPUT_LEFT = InputMapping_AddKey(mapping, GLFW_KEY_A);
    PLAYER_INPUT_BACKWARD = InputMapping_AddKey(mapping, GLFW_KEY_S);
    PLAYER_INPUT_RIGHT = InputMapping_AddKey(mapping, GLFW_KEY_D);
    PLAYER_INPUT_LOOK_LEFT = InputMapping_AddKey(mapping, GLFW_KEY_KP_4);
    PLAYER_INPUT_LOOK_RIGHT = InputMapping_AddKey(mapping, GLFW_KEY_KP_6);
    // ============ Player ============ // 
    EC_Player *ec_player = EC_Player_Create(e_player, ec_human, inputListener);
    // ============ Layer ============ //
    Entity_SetLayer(e_player, E_LAYER_CREATURE, true);
    return ec_player;
}
