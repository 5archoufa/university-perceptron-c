#include "entity/components/neural_network/ec_nn_layer.h"
#include "entity/components/neural_network/ec_nn_neuron.h"
#include "neural_networks/layer.h"
// Shader
#include "rendering/shader/shader-manager.h"
// Mesh
#include "rendering/mesh/mesh-manager.h"

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
    return EC_NEURON_WIDTH * 0.3f;
}

EC_NN_Layer *Prefab_Layer(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, Layer *layer)
{
    // Entity
    Entity *e_layer = Entity_Create(parent, false, "Layer", TS, position, rotation, scale);
    // A rectangle showing the limits of the layer
    V3 layerSize = {EC_NEURON_WIDTH, 0.0f, EC_Layer_GetHeight(layer->neurons_size)};
    Entity *e_visual = Entity_Create(e_layer, false, "Visual", TS_LOCAL, V3_ZERO, QUATERNION_IDENTITY, layerSize);
    Mesh *mesh_layer = MeshManager_GetDefaultPlane();
    Material *material = Material_Create(ShaderManager_Get(SHADER_SPRITE), 0, NULL);
    Material_SetVec4(material, "color", (vec4){0.2f, 0.2f, 0.4f, 1.0f});
    EC_MeshRenderer_Create(e_visual, mesh_layer, (V3){1.0f, 0.0f, 1.0f}, material);
    // Neurons
    EC_NN_Neuron **EC_neurons = malloc(sizeof(EC_NN_Neuron *) * layer->neurons_size);
    float neuronSpacing = NeuronSpacing();
    V3 neuronPos = {0.0, 0.001, -layerSize.z * 0.5};
    for (int i = 0; i < layer->neurons_size; i++)
    {
        EC_NN_Neuron *EC_neuron = Prefab_Neuron(e_layer, TS_LOCAL, neuronPos, QUATERNION_IDENTITY, V3_ONE, &layer->neurons[i]);
        neuronPos.z += EC_NEURON_WIDTH + neuronSpacing;
        EC_neurons[i] = EC_neuron;
    }
    // ============ EC_Layer ============ //
    EC_NN_Layer *EC_layer = EC_Layer_Create(e_layer, layer, EC_neurons);
    return EC_layer;
}

float EC_Layer_GetHeight(size_t neurons_count)
{
    float neuronSpacing = NeuronSpacing();
    return (neurons_count * EC_NEURON_WIDTH) + (neurons_count - 1) * neuronSpacing;
}

void EC_Layer_Free(Component *component)
{
    EC_NN_Layer *EC_layer = component->self;
    free(EC_layer->neurons);
    free(EC_layer);
}