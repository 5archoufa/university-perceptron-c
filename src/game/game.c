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
// Physics
#include "physics/physics-manager.h"
// Mesh Renderer
#include "entity/components/ec_mesh_renderer/ec_mesh_renderer.h"
// Camera
#include "entity/components/ec_camera/ec_camera.h"
#include "entity/components/ec_camera/ec_camera_controller.h"
// State Machine
#include "state_machine/state_machine.h"
// Neural Network
#include "neural_networks/neural_network.h"
#include "entity/components/neural_network/ec_nn_neuron.h"
#include "entity/components/neural_network/ec_nn_layer.h"
#include "entity/components/neural_network/ec_nn.h"
// Noise
#include "utilities/noise/noise.h"
// Island
#include "entity/components/island/island.h"
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
// GUI
#include "entity/components/gui/ec_gui.h"
#include "entity/components/gui/w_text.h"
#include "game/gui/text/w_text_datetime.h"
// Trigle
#include "game/trigle/ec/ec_trigle.h"
// State Machine
#include "entity/components/ec_state_machine/ec_state_machine.h"
// Rabbit
#include "entity/components/ec_rabbit/ec_rabbit.h"

// ----------------------------------------
// External Variables
// ----------------------------------------

InputContext *INPUT_CONTEXT_GAMEPLAY;
InputContext *INPUT_CONTEXT_TRIGLE;
EC_StateMachine *SM_GAME;
EC_StateMachine_State *SM_STATE_GAMEPLAY;
EC_StateMachine_State *SM_STATE_TRIGLE;

// -------------------------
// Static Variables
// -------------------------

static World *_world;
static NeuralNetwork *_neuralNetwork;
static EC_Camera *_ec_camera;
static EC_Player *_ec_player;

// -------------------------
// Events
// -------------------------

void Game_Awake()
{
    // ============ Input Contexts ============ //
    INPUT_CONTEXT_GAMEPLAY = InputContext_Create("Gameplay", true);
    INPUT_CONTEXT_TRIGLE = InputContext_Create("Trigle", true);
    InputManager_PushContext(INPUT_CONTEXT_GAMEPLAY);

    // ============ World ============ //
    _world = World_Create("Game World", 3600.0f);

    // ============ State Machine ============ //
    SM_GAME = EC_StateMachine_Create(_world->parent, "Game StateMachine");
    SM_STATE_GAMEPLAY = EC_StateMachine_State_Create(
        SM_GAME,
        "Gameplay State",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);
    SM_STATE_TRIGLE = EC_StateMachine_State_Create(
        SM_GAME,
        "Trigle State",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    // ============ Camera ============ //
    V3 cameraPosition = {5, 9, -20};
    _ec_camera = Prefab_DisplayCamera(
        _world->parent,
        "Main Camera",
        TS_WORLD,
        cameraPosition,
        Quat_FromEuler((V3){0, 0, 0}),
        V3_ONE,
        (V2){WindowConfig.imageWidth, WindowConfig.imageHeight});
    EC_Camera_RemoveFromCullingMask(_ec_camera, E_LAYER_TRIGLE);
    EC_CameraController_Create(_ec_camera->component->entity, _ec_camera, (V3){15.0, 15.0, 15.0});

    // ============ Skybox ============ //
    // Entity *e_skybox = Entity_Create(_world->parent, true, "Skybox", TS_WORLD, V3_ZERO, QUATERNION_IDENTITY, V3_ONE);
    // V3 meshScale_skybox = {1000, 1000, 1000};
    // Mesh *mesh_skybox = Mesh_CreateSphere(meshScale_skybox.x, 16, 32, V3_HALF, 0xffffff, true);
    // EC_MeshRenderer *ec_meshRenderer_skybox = ec_meshRenderer_Create(e_skybox, mesh_skybox, meshScale_skybox, NULL);

    // ============ Neural Network ============ //
    int neuronCounts[] = {4, 2, 3, 6};
    _neuralNetwork = NeuralNetwork_Create("Test", 2, 4, neuronCounts, NN_Bias_Zero, NN_Weight_Random);
    NeuralNetwork_Print(_neuralNetwork);

    // ============ Island ============ //
    V3 islandMeshScale = {300, 14, 330};
    V3 islandPos = {0, -0.5f, 0};
    EC_Island *ec_island = Prefab_Island(_world->parent, TS_WORLD, islandPos, QUATERNION_IDENTITY, V3_ONE, islandMeshScale);
    float islandRadius = ec_island->ec_meshRenderer_island->bounds.size.x * 0.5f;
    V3 islandCenter = V3_CENTER(ec_island->ec_meshRenderer_island->bounds.min, ec_island->ec_meshRenderer_island->bounds.max);

    // ============ Water ============ //
    V3 waterMeshScale = {islandMeshScale.x + 100, 0, islandMeshScale.z + 100};
    V3 waterPos = {islandPos.x, -2, islandPos.z};
    EC_Water *ec_water = Prefab_Water(_world->parent, TS_WORLD, waterPos, QUATERNION_IDENTITY, V3_ONE, waterMeshScale, 0, NULL);

    // ============ Papati's Tree ============ //

    int i;
    int posx=1;
    int posz=1;
    for (i = 0; i <= 100; i++)
    {
        V3 treePos = islandCenter;
        switch (posz){
            case 1:
             treePos.x = treePos.x+RandomFloat(0,islandMeshScale.x/2);
             treePos.z = treePos.z+RandomFloat(0,islandMeshScale.z/2);
             posz++;
             break;
        case 2:
            treePos.x = treePos.x+RandomFloat(0,islandMeshScale.x/2);
            treePos.z = -(treePos.z+RandomFloat(0,islandMeshScale.z/2));
            posz++;
            break;
        case 3:
        treePos.x = -(treePos.x+RandomFloat(0,islandMeshScale.x/2));
        treePos.z = treePos.z+RandomFloat(0,islandMeshScale.z/2);
        posz++;
        break;
        case 4:
        treePos.x = -(treePos.x+RandomFloat(0,islandMeshScale.x/2));
        treePos.z = -(treePos.z+RandomFloat(0,islandMeshScale.z/2));
        posz=1;
        break;

        }
                
       // if(i%2==0) 
      //  treePos.x = -treePos.x ; 
      //  else treePos.z =-treePos.z;
        
        treePos.y += 2.2; // #4e2b03ff
        Entity *e_tree_base = Prefab_Cube(_world->parent, true, TS_WORLD, treePos, Quat_FromEuler((V3){0, 0, 0}), V3_ONE, (V3){0.4, 2, 0.4}, 0xff032b4e)->component->entity;
        treePos.y += 0.5f;
        Entity *e_tree_leaf = Prefab_Cube(_world->parent, true, TS_WORLD, treePos, QUATERNION_IDENTITY, V3_ONE, (V3){1, 0.4, 0.6}, 0xff00ff11)->component->entity;
    }
    
    // ============ Sun ============ //
    uint32_t sunColor = 0xfffff5de; // #def5ffff
    EC_Planet *ec_sun = Prefab_Planet(_world->parent, sunColor, 1.0f);

    // ============ Player ============ //
    // Spawn random players all over the island
    // for (int i = 0; i < 5026; i++)
    // {
    //     V3 randomPos = {
    //         RandomFloat(islandCenter.x - islandRadius * 0.5f, islandCenter.x + islandRadius * 0.5f),
    //         islandCenter.y + 20,
    //         RandomFloat(islandCenter.z - islandRadius * 0.5f, islandCenter.z + islandRadius * 0.5f),
    //     };
    //     _ec_player = EC_Player_Prefab(ec_island, _world->parent, TS_WORLD, randomPos, Quat_FromEuler((V3){0, 180, 0}), V3_ONE);
    // }
    V3 playerPos = {
        islandCenter.x,
        islandCenter.y + 20,
        islandCenter.z - islandRadius * 0.5f,
    };
    _ec_player = EC_Player_Prefab(ec_island, _world->parent, TS_WORLD, playerPos, Quat_FromEuler((V3){0, 180, 0}), V3_ONE);

    // ============ ZOUBINETE's Tree ============ //
    uint32_t treeColor = 0xff0e0878; // #78080eff
    // EC_Tree *ec_tree = Prefab_Tree(_world->parent, (V3){0.5, 0, 0});

    // ============ Test GUI ============ //
    // Create a test GUI canvas
    V2 resolution = {1280, 720};
    EC_GUI *testGUI = Prefab_GUI(_world->parent, "Screen-Space GUI", GUI_RENDER_MODE_SCREEN_SPACE_OVERLAY, resolution, true);
    // Create test text
    TextFont *defaultFont = TextFont_GetDefault();
    V3 textPos = {5.0f, 5.0f, 0.0f};
    Prefab_W_Text(testGUI, NULL, defaultFont, "V0.1", 12, textPos, QUATERNION_IDENTITY, 0xffffffff);
    Prefab_W_Text_DateTime(testGUI, testGUI->component->entity, defaultFont, 12, (V3){5.0f, 20, 0.0f}, QUATERNION_IDENTITY, 0xffffffff); // Dark Blue

    // ============ Call Awake ============ //
    for (int i = 0; i < _world->parent->transform.children_size; i++)
    {
        Entity_Awake(_world->parent->transform.children[i]->entity);
    }

    // ============ WorldSpace GUI Test ============ //
    EC_GUI *worldSpaceGUI = Prefab_GUI(_ec_player->component->entity, "World-Space GUI", GUI_RENDER_MODE_WORLD_SPACE, (V2){1280, 720}, true);
    // Create 3D text
    TextFont *default3DTextFont = TextFont_GetDefault();
    W_Text *text3d = Prefab_W_Text(worldSpaceGUI, NULL, default3DTextFont, "Mohanned", 1, V3_ADD(playerPos, (V3){-1, 2, 0}), QUATERNION_IDENTITY, 0xffffffff);
    T_WPos_Set(&text3d->component->entity->transform, V3_ADD(playerPos, (V3){0, 2, 0}));
    T_WRot_Add(&text3d->component->entity->transform, (V3){180, 0, 0});

    // ============ Trigle ============ //
    // EC_Trigle *ec_trigle = Prefab_Trigle(_ec_camera->component->entity, "Trigle", TS_LOCAL, (V3){0, 0, 1}, Quat_FromEuler((V3){0, 180, 0}), V3_ONE);
    // EC_Trigle_AddPage(ec_trigle, "Neural Network");
    // // Abstract Page
    // TriglePage *page_abstract = EC_Trigle_GetPage(ec_trigle, 0);
    // Prefab_Cube(page_abstract->e_space, true, TS_LOCAL, (V3){0, 0, 0}, QUATERNION_IDENTITY, V3_ONE, (V3){0.3, 0.3, 0.3}, 0xff808080);
    // Prefab_PlaneDefault(page_abstract->e_space, true, TS_LOCAL, (V3){0, 0.4, 0}, Quat_FromEuler((V3){0, 0, 0}), (V3){0.2,0.0f, 0.2});
    // EC_NN *nn = Prefab_NeuralNetwork(page_abstract->e_space, TS_LOCAL, V3_ZERO, QUATERNION_IDENTITY, V3_ONE, _neuralNetwork);
    // W_Text *nnTitle = Prefab_W_Text(ec_trigle->ec_gui_worldSpace, page_abstract->e_GUIParent, default3DTextFont, "Neural Network", 2, (V3){0, 0, 2}, QUATERNION_IDENTITY, 0xff000000);
    // // Display Abstract Page
    // EC_Trigle_SetActivePage(ec_trigle, 0);

    // EC_Trigle_SetState(ec_trigle, TRIGLE_STATE_FOCUSED);

    // ============ YowYoh ============ //
    // Entity *e_rabbit = Entity_Create(_world->parent, false, "Rabbit", TS_WORLD, V3_ZERO, QUATERNION_IDENTITY, V3_ONE);
    // EC_Rabbit_Create(ec_island, e_rabbit);
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
    World_Select(_world);
    World_Update();
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
    PhysicsManager_FixedUpdate(_world->physicsManager);
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
    // Input contexts will be automatically freed by InputManager_Free()
    World_All_Free();
    NeuralNetwork_Free(_neuralNetwork);
}