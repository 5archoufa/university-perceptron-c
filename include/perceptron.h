#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#include "utilities/math/v3.h"
#include <stdint.h>
#include "ui/window.h"
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h> 

extern float DeltaTime;
extern float FixedDeltaTime;
extern MyWindowConfig WindowConfig;
extern const V3 WORLD_UP;
extern const vec3 WORLD_UP_vec3;
extern const V3 WORLD_RIGHT;
extern const V3 WORLD_FORWARD;
extern const V3 WORLD_DOWN;
extern const V3 WORLD_LEFT;
extern const V3 WORLD_BACK;
extern GLuint ShaderProgram;
extern float PerceptronTime;

#endif