#include "game/game.h"
#include "utilities/math/v3.h"
#include "input/input_manager.h"
#include "entity/components/camera/camera.h"
#include "entity/components/renderer/renderer.h"
#include "entity/components/renderer/circle.h"
#include "entity/components/camera/camera_controller.h"
#include "state_machine/state_machine.h"
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include "perceptron.h"
#include "neural_networks/neural_network.h"
#include "entity/components/neural_network/ec_nn_neuron.h"
#include "entity/components/neural_network/ec_nn_layer.h"
#include "entity/components/neural_network/ec_nn.h"
#include "utilities/noise/noise.h"
#include "entity/components/renderer/rd_noise.h"
#include "entity/components/island/island.h"
#include "entity/components/renderer/rd_island.h"
#include "utilities/math/stupid_math.h"
#include "entity/components/ec_renderer3d/ec_renderer3d.h"
#include "entity/components/ec_water/ec_water.h"
#include "entity/components/lighting/ls_directional_light.h"
#include "entity/components/lighting/ec_light.h"
#include "entity/components/ec_planet/ec_planet.h"

static World *_world;
static NeuralNetwork *_neuralNetwork;
static EC_Camera *_ec_camera;

static void CompileShaders(){
    // Solid Toon
    char *vertexSource = File_LoadStr("src/shader/solid_toon_vertex.glsl");
}

void Game_Awake()
{
    // Create game world
    _world = World_Create("Game World");
    // Camera
    V3 cameraPosition = {0, 10, 0};
    Quaternion cameraRotation = Quat_FromEuler((V3){-30, 0, 0});
    Entity *E_camera = Entity_Create(_world->parent, "Main Camera", TS_WORLD, cameraPosition, cameraRotation, V3_ONE);
    // Use imageWidth/imageHeight for low-res pixel art render target
    _ec_camera = EC_Camera_Create(E_camera, NULL, (V2){WindowConfig.imageWidth, WindowConfig.imageHeight}, Radians(60.0f), 0.1f, 2000.0f);
    EC_CameraController_Create(E_camera, _ec_camera, (V3){30.0, 30.0, 30.0});

    // NeuralNetwork
    int neuronCounts[] = {3, 9, 8, 2};
    _neuralNetwork = NeuralNetwork_Create("Test", 2, 4, neuronCounts, NN_Bias_Zero, NN_Weight_Random);
    // NeuralNetwork_Print(_neuralNetwork);

    // EC_NN *ec_neuralNetwork = Prefab_NeuralNetwork(_world->parent, TRANSFORM_IDENTITY, _neuralNetwork);

    // _noise = Noise_Create(300, 300, 25);
    // EC_Renderer *noise_renderer = Prefab_Noise(_world->parent, V3_ZERO, 0.0, V2_ONE, V2_HALF, _noise);

    // Island
    V3 islandMeshScale = {0.8f, 0.2f, 0.8f};
    V3 islandPos = {0, -0.5f, 0};
    EC_Island *ec_island = Prefab_Island(_world->parent, TS_WORLD, islandPos, QUATERNION_IDENTITY, V3_ONE, islandMeshScale);
    float islandRadius = ec_island->ec_renderer3d_island->bounds.size.x * 0.5f;
    V3 islandCenter = V3_CENTER(ec_island->ec_renderer3d_island->bounds.start, ec_island->ec_renderer3d_island->bounds.end);
    // Water
    V3 waterMeshScale = {2.0f, 0.2f, 2.0f};
    V3 waterPos = {0, -0.6f, 0};
    EC_Water *ec_water = Prefab_Water(_world->parent, TS_WORLD, waterPos, QUATERNION_IDENTITY, V3_ONE, waterMeshScale, 0, NULL);

    // Entity_SetPos(_ec_camera->component->entity, islandCenter);
    // Entity_AddPos(_ec_camera->component->entity, (V3){-5, 10.0, -20});
    // Cube at island center for reference
    Entity *e_cube = Prefab_Cube(_world->parent, TS_WORLD, islandCenter, QUATERNION_IDENTITY, V3_ONE, 10, NULL)->component->entity;

    // Sun
    uint32_t sunColor = 0xfffff5de; // #def5ffff
    EC_Planet *sun = Prefab_Planet(_world->parent, (V3){5, 0, 0}, sunColor, 1.0f);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Call Awake()
    for (int i = 0; i < _world->parent->transform.children_size; i++)
    {
        Entity_Awake(_world->parent->transform.children[i]->entity);
    }
}

void Game_Start()
{
    for (int i = 0; i < _world->parent->transform.children_size; i++)
    {
        Entity_Start(_world->parent->transform.children[i]->entity);
    }
}

void Game_Update()
{
    for (int i = 0; i < _world->parent->transform.children_size; i++)
    {
        Entity_Update(_world->parent->transform.children[i]->entity);
    }
}

void Game_LateUpdate()
{
    for (int i = 0; i < _world->parent->transform.children_size; i++)
    {
        Entity_LateUpdate(_world->parent->transform.children[i]->entity);
    }
}

void Game_FixedUpdate()
{
    for (int i = 0; i < _world->parent->transform.children_size; i++)
    {
        Entity_FixedUpdate(_world->parent->transform.children[i]->entity);
    }
}

void Game_EndOfFrame()
{
}

void Game_Free()
{
    World_All_Free();
    NeuralNetwork_Free(_neuralNetwork);
}