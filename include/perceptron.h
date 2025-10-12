#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#include "utilities/math/v3.h"
#include <stdint.h>
#include "ui/window.h"
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// -------------------------
// Time
// -------------------------
extern float DeltaTime;
extern float FixedDeltaTime;
extern float PerceptronTime;

// -------------------------
// Window
// -------------------------
extern MyWindowConfig WindowConfig;

// -------------------------
// World Axis
// -------------------------
extern const V3 WORLD_UP;
extern const vec3 WORLD_UP_vec3;
extern const V3 WORLD_RIGHT;
extern const V3 WORLD_FORWARD;
extern const V3 WORLD_DOWN;
extern const V3 WORLD_LEFT;
extern const V3 WORLD_BACK;

// -------------------------
// Layers
// -------------------------
#define ELAYERS_SIZE 3
#define ELAYERS                \
    {                                 \
        {.name = "Default"}, \
        {.name = "Terrain"}, \
        {.name = "UI"}}

#define ELAYERS_PHYSICS_FILTER                                       \
    {                                                  \
        /*            Default     Terrain        UI*/  \
        /* Default */ {true, /* */ true, /* */ false}, \
        /* Terrain */ {true, /* */ false, /**/ false}, \
        /* UI      */ {false, /**/ false, /**/ false}, \
    }

#endif