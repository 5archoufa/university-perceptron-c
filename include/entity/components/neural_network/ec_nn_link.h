#ifndef EC_NN_LINK_H
#define EC_NN_LINK_H

#include "entity/components/renderer/line.h"
#include "entity/components/neural_network/ec_nn_neuron.h"
#include "entity/entity.h"
#include "entity/transform.h"

typedef struct EC_NN_Link {
    Component* component;
    Entity *entity;
    EC_Renderer *EC_renderer_line;
    EC_NN_Neuron* from;
    EC_NN_Neuron* to;
} EC_NN_Link;

EC_NN_Link* Prefab_NN_Link(Entity* parent, EC_NN_Neuron* from, EC_NN_Neuron* to, float weight);
#endif