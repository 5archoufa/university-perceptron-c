#include "entity/transform.h"
#include "entity/entity.h"
#include "utilities/math/v3.h"
#include "utilities/math/stupid_math.h"
#include "logging/logger.h"
#include <string.h>
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <cglm/cglm.h>

// -------------------------
// Constants
// -------------------------

const Quaternion QUATERNION_IDENTITY = {1, 0, 0, 0};

// -------------------------
// Utilities
// -------------------------

static LogConfig _logConfig = {"Transform", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

static inline void Quat_from_mat4(Quaternion *q, const mat4 m, V3 scale)
{
    // Retrieve scale from matrix columns
    float m00 = m[0][0] / scale.x, m01 = m[0][1] / scale.x, m02 = m[0][2] / scale.x;
    float m10 = m[1][0] / scale.y, m11 = m[1][1] / scale.y, m12 = m[1][2] / scale.y;
    float m20 = m[2][0] / scale.z, m21 = m[2][1] / scale.z, m22 = m[2][2] / scale.z;

    float trace = m00 + m11 + m22;

    if (trace > 0.0f)
    {
        float s = sqrtf(trace + 1.0f) * 2.0f; // s = 4*w
        q->w = 0.25f * s;
        q->x = (m21 - m12) / s;
        q->y = (m02 - m20) / s;
        q->z = (m10 - m01) / s;
    }
    else if ((m00 > m11) && (m00 > m22))
    {
        float s = sqrtf(1.0f + m00 - m11 - m22) * 2.0f; // s = 4*x
        q->w = (m21 - m12) / s;
        q->x = 0.25f * s;
        q->y = (m01 + m10) / s;
        q->z = (m02 + m20) / s;
    }
    else if (m11 > m22)
    {
        float s = sqrtf(1.0f + m11 - m00 - m22) * 2.0f; // s = 4*y
        q->w = (m02 - m20) / s;
        q->x = (m01 + m10) / s;
        q->y = 0.25f * s;
        q->z = (m12 + m21) / s;
    }
    else
    {
        float s = sqrtf(1.0f + m22 - m00 - m11) * 2.0f; // s = 4*z
        q->w = (m10 - m01) / s;
        q->x = (m02 + m20) / s;
        q->y = (m12 + m21) / s;
        q->z = 0.25f * s;
    }
}

/// @brief Converts a quaternion to a 4x4 rotation matrix (row-major)
static inline void Quat_to_mat4(const Quaternion *q, mat4 dest)
{
    float xx = q->x * q->x;
    float yy = q->y * q->y;
    float zz = q->z * q->z;
    float xy = q->x * q->y;
    float xz = q->x * q->z;
    float yz = q->y * q->z;
    float wx = q->w * q->x;
    float wy = q->w * q->y;
    float wz = q->w * q->z;

    // Zero out matrix
    memset(dest, 0, sizeof(mat4));

    // Row-major rotation matrix
    dest[0][0] = 1.0f - 2.0f * (yy + zz);
    dest[0][1] = 2.0f * (xy - wz);
    dest[0][2] = 2.0f * (xz + wy);
    dest[0][3] = 0.0f;

    dest[1][0] = 2.0f * (xy + wz);
    dest[1][1] = 1.0f - 2.0f * (xx + zz);
    dest[1][2] = 2.0f * (yz - wx);
    dest[1][3] = 0.0f;

    dest[2][0] = 2.0f * (xz - wy);
    dest[2][1] = 2.0f * (yz + wx);
    dest[2][2] = 1.0f - 2.0f * (xx + yy);
    dest[2][3] = 0.0f;

    dest[3][0] = 0.0f;
    dest[3][1] = 0.0f;
    dest[3][2] = 0.0f;
    dest[3][3] = 1.0f;
}

/// @brief Marks the transform and all its children as dirty
static void Transform_MarkDirty(Transform *transform)
{
    if (transform->isDirty)
        return;
    transform->isDirty = true;
    for (size_t i = 0; i < transform->children_size; i++)
    {
        Transform_MarkDirty(transform->children[i]);
    }
}

// -------------------------
// Setters
// -------------------------

inline static void Transform_SetLPos(Transform *transform, V3 *position, bool setDirty)
{
    transform->l_pos = *position;
    // Update local matrix
    transform->l_matrix[3][0] = position->x;
    transform->l_matrix[3][1] = position->y;
    transform->l_matrix[3][2] = position->z;
    // Mark as dirty
    if (setDirty)
    {
        Transform_MarkDirty(transform);
    }
}

inline static void Transform_SetLRot(Transform *transform, Quaternion *rotation, bool setDirty)
{
    transform->l_rot = *rotation;
    // Rotation
    mat4 rot;
    Quat_to_mat4(rotation, rot);
    // Scale
    mat4 scale = GLM_MAT4_IDENTITY_INIT;
    scale[0][0] = transform->l_scale.x;
    scale[1][1] = transform->l_scale.y;
    scale[2][2] = transform->l_scale.z;
    // Combine rotation * scale
    mat4 rs;
    glm_mat4_mul(rot, scale, rs);
    // Apply translation
    glm_mat4_copy(rs, transform->l_matrix);
    // Mark as dirty
    if (setDirty)
    {
        Transform_MarkDirty(transform);
    }
}

inline static void Transform_SetLSca(Transform *transform, V3 *scale, bool setDirty)
{
    transform->l_scale = *scale;
    // Scale
    mat4 scl = GLM_MAT4_IDENTITY_INIT;
    scl[0][0] = scale->x;
    scl[1][1] = scale->y;
    scl[2][2] = scale->z;
    // Rotation
    mat4 rot;
    Quat_to_mat4(&transform->l_rot, rot);
    // Combine rotation * scale
    mat4 rs;
    glm_mat4_mul(rot, scl, rs);
    // Apply translation
    glm_mat4_copy(rs, transform->l_matrix);
    // Mark as dirty
    if (setDirty)
    {
        Transform_MarkDirty(transform);
    }
}

inline static void Transform_InitLocalMatrix(Transform *transform, V3 l_pos, Quaternion l_rot, V3 l_scale)
{
    transform->l_rot = l_rot;
    transform->l_scale = l_scale;
    // Rotation
    mat4 rot;
    Quat_to_mat4(&l_rot, rot);
    // Scale
    mat4 scale = GLM_MAT4_IDENTITY_INIT;
    scale[0][0] = l_scale.x;
    scale[1][1] = l_scale.y;
    scale[2][2] = l_scale.z;
    // Combine rotation * scale
    mat4 rs;
    glm_mat4_mul(rot, scale, rs);
    // Apply translation
    glm_mat4_copy(rs, transform->l_matrix);
    Transform_SetLPos(transform, &l_pos, false);
}

/// @brief You must call this for every new Transform
void Transform_Init(Transform *transform, Entity *entity, Transform *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale)
{
    transform->entity = entity;
    transform->parent = parent;
    transform->children_size = 0;
    transform->children = NULL;
    if (TS == TS_WORLD)
    {
        Transform_InitLocalMatrix(transform, position, rotation, scale);
    }
    else
    {
        Transform_InitLocalMatrix(transform, position, rotation, scale);
    }
    Transform_MarkDirty(transform);
    if (parent)
    {
        Transform_SetParent(parent, transform, true);
    }
}

// -------------------------
// Hierarchy Management
// -------------------------

/// @param parent Should NEVER be NULL
/// @param child Should NEVER be NULL
void Transform_RemoveChild(Transform *parent, Transform *child)
{
    if (parent->children_size <= 1)
    {
        parent->children_size = 0;
        parent->children = NULL;
        return;
    }
    for (int i = 0; i < parent->children_size; i++)
    {
        if (parent->children[i] == child)
        {
            for (int j = i; j < parent->children_size - 1; j++)
            {
                parent->children[j] = parent->children[j + 1];
            }
            parent->children_size--;
            Transform **children = realloc(parent->children, parent->children_size * sizeof(Transform *));
            if (children)
            {
                parent->children = children;
            }
            break;
        }
    }
}

/// @param parent Should NEVER be NULL
/// @param child Should NEVER be NULL
void Transform_SetParent(Transform *parent, Transform *child, bool worldPositionStays)
{
    // Update Child
    child->parent = parent;
    // Mark as dirty
    Transform_MarkDirty(child);
    // Update Parent
    parent->children_size++;
    parent->children = realloc(parent->children, parent->children_size * sizeof(Transform *));
    parent->children[parent->children_size - 1] = child;
}

// -------------------------
// Dirty Checking & Cleaning
// -------------------------

static void Transform_CleanDownwards(Transform *transform)
{
    // ==== WORLD MATRIX ==== //
    if (transform->parent != NULL)
    {
        // World = Parent_world * Local
        glm_mat4_mul(transform->parent->w_matrix, transform->l_matrix, transform->w_matrix);
    }
    else
    {
        // World = Local
        glm_mat4_copy(transform->l_matrix, transform->w_matrix);
    }
    // ==== CACHE WORLD DATA ==== //
    // Position
    transform->w_pos = (V3){transform->w_matrix[3][0], transform->w_matrix[3][1], transform->w_matrix[3][2]};
    // Scale
    V3 scale = (V3){
        glm_vec3_norm((vec3){transform->w_matrix[0][0], transform->w_matrix[0][1], transform->w_matrix[0][2]}),
        glm_vec3_norm((vec3){transform->w_matrix[1][0], transform->w_matrix[1][1], transform->w_matrix[1][2]}),
        glm_vec3_norm((vec3){transform->w_matrix[2][0], transform->w_matrix[2][1], transform->w_matrix[2][2]}),
    };
    transform->w_scale = scale;
    // Rotation
    Quat_from_mat4(&transform->w_rot, transform->w_matrix, scale);
    // ==== CHILDREN ==== //
    // Reset flag
    transform->isDirty = false;
    // Clean up children
    for (int i = 0; i < transform->children_size; i++)
    {
        Transform_CleanDownwards(transform->children[i]);
    }
}

inline static void Transform_CleanFromTop(Transform *transform)
{
    while (transform->parent && transform->parent->isDirty)
    {
        transform = transform->parent;
    }
    Transform_CleanDownwards(transform);
}

// -------------------------
// Debugging
// -------------------------

void T_Print_WMatrix(Transform *transform)
{
    if (transform->isDirty)
    {
        Transform_CleanFromTop(transform);
    }
    Log(&_logConfig, "[%s] World Matrix:", transform->entity->name);
    for (int i = 0; i < 4; i++)
    {
        Log(&_logConfig, "[% .2f % .2f % .2f % .2f]", transform->w_matrix[i][0], transform->w_matrix[i][1], transform->w_matrix[i][2], transform->w_matrix[i][3]);
    }
}

void T_Print_LMatrix(Transform *transform)
{
    Log(&_logConfig, "[%s] Local Matrix:", transform->entity->name);
    for (int i = 0; i < 4; i++)
    {
        Log(&_logConfig, "[% .2f % .2f % .2f % .2f]", transform->l_matrix[i][0], transform->l_matrix[i][1], transform->l_matrix[i][2], transform->l_matrix[i][3]);
    }
}

// -------------------------
// Getters
// -------------------------

void T_WMatrix(Transform *transform, mat4 dest){
    if (transform->isDirty)
    {
        Transform_CleanFromTop(transform);
    }
    glm_mat4_copy(transform->w_matrix, dest);
}

void T_LMatrix(Transform *transform, mat4 dest){
    glm_mat4_copy(transform->l_matrix, dest);
}

void T_Print_Rot(Transform *transform)
{
    if (transform->isDirty)
    {
        Transform_CleanFromTop(transform);
    }
    Log(&_logConfig, "[%s] Local Rotation: (w: %.2f, x: %.2f, y: %.2f, z: %.2f)", transform->entity->name, transform->l_rot.w, transform->l_rot.x, transform->l_rot.y, transform->l_rot.z);
    Log(&_logConfig, "[%s] World Rotation: (w: %.2f, x: %.2f, y: %.2f, z: %.2f)", transform->entity->name, transform->w_rot.w, transform->w_rot.x, transform->w_rot.y, transform->w_rot.z);
}
V3 T_WEuler(Transform *transform)
{
    if (transform->isDirty)
    {
        Transform_CleanFromTop(transform);
    }
    return Quat_ToEuler(transform->w_rot);
}
V3 T_LEuler(Transform *transform)
{
    return Quat_ToEuler(transform->l_rot);
}
V3 T_WPos(Transform *transform)
{
    if (transform->isDirty)
    {
        Transform_CleanFromTop(transform);
    }
    return transform->w_pos;
}
Quaternion T_WRot(Transform *transform)
{
    if (transform->isDirty)
    {
        Transform_CleanFromTop(transform);
    }
    return transform->w_rot;
}
V3 T_WSca(Transform *transform)
{
    if (transform->isDirty)
    {
        Transform_CleanFromTop(transform);
    }
    return transform->w_scale;
}
V3 T_LPos(Transform *transform)
{
    return transform->l_pos;
}
Quaternion T_LRot(Transform *transform)
{
    return transform->l_rot;
}
V3 T_LSca(Transform *transform)
{
    return transform->l_scale;
}
V3 T_Forward(Transform *transform)
{
    if (transform->isDirty)
    {
        Transform_CleanFromTop(transform);
    }
    V3 forward = {
        transform->w_matrix[2][0] / transform->w_scale.z,
        transform->w_matrix[2][1] / transform->w_scale.z,
        transform->w_matrix[2][2] / transform->w_scale.z};
    return forward;
}
V3 T_Right(Transform *transform)
{
    if (transform->isDirty)
    {
        Transform_CleanFromTop(transform);
    }
    V3 right = {
        -transform->w_matrix[0][0] / transform->w_scale.x,
        -transform->w_matrix[0][1] / transform->w_scale.x,
        -transform->w_matrix[0][2] / transform->w_scale.x};
    return right;
}
V3 T_Up(Transform *transform)
{
    if (transform->isDirty)
    {
        Transform_CleanFromTop(transform);
    }
    V3 up = {
        -transform->w_matrix[1][0] / transform->w_scale.y,
        -transform->w_matrix[1][1] / transform->w_scale.y,
        -transform->w_matrix[1][2] / transform->w_scale.y};
    return up;
}

// -------------------------
// Setters
// -------------------------

inline void T_LPos_Set(Transform *transform, V3 l_pos)
{
    Transform_SetLPos(transform, &l_pos, true);
}
inline void T_LRot_Set(Transform *transform, Quaternion l_rot)
{
    Transform_SetLRot(transform, &l_rot, true);
}
inline void T_LSca_Set(Transform *transform, V3 l_scale)
{
    Transform_SetLSca(transform, &l_scale, true);
}
inline void T_LRot_Add(Transform *transform, V3 euler)
{
    Quaternion q = Quat_FromEuler(euler);
    T_LRot_Mul(transform, q);
}
inline void T_LRot_Mul(Transform *transform, Quaternion q)
{
    Quaternion newRot = Quat_Mul(transform->l_rot, q);
    Transform_SetLRot(transform, &newRot, true);
}
inline void T_LSca_Mul(Transform *transform, V3 scale)
{
    V3 newScale = V3_MUL(transform->l_scale, scale);
    Transform_SetLSca(transform, &newScale, true);
}
inline void T_LPos_Add(Transform *transform, V3 addition)
{
    V3 newPos = V3_ADD(transform->l_pos, addition);
    Transform_SetLPos(transform, &newPos, true);
}
inline void T_LPos_Sub(Transform *transform, V3 subtraction)
{
    V3 newPos = V3_SUB(transform->l_pos, subtraction);
    Transform_SetLPos(transform, &newPos, true);
}

// -------------------------
// Quaternion Operations
// -------------------------

inline V3 Quat_ToEuler(Quaternion q)
{
    V3 euler;

    // Roll (x-axis rotation)
    float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
    float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    euler.x = atan2f(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    float sinp = 2.0f * (q.w * q.y - q.z * q.x);
    if (fabsf(sinp) >= 1.0f)
        euler.y = copysignf(M_PI / 2.0f, sinp); // use 90 degrees if out of range
    else
        euler.y = asinf(sinp);

    // Yaw (z-axis rotation)
    float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
    float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    euler.z = atan2f(siny_cosp, cosy_cosp);

    // Convert radians to degrees
    euler.x = Degrees(euler.x);
    euler.y = Degrees(euler.y);
    euler.z = Degrees(euler.z);

    return euler;
}

inline Quaternion Quat_FromEuler(V3 euler)
{
    float cy = cos(Radians(euler.z) * 0.5);
    float sy = sin(Radians(euler.z) * 0.5);
    float cp = cos(Radians(euler.y) * 0.5);
    float sp = sin(Radians(euler.y) * 0.5);
    float cr = cos(Radians(euler.x) * 0.5);
    float sr = sin(Radians(euler.x) * 0.5);

    Quaternion q = {
        .w = cr * cp * cy + sr * sp * sy,
        .x = sr * cp * cy - cr * sp * sy,
        .y = cr * sp * cy + sr * cp * sy,
        .z = cr * cp * sy - sr * sp * cy};
    return q;
}

/// @brief Returns the inverse of a quaternion
inline Quaternion Quat_Inverse(const Quaternion q)
{
    // Compute squared length
    float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (lenSq == 0.0f)
    {
        // Avoid division by zero — return identity quaternion
        return (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
    }

    // Conjugate divided by squared length
    float invLen = 1.0f / lenSq;
    return (Quaternion){
        -q.x * invLen,
        -q.y * invLen,
        -q.z * invLen,
        q.w * invLen};
}

inline Quaternion Quat_Mul(Quaternion a, Quaternion b)
{
    Quaternion result = {
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w};
    return result;
}

inline Quaternion Quat_Div(Quaternion a, Quaternion b)
{
    float norm_b = b.w * b.w + b.x * b.x + b.y * b.y + b.z * b.z;
    Quaternion conj_b = {b.w, -b.x, -b.y, -b.z};
    Quaternion result = {
        (a.w * conj_b.w - a.x * conj_b.x - a.y * conj_b.y - a.z * conj_b.z) / norm_b,
        (a.w * conj_b.x + a.x * conj_b.w + a.y * conj_b.z - a.z * conj_b.y) / norm_b,
        (a.w * conj_b.y - a.x * conj_b.z + a.y * conj_b.w + a.z * conj_b.x) / norm_b,
        (a.w * conj_b.z + a.x * conj_b.y - a.y * conj_b.x + a.z * conj_b.w) / norm_b};
    return result;
}