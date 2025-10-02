#include "entity/entity.h"
#include "entity/components/neural_network/ec_nn_neuron.h"
#include "neural_networks/neuron.h"
#include "entity/components/renderer/circle.h"
#include "entity/components/neural_network/ec_nn_neuron.h"

const float EC_NEURON_CIRCLE_RADIUS = 30.0;

static void EC_Neuron_Update(Component *component)
{
    EC_NN_Neuron *ec_neuron = component->self;
    printf("Neuron pos: %f %f\n", ec_neuron->component->entity->position.x, ec_neuron->component->entity->position.y);
}

EC_NN_Neuron *Prefab_Neuron(Entity *parent, V3 position, float rotation, V2 scale, V2 pivot, Neuron *neuron)
{
    Entity *E_prefab = Entity_Create(parent, "Neuron", position, rotation, scale, pivot);
    EC_Renderer *EC_renderer_circle = RD_Circle_CreateWithRenderer(E_prefab, EC_NEURON_CIRCLE_RADIUS);
    EC_NN_Neuron *EC_neuron = EC_Neuron_Create(E_prefab, neuron, EC_renderer_circle);
    return EC_neuron;
}

static void EC_Neuron_Free(Component *component)
{
    EC_NN_Neuron *ec_neuron = component->self;
    free(ec_neuron);
}

static void EC_Neuron_Awake(Component *component)
{
}

EC_NN_Neuron *EC_Neuron_Create(Entity *entity, Neuron *neuron, EC_Renderer *EC_renderer_circle)
{
    EC_NN_Neuron *EC_neuron = malloc(sizeof(EC_NN_Neuron));
    EC_neuron->neuron = neuron;
    EC_neuron->circleRenderer = EC_renderer_circle;
    // Component
    EC_neuron->component = Component_Create(EC_neuron, entity, EC_T_NN_NEURON, EC_Neuron_Free, EC_Neuron_Awake, NULL, NULL, NULL, NULL);
    return EC_neuron;
}