#include "entity/components/neural_network/ec_nn.h"
#include "entity/components/neural_network/ec_nn_link.h"
#include "entity/components/neural_network/ec_nn_neuron.h"
#include "entity/transform.h"

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

EC_NN *Prefab_NeuralNetwork(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, NeuralNetwork *neuralNetwork)
{
    Entity *e_nn = Entity_Create(parent, "NeuralNetwork", TS, position, rotation, scale);
    EC_NN_Layer **ec_layers = malloc(sizeof(EC_NN_Layer *) * neuralNetwork->layerCount);
    float layerWidth = EC_NEURON_CIRCLE_RADIUS;
    float layerSpacing = layerWidth * 4.0;
    // Calculate X position
    // Calculate Y position
    float tallestLayer = -1.0;
    for (int i = 0; i < neuralNetwork->layerCount; i++)
    {
        float height = EC_Layer_GetHeight(neuralNetwork->layers[i].neurons_size);
        if (height > tallestLayer)
        {
            tallestLayer = height;
        }
    }
    float x = 0.5 * (((neuralNetwork->layerCount - 1) * layerSpacing) - (layerWidth * neuralNetwork->layerCount));
    V3 layer_LPos = {x, 0.5 * tallestLayer, 0.0};
    // Create Layers
    for (int i = 0; i < neuralNetwork->layerCount; i++)
    {
        ec_layers[i] = Prefab_Layer(e_nn, TS_LOCAL, layer_LPos, QUATERNION_IDENTITY, V3_ONE, &neuralNetwork->layers[i]);
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
                Prefab_NN_Link(e_nn, neuronA, neuronB, layerB->neurons[nb].weights[na]);
            }
        }
    }
    EC_NN *EC_neuralNetwork = EC_NeuralNetwork_Create(e_nn, neuralNetwork->layerCount, ec_layers);
    // Component
    return EC_neuralNetwork;
}