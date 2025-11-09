#include "entity/components/ec_creature/ec_creature.h"
// Perceptron
#include "perceptron.h"
// Entity
#include "entity/entity.h"
#include "entity/transform.h"
// Physics
#include "physics/physics-manager.h"

// -------------------------
// Entity Events
// -------------------------

static void EC_Creature_FixedUpdate(Component *component)
{
    EC_Creature *ec_creature = component->self;
    CreatureController *controller = &ec_creature->controller;
    // ============ Movement ============ //
    if (controller->input_move.x != 0 || controller->input_move.y != 0)
    {
        V3 forward = T_Forward(ec_creature->transform);
        V3 right = T_Right(ec_creature->transform);
        V3 moveDir = V3_ADD(V3_SCALE(forward, controller->input_move.y * FixedDeltaTime * controller->movementSpeed.y),
                            V3_SCALE(right, controller->input_move.x * FixedDeltaTime * controller->movementSpeed.x));
        T_LPos_Add(ec_creature->transform, moveDir);
    }
    // ============ Looking Around ============ //
    if (controller->input_look.x != 0 || controller->input_look.y != 0)
    {
        // Yaw (around world up)
        Quaternion yaw = Quat_FromAxisAngle((V3){0, 1, 0}, controller->input_look.x * controller->lookSpeed.x * FixedDeltaTime);
        Quaternion rot = T_WRot(ec_creature->transform);
        rot = Quat_Mul(yaw, ec_creature->transform->l_rot);
        rot = Quat_Norm(rot);
        T_LRot_Set(ec_creature->transform, rot);
    }
    // ============ Grounding ============ //
    V3 currentPos = T_WPos(ec_creature->transform);
    if (!V3_EQUALS(currentPos, ec_creature->previousPos))
    {
        EC_Island *ec_island = ec_creature->ec_island;
        currentPos = EC_Island_GetPositionOnLand(ec_island, currentPos);
        T_LPos_Set(ec_creature->transform, currentPos);
        ec_creature->previousPos = currentPos;
    }
}

// -------------------------
// Creation & Freeing
// -------------------------

static void EC_Creature_Free(Component *component)
{
    EC_Creature *ec_creature = component->self;
    free(ec_creature);
}

EC_Creature *EC_Creature_Create(EC_Island *ec_island, Entity *entity, CreatureType type, EC_MeshRenderer *ec_meshRenderer, V2 movementSpeed, V2 lookSpeed)
{
    EC_Creature *ec_creature = malloc(sizeof(EC_Creature));
    // Type
    ec_creature->type = type;
    // References
    ec_creature->ec_meshRenderer = ec_meshRenderer;
    ec_creature->transform = &entity->transform;
    ec_creature->ec_island = ec_island;
    // Controller
    ec_creature->controller.input_move = V2_ZERO;
    ec_creature->controller.input_look = V2_ZERO;
    ec_creature->controller.movementSpeed = movementSpeed;
    ec_creature->controller.lookSpeed = lookSpeed;
    ec_creature->controller.ec_creature = ec_creature;
    // Cache
    ec_creature->previousPos = (V3){0, 0, 0};
    // Component
    ec_creature->component = Component_Create(ec_creature, entity, EC_T_CREATURE, EC_Creature_Free, NULL, NULL, NULL, NULL, EC_Creature_FixedUpdate);
    return ec_creature;
}