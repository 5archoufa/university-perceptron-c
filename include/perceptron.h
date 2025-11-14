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
// LAYERS MUST NOT EXCEED 32
#define E_LAYER_DEFAULT 0
#define E_LAYER_CREATURE 1
#define E_LAYER_RAYCAST 2
#define E_LAYER_TERRAIN 3
#define E_LAYER_TRIGLE 4
#define E_LAYER_GUI 5
#define E_LAYER_TREE 6

// -------------------------
// Tags
// -------------------------
#define E_TAG_NONE 0

#endif