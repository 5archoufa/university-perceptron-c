#include "entity/components/ec_creature/ec_creature.h"
// Perceptron
#include "perceptron.h"
// Entity
#include "entity/entity.h"
#include "entity/transform.h"

// ------------------------- 
// Entity Events 
// -------------------------

static void EC_Creature_FixedUpdate(Component *component)
{
    EC_Creature *ec_creature = component->self;
    // Movement
    V3 forward = T_Forward(ec_creature->transform);    
    V3 right = T_Right(ec_creature->transform);
    V3 moveDir = V3_ADD(V3_SCALE(WORLD_FORWARD, ec_creature->controller.input_move.y * FixedDeltaTime),
                        V3_SCALE(WORLD_RIGHT, ec_creature->controller.input_move.x * FixedDeltaTime));
    T_LPos_Add(ec_creature->transform, moveDir);
}

// ------------------------- 
// Creation & Freeing 
// -------------------------

static void EC_Creature_Free(Component *component)
{
    EC_Creature *ec_creature = component->self;
    free(ec_creature);
}

EC_Creature *EC_Creature_Create(Entity *entity, CreatureType type, EC_Renderer3D *ec_renderer3d, V2 movementSpeed, V2 lookSpeed)
{
    EC_Creature *ec_creature = malloc(sizeof(EC_Creature));
    // Type
    ec_creature->type = type;
    // References
    ec_creature->ec_renderer3d = ec_renderer3d;
    ec_creature->transform = &entity->transform;
    // Controller
    ec_creature->controller.input_move = V2_ZERO;
    ec_creature->controller.input_look = V2_ZERO;
    ec_creature->controller.movementSpeed = movementSpeed;
    ec_creature->controller.lookSpeed = lookSpeed;
    ec_creature->controller.ec_creature = ec_creature;
    // Component
    ec_creature->component = Component_Create(ec_creature, entity, EC_T_CREATURE, EC_Creature_Free, NULL, NULL, NULL, NULL, EC_Creature_FixedUpdate);
    return ec_creature;
}