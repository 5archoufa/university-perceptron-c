#include "game/game.h"
#include "perceptron.h"
// C
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
// Math
#include "utilities/math/stupid_math.h"
#include "utilities/math/v3.h"
// Input
#include "input/input_manager.h"
// Entity Components
#include "entity/components/ec_renderer3d/ec_renderer3d.h"
#include "entity/components/renderer/renderer.h"
// Camera
#include "entity/components/camera/camera.h"
#include "entity/components/camera/camera_controller.h"
// State Machine
#include "state_machine/state_machine.h"
// Neural Network
#include "neural_networks/neural_network.h"
#include "entity/components/neural_network/ec_nn_neuron.h"
#include "entity/components/neural_network/ec_nn_layer.h"
#include "entity/components/neural_network/ec_nn.h"
// Noise
#include "utilities/noise/noise.h"
#include "entity/components/renderer/rd_noise.h"
// Island
#include "entity/components/island/island.h"
#include "entity/components/renderer/rd_island.h"
// Water
#include "entity/components/ec_water/ec_water.h"
// Lighting
#include "entity/components/ec_planet/ec_planet.h"
#include "entity/components/lighting/ls_directional.h"
#include "entity/components/lighting/ec_light.h"
// Player
#include "entity/components/ec_player/ec_player.h"
// Human
#include "entity/components/ec_human/ec_human.h"

// ------------------------- 
// Static Variables 
// -------------------------

static World *_world;
static NeuralNetwork *_neuralNetwork;
static EC_Camera *_ec_camera;

// ------------------------- 
// Events 
// -------------------------

void Game_Awake()
{
    // ============ World ============ //
    _world = World_Create("Game World");

    // ============ Camera ============ //
    V3 cameraPosition = {0, 4, 0};
    Quaternion cameraRotation = Quat_FromEuler((V3){-30, 0, 0});
    Entity *E_camera = Entity_Create(_world->parent, "Main Camera", TS_WORLD, cameraPosition, cameraRotation, V3_ONE);
    _ec_camera = EC_Camera_Create(E_camera, (V2){WindowConfig.imageWidth, WindowConfig.imageHeight}, Radians(60.0f), 0.1f, 2000.0f);
    EC_CameraController_Create(E_camera, _ec_camera, (V3){15.0, 3.0, 15.0});

    // ============ Neural Network ============ //
    // int neuronCounts[] = {3, 9, 8, 2};
    // _neuralNetwork = NeuralNetwork_Create("Test", 2, 4, neuronCounts, NN_Bias_Zero, NN_Weight_Random);
    // NeuralNetwork_Print(_neuralNetwork);
    // EC_NN *ec_neuralNetwork = Prefab_NeuralNetwork(_world->parent, TRANSFORM_IDENTITY, _neuralNetwork);

    // ============ Island ============ //
    V3 islandMeshScale = {300, 8, 330};
    V3 islandPos = {0, -0.5f, 0};
    EC_Island *ec_island = Prefab_Island(_world->parent, TS_WORLD, islandPos, QUATERNION_IDENTITY, V3_ONE, islandMeshScale);
    float islandRadius = ec_island->ec_renderer3d_island->bounds.size.x * 0.5f;
    V3 islandCenter = V3_CENTER(ec_island->ec_renderer3d_island->bounds.min, ec_island->ec_renderer3d_island->bounds.max);

    // ============ Water ============ //
    V3 waterMeshScale = {islandMeshScale.x + 100, 0, islandMeshScale.z + 100};
    V3 waterPos = {islandPos.x, -2, islandPos.z};
    EC_Water *ec_water = Prefab_Water(_world->parent, TS_WORLD, waterPos, QUATERNION_IDENTITY, V3_ONE, waterMeshScale, 0, NULL);

    // ============ Cube ============ //
    // Entity *e_cube = Prefab_Cube(_world->parent, TS_WORLD, islandCenter, QUATERNION_IDENTITY, V3_ONE, 10, NULL)->component->entity;

    // ============ L8oli's Tree ============ //
    V3 treePos = islandCenter;
    treePos.y += 2.2; // #4e2b03ff
    Entity *e_tree_base = Prefab_Cube(_world->parent, TS_WORLD, treePos, Quat_FromEuler((V3){0, 0, 0}), (V3){0.4, 2, 0.4}, 1, 0xff032b4e)->component->entity;

    treePos.y += 0.5f;
    treePos = V3_SUB(treePos, V3_SCALE((V3){1, 0, 0.6}, -0.25)); // #11ff00ff
    Entity *e_tree_leaf = Prefab_Cube(_world->parent, TS_WORLD, treePos, QUATERNION_IDENTITY, (V3){1, 0.4, 0.6}, 1, 0xff00ff11)->component->entity;

    // ============ Pyramid ============ //
    V3 pyramidPos = islandCenter;
    float pyramidMeshScale = 50;
    pyramidPos.z += 0.5 * islandMeshScale.z - 0.5 * pyramidMeshScale;
    float var = 0;
    for (int i = 0; i < 5; i++)
    {
        Entity *e_pyramid_top = Prefab_Cube(_world->parent, TS_WORLD, V3_ADD(pyramidPos, (V3){0, i*1.5, 0}), Quat_FromEuler((V3){0, 0, 0}), (V3){pyramidMeshScale, 1.5, pyramidMeshScale}, 1, 0xff020202)->component->entity;
    }

    // ============ Sun ============ //
    uint32_t sunColor = 0xfffff5de; // #def5ffff
    EC_Planet *ec_sun = Prefab_Planet(_world->parent, (V3){0.5, 0, 0}, sunColor, 1.0f, 11.0f);

    // ============ Player ============ //
    EC_Player *ec_player = EC_Player_Prefab(_world->parent, TS_WORLD, V3_ADD(islandPos, (V3){0, islandMeshScale.y + 1, 0}), QUATERNION_IDENTITY, V3_ONE);

    // ============ ZOUBINETE's Tree ============ //
    uint32_t treeColor = 0xff0e0878; // #78080eff
    // EC_Tree *ec_tree = Prefab_Tree(_world->parent, (V3){0.5, 0, 0});

    // ============ Call Awake ============ //
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
    // NeuralNetwork_Free(_neuralNetwork);
}