#include "entity/components/neural_network/ec_nn_link.h"
#include "entity/entity.h"
#include <stdint.h>

static const float EC_NN_LINK_LINE_WIDTH = 5.0;

static void EC_NN_Link_Free(Component* component){
    EC_NN_Link* ec_nn_link = component->self;
    free(ec_nn_link);
}

static EC_NN_Link* EC_NN_Link_Create(Entity* entity, EC_NN_Neuron* from, EC_NN_Neuron* to/*, EC_Renderer* renderer_line*/){
    EC_NN_Link* EC_NN_link = malloc(sizeof(EC_NN_Link));
    EC_NN_link->entity = entity;
    EC_NN_link->from = from;
    EC_NN_link->to = to;
    // Renderer
    //EC_NN_link->EC_renderer_line = renderer_line;
    // Component
    EC_NN_link->component = Component_Create(EC_NN_link, entity, EC_T_NN_LINK, EC_NN_Link_Free, NULL, NULL, NULL, NULL, NULL);
    return EC_NN_link;
}

EC_NN_Link* Prefab_NN_Link(Entity* parent, EC_NN_Neuron* from, EC_NN_Neuron* to, float weight){
    // Entity
    Entity* E_NN_link = Entity_Create(parent, false, "NN_Link", TS_WORLD, V3_ZERO, QUATERNION_IDENTITY, V3_ONE);
    // EC_NN_Link
    EC_NN_Link* ec_nn_link = EC_NN_Link_Create(E_NN_link, from, to/*, renderer_line*/);
    return ec_nn_link;
}