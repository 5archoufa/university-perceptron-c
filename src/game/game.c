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

static World* _world;
static NeuralNetwork* _neuralNetwork;

void Game_Awake(){
    // Create game world
    _world = World_Create("Game World");
    // Camera
    Entity* E_camera = Entity_Create(_world->parent, "Main Camera", V3_ZERO, 0.0, V2_ONE, V2_HALF);
    EC_Camera *EC_camera = EC_Camera_Create(E_camera, MainWindow->image, (V2){1600, 900});
    EC_CameraController_Create(E_camera, EC_camera, (V2){10.0, 10.0});

    // NeuralNetwork
    int neuronCounts[] = {3, 1, 8, 2};
    _neuralNetwork = NeuralNetwork_Create("Test", 2, 4, neuronCounts, NN_Bias_Zero, NN_Weight_Random);
    NeuralNetwork_Print(_neuralNetwork);

    EC_NN* ec_neuralNetwork = Prefab_NeuralNetwork(_world->parent, V3_ZERO, 0.0, V2_ONE, V2_HALF, _neuralNetwork);

    // Call Awake()
    for(int i = 0;i<_world->parent->children_size;i++){
        Entity_Awake(_world->parent->children[i]);
    }
}

void Game_Start(){
    for(int i = 0;i<_world->parent->children_size;i++){
        Entity_Start(_world->parent->children[i]);
    }
}

void Game_Update(){
    for(int i = 0;i<_world->parent->children_size;i++){
        Entity_Update(_world->parent->children[i]);
    }
}

void Game_LateUpdate(){
    for(int i = 0;i<_world->parent->children_size;i++){
        Entity_LateUpdate(_world->parent->children[i]);
    }
}

void Game_FixedUpdate(){
    for(int i = 0;i<_world->parent->children_size;i++){
        Entity_FixedUpdate(_world->parent->children[i]);
    }
}

void Game_Free(){
    World_FreeAll();
    NeuralNetwork_Free(_neuralNetwork);
}