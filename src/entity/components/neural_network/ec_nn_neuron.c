#include "neural_networks/neuron.h"
// Entity
#include "entity/entity.h"
#include "entity/transform.h"
// NN
#include "entity/components/neural_network/ec_nn_neuron.h"
#include "entity/components/neural_network/ec_nn_neuron.h"
// Mesh
#include "rendering/mesh/mesh-manager.h"
// Shader
#include "rendering/shader/shader-manager.h"
// Material
#include "rendering/material/material.h"
#include "rendering/material/material-manager.h"

const float EC_NEURON_WIDTH = 1.0f;

static void EC_Neuron_Update(Component *component)
{
    EC_NN_Neuron *ec_neuron = component->self;
}

EC_NN_Neuron *Prefab_Neuron(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, Neuron *neuron)
{
    // ============ Entity ============ //
    Entity *e_neuron = Entity_Create(parent, false, "Neuron", TS, position, rotation, scale);
    // ============ Visual ============ //
    Entity *e_visual = Entity_Create(e_neuron, false, "Visual", TS_LOCAL, V3_ZERO, QUATERNION_IDENTITY, (V3){EC_NEURON_WIDTH, 1.0f, EC_NEURON_WIDTH});
    Mesh *mesh_neuron = MeshManager_GetDefaultPlane();
    Material *material = Material_Create(ShaderManager_Get(SHADER_SPRITE), 0, NULL);
    Material_SetVec4(material, "color", (vec4){0.0f, 0.0f, 1.0f, 1.0f});
    EC_MeshRenderer_Create(e_visual, mesh_neuron, (V3){1.0f, 0.0f, 1.0f}, material);
    // ============ EC_Neuron ============ //
    EC_NN_Neuron *EC_neuron = EC_Neuron_Create(e_neuron, neuron);
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

EC_NN_Neuron *EC_Neuron_Create(Entity *entity, Neuron *neuron)
{
    EC_NN_Neuron *EC_neuron = malloc(sizeof(EC_NN_Neuron));
    EC_neuron->neuron = neuron;
    // Component
    EC_neuron->component = Component_Create(EC_neuron, entity, EC_T_NN_NEURON, EC_Neuron_Free, EC_Neuron_Awake, NULL, NULL, NULL, NULL);
    return EC_neuron;
}