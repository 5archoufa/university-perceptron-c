#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <stdbool.h>
#include "utilities/math/v3.h"
#include "utilities/math/stupid_math.h"
#include <stdint.h>
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// -------------------------
// Types
// -------------------------

typedef struct Entity Entity;
typedef enum TransformSpace TransformSpace;
typedef struct Transform Transform;
typedef struct Quaternion Quaternion;

enum TransformSpace
{
    TS_WORLD,
    TS_LOCAL
};

struct Quaternion
{
    float w, x, y, z;
};

struct Transform
{
    Entity *entity;
    // World Transform
    /// @brief Use T_WPos() To get the world position
    V3 w_pos;
    /// @brief Use T_WRot() To get the world rotation
    Quaternion w_rot;
    /// @brief Use T_WSca() To get the world scale
    V3 w_scale;
    mat4 w_matrix;
    // Private Transform
    /// @brief Use T_LPos() To get the local position
    V3 l_pos;
    /// @brief Use T_LRot() To get the local rotation
    Quaternion l_rot;
    /// @brief Use T_LSca() To get the local scale
    V3 l_scale;
    mat4 l_matrix;
    // Hierarchy
    Transform *parent;
    size_t children_size;
    Transform **children;
    // Cache
    bool isDirty;
};

// -------------------------
// Constants
// -------------------------

extern const Quaternion QUATERNION_IDENTITY;

// -------------------------
// Transform Initialization
// -------------------------

void Transform_Init(Transform *transform, Entity *entity, Transform *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale);

// -------------------------
// Hierarchy
// -------------------------

void Transform_RemoveChild(Transform *parent, Transform *child);
void Transform_SetParent(Transform *parent, Transform *child, bool worldPositionStays);

// -------------------------
// Debugging
// -------------------------

void T_Print_WMatrix(Transform *transform);
void T_Print_LMatrix(Transform *transform);

// -------------------------
// Getters
// -------------------------

V3 T_WPos(Transform *transform);
Quaternion T_WRot(Transform *transform);
V3 T_WSca(Transform *transform);

V3 T_LPos(Transform *transform);
Quaternion T_LRot(Transform *transform);
V3 T_LSca(Transform *transform);

V3 T_Forward(Transform *transform);
V3 T_Right(Transform *transform);
V3 T_Up(Transform *transform);

void T_WMatrix(Transform *transform, mat4 dest);
void T_LMatrix(Transform *transform, mat4 dest);

// -------------------------
// Setters
// -------------------------

void T_WSca_Set(Transform *transform, V3 w_scale);
void T_WRot_Set(Transform *transform, Quaternion w_rot);
void T_WPos_Set(Transform *transform, V3 w_pos);
void T_WRot_Add(Transform *transform, V3 euler);
void T_WPos_Add(Transform *transform, V3 addition);
V3 T_WEuler(Transform *transform);
V3 T_LEuler(Transform *transform);
void T_LPos_Set(Transform *transform, V3 l_pos);
void T_LRot_Set(Transform *transform, Quaternion l_rot);
void T_LSca_Set(Transform *transform, V3 l_scale);
void T_LRot_Mul(Transform *transform, Quaternion q);
void T_LSca_Mul(Transform *transform, V3 scale);
void T_LPos_Add(Transform *transform, V3 addition);
void T_LPos_Sub(Transform *transform, V3 subtraction);
void T_LRot_Add(Transform *transform, V3 euler);
void T_Debug_Validate(Transform *transform);

// -------------------------
// Quaternion utilities
// -------------------------

Quaternion Quat_FromEuler(V3 euler);
V3 Quat_ToEuler(Quaternion q);
Quaternion Quat_FromAxisAngle(V3 axis, float angle);
Quaternion Quat_Inverse(const Quaternion q);
Quaternion Quat_Mul(Quaternion a, Quaternion b);
Quaternion Quat_Div(Quaternion a, Quaternion b);
Quaternion Quat_Norm(Quaternion q);
V3 Quat_RotateV3(Quaternion q, V3 v);
Quaternion Quat_LookDirection(V3 direction);

#endif