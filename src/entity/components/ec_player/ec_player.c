#include "entity/components/ec_player/ec_player.h"
// Entity
#include "entity/entity.h"
// Human
#include "entity/components/ec_human/ec_human.h"
// Input
#include "input/input_manager.h"
// Island
#include "entity/components/island/island.h"
// V3
#include "utilities/math/v2.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// -------------------------
// Constants
// -------------------------
static LogConfig _logConfig = {"EC_Player", LOG_LEVEL_INFO, LOG_COLOR_BLUE};
static const int PLAYER_INPUT_FORWARD = 0;
static const int PLAYER_INPUT_LEFT = 1;
static const int PLAYER_INPUT_BACKWARD = 2;
static const int PLAYER_INPUT_RIGHT = 3;
static const int PLAYER_INPUT_MOUSE_LEFT = 0;
static const int PLAYER_INPUT_MOUSE_RIGHT = 1;

// -------------------------
// Entity Events
// -------------------------

static void EC_Player_Update(Component *component)
{
    EC_Player *ec_player = component->self;
    // Input
    V2 input_move = V2_ZERO;
    if (ec_player->inputContext->keys[PLAYER_INPUT_FORWARD].isDown)
    {
        input_move.y = 1.0f;
    }
    else if (ec_player->inputContext->keys[PLAYER_INPUT_BACKWARD].isDown)
    {
        input_move.y = -1.0f;
    }
    else
    {
        input_move.y = 0.0f;
    }
    if (ec_player->inputContext->keys[PLAYER_INPUT_LEFT].isDown)
    {
        input_move.x = -1.0f;
    }
    else if (ec_player->inputContext->keys[PLAYER_INPUT_RIGHT].isDown)
    {
        input_move.x = 1.0f;
    }
    else
    {
        input_move.x = 0.0f;
    }
    ec_player->human->creature->controller.input_move = V2_NORM(input_move);
    ;
}

// -------------------------
// Creation & Freeing
// -------------------------

static void EC_Player_Free(Component *component)
{
    EC_Player *ec_player = component->self;
    InputContext_Free(ec_player->inputContext);
    free(ec_player);
}

static EC_Player *EC_Player_Create(Entity *entity, EC_Human *human, InputContext *inputContext)
{
    EC_Player *ec_player = malloc(sizeof(EC_Player));
    ec_player->human = human;
    ec_player->inputContext = inputContext;
    // Component
    ec_player->component = Component_Create(ec_player, entity, EC_T_PLAYER, EC_Player_Free, NULL, NULL, NULL, EC_Player_Update, NULL);
    return ec_player;
}

EC_Player *EC_Player_Prefab(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale)
{
    Entity *entity = Entity_Create(parent, "Player", TS, position, rotation, scale);
    EC_Human *ec_human = EC_Human_Create(entity);
    int inputKeys[] = {
        GLFW_KEY_W,
        GLFW_KEY_A,
        GLFW_KEY_S,
        GLFW_KEY_D,
    };
    int inputButtons[] = {
        GLFW_MOUSE_BUTTON_LEFT,
        GLFW_MOUSE_BUTTON_RIGHT,
    };
    InputContext *inputContext = InputContext_Create("Player Input", 4, inputKeys, 2, inputButtons, 0, NULL);
    EC_Player *ec_player = EC_Player_Create(entity, ec_human, inputContext);
    return ec_player;
}
