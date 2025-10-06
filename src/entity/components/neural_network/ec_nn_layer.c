#include "entity/components/neural_network/ec_nn_layer.h"
#include "entity/components/neural_network/ec_nn_neuron.h"
#include "neural_networks/layer.h"

static EC_NN_Layer *EC_Layer_Create(Entity *entity, Layer *layer, EC_NN_Neuron **neurons)
{
    EC_NN_Layer *EC_layer = malloc(sizeof(EC_NN_Layer));
    EC_layer->neurons_size = layer->neurons_size;
    EC_layer->neurons = neurons;
    // Component
    EC_layer->component = Component_Create(EC_layer, entity, EC_T_CAMERA, EC_Layer_Free, NULL, NULL, NULL, NULL, NULL);
    return EC_layer;
}

static float NeuronSpacing()
{
    return EC_NEURON_CIRCLE_RADIUS / 3.0;
}

EC_NN_Layer *Prefab_Layer(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, Layer *layer)
{
    // Entity
    Entity *E_layer = Entity_Create(parent, "Layer", TS, position, rotation, scale);
    // Neurons
    EC_NN_Neuron **EC_neurons = malloc(sizeof(EC_NN_Neuron *) * layer->neurons_size);
    float neuronRadius = EC_NEURON_CIRCLE_RADIUS, neuronSpacing = NeuronSpacing();
    V3 neuronPos = {0.0, position.y - EC_Layer_GetHeight(layer->neurons_size) * 0.5, 0.0};
    for (int i = 0; i < layer->neurons_size; i++)
    {
        EC_NN_Neuron *EC_neuron = Prefab_Neuron(E_layer, TS_LOCAL, neuronPos, QUATERNION_IDENTITY, V3_ONE, &layer->neurons[i]);
        neuronPos.y += neuronRadius * 2 + neuronSpacing;
        EC_neurons[i] = EC_neuron;
    }
    // EC_Layer
    EC_NN_Layer *EC_layer = EC_Layer_Create(E_layer, layer, EC_neurons);
    return EC_layer;
}

float EC_Layer_GetHeight(size_t neurons_count)
{
    float neuronRadius = EC_NEURON_CIRCLE_RADIUS;
    float neuronSpacing = NeuronSpacing();
    return neurons_count * (neuronRadius * 2 + neuronSpacing) - neuronSpacing;
}

void EC_Layer_Free(Component *component)
{
    EC_NN_Layer *EC_layer = component->self;
    free(EC_layer->neurons);
    free(EC_layer);
}