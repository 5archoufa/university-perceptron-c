#include "entity/components/neural_network/ec_nn.h"
#include "entity/components/neural_network/ec_nn_link.h"
#include "entity/components/neural_network/ec_nn_neuron.h"

static EC_NN *EC_NeuralNetwork_Create(Entity *entity, size_t layers_size, EC_NN_Layer **layers)
{
    EC_NN *ec_nn = malloc(sizeof(EC_NN));
    ec_nn->layers_size = layers_size;
    ec_nn->EC_layers = layers;
    // Component
    Component *component = Component_Create(ec_nn, entity, EC_T_NN, EC_NeuralNetwork_Free, NULL, NULL, NULL, NULL, NULL);
    ec_nn->component = component;
    return ec_nn;
}

void EC_NeuralNetwork_Free(Component *component)
{
    EC_NN *ec_nn = component->self;
    free(ec_nn->EC_layers);
    free(ec_nn);
}

EC_NN *Prefab_NeuralNetwork(Entity *parent, V3 position, float rotation, V2 scale, V2 pivot, NeuralNetwork *neuralNetwork)
{
    Entity *e_nn = Entity_Create(parent, "NeuralNetwork", position, rotation, scale, pivot);
    EC_NN_Layer **ec_layers = malloc(sizeof(EC_NN_Layer *) * neuralNetwork->layerCount);
    float layerWidth = EC_NEURON_CIRCLE_RADIUS;
    float layerSpacing = layerWidth * 4.0;
    // Calculate X position
    float x = position.x - 0.5 * (((neuralNetwork->layerCount - 1) * layerSpacing) - (layerWidth * neuralNetwork->layerCount));
    // Calculate Y position
    float y = -1.0;
    for (int i = 0; i < neuralNetwork->layerCount; i++)
    {
        float height = EC_Layer_GetHeight(neuralNetwork->layers[i].neurons_size);
        if (height > y)
        {
            y = height;
        }
    }
    y = position.y - 0.5 * y;
    // Create Layers
    for (int i = 0; i < neuralNetwork->layerCount; i++)
    {
        ec_layers[i] = Prefab_Layer(e_nn, (V3){x, y, position.z}, rotation, scale, pivot, &neuralNetwork->layers[i]);
        x += layerWidth + layerSpacing;
    }
    // Create Links
    for (int la = 0; la < neuralNetwork->layerCount - 1; la++)
    {
        Layer* layerA = &neuralNetwork->layers[la];
        int lb = la + 1;
        Layer* layerB = &neuralNetwork->layers[lb];
        for (int na = 0; na < layerA->neurons_size; na++)
        {
            EC_NN_Neuron *neuronA = ec_layers[la]->neurons[na];
            for (int nb = 0; nb < layerB->neurons_size; nb++)
            {
                EC_NN_Neuron *neuronB = ec_layers[lb]->neurons[nb];
                printf("Weight[%d] is %f\n", na, layerB->neurons[nb].weights[na]);
                Prefab_NN_Link(e_nn, neuronA, neuronB, position.z, layerB->neurons[nb].weights[na]);
            }
        }
    }
    EC_NN *EC_neuralNetwork = EC_NeuralNetwork_Create(e_nn, neuralNetwork->layerCount, ec_layers);
    // Component
    return EC_neuralNetwork;
}